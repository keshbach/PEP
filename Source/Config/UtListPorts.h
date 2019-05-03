/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtListPorts_H)
#define UtListPorts_H

#include <Includes/UtExternC.h>

#if defined(_MSC_VER)
#pragma pack(push, 1)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#define UTLISTPORTSAPI __stdcall

typedef enum EPortData
{
    epdLocation = 1,
    epdPhysicalDeviceObjectName = 2
} EPortData;

MExternC BOOL UTLISTPORTSAPI UtListPortsInitialize(VOID);
MExternC BOOL UTLISTPORTSAPI UtListPortsUninitialize(VOID);
MExternC BOOL UTLISTPORTSAPI UtListPortsGetLptPortCount(_Out_ LPINT pnCount);
MExternC BOOL UTLISTPORTSAPI UtListPortsGetLptPortData(_In_ INT nIndex, _In_ INT nPortData, _Out_ LPWSTR pszData, _Out_ LPINT pnDataLen);
MExternC BOOL UTLISTPORTSAPI UtListPortsGetUsbPrintPortCount(_Out_ LPINT pnCount);
MExternC BOOL UTLISTPORTSAPI UtListPortsGetUsbPrintPortData(_In_ INT nIndex, _In_ INT nPortData, _Out_ LPWSTR pszData, _Out_ LPINT pnDataLen);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* UtListPorts_H */

/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/
