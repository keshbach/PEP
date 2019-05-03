/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
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

typedef VOID (UTPEPCTRLCFGAPI *TUtPepCtrlCfgMsgFunc)(_In_ LPCWSTR pszMsg);

MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgInitialize(VOID);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgUninitialize(VOID);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgInstallDriver(_In_ LPCWSTR pszFile, _In_ TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgUninstallDriver(_In_ TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgStartDriver(_In_ TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgStopDriver(_In_ TUtPepCtrlCfgMsgFunc pMsgFunc);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgSetPortSettings(_In_ EUtPepCtrlCfgPortType PortType, _In_ LPCWSTR pszPortDeviceName);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgGetPortType(_Out_ EUtPepCtrlCfgPortType* pPortType);
MExternC BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgGetPortDeviceName(_Out_ LPWSTR pszPortDeviceName, _Out_ LPINT pnPortDeviceNameLen);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* UtPepCtrlCfg_H */

/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/
