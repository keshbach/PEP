/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(USBMMAP_H)
#define USBMMAP_H

/* Buffer Descriptor Status Register Initialization Parameters */
#define _BSTALL     0x04                // Buffer Stall enable
#define _DTSEN      0x08                // Data Toggle Synch enable
#define _INCDIS     0x10                // Address increment disable
#define _KEN        0x20                // SIE keeps buff descriptors enable
#define _DAT0       0x00                // DATA0 packet expected next
#define _DAT1       0x40                // DATA1 packet expected next
#define _DTSMASK    0x40                // DTS Mask
#define _USIE       0x80                // SIE owns buffer
#define _UCPU       0x00                // CPU owns buffer

/* USB Device States - To be used with [UINT8 g_nUsbDeviceState] */
#define DETACHED_STATE          0
#define ATTACHED_STATE          1
#define POWERED_STATE           2
#define DEFAULT_STATE           3
#define ADR_PENDING_STATE       4
#define ADDRESS_STATE           5
#define CONFIGURED_STATE        6

/* Memory Types for Control Transfer - used in USB_DEVICE_STATUS */
#define _RAM 0
#define _ROM 1

/** T Y P E S ****************************************************************/
typedef union _USB_DEVICE_STATUS
{
    UINT8 _byte;
    struct
    {
        unsigned RemoteWakeup:1; // [0]Disabled [1]Enabled: See usbdrv.c,usb9.c
        unsigned ctrl_trf_mem:1; // [0]RAM      [1]ROM
    };
} USB_DEVICE_STATUS;

typedef union _BD_STAT
{
    UINT8 _byte;
    struct{
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned BSTALL:1;              // Buffer Stall Enable
        unsigned DTSEN:1;               // Data Toggle Synch Enable
        unsigned INCDIS:1;              // Address Increment Disable
        unsigned KEN:1;                 // BD Keep Enable
        unsigned DTS:1;                 // Data Toggle Synch Value
        unsigned UOWN:1;                // USB Ownership
    };
    struct{
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned PID0:1;
        unsigned PID1:1;
        unsigned PID2:1;
        unsigned PID3:1;
        unsigned :1;
        unsigned UOWN:1;
    };
    struct{
        unsigned :2;
        unsigned PID:4;                 // Packet Identifier
        unsigned :2;
    };
} BD_STAT;                              // Buffer Descriptor Status Register

typedef union _BDT
{
    struct
    {
        BD_STAT Stat;
        UINT8 Cnt;
        UINT8 ADRL;                      // Buffer Address Low
        UINT8 ADRH;                      // Buffer Address High
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        UINT8* ADR;                      // Buffer Address
    };
} BDT;                                  // Buffer Descriptor Table

extern UINT8 g_nUsbDeviceState;
extern USB_DEVICE_STATUS g_UsbDeviceStatus;
extern UINT8 g_nUsbActiveCfg;
extern UINT8 g_nUsbAlternateInterface[MAX_NUM_INT];

extern volatile far BDT ep0Bo;          //Endpoint #0 BD Out
extern volatile far BDT ep0Bi;          //Endpoint #0 BD In
extern volatile far BDT ep1Bo;          //Endpoint #1 BD Out
extern volatile far BDT ep1Bi;          //Endpoint #1 BD In

extern volatile far CTRL_TRF_SETUP SetupPkt;
extern volatile far CTRL_TRF_DATA CtrlTrfData;

extern volatile far UINT8 g_nHidReportOut[HID_INT_OUT_EP_SIZE];
extern volatile far UINT8 g_nHidReportIn[HID_INT_IN_EP_SIZE];

#endif //USBMMAP_H

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
