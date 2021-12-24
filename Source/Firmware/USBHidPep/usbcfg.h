/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(USBCFG_H)
#define USBCFG_H

/** D E F I N I T I O N S *******************************************/
#define MAX_NUM_INT             1   // For tracking Alternate Setting
#define EP0_BUFF_SIZE           8   // Valid Options: 8, 16, 32, or 64 bytes.
									// There is little advantage in using 
									// more than 8 bytes on EP0 IN/OUT in most cases.

/* Is this needed? */
#define LOW_PIN_COUNT_USB_DEVELOPMENT_KIT

/*
 * MUID = Microchip USB Class ID
 * Used to identify which of the USB classes owns the current
 * session of control transfer over EP0
 */
#define MUID_NULL  0
#define MUID_USB9  1
#define MUID_HID   2
#define MUID_CDC   3

/** E N D P O I N T S  A L L O C A T I O N **************************/
/*
 * See usbmmap.c for an explanation of how the endpoint allocation works
 */

/* HID */
#define HID_INTF_ID             0x00
#define HID_UEP                 UEP1
#define HID_BD_OUT              ep1Bo
#define HID_INT_OUT_EP_SIZE     64
#define HID_BD_IN               ep1Bi
#define HID_INT_IN_EP_SIZE      64
#define HID_NUM_OF_DSC          1		//Just the Report descriptor (no physical descriptor present)
#define HID_RPT01_SIZE          29

/* HID macros */
#define mUSBGetHIDDscAdr(ptr)                          \
    if (g_nUsbActiveCfg == 1)                          \
    {                                                  \
        ptr = (rom UINT8*)&g_Configuration1.hid_i00a00; \
    }

#define mUSBGetHIDRptDscAdr(ptr)            \
    if (g_nUsbActiveCfg == 1)               \
    {                                       \
        ptr = (rom UINT8*)&g_HidReport1;     \
    }

#define mUSBGetHIDRptDscSize(count)         \
    if (g_nUsbActiveCfg == 1)               \
    {                                       \
        count = sizeof(g_HidReport1);       \
    }

#endif //USBCFG_H

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
