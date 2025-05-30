#include "PepDevice.h"

#include "UtPortTypeDefs.h"

#include "usb.h"
#include "usb_device.h"

#if defined(IMPLEMENT_MICROSOFT_OS_DESCRIPTOR)
#include "usb_function_generic.h"
#endif

#include <Firmware/PepFirmwareDefs.h>

#include "ProcessCommand.h"

#if defined(__32MX250F128B__) || defined(__32MX440F256H__)
#include "UtTimeDelay.h"
#endif

#define PEP_DEVICE_INITIALIZE_STATE 0
#define PEP_DEVICE_IDLE_STATE 1
#define PEP_DEVICE_DATA_IN_STATE 2 // device sending data to host
#define PEP_DEVICE_DATA_OUT_STATE 3 // device reading data from host

static UINT8 l_PepDeviceState;

static USB_HANDLE l_EndPoint1OutHandle;
static USB_HANDLE l_EndPoint1InHandle;

static TUtPepCommandData l_CommandData;
static TUtPepResponseData l_ResponseData;

#if defined(__32MX440F256H__)
static void lBlinkLED1OneShot(void)
{
    PMADDRbits.ADDR |= 0x20; // turn LED1 on

    UtTimeDelayMs(100); // 1 second

    PMADDRbits.ADDR &= ~0x20; // turn LED1 off

    UtTimeDelayMs(100); // 1 second
}

static void lBlinkLED2OneShot(void)
{
    PORTDbits.RD1 = 1; // turn LED2 on

    UtTimeDelayMs(100); // 1 second

    PORTDbits.RD1 = 0; // turn LED2 off

    UtTimeDelayMs(100); // 1 second
}

static void lBlinkLEDsOneShot(void)
{
    PMADDRbits.ADDR |= 0x20; // turn LED1 on
    PORTDbits.RD1 = 1; // turn LED2 on

    UtTimeDelayMs(100); // 1 second

    PMADDRbits.ADDR &= ~0x20; // turn LED1 off
    PORTDbits.RD1 = 0; // turn LED2 off

    UtTimeDelayMs(100); // 1 second
}
#endif

static void lInitializeSystem(void)
{
#if defined(__32MX250F128B__)
#elif defined(__32MX440F256H__)
    SYSTEMConfigPerformance(80000000);

    // Must configure parallel port to be able to turn LED1 on

    // Configure LED2 pin
    
    TRISDbits.TRISD1 = 0; // set LED2 pin to output port
    ODCDbits.ODCD1 = 0; // set LED2 pin as normal digital output

    // Default to LED1 and LED2 off
    
    PMADDRbits.ADDR &= ~0x20; // turn LED1 off
    PORTDbits.RD1 = 0; // turn LED2 off
#else
#error Unknown device configuration
#endif

    l_PepDeviceState = PEP_DEVICE_INITIALIZE_STATE;
    
    l_EndPoint1OutHandle = 0;
    l_EndPoint1InHandle = 0;
    
    InitProcessCommand(&l_CommandData, &l_ResponseData);

#if defined(__32MX440F256H__)
    UINT32 nIndex;
    
    for (nIndex = 0; nIndex < 40; ++nIndex)
    {
        lBlinkLED1OneShot();
        lBlinkLED2OneShot();
    }
#endif
	
    // Initializes the USB module SFRs and firmware variables to known states.
    
    USBDeviceInit();
}

static void lProcessIO(void)
{   
    //User Application USB tasks below.
    //Note: The user application should not begin attempting to read/write over the USB
    //until after the device has been fully enumerated.  After the device is fully
    //enumerated, the USBDeviceState will be set to "CONFIGURED_STATE".

    if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1))
    {
        return;
    }
    
    switch (l_PepDeviceState)
    {
        case PEP_DEVICE_IDLE_STATE:
            // Check if received a command
            
            if (!USBHandleBusy(l_EndPoint1OutHandle))
            {
                l_PepDeviceState = PEP_DEVICE_DATA_IN_STATE;
            }
            break;
        case PEP_DEVICE_DATA_IN_STATE:
            ProcessCommand(&l_CommandData, &l_ResponseData);
            
            l_PepDeviceState = PEP_DEVICE_DATA_OUT_STATE;
            break;
        case PEP_DEVICE_DATA_OUT_STATE:
            if (!USBHandleBusy(l_EndPoint1InHandle))
            {
                l_EndPoint1InHandle = USBTxOnePacket(CPepFirmwareEndPointNumber,
                                                     (UINT8*)&l_ResponseData,
                                                     sizeof(l_ResponseData));

                l_EndPoint1OutHandle = USBRxOnePacket(CPepFirmwareEndPointNumber,
                                                     (UINT8*)&l_CommandData,
                                                     sizeof(l_CommandData));

                l_PepDeviceState = PEP_DEVICE_IDLE_STATE;
            }
            break;
        case PEP_DEVICE_INITIALIZE_STATE:
            l_PepDeviceState = PEP_DEVICE_IDLE_STATE;
            break;
        default:
#if defined(__32MX440F256H__)
            blink_unknown_state:
            lBlinkLED1OneShot();
            lBlinkLED2OneShot();
            
            goto blink_unknown_state;
#endif

            l_PepDeviceState = PEP_DEVICE_IDLE_STATE;
            break;
    }
}

int ExecutePepDevice(void)
{
    lInitializeSystem();

#if defined(USB_INTERRUPT)
    USBDeviceAttach();
#endif

    while(1)
    {
#if defined(USB_POLLING)
		// Check bus status and service USB interrupts.

        USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
        				  // this function periodically.  This function will take care
        				  // of processing and responding to SETUP transactions 
        				  // (such as during the enumeration process when you first
        				  // plug in).  USB hosts require that USB devices should accept
        				  // and process SETUP packets in a timely fashion.  Therefore,
        				  // when using polling, this function should be called 
        				  // regularly (such as once every 1.8ms or faster** [see 
        				  // inline code comments in usb_device.c for explanation when
        				  // "or faster" applies])  In most cases, the USBDeviceTasks() 
        				  // function does not take very long to execute (ex: <100 
        				  // instruction cycles) before it returns.
#endif

		// Application-specific tasks.
		// Application related code may be added here, or in the lProcessIO() function.

        lProcessIO();        
    }
    
    return 0;
}

// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

// Note *: The "usb_20.pdf" specs indicate 500uA or 2.5mA, depending upon device classification. However,
// the USB-IF has officially issued an ECN (engineering change notice) changing this to 2.5mA for all 
// devices.  Make sure to re-download the latest specifications to get all of the newest ECNs.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Call back that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/

void USBCBSuspend(void)
{
	//Example power saving code.  Insert appropriate code here for the desired
	//application behavior.  If the microcontroller will be put to sleep, a
	//process similar to that shown below may be used:
	
	//ConfigureIOPinsForLowPower();
	//SaveStateOfAllInterruptEnableBits();
	//DisableAllInterruptEnableBits();
	//EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();	//should enable at least USBActivityIF as a wake source
	//Sleep();
	//RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferably, this should be done in the USBCBWakeFromSuspend() function instead.
	//RestoreIOPinsToNormal();									//Preferably, this should be done in the USBCBWakeFromSuspend() function instead.

	//IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.  This bit is 
	//cleared inside the usb_device.c file.  Clearing USBActivityIF here will cause 
	//things to not work as intended.	
}

/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *					
 *					This call back is invoked when a wakeup from USB suspend 
 *					is detected.
 *
 * Note:            None
 *****************************************************************************/

void USBCBWakeFromSuspend(void)
{
	// If clock switching or other power savings measures were taken when
	// executing the USBCBSuspend() function, now would be a good time to
	// switch back to normal full power run mode conditions.  The host allows
	// 10+ milliseconds of wakeup time, after which the device must be 
	// fully back to normal, and capable of receiving and processing USB
	// packets.  In order to do this, the USB module must receive proper
	// clocking (IE: 48MHz clock must be available to SIE for full speed USB
	// operation).  
	// Make sure the selected oscillator settings are consistent with USB operation 
	// before returning from this function.
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/

void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/

void USBCBErrorHandler(void)
{
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

	// Typically, user firmware does not need to do anything special
	// if a USB error occurs.  For example, if the host sends an OUT
	// packet to your device, but the packet gets corrupted (ex:
	// because of a bad connection, or the user unplugs the
	// USB cable during the transmission) this will typically set
	// one or more USB error interrupt flags.  Nothing specific
	// needs to be done however, since the SIE will automatically
	// send a "NAK" packet to the host.  In response to this, the
	// host will normally retry to send the packet again, and no
	// data loss occurs.  The system will typically recover
	// automatically, without the need for application firmware
	// intervention.
	
	// Nevertheless, this callback function is provided, such as
	// for debugging purposes.
}

/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 * 					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.  For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.  This
 *					is not a standard USB chapter 9 request, and 
 *					therefore not handled by usb_device.c.  Instead
 *					this request should be handled by class specific 
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *******************************************************************/

void USBCBCheckOtherReq(void)
{
#if defined(IMPLEMENT_MICROSOFT_OS_DESCRIPTOR)
    //Check for class specific requests, and if necessary, handle it.
    USBCheckVendorRequest();
#endif
}

/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *******************************************************************/

void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}

/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This 
 *					callback function should initialize the endpoints 
 *					for the device's usage according to the current 
 *					configuration.
 *
 * Note:            None
 *******************************************************************/

void USBCBInitEP(void)
{
    USBEnableEndpoint(CPepFirmwareEndPointNumber,
                      USB_IN_ENABLED | USB_OUT_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);

    l_EndPoint1OutHandle = USBRxOnePacket(CPepFirmwareEndPointNumber,
                                          (UINT8*)&l_CommandData,
                                          sizeof(l_CommandData));
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signaling, and then
 *					send a USB "command" to the PC to wake up.
 *					
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signaling which wakes 
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function will only be able to wake up the host if
 *                  all of the below are true:
 *					
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET 
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.   
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior, 
 *                  as a USB device that has not been armed to perform remote 
 *                  wakeup must not drive remote wakeup signaling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *                  
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are 
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex: 
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup. 
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in 
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/

void USBCBSendResume(void)
{
    static UINT16 delay_count;
    
    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager 
    //properties page for the USB device, power management tab, the 
    //"Allow this device to bring the computer out of standby." checkbox 
    //should be checked).
    if (USBGetRemoteWakeupStatus() == TRUE) 
    {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signaling.
        if (USBIsBusSuspended() == TRUE)
        {
#if defined(USB_INTERRUPT)
            MUSBMaskInterrupts();
#endif
            
            //Clock switch to settings consistent with normal USB operation.
            USBCBWakeFromSuspend();
            
            USBSuspendControl = 0;
            
            USBBusIsSuspended = FALSE;  //So we don't execute this code again, 
                                        //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signaling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at 
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yields
            //5ms+ total delay since start of idle).
            delay_count = 3600U;        

            do
            {
                delay_count--;
            } while(delay_count);
            
            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1;       // Start RESUME signaling

            delay_count = 1800U;        // Set RESUME line for 1-13 ms

            do
            {
                delay_count--;
            } while (delay_count);

            USBResumeControl = 0;       //Finished driving resume signaling

#if defined(USB_INTERRUPT)
            MUSBUnmaskInterrupts();
#endif
        }
    }
}

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        int event, void* pdata, UINT16 size)
 *
 * PreCondition:    None
 *
 * Input:           int event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occurred.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/

BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void* pdata, UINT16 size)
{
    switch (event)
    {
        case EVENT_TRANSFER:
            // Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED:
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            // Add application specific callback task or callback function here if desired.
            // The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            // FEATURE (endpoint halt) request on an application endpoint which was 
            // previously armed (UOWN was = 1).  Here would be a good place to:
            // 1.  Determine which endpoint the transaction that just got terminated was 
            //     on, by checking the handle value in the *pdata.
            // 2.  Re-arm the endpoint if desired (typically would be the case for OUT 
            //     endpoints).
            break;
        default:
            break;
    }

    return TRUE; 
}
