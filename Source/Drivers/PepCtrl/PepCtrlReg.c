/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include "PepCtrlReg.h"
#include "PepCtrlLog.h"

#include <Drivers/PepCtrlDefs.h>

#include <Utils/UtHeapDriver.h>

static BOOLEAN lReadRegULongValue(IN HANDLE hRegKey, IN LPCWSTR pszValueName, IN OUT PULONG pulValue);
static BOOLEAN lReadRegStringValue(IN HANDLE hRegKey, IN LPCWSTR pszValueName, OUT LPWSTR* ppszValue);
static BOOLEAN lWriteRegULongValue(IN HANDLE hRegKey, IN LPCWSTR pszValueName, IN ULONG ulValue);
static BOOLEAN lWriteRegStringValue(IN HANDLE hRegKey, IN LPCWSTR pszValueName, IN LPCWSTR pszValue);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlReadRegSettings)
#pragma alloc_text (PAGE, PepCtrlWriteRegSettings)
#pragma alloc_text (PAGE, lReadRegULongValue)
#pragma alloc_text (PAGE, lReadRegStringValue)
#pragma alloc_text (PAGE, lWriteRegULongValue)
#pragma alloc_text (PAGE, lWriteRegStringValue)
#endif

#pragma region "Local Functions"
static BOOLEAN lReadRegULongValue(
  IN HANDLE hRegKey,
  IN LPCWSTR pszValueName,
  IN OUT PULONG pulValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    PKEY_VALUE_PARTIAL_INFORMATION pValueInfo;
    NTSTATUS status;
    ULONG ulValueInfoLen, ulResultLen;

    PepCtrlLog("lReadRegULongValue called with the value name of \"%ws\".\n", pszValueName);

    PAGED_CODE()

    RtlInitUnicodeString(&ValueName, pszValueName);

    status = ZwQueryValueKey(hRegKey, &ValueName, KeyValuePartialInformation,
                             NULL, 0, &ulValueInfoLen);

    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        pValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)UtAllocPagedMem(ulValueInfoLen);

        if (!pValueInfo)
        {
            PepCtrlLog("lReadRegULongValue - Could not allocate memory.\n");

            return FALSE;
        }

        status = ZwQueryValueKey(hRegKey, &ValueName, KeyValuePartialInformation,
                                 pValueInfo, ulValueInfoLen, &ulResultLen);

        if (NT_SUCCESS(status) && pValueInfo->Type == REG_DWORD)
        {
            *pulValue = *((PULONG)pValueInfo->Data);

            bResult = TRUE;
        }
        else
        {
            if (NT_ERROR(status))
            {
                PepCtrlLog("lReadRegULongValue - Could not query the registry value. (0x%X)\n", status);
            }
            else if (pValueInfo->Type != REG_DWORD)
            {
                PepCtrlLog("lReadRegULongValue - Wrong registry value type found. (0x%X)\n", pValueInfo->Type);
            }
        }

        UtFreePagedMem(pValueInfo);
    }
    else
    {
        PepCtrlLog("lReadRegULongValue - Could not query the registry value size. (0x%X)\n", status);
    }

    return bResult;
}

static BOOLEAN lReadRegStringValue(
  IN HANDLE hRegKey,
  IN LPCWSTR pszValueName,
  OUT LPWSTR* ppszValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    PKEY_VALUE_PARTIAL_INFORMATION pValueInfo;
    NTSTATUS status;
    ULONG ulValueInfoLen, ulResultLen;

    PepCtrlLog("lReadRegStringValue called with the value name of \"%ws\".\n", pszValueName);

    PAGED_CODE()

    *ppszValue = 0;

    RtlInitUnicodeString(&ValueName, pszValueName);

    status = ZwQueryValueKey(hRegKey, &ValueName, KeyValuePartialInformation,
                             NULL, 0, &ulValueInfoLen);

    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        pValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)UtAllocPagedMem(ulValueInfoLen);

        if (!pValueInfo)
        {
            PepCtrlLog("lReadRegStringValue - Could not allocate memory.\n");

            return FALSE;
        }

        status = ZwQueryValueKey(hRegKey, &ValueName, KeyValuePartialInformation,
                                 pValueInfo, ulValueInfoLen, &ulResultLen);

        if (NT_SUCCESS(status) && pValueInfo->Type == REG_SZ)
        {
            *ppszValue = (LPWSTR)UtAllocPagedMem(pValueInfo->DataLength);

            RtlStringCbCopyW(*ppszValue, pValueInfo->DataLength,
                             (LPCWSTR)pValueInfo->Data);

            bResult = TRUE;
        }
        else
        {
            if (NT_ERROR(status))
            {
                PepCtrlLog("lReadRegStringValue - Could not query the registry value. (0x%X)\n", status);
            }
            else if (pValueInfo->Type != REG_SZ)
            {
                PepCtrlLog("lReadRegStringValue - Wrong registry value type found. (0x%X)\n", pValueInfo->Type);
            }
        }

        UtFreePagedMem(pValueInfo);
    }
    else
    {
        PepCtrlLog("lReadRegStringValue - Could not query the registry value size. (0x%X)\n", status);
    }

    return bResult;
}

static BOOLEAN lWriteRegULongValue(
  IN HANDLE hRegKey,
  IN LPCWSTR pszValueName,
  IN ULONG ulValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    NTSTATUS status;

    PepCtrlLog("lWriteRegULongValue called with the value name of \"%ws\".\n", pszValueName);

    PAGED_CODE()

    RtlInitUnicodeString(&ValueName, pszValueName);

    status = ZwSetValueKey(hRegKey, &ValueName, 0, REG_DWORD, &ulValue, sizeof(ulValue));

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("lWriteRegULongValue - Successfully set the registry value.\n");

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lWriteRegULongValue - Could not set the registry value. (0x%X)\n", status);
    }

    return bResult;
}

static BOOLEAN lWriteRegStringValue(
  IN HANDLE hRegKey,
  IN LPCWSTR pszValueName,
  IN LPCWSTR pszValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    NTSTATUS status;
    size_t length;

    PepCtrlLog("lWriteRegStringValue called with the value name of \"%ws\".\n", pszValueName);

    PAGED_CODE()

    RtlInitUnicodeString(&ValueName, pszValueName);

    RtlStringCchLengthW(pszValue, NTSTRSAFE_MAX_CCH, &length);

    status = ZwSetValueKey(hRegKey, &ValueName, 0, REG_SZ, (PVOID)pszValue, ((ULONG)length + 1) * sizeof(WCHAR));

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("lWriteRegStringValue - Successfully set the registry value.\n");

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lWriteRegStringValue - Could not set the registry value. (0x%X)\n", status);
    }

    return bResult;
}
#pragma endregion

BOOLEAN PepCtrlReadRegSettings(
  IN PUNICODE_STRING pRegistryPath,
  OUT PULONG pulPortType,
  OUT LPWSTR* ppszPortDeviceName)
{
    BOOLEAN bResult = FALSE;
    LPWSTR pszSettingsRegPath;
    UNICODE_STRING SettingsRegPath;
    OBJECT_ATTRIBUTES RegPathObj;
    HANDLE hRegKey;
    NTSTATUS status;
    size_t SettingsLen, SettingsRegPathLen;

    PepCtrlLog("PepCtrlReadRegSettings called with a registry path of \"%ws\".\n", pRegistryPath->Buffer);

    PAGED_CODE()

    RtlStringCbLengthW(CPepCtrlSettingsRegKeyName, NTSTRSAFE_MAX_CCH, &SettingsLen);

    SettingsRegPathLen = pRegistryPath->Length + SettingsLen + (sizeof(WCHAR) * 2);
    pszSettingsRegPath = (LPWSTR)UtAllocPagedMem(SettingsRegPathLen);

    if (!pszSettingsRegPath)
    {
        PepCtrlLog("PepCtrlReadRegSettings - Could not allocate memory for the settings registry key name.\n");

        return FALSE;
    }

    RtlStringCbCopyW(pszSettingsRegPath, SettingsRegPathLen,
                     pRegistryPath->Buffer);

    RtlStringCbCatW(pszSettingsRegPath, SettingsRegPathLen, L"\\");

    RtlStringCbCatW(pszSettingsRegPath, SettingsRegPathLen,
                    CPepCtrlSettingsRegKeyName);

    RtlInitUnicodeString(&SettingsRegPath, pszSettingsRegPath);

    InitializeObjectAttributes(&RegPathObj, &SettingsRegPath,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL, NULL);

    PepCtrlLog("PepCtrlReadRegSettings - Opening the registry key \"%ws\".\n", pszSettingsRegPath);

    status = ZwOpenKey(&hRegKey, GENERIC_READ, &RegPathObj);

    if (NT_SUCCESS(status))
    {
        if (lReadRegULongValue(hRegKey, CPepCtrlPortTypeRegValue, pulPortType) &&
            lReadRegStringValue(hRegKey, CPepCtrlPortDeviceNameRegValue, ppszPortDeviceName))
        {
            bResult = TRUE;
        }

        ZwClose(hRegKey);
    }
    else
    {
        PepCtrlLog("PepCtrlReadRegSettings - Could not open the root registry key. (0x%X)\n", status);
    }

    UtFreePagedMem(pszSettingsRegPath);

    return bResult;
}

BOOLEAN PepCtrlWriteRegSettings(
  IN PUNICODE_STRING pRegistryPath,
  IN ULONG ulPortType,
  IN LPWSTR pszPortDeviceName)
{
    BOOLEAN bResult = FALSE;
    LPWSTR pszSettingsRegPath;
    UNICODE_STRING SettingsRegPath;
    OBJECT_ATTRIBUTES RegPathObj;
    HANDLE hRegKey;
    NTSTATUS status;
    size_t SettingsLen, SettingsRegPathLen;
    ULONG ulDisposition;

    PepCtrlLog("PepCtrlWriteRegSettings called with a registry path of \"%ws\".\n", pRegistryPath->Buffer);

    PAGED_CODE()

    RtlStringCbLengthW(CPepCtrlSettingsRegKeyName, NTSTRSAFE_MAX_CCH, &SettingsLen);

    SettingsRegPathLen = pRegistryPath->Length + SettingsLen + (sizeof(WCHAR) * 2);
    pszSettingsRegPath = (LPWSTR)UtAllocPagedMem(SettingsRegPathLen);

    if (!pszSettingsRegPath)
    {
        PepCtrlLog("PepCtrlWriteRegSettings - Could not allocate memory for the settings registry key name.\n");

        return FALSE;
    }

    RtlStringCbCopyW(pszSettingsRegPath, SettingsRegPathLen,
                     pRegistryPath->Buffer);

    RtlStringCbCatW(pszSettingsRegPath, SettingsRegPathLen, L"\\");

    RtlStringCbCatW(pszSettingsRegPath, SettingsRegPathLen,
                    CPepCtrlSettingsRegKeyName);

    RtlInitUnicodeString(&SettingsRegPath, pszSettingsRegPath);

    InitializeObjectAttributes(&RegPathObj, &SettingsRegPath,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL, NULL);

    PepCtrlLog("PepCtrlWriteRegSettings - Creating the registry key \"%ws\".\n", pszSettingsRegPath);

    status = ZwCreateKey(&hRegKey, GENERIC_WRITE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
                         &RegPathObj, 0, NULL, REG_OPTION_NON_VOLATILE, &ulDisposition);

    if (NT_SUCCESS(status))
    {
        if (lWriteRegULongValue(hRegKey, CPepCtrlPortTypeRegValue, ulPortType) &&
            lWriteRegStringValue(hRegKey, CPepCtrlPortDeviceNameRegValue, pszPortDeviceName))
        {
            bResult = TRUE;
        }

        ZwClose(hRegKey);
    }
    else
    {
        PepCtrlLog("PepCtrlWriteRegSettings - Could not create the root registry key. (0x%X)\n", status);
    }

    UtFreePagedMem(pszSettingsRegPath);
        
    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
