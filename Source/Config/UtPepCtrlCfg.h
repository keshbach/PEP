/***************************************************************************/
/*  Copyright (C) 2007-2018 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepCtrlCfg_H)
#define UtPepCtrlCfg_H

#include <Includes/UtExternC.h>

#if defined(_MSC_VER)
#pragma pack(push, 1)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#define UTPEPCTRLCFGAPI __stdcall

typedef enum tagEUtPepCtrlCfgPortType
{
    eUtPepCtrlCfgParallelPortType,
    eUtPepCtrlCfgUsbPrintPortType
} EUtPepCtrlCfgPortType;

typedef VOID (UTPEPCTRLCFGAPI *TUtPepCtrlCfgMsgFunc)(LPCWSTR pszMsg);

MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgInitialize(VOID);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgUninitialize(VOID);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgInstallDriver(LPCWSTR pszFile, TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgUninstallDriver(TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgStartDriver(TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgStopDriver(TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgSetPortSettings(EUtPepCtrlCfgPortType PortType, LPCWSTR pszPortDeviceName);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgGetPortType(EUtPepCtrlCfgPortType* pPortType);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgGetPortDeviceName(LPWSTR pszPortDeviceName, LPINT pnPortDeviceNameLen);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* UtPepCtrlCfg_H */

/***************************************************************************/
/*  Copyright (C) 2007-2018 Kevin Eshbach                                  */
/***************************************************************************/
