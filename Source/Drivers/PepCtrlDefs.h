/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlDefs_H)
#define PepCtrlDefs_H

#if defined(_MSC_VER)
#if defined(_X86_)
#pragma pack(push, 4)
#elif defined(_WIN64)
#pragma pack(push, 8)
#else
#error Need to specify cpu architecture to configure structure padding
#endif
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#define CPepCtrlDeviceName L"\\\\.\\PepCtrl"

#define CPepCtrlServiceName L"PepCtrl"
#define CPepCtrlServiceDisplayName L"Pep Controller"

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
  Port Types
*/

#define CPepCtrlNoPortType       0x0000
#define CPepCtrlParallelPortType 0x0001
#define CPepCtrlUsbPrintPortType 0x0002

typedef struct tagTPepCtrlPortSettings
{
    UINT32 nPortType;
    WCHAR cPortDeviceName[1];
} TPepCtrlPortSettings;

typedef struct tagTPepCtrlDelaySettings
{
	UINT32 nChipEnableNanoseconds;  // number of nanoseconds before chip ready after VCC applied
	UINT32 nOutputEnableNanoseconds; // number of nanoseconds before data available after the Output Enable goes low
} TPepCtrlDelaySettings;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif // PepCtrlDefs_H

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
