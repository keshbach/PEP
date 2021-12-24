/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>

#include "usb.h"

#include "UtPortTypeDefs.h"

#pragma udata
static UINT8 l_nTRNIFCount;               // Bug fix - Work around.

#pragma code
static void lUSBModuleEnable(void);

static void lUSBSuspend(void);
static void USBWakeFromSuspend(void);

static void USBProtocolResetHandler(void);
static void USBStallHandler(void);

/******************************************************************************
 * Function:        void USBCheckBusStatus(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine enables/disables the USB module by monitoring
 *                  the USB power signal.
 *
 * Note:            None
 *****************************************************************************/
void USBCheckBusStatus(void)
{
    if (UCONbits.USBEN == 0)                // Is the module off?
        lUSBModuleEnable();                 // Is off, enable it

    /*
     * After enabling the USB module, it takes some time for the voltage
     * on the D+ or D- line to rise high enough to get out of the SE0 condition.
     * The USB Reset interrupt should not be unmasked until the SE0 condition is
     * cleared. This helps preventing the firmware from misinterpreting this
     * unique event as a USB bus reset from the USB host.
     */
    if (g_nUsbDeviceState == ATTACHED_STATE)
    {
        if (!UCONbits.SE0)
        {
            UIR = 0;                        // Clear all USB interrupts
            UIE = 0;                        // Mask all USB interrupts
            UIEbits.URSTIE = 1;             // Unmask RESET interrupt
            UIEbits.IDLEIE = 1;             // Unmask IDLE interrupt
            g_nUsbDeviceState = POWERED_STATE;
        }//end if                           // else wait until SE0 is cleared
    }//end if(usb_device_state == ATTACHED_STATE)
}

/******************************************************************************
 * Function:        void USBModuleEnable(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine enables the USB module.
 *                  An end designer should never have to call this routine
 *                  manually. This routine should only be called from
 *                  USBCheckBusStatus().
 *
 * Note:            See USBCheckBusStatus() for more information.
 *****************************************************************************/
static void lUSBModuleEnable(void)
{
    UCON = 0;
    UIE = 0;                                // Mask all USB interrupts
    UCONbits.USBEN = 1;                     // Enable module & attach to bus
    g_nUsbDeviceState = ATTACHED_STATE;
}

/******************************************************************************
 * Function:        void USBDriverService(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine is the heart of this firmware. It manages
 *                  all USB interrupts.
 *
 * Note:            Device state transitions through the following stages:
 *                  DETACHED -> ATTACHED -> POWERED -> DEFAULT ->
 *                  ADDRESS_PENDING -> ADDRESSED -> CONFIGURED -> READY
 *****************************************************************************/
void USBDriverService(void)
{
    /*
     * Pointless to continue servicing if USB cable is not even attached.
     */
    if (g_nUsbDeviceState == DETACHED_STATE) return;

    /*
     * Task A: Service USB Activity Interrupt
     */

    if (UIRbits.ACTVIF)
    {
        USBWakeFromSuspend();
    }

    /*
     * Pointless to continue servicing if the device is in suspend mode.
     */
    if (UCONbits.SUSPND == 1) return;

    /*
     * Task B: Service USB Bus Reset Interrupt.
     * When bus reset is received during suspend, ACTVIF will be set first,
     * once the UCONbits.SUSPND is clear, then the URSTIF bit will be asserted.
     * This is why URSTIF is checked after ACTVIF.
     */
    if (UIRbits.URSTIF)
    {
        USBProtocolResetHandler();
    }

    /*
     * Task C: Service other USB interrupts
     */
    if (UIRbits.IDLEIF)
    {
        lUSBSuspend();
    }

    if (UIRbits.STALLIF)
    {
        USBStallHandler();
    }

    /*
     * Pointless to continue servicing if the host has not sent a bus reset.
     * Once bus reset is received, the device transitions into the DEFAULT
     * state and is ready for communication.
     */
    if (g_nUsbDeviceState < DEFAULT_STATE) return;

    /*
     * Task D: Servicing USB Transaction Complete Interrupt
     */
    for (l_nTRNIFCount = 0; l_nTRNIFCount < 4; l_nTRNIFCount++)
    {
        if (UIRbits.TRNIF)
        {
            /*
             * USBCtrlEPService only services transactions over EP0.
             * It ignores all other EP transactions.
             */
            if (USBCtrlEPService() == 0) // If not an EP0 transaction, then clear TRNIF.
            {
                /*
		         * Other EP can be serviced later by responsible device class firmware.
		         * Each device driver knows when an OUT or IN transaction is ready by
		         * checking the buffer ownership bit.
		         * An OUT EP should always be owned by SIE until the data is ready.
		         * An IN EP should always be owned by CPU until the data is ready.
		         *
		         * Because of this logic, it is not necessary to save the USTAT value
		         * of non-EP0 transactions.
		         */
		        UIRbits.TRNIF = 0;
		        /*
		         * At least five Tcy are needed in between clearing UIR<TRNIF>
		         * and when it becomes reasserted when the USTAT FIFO has more
		         * than one entry in it.
		         *
		         * No Nops are needed here because the number of instruction
		         * cycles between clearing the TRNIF here to the next flag
		         * check is longer than the required minimum of six Tcy due
		         * to the for loop logic of bTRNIFCount. This is true even
		         * when all optimization options in C18 are enabled.
		         */
            }
        }//end if(UIRbits.TRNIF && UIEbits.TRNIE)
        else
            break;
    }// end for(bTRNIFCount = 0; bTRNIFCount < 4; bTRNIFCount++)
}

/******************************************************************************
 * Function:        void USBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None
 *****************************************************************************/
static void lUSBSuspend(void)
{
    /*
     * NOTE: Do not clear UIRbits.ACTVIF here!
     * Reason:
     * ACTVIF is only generated once an IDLEIF has been generated.
     * This is a 1:1 ratio interrupt generation.
     * For every IDLEIF, there will be only one ACTVIF regardless of
     * the number of subsequent bus transitions.
     *
     * If the ACTIF is cleared here, a problem could occur when:
     * [       IDLE       ][bus activity ->
     * <--- 3 ms ----->     ^
     *                ^     ACTVIF=1
     *                IDLEIF=1
     *  #           #           #           #   (#=Program polling flags)
     *                          ^
     *                          This polling loop will see both
     *                          IDLEIF=1 and ACTVIF=1.
     *                          However, the program services IDLEIF first
     *                          because ACTIVIE=0.
     *                          If this routine clears the only ACTIVIF,
     *                          then it can never get out of the suspend
     *                          mode.
     */
    UIEbits.ACTVIE = 1;                     // Enable bus activity interrupt
    UIRbits.IDLEIF = 0;
    UCONbits.SUSPND = 1;                    // Put USB module in power conserve
                                            // mode, SIE clock inactive
    /*
     * At this point the PIC can go into sleep,idle, or
     * switch to a slower clock, etc.
     */

    /* Modifiable Section */
    PIR2bits.USBIF = 0;
//    INTCONbits.RBIF = 0;
    PIE2bits.USBIE = 1;                     // Set USB wakeup source
//    INTCONbits.RBIE = 1;                    // Set sw2,3 wakeup source
    Sleep();                                // Goto sleep

    PIE2bits.USBIE = 0;
    /* End Modifiable Section */
}

/******************************************************************************
 * Function:        void USBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None
 *****************************************************************************/
void USBWakeFromSuspend(void)
{
    /*
     * If using clock switching, this is the place to restore the
     * original clock frequency.
     */
    UCONbits.SUSPND = 0;
    UIEbits.ACTVIE = 0;

/********************************************************************
Bug Fix: August 14, 2007
*********************************************************************
The ACTVIF bit cannot be cleared immediately after the USB module wakes
up from Suspend or while the USB module is suspended. A few clock cycles
are required to synchronize the internal hardware state machine before
the ACTIVIF bit can be cleared by firmware. Clearing the ACTVIF bit
before the internal hardware is synchronized may not have an effect on
the value of ACTVIF. Additonally, if the USB module uses the clock from
the 96 MHz PLL source, then after clearing the SUSPND bit, the USB
module may not be immediately operational while waiting for the 96 MHz
PLL to lock.
********************************************************************/
    // UIRbits.ACTVIF = 0;                      // Removed
    while (UIRbits.ACTVIF) { UIRbits.ACTVIF = 0; }  // Added
}

/******************************************************************************
 * Function:        void USBStallHandler(void)
 *
 * PreCondition:    A STALL packet is sent to the host by the SIE.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The STALLIF is set anytime the SIE sends out a STALL
 *                  packet regardless of which endpoint causes it.
 *                  A Setup transaction overrides the STALL function. A stalled
 *                  endpoint stops stalling once it receives a setup packet.
 *                  In this case, the SIE will accepts the Setup packet and
 *                  set the TRNIF flag to notify the firmware. STALL function
 *                  for that particular endpoint pipe will be automatically
 *                  disabled (direction specific).
 *
 *                  There are a few reasons for an endpoint to be stalled.
 *                  1. When a non-supported USB request is received.
 *                     Example: GET_DESCRIPTOR(DEVICE_QUALIFIER)
 *                  2. When an endpoint is currently halted.
 *                  3. When the device class specifies that an endpoint must
 *                     stall in response to a specific event.
 *                     Example: Mass Storage Device Class
 *                              If the CBW is not valid, the device shall
 *                              STALL the Bulk-In pipe.
 *                              See USB Mass Storage Class Bulk-only Transport
 *                              Specification for more details.
 *
 * Note:            UEPn.EPSTALL can be scanned to see which endpoint causes
 *                  the stall event.
 *****************************************************************************/
void USBStallHandler(void)
{
    /*
     * Does not really have to do anything here,
     * even for the control endpoint.
     * All BDs of Endpoint 0 are owned by SIE right now,
     * but once a Setup Transaction is received, the ownership
     * for EP0_OUT will be returned to CPU.
     * When the Setup Transaction is serviced, the ownership
     * for EP0_IN will then be forced back to CPU by firmware.
     */
    if(UEP0bits.EPSTALL == 1)
    {
/********************************************************************
Bug Fix: August 14, 2007 (#F4)
*********************************************************************
In a control transfer, when a request is not supported, all
subsequent transactions should be stalled until a new SETUP
transaction is received. The original firmware only stalls the
first subsequent transaction, then ACKs others. Typically, a
compliance USB host will stop sending subsequent transactions
once the first stall is received. In the original firmware,
function USBStallHandler() in usbdrv.c calls
USBPrepareForNextSetupTrf() when a STALL event occurred on EP0.
In turn, USBPrepareForNextSetupTrf() reconfigures EP0 IN and OUT
to prepare for the next SETUP transaction. The work around is not
to call USBPrepareForNextSetupTrf() in USBStallHandler().
********************************************************************/
        //USBPrepareForNextSetupTrf();      // Removed
/*******************************************************************/

/********************************************************************
Bug Fix: August 14, 2007 (#F7 - Partial 4/4)
*********************************************************************
For a control transfer read, if the host tries to read more data
than what it has requested, the peripheral device should stall the
extra IN transactions and the status stage. Typically, a host does
not try to read more data than what it has requested. The original
firmware did not handle this situation. Instead of stalling extra
IN transactions, the device kept sending out zero length packets.

This work around checks to make sure that at least one extra IN
transaction is stalled before setting the OUT endpoint to stall the
status stage.
********************************************************************/
        if((ep0Bo.Stat._byte == _USIE) && (ep0Bi.Stat._byte == (_USIE|_BSTALL)))
        {
            // Set ep0Bo to stall also
            ep0Bo.Stat._byte = _USIE|_DAT0|_DTSEN|_BSTALL;
        }//end if
/*******************************************************************/

        UEP0bits.EPSTALL = 0;         	    // Clear STALL status
    }
    UIRbits.STALLIF = 0;
}

/******************************************************************************
 * Function:        void USBProtocolResetHandler(void)
 *
 * PreCondition:    A USB bus reset is received from the host.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    Currently, this routine flushes any pending USB
 *                  transactions. It empties out the USTAT FIFO. This action
 *                  might not be desirable in some applications.
 *
 * Overview:        Once a USB bus reset is received from the host, this
 *                  routine should be called. It resets the device address to
 *                  zero, disables all non-EP0 endpoints, initializes EP0 to
 *                  be ready for default communication, clears all USB
 *                  interrupt flags, unmasks applicable USB interrupts, and
 *                  reinitializes internal state-machine variables.
 *
 * Note:            None
 *****************************************************************************/
void USBProtocolResetHandler(void)
{
    UEIR = 0;                       // Clear all USB error flags
    UIR = 0;                        // Clears all USB interrupts
    UEIE = 0b10011111;              // Unmask all USB error interrupts
    UIE = 0b01111011;               // Enable all interrupts except ACTVIE

    UADDR = 0x00;                   // Reset to default address
    mDisableEP1to15();              // Reset all non-EP0 UEPn registers
    UEP0 = EP_CTRL | HSHK_EN;       // Init EP0 as a Ctrl EP, see usbdrv.h

    while (UIRbits.TRNIF == 1)      // Flush any pending transactions
    {
        UIRbits.TRNIF = 0;
/********************************************************************
Bug Fix: August 14, 2007
*********************************************************************
Clearing the transfer complete flag bit, TRNIF, causes the SIE to
advance the FIFO. If the next data in the FIFO holding register is
valid, the SIE will reassert the interrupt within 5Tcy of clearing
TRNIF. If no additional data is preset, TRNIF will remain clear.
Additional nops were added in this fix to guarantee that TRNIF is
properly updated before being checked again.
********************************************************************/
		_asm
		bra	0	//Equivalent to bra $+2, which takes half as much code as 2 nop instructions
		bra	0	//Equivalent to bra $+2, which takes half as much code as 2 nop instructions
		_endasm		

		Nop();
    }

    UCONbits.PKTDIS = 0;            // Make sure packet processing is enabled
    USBPrepareForNextSetupTrf();    // Declared in usbctrltrf.c
    //Prepare EP0 OUT to receive the first SETUP packet
    ep0Bo.Cnt = EP0_BUFF_SIZE;
    ep0Bo.ADR = (byte*)(&SetupPkt);
    ep0Bo.Stat._byte = _USIE | _DAT0 | _DTSEN | _BSTALL;	
    
    g_UsbDeviceStatus.RemoteWakeup = 0;      // Default status flag to disable
    g_nUsbActiveCfg = 0;             // Clear active configuration
    g_nUsbDeviceState = DEFAULT_STATE;
}

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
