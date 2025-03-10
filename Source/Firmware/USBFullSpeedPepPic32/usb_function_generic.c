#include "UtPortTypeDefs.h"

#include "usb.h"

#if defined(IMPLEMENT_MICROSOFT_OS_DESCRIPTOR)
extern volatile TUsbSetupPacket SetupPkt; // Common buffer that receives the 
                                          // 8-byte SETUP packet data from the 
                                          // host during control transfer 
                                          // requests.
#endif

/********************************************************************
	Function:
		void USBCheckVendorRequest(void)

 	Summary:
 		This routine handles vendor class specific requests that happen on EP0.
        This function should be called from the USBCBCheckOtherReq() call back
        function whenever implementing a custom/vendor class device.

 	Description:
 		This routine handles vendor specific requests that may arrive on EP0 as
 		a control transfer.  These can include, but are not necessarily 
 		limited to, requests for Microsoft specific OS feature descriptor(s).  
 		This function should be called from the USBCBCheckOtherReq() call back 
 		function whenever using a vendor class device.

        Typical Usage:
        <code>
        void USBCBCheckOtherReq(void)
        {
            //Since the stack didn't handle the request I need to check
            //  my class drivers to see if it is for them
            USBCheckVendorRequest();
        }
        </code>

	PreCondition:
		None

	Parameters:
		Although this function has a void input, this handler function will
		typically need to look at the 8-byte SETUP packet contents that the
		host just sent, which may contain the vendor class specific request.
		
		Therefore, the statically allocated SetupPkt structure may be looked
		at while in the context of this function, and it will contain the most
		recently received 8-byte SETUP packet data.

	Return Values:
		None

	Remarks:
		This function normally gets called within the same context as the
		USBDeviceTasks() function, just after a new control transfer request
		from the host has arrived.  If the USB stack is operated in 
		USB_INTERRUPT mode (a usb_config.h option), then this function
		will be executed in the interrupt context.  If however the USB stack
		is operated in the USB_POLLING mode, then this function executes in the
		main loop context.
		
		In order to respond to class specific control transfer request(s) in
		this handler function, it is suggested to use one or more of the
		USBEP0SendRAMPtr(), USBEP0SendROMPtr(), or USBEP0Receive() API 
		functions.
 
 *******************************************************************/

#if defined(IMPLEMENT_MICROSOFT_OS_DESCRIPTOR)
void USBCheckVendorRequest(void)
{
    UINT16 wLength;

    //Check if the most recent SETUP request is class specific
    if (SetupPkt.bmRequestType == 0b11000000)    //Class specific, device to host, device level target
    {
        //Check if the host is requesting an MS feature descriptor
        if (SetupPkt.bRequest == GET_MS_DESCRIPTOR)
        {
            //Figure out which descriptor is being requested
            if (SetupPkt.wIndex == EXTENDED_COMPAT_ID)
            {
                //Determine number of bytes to send to host
                //Lesser of: requested amount, or total size of the descriptor
                wLength = CompatIDFeatureDescriptor.dwLength;

                if (SetupPkt.wLength < wLength)
                {
                    wLength = SetupPkt.wLength;
                }

                //Prepare to send the requested descriptor to the host
                USBEP0SendROMPtr((ROM UINT8*)&CompatIDFeatureDescriptor, wLength,
                                 USB_EP0_ROM | USB_EP0_INCLUDE_ZERO);
            }
        }
    }
    else if (SetupPkt.bmRequestType == 0b11000001)    //Class specific, device to host, interface target
    {
        //Check if the host is requesting an MS feature descriptor
        if (SetupPkt.bRequest == GET_MS_DESCRIPTOR)
        {
            //Figure out which descriptor is being requested
            if (SetupPkt.wIndex == EXTENDED_PROPERTIES)
            {
                //Determine number of bytes to send to host
                //Lesser of: requested amount, or total size of the descriptor
                wLength = ExtPropertyFeatureDescriptor.dwLength;

                if (SetupPkt.wLength < wLength)
                {
                    wLength = SetupPkt.wLength;
                }

                //Prepare to send the requested descriptor to the host
                USBEP0SendROMPtr((ROM UINT8*)&ExtPropertyFeatureDescriptor, wLength,
                                 USB_EP0_ROM | USB_EP0_INCLUDE_ZERO);
            }
        }
    }
}
#endif
