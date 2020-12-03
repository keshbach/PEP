/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

#include <Ntifs.h> // include this for ObQueryNameString

#include <wdmguid.h>

#include <ntstrsafe.h>

#include "PepCtrlPlugPlay.h"
#include "PepCtrlLog.h"
#include "PepCtrlThread.h"

#include <Utils/UtHeapDriver.h>

#include <Drivers/PepCtrlDefs.h>

#include <Includes/UtMacros.h>

#pragma region "Constants"

#define CTimeoutMs 200

#pragma endregion

#pragma region "Enums"

typedef enum tagEPepCtrlPlugPlayDeviceInterfaceChangeType {
    PepCtrlPlugPlayDeviceInterfaceArrived,
    PepCtrlPlugPlayDeviceInterfaceRemoved
} EPepCtrlPlugPlayDeviceInterfaceChangeType;

#pragma endregion

#pragma region "Structures"

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

typedef struct tagTPepCtrlPlugPlayData
{
    PDRIVER_OBJECT pDriverObject;
    PDEVICE_OBJECT pDeviceObject;
    PVOID pvPnPNotificationEntry;
    SLIST_HEADER DeviceInterfaceChangeList;
    HANDLE hDeviceInterfaceChangeThread;
    KEVENT QuitDeviceInterfaceChangeThreadEvent;
    PCWSTR pszDeviceName;
    PWSTR pszSymbolicLinkName;
    TPepCtrlPlugPlayDeviceArrivedFunc pDeviceArrivedFunc;
    TPepCtrlPlugPlayDeviceRemovedFunc pDeviceRemovedFunc;
} TPepCtrlPlugPlayData;

typedef struct tagTPepCtrlPlugPlayDeviceInterfaceChangeEntry
{
    SLIST_ENTRY ListEntry;
    EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType;
    UNICODE_STRING SymbolicLinkName;
    WCHAR cSymbolicLinkName[1];
} TPepCtrlPlugPlayDeviceInterfaceChangeEntry;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lDoesSymbolicLinkNameMatchDeviceName(_In_ PUNICODE_STRING pSymbolicLinkName, _In_ PCWSTR pszDeviceName);

_IRQL_requires_max_(PASSIVE_LEVEL)
static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lAllocDeviceInterfaceChangeEntry(_In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType, _In_ PUNICODE_STRING pSymbolicLinkName);

_IRQL_requires_max_(PASSIVE_LEVEL)
static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lPopDeviceInterfaceChangeEntry(_In_ TPepCtrlPlugPlayData* pPlugPlayData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lPushDeviceInterfaceChangeEntry(_In_ TPepCtrlPlugPlayData* pPlugPlayData, _In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType, _In_ PUNICODE_STRING pSymbolicLinkName);

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID lEmptyDeviceInterfaceChangeList(_In_ TPepCtrlPlugPlayData* pPlugPlayData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID lProcessDeviceInterfaceChangeEntry(_In_ TPepCtrlPlugPlayData* pPlugPlayData, _In_ TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry);

_IRQL_requires_same_
_Function_class_(lDeviceInterfaceChangeThreadStart)
static VOID lDeviceInterfaceChangeThreadStart(_In_ PVOID pvStartContext);

_Function_class_(lDeviceInterfaceChange)
_IRQL_requires_max_(PASSIVE_LEVEL)
static NTSTATUS lDeviceInterfaceChange(_In_ PVOID pvNotificationStructure, _Inout_opt_ PVOID pvContext);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lDoesSymbolicLinkNameMatchDeviceName)
#pragma alloc_text (PAGE, lAllocDeviceInterfaceChangeEntry)
#pragma alloc_text (PAGE, lPopDeviceInterfaceChangeEntry)
#pragma alloc_text (PAGE, lPushDeviceInterfaceChangeEntry)
#pragma alloc_text (PAGE, lEmptyDeviceInterfaceChangeList)
#pragma alloc_text (PAGE, lProcessDeviceInterfaceChangeEntry)
#pragma alloc_text (PAGE, lDeviceInterfaceChangeThreadStart)
#pragma alloc_text (PAGE, lDeviceInterfaceChange)
#pragma alloc_text (PAGE, PepCtrlPlugPlayAlloc)
#pragma alloc_text (PAGE, PepCtrlPlugPlayFree)
#pragma alloc_text (PAGE, PepCtrlPlugPlayRegister)
#pragma alloc_text (PAGE, PepCtrlPlugPlayUnregister)
#pragma alloc_text (PAGE, PepCtrlPlugPlayIsDevicePresent)
#endif

#pragma region "Local Functions"

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lDoesSymbolicLinkNameMatchDeviceName(
  _In_ PUNICODE_STRING pSymbolicLinkName,
  _In_ PCWSTR pszDeviceName)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS Status;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;
    POBJECT_NAME_INFORMATION pObjectNameInformation;
    ULONG ulNameLen;
    UNICODE_STRING DeviceName;

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Calling IoGetDeviceObjectPointer.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    Status = IoGetDeviceObjectPointer(pSymbolicLinkName, FILE_ALL_ACCESS, &pFileObject, &pDeviceObject);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Finished calling IoGetDeviceObjectPointer (Status: 0x%X).  (Thread: 0x%p)\n",
		       Status, PsGetCurrentThread());

    if (Status != STATUS_SUCCESS)
    {
        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving (Call to IoGetDeviceObjectPointer failed).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Calling ObQueryNameString to get the size of the device name.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    Status = ObQueryNameString(pFileObject, NULL, 0, &ulNameLen);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Finished calling ObQueryNameString to get the size of the device name (Status: 0x%X).  (Thread: 0x%p)\n",
               Status, PsGetCurrentThread());

    if (Status == STATUS_NOT_SUPPORTED)
    {
        // When querying the symbolic link for a parallel port this will will always fail.

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Device does not support the querying of it's name.  Assuming the device is a parallel port.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Dereferencing the file object.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        ObDereferenceObject(pFileObject);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return TRUE;
    }

    if (Status != STATUS_INFO_LENGTH_MISMATCH)
    {
        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Dereferencing the file object.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        ObDereferenceObject(pFileObject);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving  (Call to ObQueryNameString to get the size of the device name failed).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Allocating memory for the device name.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pObjectNameInformation = (POBJECT_NAME_INFORMATION)UtAllocPagedMem(ulNameLen);

    if (pObjectNameInformation == NULL)
    {
        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Dereferencing the file object.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        ObDereferenceObject(pFileObject);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving (Call to allocate memory for the device name failed).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Calling ObQueryNameString to get the device name.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    Status = ObQueryNameString(pFileObject, pObjectNameInformation, ulNameLen, &ulNameLen);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Finished calling ObQueryNameString to get the device name (Status: 0x%X).  (Thread: 0x%p)\n",
		       Status, PsGetCurrentThread());

    if (Status != STATUS_SUCCESS)
    {
        UtFreePagedMem(pObjectNameInformation);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Dereferencing the file object.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        ObDereferenceObject(pFileObject);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving.  (Call to ObQueryNameString to get the device name failed).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Symbolic link name resolved to the device name \"%ws\".  (Thread: 0x%p)\n",
		       pObjectNameInformation->Name.Buffer, PsGetCurrentThread());

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Checking if the device name is \"%ws\".  (Thread: 0x%p)\n",
		       pszDeviceName, PsGetCurrentThread());

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    bResult = RtlEqualUnicodeString(&pObjectNameInformation->Name, &DeviceName, FALSE);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Dereferencing the file object.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    ObDereferenceObject(pFileObject);

    UtFreePagedMem(pObjectNameInformation);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lAllocDeviceInterfaceChangeEntry(
  _In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType,
  _In_ PUNICODE_STRING pSymbolicLinkName)
{
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;
    SIZE_T DeviceInterfaceChangeEntryLen;

    PepCtrlLog("lAllocDeviceInterfaceChangeEntry entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    DeviceInterfaceChangeEntryLen = sizeof(TPepCtrlPlugPlayDeviceInterfaceChangeEntry) + pSymbolicLinkName->Length;

    PepCtrlLog("lAllocDeviceInterfaceChangeEntry - Allocating memory for the entry (Allocating %d bytes).  (Thread: 0x%p)\n",
               (ULONG)DeviceInterfaceChangeEntryLen, PsGetCurrentThread());

    pDeviceInterfaceChangeEntry = (TPepCtrlPlugPlayDeviceInterfaceChangeEntry*)UtAllocPagedMem(DeviceInterfaceChangeEntryLen);

    if (pDeviceInterfaceChangeEntry)
    {
        PepCtrlLog("lAllocDeviceInterfaceChangeEntry - Successfully allocated memory for the device interface change entry pointer: 0x%p.  (Thread: 0x%p)\n",
                   pDeviceInterfaceChangeEntry, PsGetCurrentThread());

        RtlZeroMemory(pDeviceInterfaceChangeEntry, DeviceInterfaceChangeEntryLen);

        pDeviceInterfaceChangeEntry->ChangeType = ChangeType;
        pDeviceInterfaceChangeEntry->SymbolicLinkName.Buffer = pDeviceInterfaceChangeEntry->cSymbolicLinkName;
        pDeviceInterfaceChangeEntry->SymbolicLinkName.Length = pSymbolicLinkName->Length;
        pDeviceInterfaceChangeEntry->SymbolicLinkName.MaximumLength = pSymbolicLinkName->Length + sizeof(WCHAR);

        RtlCopyMemory(pDeviceInterfaceChangeEntry->cSymbolicLinkName, pSymbolicLinkName->Buffer,
                      pSymbolicLinkName->Length);
    }
    else
    {
        PepCtrlLog("lAllocDeviceInterfaceChangeEntry - Failed to allocate memory for the device interface change entry.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("lAllocDeviceInterfaceChangeEntry leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return pDeviceInterfaceChangeEntry;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID lEmptyDeviceInterfaceChangeList(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData)
{
    PSLIST_ENTRY pEntry;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;

    PepCtrlLog("lEmptyDeviceInterfaceChangeList entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lEmptyDeviceInterfaceChangeList - Flushing the list.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pEntry = ExInterlockedFlushSList(&pPlugPlayData->DeviceInterfaceChangeList);

    while (pEntry)
    {
        PepCtrlLog("lEmptyDeviceInterfaceChangeList - Entry found.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        pDeviceInterfaceChangeEntry = CONTAINING_RECORD(pEntry, 
                                                        TPepCtrlPlugPlayDeviceInterfaceChangeEntry,
                                                        ListEntry);

        pEntry = pEntry->Next;

        PepCtrlLog("lEmptyDeviceInterfaceChangeList - Attempting to free the memory for the device interface change entry pointer: 0x%p.  (Thread: 0x%p)\n",
                   pDeviceInterfaceChangeEntry, PsGetCurrentThread());

        UtFreePagedMem(pDeviceInterfaceChangeEntry);

        PepCtrlLog("lEmptyDeviceInterfaceChangeList - Memory for the entry was freed.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("lEmptyDeviceInterfaceChangeList leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lPopDeviceInterfaceChangeEntry(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData)
{
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry = NULL;
    PSLIST_ENTRY pEntry;

#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
    PepCtrlLog("lPopDeviceInterfaceChangeEntry entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
#endif

    PAGED_CODE()

#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
    PepCtrlLog("lPopDeviceInterfaceChangeEntry - Attempting to pop an entry from the list.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
#endif

    pEntry = InterlockedPopEntrySList(&pPlugPlayData->DeviceInterfaceChangeList);

    if (pEntry)
    {
#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
        PepCtrlLog("lPopDeviceInterfaceChangeEntry - Entry found.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
#endif

        pDeviceInterfaceChangeEntry = CONTAINING_RECORD(pEntry, TPepCtrlPlugPlayDeviceInterfaceChangeEntry, ListEntry);

#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
        PepCtrlLog("lPopDeviceInterfaceChangeEntry - Device Interface Change Entry pointer: 0x%p.  (Thread: 0x%p)\n",
                   pDeviceInterfaceChangeEntry, PsGetCurrentThread());
#endif
    }
    else
    {
#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
        PepCtrlLog("lPopDeviceInterfaceChangeEntry - No entry found.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
#endif
    }

#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
    PepCtrlLog("lPopDeviceInterfaceChangeEntry leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
#endif

    return pDeviceInterfaceChangeEntry;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lPushDeviceInterfaceChangeEntry(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData,
  _In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType,
  _In_ PUNICODE_STRING pSymbolicLinkName)
{
    BOOLEAN bResult = FALSE;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;

    PepCtrlLog("lPushDeviceInterfaceChangeEntry entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lPushDeviceInterfaceChangeEntry - Allocating a device interface change entry.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pDeviceInterfaceChangeEntry = lAllocDeviceInterfaceChangeEntry(ChangeType, pSymbolicLinkName);

    if (pDeviceInterfaceChangeEntry)
    {
        PepCtrlLog("lPushDeviceInterfaceChangeEntry - Device interface change entry allocated  (Entry Pointer: 0x%p).  (Thread: 0x%p)\n",
                   pDeviceInterfaceChangeEntry, PsGetCurrentThread());

        InterlockedPushEntrySList(&pPlugPlayData->DeviceInterfaceChangeList,
                                  &pDeviceInterfaceChangeEntry->ListEntry);

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lPushDeviceInterfaceChangeEntry - Failed to allocate memory for the entry.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("lPushDeviceInterfaceChangeEntry leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID lProcessDeviceInterfaceChangeEntry(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData,
  _In_ TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry)
{
    UNICODE_STRING SymbolicLinkName;
    SIZE_T SymbolicLinkNameLen;

    PepCtrlLog("lProcessDeviceInterfaceChangeEntry entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    if (pDeviceInterfaceChangeEntry->ChangeType == PepCtrlPlugPlayDeviceInterfaceArrived)
    {
        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Processing device interface arrived.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        if (pPlugPlayData->pszSymbolicLinkName == NULL)
        {
            PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device arrived checking if matches requested device.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            if (lDoesSymbolicLinkNameMatchDeviceName(&pDeviceInterfaceChangeEntry->SymbolicLinkName,
                                                     pPlugPlayData->pszDeviceName))
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device arrived matches the requested device.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

                SymbolicLinkNameLen = pDeviceInterfaceChangeEntry->SymbolicLinkName.Length + sizeof(WCHAR);

                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Allocating memory for the symbolic link name.  (Allocating %d bytes)  (Thread: 0x%p)\n",
                           (ULONG)SymbolicLinkNameLen, PsGetCurrentThread());

                pPlugPlayData->pszSymbolicLinkName = (PWSTR)UtAllocPagedMem(SymbolicLinkNameLen);

                if (pPlugPlayData->pszSymbolicLinkName)
                {
                    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Memory allocated for the symbolic link name.  (Thread: 0x%p)\n",
						       PsGetCurrentThread());

                    RtlZeroMemory(pPlugPlayData->pszSymbolicLinkName, SymbolicLinkNameLen);

                    RtlCopyMemory(pPlugPlayData->pszSymbolicLinkName, pDeviceInterfaceChangeEntry->SymbolicLinkName.Buffer,
                                  pDeviceInterfaceChangeEntry->SymbolicLinkName.Length);

                    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Attempting to allocate the device.  (Thread: 0x%p)\n",
						       PsGetCurrentThread());

                    if (pPlugPlayData->pDeviceArrivedFunc(pPlugPlayData->pDeviceObject))
                    {
                        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device was successfully allocated.  (Thread: 0x%p)\n",
							       PsGetCurrentThread());
                    }
                    else
                    {
                        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device could not be allocated.  (Thread: 0x%p)\n",
							       PsGetCurrentThread());

                        UtFreePagedMem(pPlugPlayData->pszSymbolicLinkName);

                        pPlugPlayData->pszSymbolicLinkName = NULL;
                    }
                }
                else
                {
                    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Failed to allocate memory for the symbolic link name.  (Thread: 0x%p)\n",
						       PsGetCurrentThread());
                }
            }
            else
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device arrived does not match the requested device.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
        }
        else
        {
            PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Already have the device ignoring device interface arrived.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
    }
    else if (pDeviceInterfaceChangeEntry->ChangeType == PepCtrlPlugPlayDeviceInterfaceRemoved)
    {
        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Processing device interface removed.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
        
        if (pPlugPlayData->pszSymbolicLinkName != NULL)
        {
            RtlInitUnicodeString(&SymbolicLinkName, pPlugPlayData->pszSymbolicLinkName);

            if (RtlEqualUnicodeString(&SymbolicLinkName, &pDeviceInterfaceChangeEntry->SymbolicLinkName, TRUE))
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device removed matches the existing device.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

                UtFreePagedMem(pPlugPlayData->pszSymbolicLinkName);
                
                pPlugPlayData->pszSymbolicLinkName = NULL;

                pPlugPlayData->pDeviceRemovedFunc(pPlugPlayData->pDeviceObject);
            }
            else
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device removed does not match the existing device.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
        }
        else
        {
            PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Have no device ignoring device interface removed.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
    }
    else
    {
        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Unknown change type received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Attempting to free the memory for the device interface change entry. (Entry Pointer: 0x%p)  (Thread: 0x%p)\n",
               pDeviceInterfaceChangeEntry, PsGetCurrentThread());

    UtFreePagedMem(pDeviceInterfaceChangeEntry);

    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Memory freed for the device interface change entry.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PepCtrlLog("lProcessDeviceInterfaceChangeEntry leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
}

_IRQL_requires_same_
_Function_class_(lDeviceInterfaceChangeThreadStart)
static VOID lDeviceInterfaceChangeThreadStart(
  _In_ PVOID pvStartContext)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvStartContext;
    BOOLEAN bQuit = FALSE;
    LARGE_INTEGER Timeout;
    NTSTATUS Status;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;

    PepCtrlLog("lDeviceInterfaceChangeThreadStart entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lDeviceInterfaceChangeThreadStart - Plug 'n Play Data Pointer: 0x%p.  (Thread: 0x%p)\n",
               pPlugPlayData, PsGetCurrentThread());

    Timeout.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

    while (!bQuit)
    {
        Status = KeWaitForSingleObject(&pPlugPlayData->QuitDeviceInterfaceChangeThreadEvent,
                                       Executive, KernelMode, FALSE, &Timeout);

        if (Status == STATUS_SUCCESS)
        {
            PepCtrlLog("lDeviceInterfaceChangeThreadStart - Thread told to quit.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            bQuit = TRUE;
        }
        else if (Status == STATUS_TIMEOUT)
        {
            pDeviceInterfaceChangeEntry = lPopDeviceInterfaceChangeEntry(pPlugPlayData);

            if (pDeviceInterfaceChangeEntry)
            {
                PepCtrlLog("lDeviceInterfaceChangeThreadStart - Device Interface Change Entry found (Entry Pointer: 0x%p).  (Thread: 0x%p)\n",
                           pDeviceInterfaceChangeEntry, PsGetCurrentThread());

                lProcessDeviceInterfaceChangeEntry(pPlugPlayData, pDeviceInterfaceChangeEntry);
            }
        }
        else
        {
            PepCtrlLog("lDeviceInterfaceChangeThreadStart - Unknown Status: 0x%X.  (Thread: 0x%p)\n",
				       Status, PsGetCurrentThread());
        }
    }

    PepCtrlLog("lDeviceInterfaceChangeThreadStart leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
}

_Function_class_(lDeviceInterfaceChange)
_IRQL_requires_max_(PASSIVE_LEVEL)
static NTSTATUS lDeviceInterfaceChange(
  _In_ PVOID pvNotificationStructure,
  _Inout_opt_ PVOID pvContext)
{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION pNotification;
    TPepCtrlPlugPlayData* pPlugPlayData;

    PepCtrlLog("lDeviceInterfaceChange entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    pNotification = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION)pvNotificationStructure;
    pPlugPlayData = (TPepCtrlPlugPlayData*)pvContext;

    PepCtrlLog("lDeviceInterfaceChange - Plug 'n Play Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPlugPlayData, PsGetCurrentThread());

    if (pNotification->Version != 1)
    {
        PepCtrlLog("lDeviceInterfaceChange leaving - Received unknown notification version of %d.  (Thread: 0x%p)\n",
                   (ULONG)pNotification->Version, PsGetCurrentThread());

        return STATUS_SUCCESS;
    }

    PepCtrlLog("lDeviceInterfaceChange - Symbolic link name \"%ws\".  (Thread: 0x%p)\n",
               pNotification->SymbolicLinkName->Buffer, PsGetCurrentThread());

    if (IsEqualGUID((LPGUID)&pNotification->Event, (LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL))
    {
        PepCtrlLog("lDeviceInterfaceChange - GUID_DEVICE_INTERFACE_ARRIVAL event GUID received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        if (lPushDeviceInterfaceChangeEntry(pPlugPlayData, PepCtrlPlugPlayDeviceInterfaceArrived,
                                            pNotification->SymbolicLinkName))
        {
            PepCtrlLog("lDeviceInterfaceChange - Adding new device interface change entry.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else
        {
            PepCtrlLog("lDeviceInterfaceChange - Failed to add new device interface change entry.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
    }
    else if (IsEqualGUID((LPGUID)&pNotification->Event, (LPGUID)&GUID_DEVICE_INTERFACE_REMOVAL))
    {
        PepCtrlLog("lDeviceInterfaceChange - GUID_DEVICE_INTERFACE_REMOVAL event GUID received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        if (lPushDeviceInterfaceChangeEntry(pPlugPlayData, PepCtrlPlugPlayDeviceInterfaceRemoved,
                                            pNotification->SymbolicLinkName))
        {
            PepCtrlLog("lDeviceInterfaceChange - Adding new device interface change entry.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else
        {
            PepCtrlLog("lDeviceInterfaceChange - Failed to add new device interface change entry.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
    }
    else
    {
        PepCtrlLog("lDeviceInterfaceChange - Unknown Notification Event GUID received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("lDeviceInterfaceChange leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return STATUS_SUCCESS;
}

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
PVOID PepCtrlPlugPlayAlloc(
  _In_ PDRIVER_OBJECT pDriverObject,
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ TPepCtrlPlugPlayDeviceArrivedFunc pDeviceArrivedFunc,
  _In_ TPepCtrlPlugPlayDeviceRemovedFunc pDeviceRemovedFunc)
{
    SIZE_T PlugPlayDataLen;
    TPepCtrlPlugPlayData* pPlugPlayData;

    PepCtrlLog("PepCtrlPlugPlayAlloc entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PlugPlayDataLen = sizeof(TPepCtrlPlugPlayData);

    PepCtrlLog("PepCtrlPlugPlayAlloc - Attempting to allocate memory for the Plug 'n Play Data.  (Allocating %d bytes)  (Thread: 0x%p)\n",
               (ULONG)PlugPlayDataLen, PsGetCurrentThread());

    pPlugPlayData = (TPepCtrlPlugPlayData*)UtAllocNonPagedMem(PlugPlayDataLen);

    if (pPlugPlayData)
    {
        PepCtrlLog("PepCtrlPlugPlayAlloc - Memory allocated for the Plug 'n Play Data.  (Data Pointer: 0x%p)  (Thread: 0x%p)\n",
                   pPlugPlayData, PsGetCurrentThread());

        RtlZeroMemory(pPlugPlayData, PlugPlayDataLen);

        pPlugPlayData->pDriverObject = pDriverObject;
        pPlugPlayData->pDeviceObject = pDeviceObject;
        pPlugPlayData->pDeviceArrivedFunc = pDeviceArrivedFunc;
        pPlugPlayData->pDeviceRemovedFunc = pDeviceRemovedFunc;

        InitializeSListHead(&pPlugPlayData->DeviceInterfaceChangeList);

        KeInitializeEvent(&pPlugPlayData->QuitDeviceInterfaceChangeThreadEvent, NotificationEvent, FALSE);
    }
    else
    {
        PepCtrlLog("PepCtrlPlugPlayAlloc - Failed to allocate memory for the Plug 'n Play Data.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("PepCtrlPlugPlayAlloc leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return pPlugPlayData;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlPlugPlayFree(
  PVOID pvData)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;

    PepCtrlLog("PepCtrlPlugPlayFree entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlPlugPlayFree - Freeing the memory for the Plug 'n Play Data  (Pointer: 0x%p).  (Thread: 0x%p)\n",
               pPlugPlayData, PsGetCurrentThread());

    UtFreeNonPagedMem(pPlugPlayData);

    PepCtrlLog("PepCtrlPlugPlayFree - Memory freed for the Plug 'n Play Data.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PepCtrlLog("PepCtrlPlugPlayFree leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlPlugPlayRegister(
  _In_ LPGUID pGuid,
  _In_ PCWSTR pszDeviceName,
  _In_ PVOID pvData)
{
    BOOLEAN bResult = FALSE;
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;
    NTSTATUS Status;

    PepCtrlLog("PepCtrlPlugPlayRegister entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlPlugPlayRegister - Plug 'n Play Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPlugPlayData, PsGetCurrentThread());

	PepCtrlLog("PepCtrlPlugPlayRegister - Clearing the quit device interface change thread event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	KeClearEvent(&pPlugPlayData->QuitDeviceInterfaceChangeThreadEvent);

    PepCtrlLog("PepCtrlPlugPlayRegister - Creating the device interface change thread.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (FALSE == PepCtrlThreadStart(lDeviceInterfaceChangeThreadStart, pPlugPlayData,
                                    &pPlugPlayData->hDeviceInterfaceChangeThread))
    {
        PepCtrlLog("PepCtrlPlugPlayRegister leaving.  (Failed to create the device interface change thread)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("PepCtrlPlugPlayRegister - Device interface change thread successfully created.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pPlugPlayData->pszDeviceName = pszDeviceName;

    PepCtrlLog("PepCtrlPlugPlayRegister - Registering for Plug and Play Notification.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    Status = IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange,
                                            PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                                            pGuid,
                                            pPlugPlayData->pDriverObject,
                                            lDeviceInterfaceChange,
                                            pPlugPlayData,
                                            &pPlugPlayData->pvPnPNotificationEntry);

    PepCtrlLog("PepCtrlPlugPlayRegister - Registered Plug and Play Notification (Status: 0x%X).  (Thread: 0x%p)\n",
		       Status, PsGetCurrentThread());

    if (NT_SUCCESS(Status))
    {
        PepCtrlLog("PepCtrlPlugPlayRegister - Successfully registered for Plug and Play notification.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("PepCtrlPlugPlayRegister - Failed to register for Plug and Play notification.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        PepCtrlLog("PepCtrlPlugPlayRegister - Stopping the device interface change thread.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        if (PepCtrlThreadStop(pPlugPlayData->hDeviceInterfaceChangeThread))
        {
            PepCtrlLog("PepCtrlPlugPlayRegister - Successfully stopped the device interface change thread.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else
        {
            PepCtrlLog("PepCtrlPlugPlayRegister - Failed to stop the device interface change thread.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }

        pPlugPlayData->hDeviceInterfaceChangeThread = NULL;
    }
        
    PepCtrlLog("PepCtrlPlugPlayRegister leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlPlugPlayUnregister(
  _In_ PVOID pvData)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;
    NTSTATUS Status;
    UNICODE_STRING SymbolicLinkName;
 
    PepCtrlLog("PepCtrlPlugPlayUnregister entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlPlugPlayUnregister - Plug 'n Play Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPlugPlayData, PsGetCurrentThread());

    PepCtrlLog("PepCtrlPlugPlayUnregister - Setting the quit device interface change thread event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    KeSetEvent(&pPlugPlayData->QuitDeviceInterfaceChangeThreadEvent, 0, FALSE);

    PepCtrlLog("PepCtrlPlugPlayUnregister - Stopping the device interface change thread.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (PepCtrlThreadStop(pPlugPlayData->hDeviceInterfaceChangeThread))
    {
        PepCtrlLog("PepCtrlPlugPlayUnregister - Successfully stopped the device interface change thread.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }
    else
    {
        PepCtrlLog("PepCtrlPlugPlayUnregister - Failed to stop the device interface change thread.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    pPlugPlayData->hDeviceInterfaceChangeThread = NULL;

    PepCtrlLog("PepCtrlPlugPlayUnregister - Unregistering Plug and Play Notification.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    Status = IoUnregisterPlugPlayNotification(pPlugPlayData->pvPnPNotificationEntry);

    PepCtrlLog("PepCtrlPlugPlayUnregister - Unregistered Plug and Play Notification  (Error Code: 0x%X).  (Thread: 0x%p)\n",
               Status, PsGetCurrentThread());

    PepCtrlLog("PepCtrlPlugPlayUnregister - Emptying the device interface change list.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    lEmptyDeviceInterfaceChangeList(pPlugPlayData);

    if (pPlugPlayData->pszSymbolicLinkName)
    {
        PepCtrlLog("PepCtrlPlugPlayUnregister - Device present simulating a device interface removed change event.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        RtlInitUnicodeString(&SymbolicLinkName, pPlugPlayData->pszSymbolicLinkName);

        pDeviceInterfaceChangeEntry = lAllocDeviceInterfaceChangeEntry(PepCtrlPlugPlayDeviceInterfaceRemoved,
                                                                       &SymbolicLinkName);

        if (pDeviceInterfaceChangeEntry)
        {
            PepCtrlLog("PepCtrlPlugPlayUnregister - Processing the simulated device interface removed change event.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            lProcessDeviceInterfaceChangeEntry(pPlugPlayData, pDeviceInterfaceChangeEntry);
        }
        else
        {
            PepCtrlLog("PepCtrlPlugPlayUnregister - Failed to simulate a device interface removed change event.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
    }

    PepCtrlLog("PepCtrlPlugPlayUnregister leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return TRUE;
}

_IRQL_requires_max_(APC_LEVEL)
BOOLEAN PepCtrlPlugPlayIsDevicePresent(
  _In_ PVOID pvData)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;
    BOOLEAN bDevicePresent;

    PepCtrlLog("PepCtrlPlugPlayIsDevicePresent entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlPlugPlayIsDevicePresent - Plug 'n Play Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPlugPlayData, PsGetCurrentThread());

    bDevicePresent = (pPlugPlayData->pszSymbolicLinkName != NULL) ? TRUE : FALSE;

    PepCtrlLog("PepCtrlPlugPlayIsDevicePresent leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bDevicePresent;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
