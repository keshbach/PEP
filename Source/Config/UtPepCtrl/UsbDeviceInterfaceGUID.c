/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <rpc.h>

#include "UsbDeviceInterfaceGUID.h"

#include <Firmware/PepFirmwareDefs.h>

#pragma region "Constants"

#define CHidLibrary L"hid.dll"

#define CRpcLibrary L"Rpcrt4.dll"

#pragma endregion

#pragma region "Type Definitions"

typedef void (__stdcall *TGetHidGuidFunc)( _Out_ LPGUID pHidGuid);

typedef RPC_STATUS (RPC_ENTRY *TUuidFromString)(_In_opt_ RPC_WSTR StringUuid, _Out_ UUID __RPC_FAR* Uuid);

#pragma endregion

BOOL UsbHIDDeviceInterfaceGUID(
  _Out_writes_(sizeof(GUID)) LPGUID pGuid)
{
    BOOL bResult = FALSE;
    HMODULE hModule = LoadLibraryW(CHidLibrary);
    TGetHidGuidFunc pGetHidGuidFunc;

    ZeroMemory(pGuid, sizeof(*pGuid));

    if (hModule == NULL)
    {
        return FALSE;
    }

    pGetHidGuidFunc = (TGetHidGuidFunc)GetProcAddress(hModule, "HidD_GetHidGuid");

    if (pGetHidGuidFunc)
    {
        pGetHidGuidFunc(pGuid);

        bResult = TRUE;
    }

    FreeLibrary(hModule);

    return bResult;
}

BOOL UsbFullSpeedDeviceInterfaceGUID(
   _Out_writes_(sizeof(GUID)) LPGUID pGuid)
{
    BOOL bResult = FALSE;
    HMODULE hModule = LoadLibraryW(CRpcLibrary);
    TUuidFromString pUuidFromStringFunc;

    ZeroMemory(pGuid, sizeof(*pGuid));

    if (hModule == NULL)
    {
        return FALSE;
    }

    pUuidFromStringFunc = (TUuidFromString)GetProcAddress(hModule, "UuidFromStringW");

    if (pUuidFromStringFunc)
    {
        if (S_OK == pUuidFromStringFunc(CPepFirmwareDeviceInterfaceGuid, pGuid))
        {
            bResult = TRUE;
        }
    }

    FreeLibrary(hModule);

    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
