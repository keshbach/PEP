/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

#include <ntddk.h>

#include <ntstrsafe.h>

#include "PepCtrlReg.h"
#include "PepCtrlLog.h"

#include <Drivers/PepCtrlDefs.h>

#include <Utils/UtHeapDriver.h>

#pragma region "Constants"

/*
   Registry Key and Value names
*/

#define CPepCtrlRootRegKey L"System\\CurrentControlSet\\Services\\PepCtrl"
#define CPepCtrlSettingsRegKey L"System\\CurrentControlSet\\Services\\PepCtrl\\Settings"

#define CPepCtrlSettingsRegKeyName L"Settings"

#define CPepCtrlPortTypeRegValue L"PortType"
#define CPepCtrlPortDeviceNameRegValue L"PortDeviceName"

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadRegULongValue(_In_ HANDLE hRegKey, _In_ LPCWSTR pszValueName, _In_ ULONG ulDefaultValue, _Out_ PULONG pulValue);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadRegStringValue(_In_ HANDLE hRegKey, _In_ LPCWSTR pszValueName, _In_ LPCWSTR pszDefaultValue, _Out_ LPWSTR* ppszValue);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteRegULongValue(_In_ HANDLE hRegKey, _In_ LPCWSTR pszValueName, _In_ ULONG ulValue);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteRegStringValue(_In_ HANDLE hRegKey, _In_ LPCWSTR pszValueName, _In_ LPCWSTR pszValue);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlReadRegSettings)
#pragma alloc_text (PAGE, PepCtrlWriteRegSettings)
#pragma alloc_text (PAGE, lReadRegULongValue)
#pragma alloc_text (PAGE, lReadRegStringValue)
#pragma alloc_text (PAGE, lWriteRegULongValue)
#pragma alloc_text (PAGE, lWriteRegStringValue)
#endif

#pragma region "Local Functions"

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadRegULongValue(
  _In_ HANDLE hRegKey,
  _In_ LPCWSTR pszValueName,
  _In_ ULONG ulDefaultValue,
  _Out_ PULONG pulValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    PKEY_VALUE_PARTIAL_INFORMATION pValueInfo;
    NTSTATUS status;
    ULONG ulValueInfoLen, ulResultLen;

	PepCtrlLog("lReadRegULongValue entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

	*pulValue = 0;

    PepCtrlLog("lReadRegULongValue - Value Name: \"%ws\".  (Thread: 0x%p)\n",
		       pszValueName, PsGetCurrentThread());

    RtlInitUnicodeString(&ValueName, pszValueName);

    status = ZwQueryValueKey(hRegKey, &ValueName, KeyValuePartialInformation,
                             NULL, 0, &ulValueInfoLen);

    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        pValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)UtAllocPagedMem(ulValueInfoLen);

        if (!pValueInfo)
        {
            PepCtrlLog("lReadRegULongValue leaving  (Could not allocate memory).  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

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
                PepCtrlLog("lReadRegULongValue - Could not query the registry value (0x%X).  (Thread: 0x%p)\n",
					       status, PsGetCurrentThread());
            }
            else if (pValueInfo->Type != REG_DWORD)
            {
                PepCtrlLog("lReadRegULongValue - Wrong registry value type found (0x%X).  (Thread: 0x%p)\n",
					       pValueInfo->Type, PsGetCurrentThread());
            }
        }

        UtFreePagedMem(pValueInfo);
    }
    else if (status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
        PepCtrlLog("lReadRegULongValue - The registry value does not exist.  Returning the default value.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        *pulValue = ulDefaultValue;

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lReadRegULongValue - Could not query the registry value size (0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
    }

	PepCtrlLog("lReadRegULongValue leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadRegStringValue(
  _In_ HANDLE hRegKey,
  _In_ LPCWSTR pszValueName,
  _In_ LPCWSTR pszDefaultValue,
  _Out_ LPWSTR* ppszValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    PKEY_VALUE_PARTIAL_INFORMATION pValueInfo;
    NTSTATUS status;
    ULONG ulValueInfoLen, ulResultLen;
    size_t DefaultValueLen = 0;

    PepCtrlLog("lReadRegStringValue entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lReadRegStringValue - Value Name: \"%ws\".  (Thread: 0x%p)\n",
		       pszValueName, PsGetCurrentThread());

    *ppszValue = 0;

    RtlInitUnicodeString(&ValueName, pszValueName);

    status = ZwQueryValueKey(hRegKey, &ValueName, KeyValuePartialInformation,
                             NULL, 0, &ulValueInfoLen);

    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        pValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)UtAllocPagedMem(ulValueInfoLen);

        if (!pValueInfo)
        {
            PepCtrlLog("lReadRegStringValue leaving.  (Could not allocate memory)  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

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
                PepCtrlLog("lReadRegStringValue - Could not query the registry value (0x%X).  (Thread: 0x%p)\n",
					       status, PsGetCurrentThread());
            }
            else if (pValueInfo->Type != REG_SZ)
            {
                PepCtrlLog("lReadRegStringValue - Wrong registry value type found (0x%X).  (Thread: 0x%p)\n",
					       pValueInfo->Type, PsGetCurrentThread());
            }
        }

        UtFreePagedMem(pValueInfo);
    }
    else if (status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
        PepCtrlLog("lReadRegStringValue - The registry value does not exist.  Returning the default value.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        RtlStringCchLengthW(pszDefaultValue, NTSTRSAFE_MAX_CCH, &DefaultValueLen);

        *ppszValue = (LPWSTR)UtAllocPagedMem((DefaultValueLen + 1) * sizeof(WCHAR));

        if (*ppszValue == NULL)
        {
            PepCtrlLog("lReadRegStringValue - Could not allocate memory for the default value.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            return FALSE;
        }

        RtlStringCchCopyW(*ppszValue, DefaultValueLen + 1, pszDefaultValue);

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lReadRegStringValue - Could not query the registry value size (0x%X).  (Thread: 0x%p)\n", 
			       status, PsGetCurrentThread());
    }

	PepCtrlLog("lReadRegStringValue leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteRegULongValue(
  _In_ HANDLE hRegKey,
  _In_ LPCWSTR pszValueName,
  _In_ ULONG ulValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    NTSTATUS status;

    PepCtrlLog("lWriteRegULongValue entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

	PepCtrlLog("lWriteRegULongValue - Value Name: \"%ws\".  (Thread: 0x%p)\n",
		       pszValueName, PsGetCurrentThread());

    RtlInitUnicodeString(&ValueName, pszValueName);

    status = ZwSetValueKey(hRegKey, &ValueName, 0, REG_DWORD, &ulValue, sizeof(ulValue));

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("lWriteRegULongValue - Successfully set the registry value.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lWriteRegULongValue - Could not set the registry value (0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
    }

	PepCtrlLog("lWriteRegULongValue leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteRegStringValue(
  _In_ HANDLE hRegKey,
  _In_ LPCWSTR pszValueName,
  _In_ LPCWSTR pszValue)
{
    BOOLEAN bResult = FALSE;
    UNICODE_STRING ValueName;
    NTSTATUS status;
    size_t length;

    PepCtrlLog("lWriteRegStringValue entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lWriteRegStringValue - Value Name: \"%ws\".  (Thread: 0x%p)\n",
		       pszValueName, PsGetCurrentThread());

    RtlInitUnicodeString(&ValueName, pszValueName);

    RtlStringCchLengthW(pszValue, NTSTRSAFE_MAX_CCH, &length);

    status = ZwSetValueKey(hRegKey, &ValueName, 0, REG_SZ, (PVOID)pszValue, ((ULONG)length + 1) * sizeof(WCHAR));

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("lWriteRegStringValue - Successfully set the registry value.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lWriteRegStringValue - Could not set the registry value (0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
    }

	PepCtrlLog("lWriteRegStringValue leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}
#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlReadRegSettings(
  _In_ PUNICODE_STRING pRegistryPath,
  _Out_ PULONG pulPortType,
  _Out_ LPWSTR* ppszPortDeviceName)
{
    BOOLEAN bResult = FALSE;
    LPWSTR pszSettingsRegPath;
    UNICODE_STRING SettingsRegPath;
    OBJECT_ATTRIBUTES RegPathObj;
    HANDLE hRegKey;
    NTSTATUS status;
    size_t SettingsLen, SettingsRegPathLen;
    ULONG ulDisposition;

    PepCtrlLog("PepCtrlReadRegSettings entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

	*pulPortType = 0;
	*ppszPortDeviceName = NULL;

	PepCtrlLog("PepCtrlReadRegSettings - Registry Path: \"%ws\".  (Thread: 0x%p)\n",
		       pRegistryPath->Buffer, PsGetCurrentThread());

    RtlStringCbLengthW(CPepCtrlSettingsRegKeyName, NTSTRSAFE_MAX_CCH, &SettingsLen);

    SettingsRegPathLen = pRegistryPath->Length + SettingsLen + (sizeof(WCHAR) * 2);
    pszSettingsRegPath = (LPWSTR)UtAllocPagedMem(SettingsRegPathLen);

    if (!pszSettingsRegPath)
    {
        PepCtrlLog("PepCtrlReadRegSettings leaving.  (Could not allocate memory for the settings registry key name)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

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

    PepCtrlLog("PepCtrlReadRegSettings - Opening the registry key \"%ws\".  (Thread: 0x%p)\n",
		       pszSettingsRegPath, PsGetCurrentThread());

    status = ZwCreateKey(&hRegKey, KEY_READ | KEY_WRITE, &RegPathObj, 0, NULL,
                         REG_OPTION_NON_VOLATILE, &ulDisposition);

    if (status == STATUS_SUCCESS)
    {
        if (lReadRegULongValue(hRegKey, CPepCtrlPortTypeRegValue, CPepCtrlNoPortType, pulPortType) &&
            lReadRegStringValue(hRegKey, CPepCtrlPortDeviceNameRegValue, L"", ppszPortDeviceName))
        {
            bResult = TRUE;
        }

        ZwClose(hRegKey);
    }
    else
    {
        PepCtrlLog("PepCtrlReadRegSettings - Could not open the root registry key (0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
    }

    UtFreePagedMem(pszSettingsRegPath);

	PepCtrlLog("PepCtrlReadRegSettings leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlWriteRegSettings(
  _In_ PUNICODE_STRING pRegistryPath,
  _In_ ULONG ulPortType,
  _In_ LPWSTR pszPortDeviceName)
{
    BOOLEAN bResult = FALSE;
    LPWSTR pszSettingsRegPath;
    UNICODE_STRING SettingsRegPath;
    OBJECT_ATTRIBUTES RegPathObj;
    HANDLE hRegKey;
    NTSTATUS status;
    size_t SettingsLen, SettingsRegPathLen;
    ULONG ulDisposition;

    PepCtrlLog("PepCtrlWriteRegSettings entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

	PepCtrlLog("PepCtrlWriteRegSettings - Registry Path: \"%ws\", Port Type: 0x%X, Port Device Name: \"%ws\".  (Thread: 0x%p)\n",
               pRegistryPath->Buffer, ulPortType, pszPortDeviceName, PsGetCurrentThread());

    RtlStringCbLengthW(CPepCtrlSettingsRegKeyName, NTSTRSAFE_MAX_CCH, &SettingsLen);

    SettingsRegPathLen = pRegistryPath->Length + SettingsLen + (sizeof(WCHAR) * 2);
    pszSettingsRegPath = (LPWSTR)UtAllocPagedMem(SettingsRegPathLen);

    if (!pszSettingsRegPath)
    {
        PepCtrlLog("PepCtrlWriteRegSettings leaving.  (Could not allocate memory for the settings registry key name)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

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

    PepCtrlLog("PepCtrlWriteRegSettings - Creating the registry key \"%ws\".  (Thread: 0x%p)\n",
		       pszSettingsRegPath, PsGetCurrentThread());

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
        PepCtrlLog("PepCtrlWriteRegSettings - Could not create the root registry key (0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
    }

    UtFreePagedMem(pszSettingsRegPath);

    PepCtrlLog("PepCtrlWriteRegSettings leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
