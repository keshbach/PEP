/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>
#include <wdmguid.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeapDriver.h>

#include <Drivers/PepCtrlDefs.h>
#include <Drivers/PepCtrlIOCTL.h>

#include "PepCtrlPortData.h"
#include "PepCtrlLogic.h"
#include "PepCtrlParallelPort.h"
#include "PepCtrlUsbPort.h"

#define CPepCtrlMemPoolTag 'pep'

#define CPepCtrlInternalDeviceName L"\\Device\\PepCtrl"
#define CPepCtrlInternalDosDeviceName L"\\DosDevices\\PepCtrl"

DRIVER_INITIALIZE DriverEntry;

__drv_dispatchType(IRP_MJ_CLOSE)
static DRIVER_DISPATCH lPepCtrlIrpClose;

__drv_dispatchType(IRP_MJ_POWER)
static DRIVER_DISPATCH lPepCtrlIrpPower;

__drv_dispatchType(IRP_MJ_CREATE)
static DRIVER_DISPATCH lPepCtrlIrpCreate;

__drv_dispatchType(IRP_MJ_DEVICE_CONTROL)
static DRIVER_DISPATCH lPepCtrlIrpDeviceControl;

static DRIVER_UNLOAD lPepCtrlDriverUnload;

static BOOLEAN lReadRegULongValue(IN HANDLE hRegKey, IN LPCWSTR pszValueName, IN OUT PULONG pulValue);
static BOOLEAN lReadRegStringValue(IN HANDLE hRegKey, IN LPCWSTR pszValueName, OUT LPWSTR* ppszValue);
static BOOLEAN lReadRegSettings(IN PUNICODE_STRING pRegistryPath, IN OUT PULONG pulPortType, OUT LPWSTR* ppszPortDeviceName);
static BOOLEAN lInitializePortData(IN PUNICODE_STRING pRegistryPath, IN TPepCtrlPortData* pPortData);
static VOID lCloseThread(IN HANDLE hThread);
static VOID lClosePortThreads(IN TPepCtrlPortData* pPortData);
static KSTART_ROUTINE lPortArrivedThreadStart;
static KSTART_ROUTINE lPortRemovedThreadStart;
static VOID lDeviceInterfaceArrival(IN PDEVICE_OBJECT pDeviceObject);
static VOID lDeviceInterfaceRemoval(IN PDEVICE_OBJECT pDeviceObject);
static DRIVER_NOTIFICATION_CALLBACK_ROUTINE lPlugPlayDeviceInterfaceChange;
static DRIVER_CANCEL lCancelIrpRoutine;

static NTSTATUS lDeviceControl_SetProgrammerMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_SetVccMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_SetPinPulseMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_SetVppMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_SetAddress(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_GetData(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_SetData(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_TriggerProgram(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_SetOutputEnable(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_GetDeviceStatus(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
static NTSTATUS lDeviceControl_DeviceNotification(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (INIT, DriverEntry)

#pragma alloc_text (PAGE, lPepCtrlIrpClose)
#pragma alloc_text (PAGE, lPepCtrlIrpPower)
#pragma alloc_text (PAGE, lPepCtrlIrpCreate)
#pragma alloc_text (PAGE, lPepCtrlIrpDeviceControl)
#pragma alloc_text (PAGE, lPepCtrlDriverUnload)
#pragma alloc_text (PAGE, lReadRegULongValue)
#pragma alloc_text (PAGE, lReadRegStringValue)
#pragma alloc_text (PAGE, lReadRegSettings)
#pragma alloc_text (PAGE, lInitializePortData)
#pragma alloc_text (PAGE, lCloseThread)
#pragma alloc_text (PAGE, lClosePortThreads)
#pragma alloc_text (PAGE, lPortArrivedThreadStart)
#pragma alloc_text (PAGE, lPortRemovedThreadStart)
#pragma alloc_text (PAGE, lDeviceInterfaceArrival)
#pragma alloc_text (PAGE, lDeviceInterfaceRemoval)
#pragma alloc_text (PAGE, lPlugPlayDeviceInterfaceChange)
#pragma alloc_text (PAGE, lDeviceControl_SetProgrammerMode)
#pragma alloc_text (PAGE, lDeviceControl_SetVccMode)
#pragma alloc_text (PAGE, lDeviceControl_SetPinPulseMode)
#pragma alloc_text (PAGE, lDeviceControl_SetVppMode)
#pragma alloc_text (PAGE, lDeviceControl_SetAddress)
#pragma alloc_text (PAGE, lDeviceControl_GetData)
#pragma alloc_text (PAGE, lDeviceControl_SetData)
#pragma alloc_text (PAGE, lDeviceControl_TriggerProgram)
#pragma alloc_text (PAGE, lDeviceControl_SetOutputEnable)
#pragma alloc_text (PAGE, lDeviceControl_GetDeviceStatus)
#pragma alloc_text (PAGE, lDeviceControl_DeviceNotification)
#endif

typedef NTSTATUS (*TDeviceControlFunc)(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);

typedef struct tagTDeviceControlFuncs
{
    ULONG ulIOControlCode;
    TDeviceControlFunc DeviceControlFunc;
} TDeviceControlFuncs;

static TDeviceControlFuncs l_DeviceControlFuncs[] = {
    {IOCTL_PEPCTRL_SET_PROGRAMMER_MODE, &lDeviceControl_SetProgrammerMode},
    {IOCTL_PEPCTRL_SET_VCC_MODE,        &lDeviceControl_SetVccMode},
    {IOCTL_PEPCTRL_SET_PIN_PULSE_MODE,  &lDeviceControl_SetPinPulseMode},
    {IOCTL_PEPCTRL_SET_VPP_MODE,        &lDeviceControl_SetVppMode},
    {IOCTL_PEPCTRL_SET_ADDRESS,         &lDeviceControl_SetAddress},
    {IOCTL_PEPCTRL_GET_DATA,            &lDeviceControl_GetData},
    {IOCTL_PEPCTRL_SET_DATA,            &lDeviceControl_SetData},
    {IOCTL_PEPCTRL_TRIGGER_PROGRAM,     &lDeviceControl_TriggerProgram},
    {IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,   &lDeviceControl_SetOutputEnable},
    {IOCTL_PEPCTRL_GET_DEVICE_STATUS,   &lDeviceControl_GetDeviceStatus},
    {IOCTL_PEPCTRL_DEVICE_NOTIFICATION, &lDeviceControl_DeviceNotification} };

/*
  Local Functions
*/

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

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: lReadRegULongValue called with the value name of \"%ws\".\n",
               pszValueName) );

    PAGED_CODE()

    RtlInitUnicodeString(&ValueName, pszValueName);

    status = ZwQueryValueKey(hRegKey, &ValueName, KeyValuePartialInformation,
                             NULL, 0, &ulValueInfoLen);

    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        pValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)UtAllocPagedMem(ulValueInfoLen);

        if (!pValueInfo)
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Could not allocate memory.\n") );

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
    	        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                           "PepCtrl: Could not query the registry value. (0x%X)\n",
                           status) );
            }
            else if (pValueInfo->Type != REG_DWORD)
            {
    	        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                           "PepCtrl: Wrong registry value type found. (0x%X)\n",
                           pValueInfo->Type) );
            }
        }

        UtFreePagedMem(pValueInfo);
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not query the registry value size. (0x%X)\n",
                   status) );
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

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: lReadRegStringValue called with the value name of \"%ws\".\n",
               pszValueName) );

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
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Could not allocate memory.\n") );

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
    	        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                           "PepCtrl: Could not query the registry value. (0x%X)\n",
                           status) );
            }
            else if (pValueInfo->Type != REG_SZ)
            {
    	        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                           "PepCtrl: Wrong registry value type found. (0x%X)\n",
                           pValueInfo->Type) );
            }
        }

        UtFreePagedMem(pValueInfo);
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not query the registry value size. (0x%X)\n",
                   status) );
    }

    return bResult;
}

static BOOLEAN lReadRegSettings(
  IN PUNICODE_STRING pRegistryPath,
  IN OUT PULONG pulPortType,
  OUT LPWSTR* ppszPortDeviceName)
{
    BOOLEAN bResult = FALSE;
    LPWSTR pszSettingsRegPath;
    UNICODE_STRING SettingsRegPath;
    OBJECT_ATTRIBUTES RegPathObj;
    HANDLE hRegKey;
    NTSTATUS status;
    size_t SettingsLen, SettingsRegPathLen;

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: lReadRegSettings called with a registry path of \"%ws\".\n",
               pRegistryPath->Buffer) );

    PAGED_CODE()

    RtlStringCbLengthW(CPepCtrlSettingsRegKeyName, NTSTRSAFE_MAX_CCH,
                       &SettingsLen);

    SettingsRegPathLen = pRegistryPath->Length + SettingsLen + (sizeof(WCHAR) * 2);
    pszSettingsRegPath = (LPWSTR)UtAllocPagedMem(SettingsRegPathLen);

    if (!pszSettingsRegPath)
    {
    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not allocate memory for the settings registry key name.\n") );

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

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: Opening the registry key \"%ws\".\n", pszSettingsRegPath) );

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
    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not open the root registry key. (0x%X)\n",
                   status) );
    }

    UtFreePagedMem(pszSettingsRegPath);

    return bResult;
}

static BOOLEAN lInitializePortData(
  IN PUNICODE_STRING pRegistryPath, 
  IN TPepCtrlPortData* pPortData)
{
    BOOLEAN bResult = FALSE;
    ULONG ulPortType;

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lInitializePortData called.\n") );

    PAGED_CODE()

    RtlZeroMemory(pPortData, sizeof(TPepCtrlPortData));

    pPortData->bPortEjected = TRUE;

    ExInitializeFastMutex(&pPortData->FastMutex);

    PepCtrlInitModes(pPortData);

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Reading the registry settings.\n") );

    if (!lReadRegSettings(pRegistryPath, &ulPortType,
                          &pPortData->pszPortDeviceName))
    {
    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not read the registry settings.\n") );

        return FALSE;
    }

    if (ulPortType == CPepCtrlParallelPortType)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Parallel Port type retrieved from the registry.\n") );

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocParallelPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeParallelPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitParallelPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteParallelPort;
        pPortData->Funcs.pGetDevInterfaceGuidFunc = &PepCtrlGetParallelPortDevInterfaceGuid;

        bResult = TRUE;
    }
    else if (ulPortType == CPepCtrlUsbPrintPortType)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: USB Print Port type retrieved from the registry.\n") );

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocUsbPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeUsbPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitUsbPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteUsbPort;
        pPortData->Funcs.pGetDevInterfaceGuidFunc = &PepCtrlGetUsbPortDevInterfaceGuid;

        bResult = TRUE;
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Unknown Port type retrieved from the registry.\n") );

        UtFreePagedMem(pPortData->pszPortDeviceName);
    }

    return bResult;
}

static VOID lCloseThread(
  IN HANDLE hThread)
{
    NTSTATUS Status;
    PKTHREAD Thread;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lCloseThread called.\n") );

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Retrieving a thread object from the thread handle.\n") );

    Status = ObReferenceObjectByHandle(hThread, 0, *PsThreadType,
                                       KernelMode, &Thread, NULL);

    if (NT_SUCCESS(Status))
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: Retrieved the thread object.\n") );

        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Waiting for the thread to end.\n") );

        Status = KeWaitForSingleObject(Thread, Executive, KernelMode, FALSE, NULL);

        if (NT_SUCCESS(Status))
        {
	        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Thread has ended.\n") );
        }
        else
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Failed to wait for the thread to end.  (0x%X)\n",
                        Status) );
        }

        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Closing the thread handle.\n") );

        Status = ZwClose(hThread);

        if (NT_SUCCESS(Status))
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Thread handle closed.\n") );
        }
        else
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Failed to close the thread handle.  (0x%X)\n",
                        Status) );
        }

        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Releasing the thread object.\n") );

        ObDereferenceObject(Thread);
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: Could not retrieve the thread object from the thread handle.  (0x%X)\n",
                    Status) );
    }
}

static VOID lClosePortThreads(
  IN TPepCtrlPortData* pPortData)
{
    HANDLE hPortArrivedThread, hPortRemovedThread;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lClosePortThreads called.\n") );

    ExAcquireFastMutex(&pPortData->FastMutex);

    hPortArrivedThread = pPortData->hPortArrivedThread;
    hPortRemovedThread = pPortData->hPortRemovedThread;

    pPortData->hPortArrivedThread = NULL;
    pPortData->hPortRemovedThread = NULL;

    ExReleaseFastMutex(&pPortData->FastMutex);

    if (hPortArrivedThread)
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Closing port arrived thread handle.\n") );

        lCloseThread(hPortArrivedThread);
    }

    if (hPortRemovedThread)
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Closing port removed thread handle.\n") );

        lCloseThread(hPortRemovedThread);
    }
}

static VOID lPortArrivedThreadStart(
  IN PVOID pvStartContext)
{
    PDEVICE_OBJECT pDeviceObject = (PDEVICE_OBJECT)pvStartContext;
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    PIRP pIrp = NULL;
    TPepCtrlObject TmpObject;
    UINT32* pnStatusChange;
        
    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPortArrivedThreadStart called.\n") );

    if (pPortData->bPortEjected &&
        pPortData->Funcs.pAllocPortFunc(&TmpObject,
                                        pPortData->pszPortDeviceName))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Port was acquired now saving the data.\n") );

        ExAcquireFastMutex(&pPortData->FastMutex);

        pPortData->bPortEjected = FALSE;

        pIrp = pPortData->pIrp;

        pPortData->pIrp = NULL;

        RtlCopyMemory(&pPortData->Object, &TmpObject, sizeof(TmpObject));

        ExReleaseFastMutex(&pPortData->FastMutex);

        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Port data was successfully saved.\n") );
    }

    if (pIrp)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Completing status change IRP.\n") );

        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceArrived;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPortArrivedThreadStart ending.\n") );
}

static VOID lPortRemovedThreadStart(
  IN PVOID pvStartContext)
{
    PDEVICE_OBJECT pDeviceObject = (PDEVICE_OBJECT)pvStartContext;
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    PIRP pIrp = NULL;
    UINT32* pnStatusChange;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPortRemovedThreadStart called.\n") );

    ExAcquireFastMutex(&pPortData->FastMutex);

    if (!pPortData->bPortEjected)
    {
        pPortData->bPortEjected = TRUE;

        pPortData->Funcs.pFreePortFunc(&pPortData->Object);

        pIrp = pPortData->pIrp;

        pPortData->pIrp = NULL;
    }

    ExReleaseFastMutex(&pPortData->FastMutex);

    if (pIrp)
    {
        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceRemoved;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPortRemovedThreadStart ending.\n") );
}

static VOID lDeviceInterfaceArrival(
  IN PDEVICE_OBJECT pDeviceObject)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceInterfaceArrival called.\n") );

    lClosePortThreads(pPortData);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                "PepCtrl: Creating the port arrived thread.\n") );

    /*
       The assumption is made that plug and plug notifications are sequential
       since the document says it is possible to receive two for the same event.
    */

    status = PsCreateSystemThread(&pPortData->hPortArrivedThread,
                                  DELETE | SYNCHRONIZE, NULL, NULL, NULL,
                                  &lPortArrivedThreadStart, pDeviceObject);

    if (NT_ERROR(status))
    {
     	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Creation of the port arrived thread failed.  (Error Code: 0x%X)\n",
                   status) );
    }
}

static VOID lDeviceInterfaceRemoval(
  IN PDEVICE_OBJECT pDeviceObject)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceInterfaceRemoval called.\n") );

    lClosePortThreads(pPortData);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                "PepCtrl: Creating the port removed thread.\n") );

    /*
       The assumption is made that plug and plug notifications are sequential
       since the document says it is possible to receive two for the same event.
    */

    status = PsCreateSystemThread(&pPortData->hPortRemovedThread,
                                  DELETE | SYNCHRONIZE, NULL, NULL, NULL,
                                  &lPortRemovedThreadStart, pDeviceObject);

    if (NT_ERROR(status))
    {
     	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Creation of the port removed thread failed.  (Error Code: 0x%X)\n",
                   status) );
    }
}

static NTSTATUS lPlugPlayDeviceInterfaceChange(
  IN PVOID pvNotificationStructure,
  IN PVOID pvContext)
{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION pNotification;
    PDEVICE_OBJECT pDeviceObject;
    TPepCtrlPortData* pPortData;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPlugPlayDeviceInterfaceChange called.\n") );

    pNotification = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION)pvNotificationStructure;
    pDeviceObject = (PDEVICE_OBJECT)pvContext;
    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    if (pNotification->Version != 1)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Received unknown notification version of %d. \n",
                   (ULONG)pNotification->Version) );

        return STATUS_SUCCESS;
    }

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                "PepCtrl: Symbolic link name \"%ws\". \n",
                pNotification->SymbolicLinkName->Buffer) );

    if (IsEqualGUID((LPGUID)&pNotification->Event,
                    (LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: GUID_DEVICE_INTERFACE_ARRIVAL event GUID received.\n") );

        lDeviceInterfaceArrival(pDeviceObject);
    }
    else if (IsEqualGUID((LPGUID)&pNotification->Event,
                         (LPGUID)&GUID_DEVICE_INTERFACE_REMOVAL))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: GUID_DEVICE_INTERFACE_REMOVAL event GUID received.\n") );

        lDeviceInterfaceRemoval(pDeviceObject);
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Unknown Notification Event GUID received.\n") );
    }

    return STATUS_SUCCESS;
}

static VOID lCancelIrpRoutine(
  IN OUT PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lCancelIrpRoutine called.\n") );

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    ExAcquireFastMutex(&pPortData->FastMutex);

    if (pPortData->pIrp == pIrp)
    {
        pPortData->pIrp = NULL;
    }

    ExReleaseFastMutex(&pPortData->FastMutex);

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}

static NTSTATUS lDeviceControl_SetProgrammerMode(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_SetProgrammerMode called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulInBufLen == sizeof(UINT32))
	    {
            if (PepCtrlSetProgrammerMode(pPortData, *((PUINT32)pvInBuf)))
            {
                Status = STATUS_SUCCESS;
            }
	    }
	    else
	    {
            if (ulInBufLen < sizeof(UINT32))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
	    }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_SetVccMode(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_SetVccMode called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulInBufLen == sizeof(UINT32))
	    {
            if (PepCtrlSetVccMode(pPortData, *((PUINT32)pvInBuf)))
            {
                Status = STATUS_SUCCESS;
            }
	    }
	    else
	    {
            if (ulInBufLen < sizeof(UINT32))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
	    }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_SetPinPulseMode(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_SetPinPulseMode called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulInBufLen == sizeof(UINT32))
	    {
            if (PepCtrlSetPinPulseMode(pPortData, *((PUINT32)pvInBuf)))
            {
                Status = STATUS_SUCCESS;
            }
	    }
	    else
	    {
            if (ulInBufLen < sizeof(UINT32))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
	    }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_SetVppMode(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_SetVppMode called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulInBufLen == sizeof(UINT32))
	    {
            if (PepCtrlSetVppMode(pPortData, *((PUINT32)pvInBuf)))
            {
                Status = STATUS_SUCCESS;
            }
	    }
	    else
	    {
            if (ulInBufLen < sizeof(UINT32))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
	    }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_SetAddress(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_SetAddress called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulInBufLen == sizeof(UINT32))
        {
            if (PepCtrlSetAddress(pPortData, *(UINT32*)pvInBuf))
            {
                Status = STATUS_SUCCESS;
            }
        }
        else
        {
            if (ulOutBufLen < sizeof(UINT32))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
        }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_GetData(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvInBuf;
    ulInBufLen;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_GetData called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulOutBufLen == sizeof(UCHAR))
        {
            if (PepCtrlGetData(pPortData, (UCHAR*)pvOutBuf))
            {
                pIrp->IoStatus.Information = ulOutBufLen;

                Status = STATUS_SUCCESS;
            }
        }
        else
        {
            if (ulOutBufLen < sizeof(UCHAR))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
        }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_SetData(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_SetData called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulInBufLen == sizeof(UCHAR))
	    {
            if (PepCtrlSetData(pPortData, *((PUCHAR)pvInBuf)))
            {
                Status = STATUS_SUCCESS;
            }
	    }
	    else
	    {
            if (ulInBufLen < sizeof(UCHAR))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
	    }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_TriggerProgram(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOLEAN bProgramSuccess = FALSE;

    pvInBuf;
    ulInBufLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_TriggerProgram called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulOutBufLen == sizeof(UINT32))
        {
            if (PepCtrlTriggerProgram(pPortData, &bProgramSuccess))
            {
                pIrp->IoStatus.Information = ulOutBufLen;

                Status = STATUS_SUCCESS;
            }

            *((UINT32*)pvOutBuf) = bProgramSuccess ? 1 : 0;
        }
        else
        {
            if (ulOutBufLen < sizeof(UINT32))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
        }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_SetOutputEnable(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_SetOutputEnable called.\n") );

    if (!pPortData->bPortEjected)
    {
        if (ulInBufLen == sizeof(UINT32))
	    {
            if (PepCtrlSetOutputEnable(pPortData, *((PUINT32)pvInBuf)))
            {
                Status = STATUS_SUCCESS;
            }
	    }
	    else
	    {
            if (ulInBufLen < sizeof(UINT32))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
	    }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_GetDeviceStatus(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvInBuf;
    ulInBufLen;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_GetPortStatus called.\n") );

    if (ulOutBufLen == sizeof(UINT32))
    {
        *((UINT32*)pvOutBuf) = pPortData->bPortEjected ? CPepCtrlDeviceNotPresent :
                                                         CPepCtrlDevicePresent;

        pIrp->IoStatus.Information = ulOutBufLen;

        Status = STATUS_SUCCESS;
    }
    else
    {
        if (ulOutBufLen < sizeof(UINT32))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
        }
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

static NTSTATUS lDeviceControl_DeviceNotification(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pvInBuf;
    ulInBufLen;
    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lDeviceControl_DeviceNotification called.\n") );

    if (ulOutBufLen != sizeof(UINT32))
    {
        if (ulOutBufLen < sizeof(UINT32))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            Status = STATUS_ARRAY_BOUNDS_EXCEEDED;
        }

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        return Status;
    }

    if (pPortData->pIrp == NULL)
    {            
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Saving off the IRP.\n") );

        pPortData->pIrp = pIrp;

        IoMarkIrpPending(pIrp);

        IoSetCancelRoutine(pIrp, lCancelIrpRoutine);
        
        Status = STATUS_PENDING;

        pIrp->IoStatus.Status = Status;
    }
    else
    {
        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return Status;
}

static NTSTATUS lPepCtrlIrpClose(
  IN PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_SUCCESS;

    pDeviceObject;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPepCtrlIrpClose called.\n") );

    PepCtrlReset(pPortData);

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

static NTSTATUS lPepCtrlIrpPower(
  IN PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp)
{
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS Status = STATUS_SUCCESS;

    pDeviceObject;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPepCtrlIrpPower called.\n") );

    if (pIrpSp->MinorFunction == IRP_MN_POWER_SEQUENCE)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IRP_MN_POWER_SEQUENCE received.\n") );

        Status = STATUS_NOT_IMPLEMENTED;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_QUERY_POWER)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IRP_MN_QUERY_POWER received.\n") );

        Status = STATUS_SUCCESS;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_SET_POWER)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IRP_MN_SET_POWER received.\n") );

        Status = STATUS_SUCCESS;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_WAIT_WAKE)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IRP_MN_WAIT_WAKE received.\n") );

        Status = STATUS_NOT_SUPPORTED;
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Unknown minor power code.\n") );

        Status = STATUS_NOT_IMPLEMENTED;
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

static NTSTATUS lPepCtrlIrpCreate(
  IN PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPepCtrlIrpCreate called.\n") );

    PAGED_CODE()

    pPortData->nLastAddress = 0xFFFFFFFF;

    if (!pPortData->bPortEjected)
    {
        PepCtrlSetAddress(pPortData, 0);
    }

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

static NTSTATUS lPepCtrlIrpDeviceControl(
  IN PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION pIrpSp;
    PVOID pvInBuf, pvOutBuf;
    ULONG ulInBufLen, ulOutBufLen, ulIndex;
    BOOLEAN bFuncFound;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPepCtrlIrpDeviceControl called.\n") );

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pIrp->IoStatus.Information = 0;

    ulInBufLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    ulOutBufLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    pvInBuf = pIrp->AssociatedIrp.SystemBuffer;
    pvOutBuf = pIrp->AssociatedIrp.SystemBuffer;

    bFuncFound = FALSE;

    for (ulIndex = 0;
         bFuncFound == FALSE && ulIndex < MArrayLen(l_DeviceControlFuncs);
         ++ulIndex)
    {
        if (l_DeviceControlFuncs[ulIndex].ulIOControlCode == pIrpSp->Parameters.DeviceIoControl.IoControlCode)
        {
            ExAcquireFastMutex(&pPortData->FastMutex);

            bFuncFound = TRUE;

            Status = l_DeviceControlFuncs[ulIndex].DeviceControlFunc(pIrp, pPortData, pvInBuf,
                                                                        ulInBufLen, pvOutBuf,
                                                                        ulOutBufLen);

            ExReleaseFastMutex(&pPortData->FastMutex);
        }
    }

    if (bFuncFound == FALSE)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Unsupported I/O Control Code\n") );

        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return Status;
}

static VOID lPepCtrlDriverUnload(
  IN PDRIVER_OBJECT pDriverObject)
{
    TPepCtrlPortData* pPortData;
    UNICODE_STRING DOSName;
    NTSTATUS Status;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: lPepCtrlDriverUnload called.\n") );

    pPortData = (TPepCtrlPortData*)pDriverObject->DeviceObject->DeviceExtension;

    lClosePortThreads(pPortData);

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Unregistering Plug n Play notification of a device change.\n") );

    Status = IoUnregisterPlugPlayNotification(pPortData->pvPnPNotificationEntry);

    if (NT_SUCCESS(Status))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Plug n Play notification unregistered.\n") );
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not unregister the Plug n Play notification.  (0x%X)\n",
                   Status) );
    }

    if (!pPortData->bPortEjected)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Freeing the port.\n") );

        pPortData->Funcs.pFreePortFunc(&pPortData->Object);
    }

    if (pPortData->pszPortDeviceName)
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Deleting the memory allocated for the port's device name.\n") );

        UtFreePagedMem(pPortData->pszPortDeviceName);
    }

    RtlInitUnicodeString(&DOSName, CPepCtrlInternalDosDeviceName);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Deleting the symbolic link of \"%ws\".\n", DOSName.Buffer) );

    IoDeleteSymbolicLink(&DOSName);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Deleting the device.\n") );

    IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS __stdcall DriverEntry(
  IN PDRIVER_OBJECT pDriverObject,
  IN PUNICODE_STRING pRegistryPath)
{
    BOOLEAN bInitialized = TRUE;
    TPepCtrlPortData* pPortData;
    UNICODE_STRING DeviceName, DOSName, DefSecuritySettings;
    PDEVICE_OBJECT pDeviceObject;
    NTSTATUS status;

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Initializing the memory pool tag.\n") );

    UtInitMemPoolTag(CPepCtrlMemPoolTag);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: DriverEntry called with a registry path of \"%ws\".\n",
               pRegistryPath->Buffer) );

    RtlInitUnicodeString(&DeviceName, CPepCtrlInternalDeviceName);
    RtlInitUnicodeString(&DOSName, CPepCtrlInternalDosDeviceName);
    RtlInitUnicodeString(&DefSecuritySettings, L"");

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Creating the device \"%ws\".\n", DeviceName.Buffer) );

    status = IoCreateDevice(pDriverObject, sizeof(TPepCtrlPortData),
                            &DeviceName, FILE_DEVICE_UNKNOWN,
                            0, TRUE, &pDeviceObject);

    if (!NT_SUCCESS(status))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not create the device.  (0x%X)\n",
                   status) );

        return status;
    }

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Creating the symbolic link of \"%ws\".\n",
               DOSName.Buffer) );

    status = IoCreateSymbolicLink(&DOSName, &DeviceName);

    if (!NT_SUCCESS(status))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not create the symbolic link.  (0x%X)\n",
                   status) );

        IoDeleteDevice(pDeviceObject);

        return status;
    }

    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    if (FALSE == lInitializePortData(pRegistryPath, pPortData))
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Could not initialize the port data or allocate the port.\n") );

        bInitialized = FALSE;
    }

    if (bInitialized == TRUE)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Registering for Plug n Play notification of a device change.\n") );

        status = IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange,
                                                PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                                                (PVOID)pPortData->Funcs.pGetDevInterfaceGuidFunc(),
                                                pDriverObject,
                                                lPlugPlayDeviceInterfaceChange,
                                                pDeviceObject,
                                                &pPortData->pvPnPNotificationEntry);

        if (NT_ERROR(status))
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Could not register for the device removal notification.  (0x%X)\n",
                       status) );

            bInitialized = FALSE;
        }
    }

    if (!bInitialized)
    {
        if (pPortData->pszPortDeviceName)
        {
	        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Deleting the memory allocated for the port's device name.\n") );

            UtFreePagedMem(pPortData->pszPortDeviceName);
        }

	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Deleting the symbolic link of \"%ws\".\n", DOSName.Buffer) );

        IoDeleteSymbolicLink(&DOSName);

	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Deleting the device.\n") );

        IoDeleteDevice(pDriverObject->DeviceObject);

        return STATUS_UNSUCCESSFUL;
    }

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Initializing the driver object callback functions.\n") );

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = lPepCtrlIrpCreate;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = lPepCtrlIrpDeviceControl;
    pDriverObject->MajorFunction[IRP_MJ_POWER] = lPepCtrlIrpPower;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = lPepCtrlIrpClose;
    pDriverObject->DriverUnload = lPepCtrlDriverUnload;

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Finished initializing the driver.\n") );

    return STATUS_SUCCESS;
}

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
