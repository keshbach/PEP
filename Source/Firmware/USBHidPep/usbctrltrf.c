/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>

#include "usb.h"

#include "UsbTransferData.h"

#include "Includes/UtMacros.h"

/******************************************************************************
 * Macro:           void mUSBCheckAdrPendingState(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Specialized checking routine, it checks if the device
 *                  is in the ADDRESS PENDING STATE and services it if it is.
 *
 * Note:            None
 *****************************************************************************/
#define mUSBCheckAdrPendingState() \
    if (g_nUsbDeviceState == ADR_PENDING_STATE) \
    {                                           \
        UADDR = SetupPkt.bDevADR;               \
        if (UADDR > 0)                          \
            g_nUsbDeviceState = ADDRESS_STATE;  \
        else                                    \
            g_nUsbDeviceState = DEFAULT_STATE;  \
    }

#pragma udata
static UINT8 l_nControlTransferState;

/********************************************************************
Bug Fix: May 14, 2007 (#F7)
*********************************************************************
For a control transfer read, if the host tries to read more data
than what it has requested, the peripheral device should stall the
extra IN transactions and the status stage. Typically, a host does
not try to read more data than what it has requested. The original
firmware did not handle this situation. Instead of stalling extra
IN transactions, the device kept sending out zero length packets.

The new variable introduced is used to keep track if a short IN
packet has been sent or not. From this the state machine can
decide if it should stall future extra IN transactions or not.
********************************************************************/
static UINT8 l_nShortPktStatus;              // Flag used by Control Transfer Read

static void lUSBCtrlTrfSetupHandler(void);
static void lUSBCtrlTrfOutHandler(void);
static void lUSBCtrlTrfInHandler(void);

#pragma code
/******************************************************************************
 * Function:        UINT8 USBCtrlEPService(void)
 *
 * PreCondition:    USTAT is loaded with a valid endpoint address.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        USBCtrlEPService checks for three transaction types that
 *                  it knows how to service and services them:
 *                  1. EP0 SETUP
 *                  2. EP0 OUT
 *                  3. EP0 IN
 *                  It ignores all other types (i.e. EP1, EP2, etc.)
 *
 * Note:            None
 *****************************************************************************/
/********************************************************************
Bug Fix: May 14, 2007
********************************************************************/
UINT8 USBCtrlEPService(void)
{
    if (USTAT == EP00_OUT)
    {
        UIRbits.TRNIF = 0;

		//If the current EP0 OUT buffer has a SETUP packet
        if (ep0Bo.Stat.PID == SETUP_TOKEN)
        {
	        //Check if the SETUP transaction data went into the CtrlTrfData buffer.
	        //If so, need to copy it to the SetupPkt buffer so that it can be 
	        //processed correctly by lUSBCtrlTrfSetupHandler().
	        if (ep0Bo.ADR == (UINT8*)(&CtrlTrfData))	
	        {
		        UINT8 setup_cnt;
		
		        ep0Bo.ADR = (UINT8*)(&SetupPkt);
                
		        for (setup_cnt = 0; setup_cnt < sizeof(CTRL_TRF_SETUP); setup_cnt++)
		        {
		            *(((UINT8*)&SetupPkt) + setup_cnt) = *(((UINT8*)&CtrlTrfData) + setup_cnt);
		        }//end for
		    } 
	        
			//Handle the control transfer (parse the 8-byte SETUP command and figure out what to do)
            lUSBCtrlTrfSetupHandler();
        }
        else
        {
			//Handle the DATA transfer
            lUSBCtrlTrfOutHandler();
        }
    }
    else if (USTAT == EP00_IN)            	        // EP0 IN
    {
        UIRbits.TRNIF = 0;
        
        lUSBCtrlTrfInHandler();
    }
    else
    {
        return 0;           // Return '0', if not an EP0 transaction
    }
    
    return 1;               // Return '1', if TRNIF has been cleared
}

/******************************************************************************
 * Function:        void lUSBCtrlTrfSetupHandler(void)
 *
 * PreCondition:    SetupPkt buffer is loaded with valid USB Setup Data
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine is a task dispatcher and has 3 stages.
 *                  1. It initializes the control transfer state machine.
 *                  2. It calls on each of the module that may know how to
 *                     service the Setup Request from the host.
 *                     Module Example: USB9, HID, CDC, MSD, ...
 *                     As new classes are added, g_ClassReqHandler table in
 *                     usbdsc.c should be updated to call all available
 *                     class handlers.
 *                  3. Once each of the modules has had a chance to check if
 *                     it is responsible for servicing the request, stage 3
 *                     then checks direction of the transfer to determine how
 *                     to prepare EP0 for the control transfer.
 *                     Refer to USBCtrlEPServiceComplete() for more details.
 *
 * Note:            Microchip USB Firmware has three different states for
 *                  the control transfer state machine:
 *                  1. WAIT_SETUP
 *                  2. CTRL_TRF_TX
 *                  3. CTRL_TRF_RX
 *                  Refer to firmware manual to find out how one state
 *                  is transitioned to another.
 *
 *                  A Control Transfer is composed of many USB transactions.
 *                  When transferring data over multiple transactions,
 *                  it is important to keep track of data source, data
 *                  destination, and data count. These three parameters are
 *                  stored in g_pSrc, g_pDst, and g_wCount. A flag is used to
 *                  note if the data source is from ROM or RAM.
 *
 *****************************************************************************/
void lUSBCtrlTrfSetupHandler(void)
{
    UINT8 i;

/********************************************************************
Bug Fix: May 14, 2007 (#F7 - Partial 1/4)
*********************************************************************
For a control transfer read, if the host tries to read more data
than what it has requested, the peripheral device should stall the
extra IN transactions and the status stage. Typically, a host does
not try to read more data than what it has requested. The original
firmware did not handle this situation. Instead of stalling extra
IN transactions, the device kept sending out zero length packets.

This work around forces the IN endpoint back to be owned by the
CPU after if it was stalled previously. The short_pkt_status flag
is also re-initialized.
********************************************************************/
    if (ep0Bi.Stat.UOWN != 0)
    {
        ep0Bi.Stat._byte = _UCPU;           // Compensate for after a STALL
    }
    
    l_nShortPktStatus = SHORT_PKT_NOT_USED;
/*******************************************************************/

    /* Stage 1 */
    l_nControlTransferState = WAIT_SETUP;
    g_nControlTransferSessionOwner = MUID_NULL;     // Set owner to NULL
    g_wCount = 0;

    /* Stage 2 */
    USBCheckStdRequest();                   // See system\usb9\usb9.c

    for (i = 0; i < MArrayLen(g_ClassReqHandler); i++)
    {
        if (g_nControlTransferSessionOwner != MUID_NULL) break;
        
        g_ClassReqHandler[i]();            // See autofiles\usbdsc.c
    }//end while

    /* Stage 3 */
    USBCtrlEPServiceComplete();
}

/******************************************************************************
 * Function:        void lUSBCtrlTrfOutHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles an OUT transaction according to
 *                  which control transfer state is currently active.
 *
 * Note:            Note that if the the control transfer was from
 *                  host to device, the session owner should be notified
 *                  at the end of each OUT transaction to service the
 *                  received data.
 *
 *****************************************************************************/
void lUSBCtrlTrfOutHandler(void)
{
    if (l_nControlTransferState == CTRL_TRF_RX)
    {
        USBCtrlTrfRxService();

        /*
         * Don't have to worry about overwriting _KEEP bit
         * because if _KEEP was set, TRNIF would not have been
         * generated in the first place.
         */
		ep0Bo.ADR = (UINT8*)&CtrlTrfData;
		ep0Bo.Cnt = EP0_BUFF_SIZE;
        
        if(ep0Bo.Stat.DTS == 0)
            ep0Bo.Stat._byte = _USIE | _DAT1 | _DTSEN;
        else
            ep0Bo.Stat._byte = _USIE | _DAT0 | _DTSEN;
    }
    else //In this case the last OUT transaction must have been a status stage of a CTRL_TRF_TX
    {
	    //Prepare EP0 OUT for the next SETUP transaction.
		USBPrepareForNextSetupTrf();
        
        ep0Bo.Cnt = EP0_BUFF_SIZE;
        ep0Bo.ADR = (UINT8*)&SetupPkt;
        ep0Bo.Stat._byte = _USIE | _DAT0 | _DTSEN | _BSTALL;			
    }
}

/******************************************************************************
 * Function:        void lUSBCtrlTrfInHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles an IN transaction according to
 *                  which control transfer state is currently active.
 *
 *
 * Note:            A Set Address Request must not change the acutal address
 *                  of the device until the completion of the control
 *                  transfer. The end of the control transfer for Set Address
 *                  Request is an IN transaction. Therefore it is necessary
 *                  to service this unique situation when the condition is
 *                  right. Macro mUSBCheckAdrPendingState is defined in
 *                  usb9.h and its function is to specifically service this
 *                  event.
 *****************************************************************************/
void lUSBCtrlTrfInHandler(void)
{
    mUSBCheckAdrPendingState();         // Must check if in ADR_PENDING_STATE

    if (l_nControlTransferState == CTRL_TRF_TX)
    {
        USBCtrlTrfTxService();

/********************************************************************
Bug Fix: May 14, 2007 (#F7 - Partial 2/4)
*********************************************************************
For a control transfer read, if the host tries to read more data
than what it has requested, the peripheral device should stall the
extra IN transactions and the status stage. Typically, a host does
not try to read more data than what it has requested. The original
firmware did not handle this situation. Instead of stalling extra
IN transactions, the device kept sending out zero length packets.

This work around checks if a short IN packet has been sent or not.
If it has, the IN endpoint will be set to install the next IN token.
If not, then the original endpoint setup code will be executed.
********************************************************************/
        if (l_nShortPktStatus == SHORT_PKT_SENT)
        {
            // If a short packet has been sent, don't want to send any more,
            // stall next time if host is still trying to read.
            ep0Bi.Stat._byte = _USIE | _BSTALL;
        }
/*******************************************************************/
        else
        {
            if (ep0Bi.Stat.DTS == 0)
            {
                ep0Bi.Stat._byte = _USIE | _DAT1 | _DTSEN;
            }
            else
            {
                ep0Bi.Stat._byte = _USIE | _DAT0 | _DTSEN;
            }
        }//end if(...)else
    }
    else // CTRL_TRF_RX
    {
        USBPrepareForNextSetupTrf();
    }
}

/******************************************************************************
 * Function:        void USBCtrlTrfTxService(void)
 *
 * PreCondition:    g_pSrc, g_wCount, and usb_stat.ctrl_trf_mem are setup properly.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should be called from only two places.
 *                  One from USBCtrlEPServiceComplete() and one from
 *                  lUSBCtrlTrfInHandler(). It takes care of managing a
 *                  transfer over multiple USB transactions.
 *
 * Note:            This routine works with isochronous endpoint larger than
 *                  256 bytes and is shown here as an example of how to deal
 *                  with BC9 and BC8. In reality, a control endpoint can never
 *                  be larger than 64 bytes.
 *****************************************************************************/
void USBCtrlTrfTxService(void)
{
    UINT16 byte_to_send;

    /*
     * First, have to figure out how many byte of data to send.
     */
    if (g_wCount < EP0_BUFF_SIZE)
    {
        byte_to_send = g_wCount;

/********************************************************************
Bug Fix: May 14, 2007 (#F7 - Partial 3/4)
*********************************************************************
For a control transfer read, if the host tries to read more data
than what it has requested, the peripheral device should stall the
extra IN transactions and the status stage. Typically, a host does
not try to read more data than what it has requested. The original
firmware did not handle this situation. Instead of stalling extra
IN transactions, the device kept sending out zero length packets.

This work around updates the short_pkt_status flag to indicate
if a short packet has been sent or not.
********************************************************************/
        if (l_nShortPktStatus == SHORT_PKT_NOT_USED)
        {
            l_nShortPktStatus = SHORT_PKT_PENDING;
        }
        else if (l_nShortPktStatus == SHORT_PKT_PENDING)
        {
            l_nShortPktStatus = SHORT_PKT_SENT;
        }//end if
/*******************************************************************/
    }
    else
    {
        byte_to_send = EP0_BUFF_SIZE;
    }

    /*
     * Next, load the number of bytes to send to BC9..0 in buffer descriptor
     */
    ep0Bi.Stat.BC9 = 0;
    ep0Bi.Stat.BC8 = 0;
    ep0Bi.Stat._byte |= MHighByteFromWord(byte_to_send);
    ep0Bi.Cnt = MLowByteFromWord(byte_to_send);

    /*
     * Subtract the number of bytes just about to be sent from the total.
     */
    g_wCount = g_wCount - byte_to_send;

    g_pDst.pbyRam = (UINT8*)&CtrlTrfData; // Set destination pointer

    if (g_UsbDeviceStatus.ctrl_trf_mem == _ROM) // Determine type of memory source
    {
        while (byte_to_send)
        {
            *g_pDst.pbyRam = *g_pSrc.pbyRom;
            
            g_pDst.pbyRam++;
            g_pSrc.pbyRom++;
            byte_to_send--;
        }//end while(byte_to_send._word)
    }
    else // RAM
    {
        while (byte_to_send)
        {
            *g_pDst.pbyRam = *g_pSrc.pbyRam;
            
            g_pDst.pbyRam++;
            g_pSrc.pbyRam++;
            byte_to_send--;
        }//end while(byte_to_send._word)
    }//end if(usb_stat.ctrl_trf_mem == _ROM)
}

/******************************************************************************
 * Function:        void USBCtrlTrfRxService(void)
 *
 * PreCondition:    g_pDst and g_wCount are setup properly.
 *                  g_pSrc is always &CtrlTrfData
 *                  usb_stat.ctrl_trf_mem is always _RAM.
 *                  g_wCount should be set to 0 at the start of each control
 *                  transfer.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        *** This routine is only partially complete. Check for
 *                  new version of the firmware.
 *
 * Note:            None
 *****************************************************************************/
void USBCtrlTrfRxService(void)
{
    UINT16 byte_to_read;
    UINT8* pnByte = (UINT8*)&byte_to_read;

    pnByte[0] = ep0Bo.Cnt;
    pnByte[1] = 0x03 & ep0Bo.Stat._byte;    // Filter out last 2 bits

    /*
     * Accumulate total number of bytes read
     */
    g_wCount = g_wCount + byte_to_read;

    g_pSrc.pbyRam = (UINT8*)&CtrlTrfData;

    while (byte_to_read)
    {
        *g_pDst.pbyRam = *g_pSrc.pbyRam;
        
        g_pDst.pbyRam++;
        g_pSrc.pbyRam++;
        byte_to_read--;
    }//end while(byte_to_read._word)
}

/******************************************************************************
 * Function:        void USBCtrlEPServiceComplete(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine wrap up the remaining tasks in servicing
 *                  a Setup Request. Its main task is to set the endpoint
 *                  controls appropriately for a given situation. See code
 *                  below.
 *                  There are three main scenarios:
 *                  a) There was no handler for the Request, in this case
 *                     a STALL should be sent out.
 *                  b) The host has requested a read control transfer,
 *                     endpoints are required to be setup in a specific way.
 *                  c) The host has requested a write control transfer, or
 *                     a control data stage is not required, endpoints are
 *                     required to be setup in a specific way.
 *
 *                  Packet processing is resumed by clearing PKTDIS bit.
 *
 * Note:            None
 *****************************************************************************/
void USBCtrlEPServiceComplete(void)
{
/********************************************************************
Bug Fix: May 14, 2007 (#AF1)
*********************************************************************
See silicon errata for 4550 A3. Now clearing PKTDIS before re-arming
any EP0 endpoints.
********************************************************************/
    /*
     * PKTDIS bit is set when a Setup Transaction is received.
     * Clear to resume packet processing.
     */
    UCONbits.PKTDIS = 0;
/*******************************************************************/

    if (g_nControlTransferSessionOwner == MUID_NULL)
    {
        /*
         * If no one knows how to service this request then stall.
         * Must also prepare EP0 to receive the next SETUP transaction.
         */
        ep0Bo.Cnt = EP0_BUFF_SIZE;
        ep0Bo.ADR = (UINT8*)&SetupPkt;

        ep0Bo.Stat._byte = _USIE | _BSTALL;
        ep0Bi.Stat._byte = _USIE | _BSTALL;
    }
    else    // A module has claimed ownership of the control transfer session.
    {
        if (SetupPkt.DataDir == DEV_TO_HOST)
        {
            if (SetupPkt.wLength < g_wCount)
            {
                g_wCount = SetupPkt.wLength;
            }
        
            USBCtrlTrfTxService();

            l_nControlTransferState = CTRL_TRF_TX;
            /*
             * Control Read:
             * <SETUP[0]><IN[1]><IN[0]>...<OUT[1]> | <SETUP[0]>
             * 1. Prepare OUT EP to respond to early termination
             *
             * NOTE:
             * If something went wrong during the control transfer,
             * the last status stage may not be sent by the host.
             * When this happens, two different things could happen
             * depending on the host.
             * a) The host could send out a RESET.
             * b) The host could send out a new SETUP transaction
             *    without sending a RESET first.
             * To properly handle case (b), the OUT EP must be setup
             * to receive either a zero length OUT transaction, or a
             * new SETUP transaction.
             *
             * Since the SETUP transaction requires the DTS bit to be
             * DAT0 while the zero length OUT status requires the DTS
             * bit to be DAT1, the DTS bit check by the hardware should
             * be disabled. This way the SIE could accept either of
             * the two transactions.
             *
             * Furthermore, the Cnt byte should be set to prepare for
             * the SETUP data (8-byte or more), and the buffer address
             * should be pointed to SetupPkt.
             */
            ep0Bo.Cnt = EP0_BUFF_SIZE;
            ep0Bo.ADR = (UINT8*)&SetupPkt;
            ep0Bo.Stat._byte = _USIE;           // Note: DTSEN is 0!

            /*
             * 2. Prepare IN EP to transfer data, Cnt should have
             *    been initialized by responsible request owner.
             */
            ep0Bi.ADR = (UINT8*)&CtrlTrfData;
            ep0Bi.Stat._byte = _USIE | _DAT1 | _DTSEN;
        }
        else    // (SetupPkt.DataDir == HOST_TO_DEV)
        {
            l_nControlTransferState = CTRL_TRF_RX;
            /*
             * Control Write:
             * <SETUP[0]><OUT[1]><OUT[0]>...<IN[1]> | <SETUP[0]>
             *
             * 1. Prepare IN EP to respond to early termination
             *
             *    This is the same as a Zero Length Packet Response
             *    for control transfer without a data stage
             */
            ep0Bi.Cnt = 0;
            ep0Bi.Stat._byte = _USIE | _DAT1 | _DTSEN;

            /*
             * 2. Prepare OUT EP to receive data.
             */
            ep0Bo.Cnt = EP0_BUFF_SIZE;
            ep0Bo.ADR = (UINT8*)&CtrlTrfData;
            ep0Bo.Stat._byte = _USIE | _DAT1 | _DTSEN;
        }//end if(SetupPkt.DataDir == DEV_TO_HOST)
    }
}

/******************************************************************************
 * Function:        void USBPrepareForNextSetupTrf(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The routine forces EP0 OUT to be ready for a new Setup
 *                  transaction, and forces EP0 IN to be owned by CPU.
 *
 * Note:            None
 *****************************************************************************/
void USBPrepareForNextSetupTrf(void)
{
    l_nControlTransferState = WAIT_SETUP;            // See usbctrltrf.h

/********************************************************************
Bug Fix: May 14, 2007 (#F3)
*********************************************************************
In the original firmware, if an IN token is sent by the host
before a SETUP token is sent, the firmware would respond with an ACK.
This is not a correct response, the firmware should have sent a STALL.
This is a minor non-compliance since a compliant host should not
send an IN before sending a SETUP token. The fix allows a SETUP
transaction to be accepted while stalling IN transactions.

Although this fix is known, it is not implemented because it
breaks the #AF1 fix in USBCtrlEPServiceComplete().
Since #AF1 fix is more important, this fix, #F3 is commented out.
********************************************************************/
    ep0Bi.Stat._byte = _UCPU;               // Should be removed
    //ep0Bi.Stat._byte = _USIE|_BSTALL;     // Should be added #F3
}

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
