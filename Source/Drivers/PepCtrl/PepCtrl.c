/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>
#include <wdmguid.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeapDriver.h>

#include <UtilsPep/UtPepLogic.h>

#include <Drivers/PepCtrlDefs.h>
#include <Drivers/PepCtrlIOCTL.h>

#include "PepCtrlLog.h"
#include "PepCtrlPortData.h"
#include "PepCtrlDeviceControl.h"
#include "PepCtrlPlugPlay.h"
#include "PepCtrlInit.h"

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

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (INIT, DriverEntry)

#pragma alloc_text (PAGE, lPepCtrlIrpClose)
#pragma alloc_text (PAGE, lPepCtrlIrpPower)
#pragma alloc_text (PAGE, lPepCtrlIrpCreate)
#pragma alloc_text (PAGE, lPepCtrlIrpDeviceControl)
#pragma alloc_text (PAGE, lPepCtrlDriverUnload)
#endif

typedef NTSTATUS (*TDeviceControlFunc)(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);

typedef struct tagTDeviceControlFuncs
{
    ULONG ulIOControlCode;
    TDeviceControlFunc DeviceControlFunc;
} TDeviceControlFuncs;

static TDeviceControlFuncs l_DeviceControlFuncs[] = {
    {IOCTL_PEPCTRL_SET_PROGRAMMER_MODE, &PepCtrlDeviceControl_SetProgrammerMode},
    {IOCTL_PEPCTRL_SET_VCC_MODE,        &PepCtrlDeviceControl_SetVccMode},
    {IOCTL_PEPCTRL_SET_PIN_PULSE_MODE,  &PepCtrlDeviceControl_SetPinPulseMode},
    {IOCTL_PEPCTRL_SET_VPP_MODE,        &PepCtrlDeviceControl_SetVppMode},
    {IOCTL_PEPCTRL_SET_ADDRESS,         &PepCtrlDeviceControl_SetAddress},
    {IOCTL_PEPCTRL_GET_DATA,            &PepCtrlDeviceControl_GetData},
    {IOCTL_PEPCTRL_SET_DATA,            &PepCtrlDeviceControl_SetData},
    {IOCTL_PEPCTRL_TRIGGER_PROGRAM,     &PepCtrlDeviceControl_TriggerProgram},
    {IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,   &PepCtrlDeviceControl_SetOutputEnable},
    {IOCTL_PEPCTRL_GET_DEVICE_STATUS,   &PepCtrlDeviceControl_GetDeviceStatus},
    {IOCTL_PEPCTRL_DEVICE_NOTIFICATION, &PepCtrlDeviceControl_DeviceNotification},
    {IOCTL_PEPCTRL_GET_SETTINGS,        &PepCtrlDeviceControl_GetSettings},
    {IOCTL_PEPCTRL_SET_SETTINGS,        &PepCtrlDeviceControl_SetSettings} };

#pragma region "Local Functions"

static NTSTATUS lPepCtrlIrpClose(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_SUCCESS;

    pDeviceObject;

    PAGED_CODE()

    PepCtrlLog("lPepCtrlIrpClose entering.\n");

    UtPepLogicReset(&pPortData->LogicData);

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lPepCtrlIrpClose leaving.\n");

    return STATUS_SUCCESS;
}

static NTSTATUS lPepCtrlIrpPower(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp)
{
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS Status = STATUS_SUCCESS;

    pDeviceObject;

    PAGED_CODE()

    PepCtrlLog("lPepCtrlIrpPower entering.\n");

    if (pIrpSp->MinorFunction == IRP_MN_POWER_SEQUENCE)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_POWER_SEQUENCE received.\n");

        Status = STATUS_NOT_IMPLEMENTED;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_QUERY_POWER)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_QUERY_POWER received.\n");

        Status = STATUS_SUCCESS;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_SET_POWER)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_SET_POWER received.\n");

        Status = STATUS_SUCCESS;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_WAIT_WAKE)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_WAIT_WAKE received.\n");

        Status = STATUS_NOT_SUPPORTED;
    }
    else
    {
        PepCtrlLog("lPepCtrlIrpPower - Unknown minor power code.\n");

        Status = STATUS_NOT_IMPLEMENTED;
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lPepCtrlIrpPower leaving.\n");

    return STATUS_SUCCESS;
}

static NTSTATUS lPepCtrlIrpCreate(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lPepCtrlIrpCreate entering.\n");

    PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
        PepCtrlLog("lPepCtrlIrpCreate - Device present setting address to 0.\n");

        UtPepLogicSetAddress(&pPortData->LogicData, 0);
    }
    else
    {
        PepCtrlLog("lPepCtrlIrpCreate - Device not present.\n");
    }

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lPepCtrlIrpCreate leaving.\n");

    return STATUS_SUCCESS;
}

static NTSTATUS lPepCtrlIrpDeviceControl(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION pIrpSp;
    PVOID pvInBuf, pvOutBuf;
    ULONG ulInBufLen, ulOutBufLen, ulIndex;
    BOOLEAN bFuncFound;

    PAGED_CODE()

    PepCtrlLog("lPepCtrlIrpDeviceControl entering.\n");

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
            ExAcquireFastMutexUnsafe(&pPortData->FastMutex);

            pPortData->nState = CPepCtrlStateDeviceControl;

            bFuncFound = TRUE;

            Status = l_DeviceControlFuncs[ulIndex].DeviceControlFunc(pIrp, pPortData, pvInBuf,
                                                                     ulInBufLen, pvOutBuf,
                                                                     ulOutBufLen);

            pPortData->nState = CPepCtrlStateRunning;

            ExReleaseFastMutexUnsafe(&pPortData->FastMutex);
        }
    }

    if (bFuncFound == FALSE)
    {
        PepCtrlLog("lPepCtrlIrpDeviceControl - Unsupported I/O Control Code\n");

        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPepCtrlIrpDeviceControl leaving.\n");

    return Status;
}

static VOID lPepCtrlDriverUnload(
  _In_ PDRIVER_OBJECT pDriverObject)
{
    TPepCtrlPortData* pPortData;
    UNICODE_STRING DOSName;

    PAGED_CODE()

    PepCtrlLog("lPepCtrlDriverUnload entering.\n");

    pPortData = (TPepCtrlPortData*)pDriverObject->DeviceObject->DeviceExtension;

    if (pPortData->RegSettings.nPortType != CPepCtrlNoPortType)
    {
        ExAcquireFastMutexUnsafe(&pPortData->FastMutex);

        pPortData->nState = CPepCtrlStateUnloading;

        if (PepCtrlPlugPlayUnregister(pPortData->pvPlugPlayData))
        {
            PepCtrlLog("lPepCtrlDriverUnload - Plug and Play notification unregistered.\n");
        }
        else
        {
            PepCtrlLog("lPepCtrlDriverUnload - Could not unregister the Plug and Play notification.\n");
        }

        ExReleaseFastMutexUnsafe(&pPortData->FastMutex);
    }

    PepCtrlUninitPortData(pPortData);

    RtlInitUnicodeString(&DOSName, CPepCtrlInternalDosDeviceName);

    PepCtrlLog("lPepCtrlDriverUnload - Deleting the symbolic link of \"%ws\".\n", DOSName.Buffer);

    IoDeleteSymbolicLink(&DOSName);

    PepCtrlLog("lPepCtrlDriverUnload - Deleting the device.\n");

    IoDeleteDevice(pDriverObject->DeviceObject);

    PepCtrlLog("lPepCtrlDriverUnload leaving.\n");
}

#pragma endregion

#pragma region "Driver Entry"

NTSTATUS __stdcall DriverEntry(
  _In_ PDRIVER_OBJECT pDriverObject,
  _In_ PUNICODE_STRING pRegistryPath)
{
    BOOLEAN bInitialized = TRUE;
    TPepCtrlPortData* pPortData;
    UNICODE_STRING DeviceName, DOSName;
    PDEVICE_OBJECT pDeviceObject;
    NTSTATUS status;
    
    PepCtrlLog("DriverEntry entering");

    PepCtrlLog("DriverEntry - Initializing the memory pool tag.\n");

    UtInitMemPoolTag(CPepCtrlMemPoolTag);

    PepCtrlLog("DriverEntry - Called with a registry path of \"%ws\".\n", pRegistryPath->Buffer);

    RtlInitUnicodeString(&DeviceName, CPepCtrlInternalDeviceName);
    RtlInitUnicodeString(&DOSName, CPepCtrlInternalDosDeviceName);

    PepCtrlLog("DriverEntry - Creating the device \"%ws\".\n", DeviceName.Buffer);

    status = IoCreateDevice(pDriverObject, sizeof(TPepCtrlPortData),
                            &DeviceName, FILE_DEVICE_UNKNOWN,
                            0, TRUE, &pDeviceObject);

    if (!NT_SUCCESS(status))
    {
        PepCtrlLog("DriverEntry leaving (Could not create the device.  (0x%X))\n", status);

        return status;
    }

    PepCtrlLog("DriverEntry - Creating the symbolic link of \"%ws\".\n", DOSName.Buffer);

    status = IoCreateSymbolicLink(&DOSName, &DeviceName);

    if (!NT_SUCCESS(status))
    {
        PepCtrlLog("DriverEntry leaving (Could not create the symbolic link.  (0x%X))\n", status);

        IoDeleteDevice(pDeviceObject);

        return status;
    }

    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    RtlZeroMemory(pPortData, sizeof(TPepCtrlPortData));

    if (FALSE == PepCtrlInitPortData(pDriverObject, pDeviceObject, pRegistryPath, pPortData))
    {
        PepCtrlLog("DriverEntry - Could not initialize the port data.\n");

        bInitialized = FALSE;
    }

    if (bInitialized == TRUE && pPortData->RegSettings.nPortType != CPepCtrlNoPortType)
    {
        PepCtrlLog("DriverEntry - Port settings found - attempting to register for plug 'n play events.\n");

        if (PepCtrlPlugPlayRegister(pPortData->Funcs.pGetDeviceInterfaceGuidFunc(),
                                    pPortData->RegSettings.pszPortDeviceName,
                                    pPortData->pvPlugPlayData))
        {
            PepCtrlLog("DriverEntry - Successfully registered for plug 'n play events.\n");
        }
        else
        {
            PepCtrlLog("DriverEntry - Failed to register for plug 'n play events.\n");

            bInitialized = FALSE;
        }
    }

    if (!bInitialized)
    {
        PepCtrlLog("DriverEntry - Uninitializing the port data.\n");

        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("DriverEntry - Deleting the symbolic link of \"%ws\".\n", DOSName.Buffer);

        status = IoDeleteSymbolicLink(&DOSName);

        if (!NT_SUCCESS(status))
        {
            PepCtrlLog("DriverEntry - Waringing: Failed to delete the symbolic link.  (0x%X)\n", status);
        }

        PepCtrlLog("DriverEntry - Deleting the device.\n");

        IoDeleteDevice(pDriverObject->DeviceObject);

        PepCtrlLog("DriverEntry leaving (Failed to initialize.)\n");

        return STATUS_UNSUCCESSFUL;
    }

    PepCtrlLog("DriverEntry - Initializing the driver object callback functions.\n");

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = lPepCtrlIrpCreate;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = lPepCtrlIrpDeviceControl;
    pDriverObject->MajorFunction[IRP_MJ_POWER] = lPepCtrlIrpPower;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = lPepCtrlIrpClose;
    pDriverObject->DriverUnload = lPepCtrlDriverUnload;

    PepCtrlLog("DriverEntry leaving (Finished initializing the driver).\n");

    return STATUS_SUCCESS;
}

#pragma endregion

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
