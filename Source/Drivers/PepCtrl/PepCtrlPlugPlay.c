/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

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

static BOOLEAN lDoesSymbolicLinkNameMatchDeviceName(_In_ PUNICODE_STRING pSymbolicLinkName, _In_ PCWSTR pszDeviceName);
static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lAllocDeviceInterfaceChangeEntry(_In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType, _In_ PUNICODE_STRING pSymbolicLinkName);
static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lPopDeviceInterfaceChangeEntry(_In_ TPepCtrlPlugPlayData* pPlugPlayData);
static BOOLEAN lPushDeviceInterfaceChangeEntry(_In_ TPepCtrlPlugPlayData* pPlugPlayData, _In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType, _In_ PUNICODE_STRING pSymbolicLinkName);
static VOID lEmptyDeviceInterfaceChangeList(_In_ TPepCtrlPlugPlayData* pPlugPlayData);
static VOID lProcessDeviceInterfaceChangeEntry(_In_ TPepCtrlPlugPlayData* pPlugPlayData, _In_ TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry);
static KSTART_ROUTINE lDeviceInterfaceChangeThreadStart;
static DRIVER_NOTIFICATION_CALLBACK_ROUTINE lDeviceInterfaceChange;

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

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName entering.\n");

    PAGED_CODE()

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Calling IoGetDeviceObjectPointer.\n");

    Status = IoGetDeviceObjectPointer(pSymbolicLinkName, FILE_ALL_ACCESS, &pFileObject, &pDeviceObject);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Finished calling IoGetDeviceObjectPointer (Status: 0x%X)\n", Status);

    if (Status != STATUS_SUCCESS)
    {
        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving (Call to IoGetDeviceObjectPointer failed).\n");

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Calling ObQueryNameString to get the size of the device name.\n");

    Status = ObQueryNameString(pFileObject, NULL, 0, &ulNameLen);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Finished calling ObQueryNameString to get the size of the device name (Status: 0x%X)\n", Status);

    if (Status != STATUS_INFO_LENGTH_MISMATCH)
    {
        ObDereferenceObject(pFileObject);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving (Call to ObQueryNameString to get the size of the device name failed).\n");

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Allocating memory for the device name.\n");

    pObjectNameInformation = (POBJECT_NAME_INFORMATION)UtAllocPagedMem(ulNameLen);

    if (pObjectNameInformation == NULL)
    {
        ObDereferenceObject(pFileObject);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving (Call to allocate memory for the device name failed).\n");

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Calling ObQueryNameString to get the device name.\n");

    Status = ObQueryNameString(pFileObject, pObjectNameInformation, ulNameLen, &ulNameLen);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Finished calling ObQueryNameString to get the device name (Status: 0x%X)\n", Status);

    if (Status != STATUS_SUCCESS)
    {
        UtFreePagedMem(pObjectNameInformation);

        ObDereferenceObject(pFileObject);

        PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving (Call to ObQueryNameString to get the device name failed).\n");

        return FALSE;
    }

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Symbolic link name resolved to the device name \"%ws\".\n", pObjectNameInformation->Name.Buffer);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName - Checking if the device name is \"%ws\".\n", pszDeviceName);

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    bResult = RtlEqualUnicodeString(&pObjectNameInformation->Name, &DeviceName, FALSE);

    ObDereferenceObject(pFileObject);

    UtFreePagedMem(pObjectNameInformation);

    PepCtrlLog("lDoesSymbolicLinkNameMatchDeviceName leaving.\n");

    return bResult;
}

static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lAllocDeviceInterfaceChangeEntry(
  _In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType,
  _In_ PUNICODE_STRING pSymbolicLinkName)
{
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;

    PepCtrlLog("lAllocDeviceInterfaceChangeEntry entering.\n");

    PAGED_CODE()

    pDeviceInterfaceChangeEntry = (TPepCtrlPlugPlayDeviceInterfaceChangeEntry*)UtAllocPagedMem(sizeof(TPepCtrlPlugPlayDeviceInterfaceChangeEntry) +
                                                                                                   (pSymbolicLinkName->Length * sizeof(WCHAR)));

    if (pDeviceInterfaceChangeEntry)
    {
        PepCtrlLog("lAllocDeviceInterfaceChangeEntry - Successfully allocated memory for the device interface change entry.\n");

        pDeviceInterfaceChangeEntry->ChangeType = ChangeType;
        pDeviceInterfaceChangeEntry->SymbolicLinkName.Buffer = pDeviceInterfaceChangeEntry->cSymbolicLinkName;
        pDeviceInterfaceChangeEntry->SymbolicLinkName.Length = pSymbolicLinkName->Length;
        pDeviceInterfaceChangeEntry->SymbolicLinkName.MaximumLength = pSymbolicLinkName->Length + 1;

        RtlStringCchCopyW(pDeviceInterfaceChangeEntry->cSymbolicLinkName, pSymbolicLinkName->Length + 1,
                          pSymbolicLinkName->Buffer);
    }
    else
    {
        PepCtrlLog("lAllocDeviceInterfaceChangeEntry - Failed to allocate memory for the device interface change entry.\n");
    }

    PepCtrlLog("lAllocDeviceInterfaceChangeEntry leaving.\n");

    return pDeviceInterfaceChangeEntry;
}

static VOID lEmptyDeviceInterfaceChangeList(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData)
{
    PSLIST_ENTRY pEntry;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;

    PepCtrlLog("lEmptyDeviceInterfaceChangeList entering.\n");

    PAGED_CODE()

    PepCtrlLog("lEmptyDeviceInterfaceChangeList - Flushing the list.\n");

    pEntry = ExInterlockedFlushSList(&pPlugPlayData->DeviceInterfaceChangeList);

    while (pEntry)
    {
        PepCtrlLog("lEmptyDeviceInterfaceChangeList - Entry found.\n");

        pDeviceInterfaceChangeEntry = CONTAINING_RECORD(pEntry, 
                                                        TPepCtrlPlugPlayDeviceInterfaceChangeEntry,
                                                        ListEntry);

        pEntry = pEntry->Next;

        UtFreePagedMem(pDeviceInterfaceChangeEntry);
    }

    PepCtrlLog("lEmptyDeviceInterfaceChangeList leaving.\n");
}

static TPepCtrlPlugPlayDeviceInterfaceChangeEntry* lPopDeviceInterfaceChangeEntry(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData)
{
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry = NULL;
    PSLIST_ENTRY pEntry;

#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
    PepCtrlLog("lPopDeviceInterfaceChangeEntry entering.\n");
#endif

    PAGED_CODE()

#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
    PepCtrlLog("lPopDeviceInterfaceChangeEntry - Attempting to pop an entry from the list.\n");
#endif

    pEntry = ExpInterlockedPopEntrySList(&pPlugPlayData->DeviceInterfaceChangeList);

    if (pEntry)
    {
#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
        PepCtrlLog("lPopDeviceInterfaceChangeEntry - Entry found.\n");
#endif

        pDeviceInterfaceChangeEntry = CONTAINING_RECORD(pEntry, TPepCtrlPlugPlayDeviceInterfaceChangeEntry, ListEntry);
    }
    else
    {
#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
        PepCtrlLog("lPopDeviceInterfaceChangeEntry - No entry found.\n");
#endif
    }

#if defined(PEP_CTRL_LOG_ALL_MESSAGES)
    PepCtrlLog("lPopDeviceInterfaceChangeEntry leaving.\n");
#endif

    return pDeviceInterfaceChangeEntry;
}

static BOOLEAN lPushDeviceInterfaceChangeEntry(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData,
  _In_ EPepCtrlPlugPlayDeviceInterfaceChangeType ChangeType,
  _In_ PUNICODE_STRING pSymbolicLinkName)
{
    BOOLEAN bResult = FALSE;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;

    PepCtrlLog("lPushDeviceInterfaceChangeEntry entering.\n");

    PAGED_CODE()

    PepCtrlLog("lPushDeviceInterfaceChangeEntry - Allocating a device interface change entry.\n");

    pDeviceInterfaceChangeEntry = lAllocDeviceInterfaceChangeEntry(ChangeType, pSymbolicLinkName);

    if (pDeviceInterfaceChangeEntry)
    {
        PepCtrlLog("lPushDeviceInterfaceChangeEntry - Memory allocated for the entry.\n");

        ExpInterlockedPushEntrySList(&pPlugPlayData->DeviceInterfaceChangeList,
                                    &pDeviceInterfaceChangeEntry->ListEntry);

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lPushDeviceInterfaceChangeEntry - Failed to allocate memory for the entry.\n");
    }

    PepCtrlLog("lPushDeviceInterfaceChangeEntry leaving.\n");

    return bResult;
}

static VOID lProcessDeviceInterfaceChangeEntry(
  _In_ TPepCtrlPlugPlayData* pPlugPlayData,
  _In_ TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry)
{
    UNICODE_STRING SymbolicLinkName;

    PepCtrlLog("lProcessDeviceInterfaceChangeEntry entering.\n");

    PAGED_CODE()

    if (pDeviceInterfaceChangeEntry->ChangeType == PepCtrlPlugPlayDeviceInterfaceArrived)
    {
        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Processing device interface arrived.\n");

        if (pPlugPlayData->pszSymbolicLinkName == NULL)
        {
            PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device arrived checking if matches requested device.\n");

            if (lDoesSymbolicLinkNameMatchDeviceName(&pDeviceInterfaceChangeEntry->SymbolicLinkName,
                                                     pPlugPlayData->pszDeviceName))
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device arrived matches the requested device.\n");

                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Allocating memory for the symbolic link name.\n");

                pPlugPlayData->pszSymbolicLinkName = (PWSTR)UtAllocPagedMem((pDeviceInterfaceChangeEntry->SymbolicLinkName.Length + 1) * sizeof(WCHAR));

                if (pPlugPlayData->pszSymbolicLinkName)
                {
                    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Memory allocated for the symbolic link name.\n");

                    RtlStringCchCopyW(pPlugPlayData->pszSymbolicLinkName, pDeviceInterfaceChangeEntry->SymbolicLinkName.Length + 1,
                                      pDeviceInterfaceChangeEntry->SymbolicLinkName.Buffer);

                    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Attempting to allocate the device.\n");

                    if (pPlugPlayData->pDeviceArrivedFunc(pPlugPlayData->pDeviceObject))
                    {
                        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device was successfully allocated.\n");
                    }
                    else
                    {
                        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device could not be allocated.\n");

                        UtFreePagedMem(pPlugPlayData->pszSymbolicLinkName);

                        pPlugPlayData->pszSymbolicLinkName = NULL;
                    }
                }
                else
                {
                    PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Failed to allocate memory for the symbolic link name.\n");
                }
            }
            else
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device arrived does not match the requested device.\n");
            }
        }
        else
        {
            PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Already have the device ignoring device interface arrived.\n");
        }
    }
    else if (pDeviceInterfaceChangeEntry->ChangeType == PepCtrlPlugPlayDeviceInterfaceRemoved)
    {
        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Processing device interface removed.\n");
        
        if (pPlugPlayData->pszSymbolicLinkName != NULL)
        {
            RtlInitUnicodeString(&SymbolicLinkName, pPlugPlayData->pszSymbolicLinkName);

            if (RtlEqualUnicodeString(&SymbolicLinkName, &pDeviceInterfaceChangeEntry->SymbolicLinkName, TRUE))
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device removed matches the existing device.\n");

                UtFreePagedMem(pPlugPlayData->pszSymbolicLinkName);
                
                pPlugPlayData->pszSymbolicLinkName = NULL;

                pPlugPlayData->pDeviceRemovedFunc(pPlugPlayData->pDeviceObject);
            }
            else
            {
                PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Device removed does not match the existing device.\n");
            }
        }
        else
        {
            PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Have no device ignoring device interface removed.\n");
        }
    }
    else
    {
        PepCtrlLog("lProcessDeviceInterfaceChangeEntry - Unknown change type received.\n");
    }

    UtFreePagedMem(pDeviceInterfaceChangeEntry);

    PepCtrlLog("lProcessDeviceInterfaceChangeEntry leaving.\n");
}

static VOID lDeviceInterfaceChangeThreadStart(
  _In_ PVOID pvStartContext)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvStartContext;
    BOOLEAN bQuit = FALSE;
    LARGE_INTEGER Timeout;
    NTSTATUS Status;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;

    PepCtrlLog("lDeviceInterfaceChangeThreadStart entering.\n");

    PAGED_CODE()

    Timeout.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

    while (!bQuit)
    {
        Status = KeWaitForSingleObject(&pPlugPlayData->QuitDeviceInterfaceChangeThreadEvent,
                                       Executive, KernelMode, TRUE, &Timeout);

        if (Status == STATUS_SUCCESS)
        {
            PepCtrlLog("lDeviceInterfaceChangeThreadStart - quitting the thread.\n");

            bQuit = TRUE;
        }
        else if (Status == STATUS_TIMEOUT)
        {
            pDeviceInterfaceChangeEntry = lPopDeviceInterfaceChangeEntry(pPlugPlayData);

            if (pDeviceInterfaceChangeEntry)
            {
                PepCtrlLog("lDeviceInterfaceChangeThreadStart - Entry found\n");

                lProcessDeviceInterfaceChangeEntry(pPlugPlayData, pDeviceInterfaceChangeEntry);
            }
        }
        else
        {
            PepCtrlLog("lDeviceInterfaceChangeThreadStart - Unknown Status: 0x%X\n", Status);
        }
    }

    PepCtrlLog("lDeviceInterfaceChangeThreadStart leaving.\n");
}

static NTSTATUS lDeviceInterfaceChange(
  _In_ PVOID pvNotificationStructure,
  _In_ PVOID pvContext)
{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION pNotification;
    TPepCtrlPlugPlayData* pPlugPlayData;

    PepCtrlLog("lDeviceInterfaceChange entering.\n");

    PAGED_CODE()

    pNotification = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION)pvNotificationStructure;
    pPlugPlayData = (TPepCtrlPlugPlayData*)pvContext;

    if (pNotification->Version != 1)
    {
        PepCtrlLog("lDeviceInterfaceChange leaving - Received unknown notification version of %d.\n", (ULONG)pNotification->Version);

        return STATUS_SUCCESS;
    }

    PepCtrlLog("lDeviceInterfaceChange - Symbolic link name \"%ws\".\n", pNotification->SymbolicLinkName->Buffer);

    if (IsEqualGUID((LPGUID)&pNotification->Event, (LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL))
    {
        PepCtrlLog("lDeviceInterfaceChange - GUID_DEVICE_INTERFACE_ARRIVAL event GUID received.\n");

        if (lPushDeviceInterfaceChangeEntry(pPlugPlayData, PepCtrlPlugPlayDeviceInterfaceArrived,
                                            pNotification->SymbolicLinkName))
        {
            PepCtrlLog("lDeviceInterfaceChange - Adding new device interface change entry.\n");
        }
        else
        {
            PepCtrlLog("lDeviceInterfaceChange - Failed to add new device interface change entry.\n");
        }
    }
    else if (IsEqualGUID((LPGUID)&pNotification->Event, (LPGUID)&GUID_DEVICE_INTERFACE_REMOVAL))
    {
        PepCtrlLog("lDeviceInterfaceChange - GUID_DEVICE_INTERFACE_REMOVAL event GUID received.\n");

        if (lPushDeviceInterfaceChangeEntry(pPlugPlayData, PepCtrlPlugPlayDeviceInterfaceRemoved,
                                            pNotification->SymbolicLinkName))
        {
            PepCtrlLog("lDeviceInterfaceChange - Adding new device interface change entry.\n");
        }
        else
        {
            PepCtrlLog("lDeviceInterfaceChange - Failed to add new device interface change entry.\n");
        }
    }
    else
    {
        PepCtrlLog("lDeviceInterfaceChange - Unknown Notification Event GUID received.\n");
    }

    PepCtrlLog("lDeviceInterfaceChange leaving\n");

    return STATUS_SUCCESS;
}

#pragma endregion

PVOID PepCtrlPlugPlayAlloc(
  _In_ PDRIVER_OBJECT pDriverObject,
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ TPepCtrlPlugPlayDeviceArrivedFunc pDeviceArrivedFunc,
  _In_ TPepCtrlPlugPlayDeviceRemovedFunc pDeviceRemovedFunc)
{
    TPepCtrlPlugPlayData* pPlugPlayData;

    PepCtrlLog("PepCtrlPlugPlayAlloc entering.\n");

    PAGED_CODE()

    pPlugPlayData = (TPepCtrlPlugPlayData*)UtAllocPagedMem(sizeof(TPepCtrlPlugPlayData));

    RtlZeroMemory(pPlugPlayData, sizeof(TPepCtrlPlugPlayData));

    pPlugPlayData->pDriverObject = pDriverObject;
    pPlugPlayData->pDeviceObject = pDeviceObject;
    pPlugPlayData->pDeviceArrivedFunc = pDeviceArrivedFunc;
    pPlugPlayData->pDeviceRemovedFunc = pDeviceRemovedFunc;

    InitializeSListHead(&pPlugPlayData->DeviceInterfaceChangeList);

    KeInitializeEvent(&pPlugPlayData->QuitDeviceInterfaceChangeThreadEvent, NotificationEvent, FALSE);

    PepCtrlLog("PepCtrlPlugPlayAlloc leaving.\n");

    return pPlugPlayData;
}

VOID PepCtrlPlugPlayFree(
  PVOID pvData)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;

    PepCtrlLog("PepCtrlPlugPlayFree entering.\n");

    PAGED_CODE()

    UtFreePagedMem(pPlugPlayData);

    PepCtrlLog("PepCtrlPlugPlayFree leaving.\n");
}

BOOLEAN PepCtrlPlugPlayRegister(
  _In_ LPGUID pGuid,
  _In_ PCWSTR pszDeviceName,
  _In_ PVOID pvData)
{
    BOOLEAN bResult = FALSE;
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;
    NTSTATUS Status;

    PepCtrlLog("PepCtrlPlugPlayRegister entering.\n");

    PAGED_CODE()

    PepCtrlLog("PepCtrlPlugPlayRegister - Creating the device interface change thread.\n");

    if (FALSE == PepCtrlThreadStart(lDeviceInterfaceChangeThreadStart, pPlugPlayData,
                                    &pPlugPlayData->hDeviceInterfaceChangeThread))
    {
        PepCtrlLog("PepCtrlPlugPlayRegister leaving (Failed to create the device interface change thread.)\n");

        return FALSE;
    }

    pPlugPlayData->pszDeviceName = pszDeviceName;

    PepCtrlLog("PepCtrlPlugPlayRegister - Registering for Plug and Play Notification\n");

    Status = IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange,
                                            PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                                            pGuid,
                                            pPlugPlayData->pDriverObject,
                                            lDeviceInterfaceChange,
                                            pPlugPlayData,
                                            &pPlugPlayData->pvPnPNotificationEntry);

    PepCtrlLog("PepCtrlPlugPlayRegister - Registered Plug and Play Notification (Status: 0x%X)\n", Status);

    if (NT_SUCCESS(Status))
    {
        PepCtrlLog("PepCtrlPlugPlayRegister - Successfully registered for Plug and Play notification.\n");

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("PepCtrlPlugPlayRegister - Failed to register for Plug and Play notification.\n");

        PepCtrlLog("PepCtrlPlugPlayRegister - Stopping the device interface change thread.\n");

        if (PepCtrlThreadStop(pPlugPlayData->hDeviceInterfaceChangeThread))
        {
            PepCtrlLog("PepCtrlPlugPlayRegister - Successfully stopped the device interface change thread.\n");
        }
        else
        {
            PepCtrlLog("PepCtrlPlugPlayRegister - Failed to stop the device interface change thread.\n");
        }

        pPlugPlayData->hDeviceInterfaceChangeThread = NULL;
    }
        
    PepCtrlLog("PepCtrlPlugPlayRegister leaving.\n");

    return bResult;
}

BOOLEAN PepCtrlPlugPlayUnregister(
  _In_ PVOID pvData)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;
    TPepCtrlPlugPlayDeviceInterfaceChangeEntry* pDeviceInterfaceChangeEntry;
    NTSTATUS Status;
    UNICODE_STRING SymbolicLinkName;
 
    PepCtrlLog("PepCtrlPlugPlayUnregister entering.\n");

    PAGED_CODE()

    PepCtrlLog("PepCtrlPlugPlayUnregister - Setting the quit device interface change thread event.\n");

    KeSetEvent(&pPlugPlayData->QuitDeviceInterfaceChangeThreadEvent, 0, FALSE);

    PepCtrlLog("PepCtrlPlugPlayUnregister - Stopping the device interface change thread.\n");

    if (PepCtrlThreadStop(pPlugPlayData->hDeviceInterfaceChangeThread))
    {
        PepCtrlLog("PepCtrlPlugPlayUnregister - Successfully stopped the device interface change thread.\n");
    }
    else
    {
        PepCtrlLog("PepCtrlPlugPlayUnregister - Failed to stop the device interface change thread.\n");
    }

    pPlugPlayData->hDeviceInterfaceChangeThread = NULL;

    PepCtrlLog("PepCtrlPlugPlayUnregister - Unregistering Plug and Play Notification.\n");

    Status = IoUnregisterPlugPlayNotification(pPlugPlayData->pvPnPNotificationEntry);

    PepCtrlLog("PepCtrlPlugPlayUnregister - Unregistered Plug and Play Notification.  (Error Code: 0x%X)\n", Status);

    PepCtrlLog("PepCtrlPlugPlayUnregister - Emptying the device interface change list.\n");

    lEmptyDeviceInterfaceChangeList(pPlugPlayData);

    if (pPlugPlayData->pszSymbolicLinkName)
    {
        PepCtrlLog("PepCtrlPlugPlayUnregister - Device present simulating a device interface removed change event.\n");

        RtlInitUnicodeString(&SymbolicLinkName, pPlugPlayData->pszSymbolicLinkName);

        pDeviceInterfaceChangeEntry = lAllocDeviceInterfaceChangeEntry(PepCtrlPlugPlayDeviceInterfaceRemoved,
                                                                       &SymbolicLinkName);

        if (pDeviceInterfaceChangeEntry)
        {
            PepCtrlLog("PepCtrlPlugPlayUnregister - Processing the simulated device interface removed change event.\n");

            lProcessDeviceInterfaceChangeEntry(pPlugPlayData, pDeviceInterfaceChangeEntry);
        }
        else
        {
            PepCtrlLog("PepCtrlPlugPlayUnregister - Failed to simulate a device interface removed change event.\n");
        }
    }

    PepCtrlLog("PepCtrlPlugPlayUnregister leaving.\n");

    return TRUE;
}

BOOLEAN PepCtrlPlugPlayIsDevicePresent(
  _In_ PVOID pvData)
{
    TPepCtrlPlugPlayData* pPlugPlayData = (TPepCtrlPlugPlayData*)pvData;
    BOOLEAN bDevicePresent;

    PepCtrlLog("PepCtrlPlugPlayIsDevicePresent entering.\n");

    PAGED_CODE()

    bDevicePresent = pPlugPlayData->pszSymbolicLinkName != NULL;

    PepCtrlLog("PepCtrlPlugPlayIsDevicePresent leaving.\n");

    return bDevicePresent;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
