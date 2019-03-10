/***************************************************************************/
/*  Copyright (C) 2006-2018 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlDefs_H)
#define PepCtrlDefs_H

#define CPepCtrlDeviceName L"\\\\.\\PepCtrl"

#define CPepCtrlServiceName L"PepCtrl"
#define CPepCtrlServiceDisplayName L"Pep Controller"

/*
   Registry Key and Value names
*/

#define CPepCtrlRootRegKey L"System\\CurrentControlSet\\Services\\PepCtrl"
#define CPepCtrlSettingsRegKey L"System\\CurrentControlSet\\Services\\PepCtrl\\Settings"

#define CPepCtrlSettingsRegKeyName L"Settings"

#define CPepCtrlPortTypeRegValue L"PortType"
#define CPepCtrlPortDeviceNameRegValue L"PortDeviceName"

/*
  Port Types
*/

#define CPepCtrlParallelPortType 1
#define CPepCtrlUsbPrintPortType 2

#endif

/***************************************************************************/
/*  Copyright (C) 2006-2018 Kevin Eshbach                                  */
/***************************************************************************/
