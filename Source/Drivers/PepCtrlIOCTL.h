/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlIOCTL_H)
#define PepCtrlIOCTL_H

/*
  Programmer Modes
*/

#define CPepCtrlProgrammerNoneMode  0x0001
#define CPepCtrlProgrammerReadMode  0x0002
#define CPepCtrlProgrammerWriteMode 0x0004 /* OE pin always going to be high */

/*
  Switched VCC Modes (Power voltage)
*/

#define CPepCtrl5VDCMode   0x0001 /* +5 VDC */
#define CPepCtrl625VDCMode 0x0002 /* +6.25 VDC */

/*
  Pin/Pulse Modes
*/

#define CPepCtrlPinPulse1Mode 0x0001 /* Pin 23 as +12VDC, Pin 24 as +12VDC (Enable Vpp Mode), Pin 25 as -5VDC, 1 msec Vpp pulse */
#define CPepCtrlPinPulse2Mode 0x0002 /* Pin 25 as +5VDC/Vpp (Enable Vpp Mode), 250 us Vpp pulse */
#define CPepCtrlPinPulse3Mode 0x0003 /* Pin 24 as Vpp (Enable Vpp Mode), 250 us Chip Enable pulse */
#define CPepCtrlPinPulse4Mode 0x0004 /* Pin 1 and 3 as Vpp (Enable Vpp Mode), User-defined Chip Enable pulse */

/*
  Vpp Modes (Program voltage)
*/

#define CPepCtrl12VDCVppMode 0x0001 /* +12 VDC */
#define CPepCtrl21VDCVppMode 0x0002 /* +21 VDC */
#define CPepCtrl25VDCVppMode 0x0004 /* +25 VDC */

/*
  Device Status
*/

#define CPepCtrlDeviceNotPresent 0x0001
#define CPepCtrlDevicePresent    0x0002

/*
  Device status change notification
*/

#define CPepCtrlDeviceArrived 0x0001
#define CPepCtrlDeviceRemoved 0x0002

/*
   IO Control Codes
*/

#define IOCTL_PEPCTRL_SET_PROGRAMMER_MODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_SET_VCC_MODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_SET_PIN_PULSE_MODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_SET_VPP_MODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_SET_ADDRESS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_GET_DATA \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_SET_DATA \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_TRIGGER_PROGRAM \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_SET_OUTPUT_ENABLE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_GET_DEVICE_STATUS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_DEVICE_NOTIFICATION \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80a, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_GET_SETTINGS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80b, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PEPCTRL_SET_SETTINGS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80c, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif /* PepCtrlIOCTL_H */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
