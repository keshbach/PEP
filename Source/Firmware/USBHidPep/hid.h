/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(HID_H)
#define HID_H

/* HID Interface Class Code */
#define HID_INTF        0x03

/******************************************************************************
 * Macro:           (bit) mHIDRxIsBusy(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This macro is used to check if HID OUT endpoint is
 *                  busy (owned by SIE) or not.
 *                  Typical Usage: if(mHIDRxIsBusy())
 *
 * Note:            None
 *****************************************************************************/
#define mHIDRxIsBusy()              HID_BD_OUT.Stat.UOWN

/******************************************************************************
 * Macro:           (bit) mHIDTxIsBusy(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This macro is used to check if HID IN endpoint is
 *                  busy (owned by SIE) or not.
 *                  Typical Usage: if(mHIDTxIsBusy())
 *
 * Note:            None
 *****************************************************************************/
#define mHIDTxIsBusy()              HID_BD_IN.Stat.UOWN

typedef struct _USB_HID_DSC_HEADER
{
    UINT8 bDscType;
    UINT16 wDscLength;
} USB_HID_DSC_HEADER;

typedef struct _USB_HID_DSC
{
    UINT8 bLength;
    UINT8 bDscType;
    UINT16 bcdHID;
    UINT8 bCountryCode;
    UINT8 bNumDsc;
    USB_HID_DSC_HEADER hid_dsc_header[HID_NUM_OF_DSC];
    /*
     * HID_NUM_OF_DSC is defined in autofiles\usbcfg.h
     */
} USB_HID_DSC;

void HIDInitEP(void);
void USBCheckHIDRequest(void);
void HIDTxReport(char *buffer, UINT8 nLen);
byte HIDRxReport(char *buffer, UINT8 nLen);

#endif //HID_H

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
