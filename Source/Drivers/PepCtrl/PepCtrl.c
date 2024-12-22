/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

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
#include "PepCtrlHelper.h"

#pragma region "Constants"

#define CPepCtrlMemPoolTag 'pep'

#define CPepCtrlInternalDeviceName L"\\Device\\PepCtrl"
#define CPepCtrlInternalDosDeviceName L"\\DosDevices\\PepCtrl"

#define CTimeoutMs 50

#pragma endregion

DRIVER_INITIALIZE DriverEntry;

_Dispatch_type_(IRP_MJ_CLOSE)
static DRIVER_DISPATCH lPepCtrlIrpClose;

_Dispatch_type_(IRP_MJ_POWER)
static DRIVER_DISPATCH lPepCtrlIrpPower;

_Dispatch_type_(IRP_MJ_CREATE)
static DRIVER_DISPATCH lPepCtrlIrpCreate;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
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

#pragma region "Type Defs"

typedef NTSTATUS (*TDeviceControlFunc)(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_writes_(ulOutBufLen) PVOID pvOutBuf, _In_ ULONG ulOutBufLen);

typedef struct tagTDeviceControlFuncs
{
    ULONG ulIOControlCode;
    TDeviceControlFunc DeviceControlFunc;
} TDeviceControlFuncs;

#pragma endregion

#pragma region "Local Variables"

#pragma data_seg("PAGEDATA")
#pragma bss_seg("PAGEBSS")

static TDeviceControlFuncs l_DeviceControlFuncs[] = {
    {IOCTL_PEPCTRL_SET_PROGRAMMER_MODE,    &PepCtrlDeviceControl_SetProgrammerMode},
    {IOCTL_PEPCTRL_SET_VCC_MODE,           &PepCtrlDeviceControl_SetVccMode},
    {IOCTL_PEPCTRL_SET_PIN_PULSE_MODE,     &PepCtrlDeviceControl_SetPinPulseMode},
    {IOCTL_PEPCTRL_SET_VPP_MODE,           &PepCtrlDeviceControl_SetVppMode},
    {IOCTL_PEPCTRL_SET_ADDRESS,            &PepCtrlDeviceControl_SetAddress},
    {IOCTL_PEPCTRL_SET_ADDRESS_WITH_DELAY, &PepCtrlDeviceControl_SetAddressWithDelay},
    {IOCTL_PEPCTRL_GET_DATA,               &PepCtrlDeviceControl_GetData},
    {IOCTL_PEPCTRL_SET_DATA,               &PepCtrlDeviceControl_SetData},
    {IOCTL_PEPCTRL_TRIGGER_PROGRAM,        &PepCtrlDeviceControl_TriggerProgram},
    {IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,      &PepCtrlDeviceControl_SetOutputEnable},
    {IOCTL_PEPCTRL_GET_DEVICE_STATUS,      &PepCtrlDeviceControl_GetDeviceStatus},
    {IOCTL_PEPCTRL_DEVICE_NOTIFICATION,    &PepCtrlDeviceControl_DeviceNotification},
    {IOCTL_PEPCTRL_GET_PORT_SETTINGS,      &PepCtrlDeviceControl_GetPortSettings},
    {IOCTL_PEPCTRL_SET_PORT_SETTINGS,      &PepCtrlDeviceControl_SetPortSettings},
    {IOCTL_PEPCTRL_SET_DELAY_SETTINGS,     &PepCtrlDeviceControl_SetDelaySettings},
    {IOCTL_PEPCTRL_DEBUG_WRITE_PORT_DATA,  &PepCtrlDeviceControl_DebugWritePortData} };

#pragma data_seg()
#pragma bss_seg()

#pragma endregion

#pragma region "Local Functions"

_Use_decl_annotations_
static NTSTATUS lPepCtrlIrpClose(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _Inout_ PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE()

    PepCtrlLog("lPepCtrlIrpClose entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
        PepCtrlLog("lPepCtrlIrpClose - Device present and being reset.  (Thread: 0x%p)\n",
                   PsGetCurrentThread());

        UtPepLogicReset(&pPortData->LogicData);
    }
    else
    {
        PepCtrlLog("lPepCtrlIrpClose - Device not present.  (Thread: 0x%p)\n",
                   PsGetCurrentThread());
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lPepCtrlIrpClose leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
static NTSTATUS lPepCtrlIrpPower(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _Inout_ PIRP pIrp)
{
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS Status = STATUS_SUCCESS;

    pDeviceObject;

    PAGED_CODE()

    PepCtrlLog("lPepCtrlIrpPower entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (pIrpSp->MinorFunction == IRP_MN_POWER_SEQUENCE)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_POWER_SEQUENCE received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        Status = STATUS_NOT_IMPLEMENTED;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_QUERY_POWER)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_QUERY_POWER received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        Status = STATUS_SUCCESS;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_SET_POWER)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_SET_POWER received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        Status = STATUS_SUCCESS;
    }
    else if (pIrpSp->MinorFunction == IRP_MN_WAIT_WAKE)
    {
        PepCtrlLog("lPepCtrlIrpPower - IRP_MN_WAIT_WAKE received.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        Status = STATUS_NOT_SUPPORTED;
    }
    else
    {
        PepCtrlLog("lPepCtrlIrpPower - Unknown minor power code.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        Status = STATUS_NOT_IMPLEMENTED;
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lPepCtrlIrpPower leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
static NTSTATUS lPepCtrlIrpCreate(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _Inout_ PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lPepCtrlIrpCreate entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
        PepCtrlLog("lPepCtrlIrpCreate - Device present setting address to 0.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        UtPepLogicSetAddress(&pPortData->LogicData, 0);
    }
    else
    {
        PepCtrlLog("lPepCtrlIrpCreate - Device not present.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lPepCtrlIrpCreate leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
static NTSTATUS lPepCtrlIrpDeviceControl(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _Inout_ PIRP pIrp)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOLEAN bQuitFunc = TRUE;
	BOOLEAN bMonitorState = TRUE;
    PIO_STACK_LOCATION pIrpSp;
    PVOID pvInBuf, pvOutBuf;
    ULONG ulInBufLen, ulOutBufLen, ulIndex;
    BOOLEAN bFuncFound, bExecuteSleep;
	LARGE_INTEGER SleepInteger;
    INT32 nPreviousState;

    PAGED_CODE()

    PepCtrlLog("lPepCtrlIrpDeviceControl entering.  (Thread: 0x%p)\n", PsGetCurrentThread());

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

	pIrp->IoStatus.Information = 0;

    while (bMonitorState)
    {
		bExecuteSleep = FALSE;

        ExAcquireFastMutex(&pPortData->FastMutex);

        nPreviousState = pPortData->nState;

        switch (pPortData->nState)
        {
            case CPepCtrlStateRunning:
                pPortData->nState = CPepCtrlStateDeviceControl;

                bQuitFunc = FALSE;
                bMonitorState = FALSE;
                break;
            case CPepCtrlStateDeviceArrived:
            case CPepCtrlStateDeviceRemoved:
				if (pIrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_PEPCTRL_GET_DEVICE_STATUS ||
					pIrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_PEPCTRL_DEVICE_NOTIFICATION)
                {
					if (!pIrp->Cancel)
					{
						bExecuteSleep = TRUE;
					}
					else
					{
						bMonitorState = FALSE;
					}
				}
				else
				{
					bMonitorState = FALSE;
				}
                break;
            case CPepCtrlStateDeviceControl:
            case CPepCtrlStateUnloading:
            case CPepCtrlStateChangePortSettings:
                bMonitorState = FALSE;
                break;
            default:
                bMonitorState = FALSE;
                break;
        }

        ExReleaseFastMutex(&pPortData->FastMutex);

        switch (nPreviousState)
        {
            case CPepCtrlStateRunning:
                break;
            case CPepCtrlStateDeviceArrived:
            case CPepCtrlStateDeviceRemoved:
                PepCtrlLog("lPepCtrlIrpDeviceControl - State of \"%s\" detected for I/O Control Code: %s.  (Thread: 0x%p)\n",
                           PepCtrlHelperTranslateState(nPreviousState),
                           PepCtrlHelperTranslateControlCode(pIrpSp->Parameters.DeviceIoControl.IoControlCode),
                           PsGetCurrentThread());
                break;
            case CPepCtrlStateDeviceControl:
            case CPepCtrlStateUnloading:
            case CPepCtrlStateChangePortSettings:
                PepCtrlLog("lPepCtrlIrpDeviceControl - Invalid state of \"%s\" for I/O Control Code: %s.  (Thread: 0x%p)\n",
                           PepCtrlHelperTranslateState(nPreviousState),
                           PepCtrlHelperTranslateControlCode(pIrpSp->Parameters.DeviceIoControl.IoControlCode),
                           PsGetCurrentThread());
                break;
            default:
                PepCtrlLog("lPepCtrlIrpDeviceControl - ERROR: Unknown state of \"%s\" for I/O Control Code: %s.  (Thread: 0x%p)\n",
                           PepCtrlHelperTranslateState(nPreviousState),
                           PepCtrlHelperTranslateControlCode(pIrpSp->Parameters.DeviceIoControl.IoControlCode),
                           PsGetCurrentThread());
                break;
        }

		if (bExecuteSleep)
		{
			SleepInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

			KeDelayExecutionThread(KernelMode, FALSE, &SleepInteger);
		}
    }

    if (bQuitFunc)
    {
        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("lPepCtrlIrpDeviceControl leaving (Invalid state, Thread: 0x%p).\n", PsGetCurrentThread());

        return Status;
    }

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
            PepCtrlLog("lPepCtrlIrpDeviceControl - Function found for I/O Control Code: %s.  (Thread: 0x%p)\n",
                       PepCtrlHelperTranslateControlCode(pIrpSp->Parameters.DeviceIoControl.IoControlCode),
                       PsGetCurrentThread());

            bFuncFound = TRUE;

            Status = l_DeviceControlFuncs[ulIndex].DeviceControlFunc(pIrp, pPortData, pvInBuf,
                                                                     ulInBufLen, pvOutBuf,
                                                                     ulOutBufLen);
        }
    }

    ExAcquireFastMutex(&pPortData->FastMutex);

    if (pPortData->nState != CPepCtrlStateDeviceControl)
    {
        PepCtrlLog("lPepCtrlIrpDeviceControl - ERROR: State should be \"%s\" not \"%s\" for I/O Control Code: %s.  (Thread: 0x%p)\n",
                   PepCtrlHelperTranslateState(CPepCtrlStateDeviceControl),
                   PepCtrlHelperTranslateState(pPortData->nState),
                   PepCtrlHelperTranslateControlCode(pIrpSp->Parameters.DeviceIoControl.IoControlCode),
			       PsGetCurrentThread());
    }

    pPortData->nState = CPepCtrlStateRunning;

    ExReleaseFastMutex(&pPortData->FastMutex);

    if (bFuncFound == FALSE)
    {
        PepCtrlLog("lPepCtrlIrpDeviceControl - Unsupported I/O Control Code: %s  (Thread: 0x%p)\n",
                   PepCtrlHelperTranslateControlCode(pIrpSp->Parameters.DeviceIoControl.IoControlCode),
			       PsGetCurrentThread());

        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPepCtrlIrpDeviceControl leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return Status;
}

_Use_decl_annotations_
static VOID lPepCtrlDriverUnload(
  _In_ PDRIVER_OBJECT pDriverObject)
{
    TPepCtrlPortData* pPortData;
    UNICODE_STRING DOSName;

    PepCtrlLog("lPepCtrlDriverUnload entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    pPortData = (TPepCtrlPortData*)pDriverObject->DeviceObject->DeviceExtension;

    if (pPortData->RegSettings.nPortType != CPepCtrlNoPortType)
    {
        ExAcquireFastMutex(&pPortData->FastMutex);

        pPortData->nState = CPepCtrlStateUnloading;

        ExReleaseFastMutex(&pPortData->FastMutex);

        if (PepCtrlPlugPlayUnregister(pPortData->pvPlugPlayData))
        {
            PepCtrlLog("lPepCtrlDriverUnload - Plug and Play notification unregistered.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else
        {
            PepCtrlLog("lPepCtrlDriverUnload - Could not unregister the Plug and Play notification.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
    }

    PepCtrlUninitPortData(pPortData);

    RtlInitUnicodeString(&DOSName, CPepCtrlInternalDosDeviceName);

    PepCtrlLog("lPepCtrlDriverUnload - Deleting the symbolic link of \"%ws\".  (Thread: 0x%p)\n",
		       DOSName.Buffer, PsGetCurrentThread());

    IoDeleteSymbolicLink(&DOSName);

    PepCtrlLog("lPepCtrlDriverUnload - Deleting the device.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoDeleteDevice(pDriverObject->DeviceObject);

    PepCtrlLog("lPepCtrlDriverUnload leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    //PepCtrlLogCloseFile();
}

#pragma endregion

#pragma region "Driver Entry"

_Use_decl_annotations_
NTSTATUS DriverEntry(
  _In_ PDRIVER_OBJECT pDriverObject,
  _In_ PUNICODE_STRING pRegistryPath)
{
    BOOLEAN bInitialized = TRUE;
    TPepCtrlPortData* pPortData;
    UNICODE_STRING DeviceName, DOSName;
    PDEVICE_OBJECT pDeviceObject;
    NTSTATUS status;

    //PepCtrlLogOpenFile(L"\\??\\C:\\pepctrl.log");

    PepCtrlLog("DriverEntry entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PepCtrlLog("DriverEntry - Initializing the memory pool tag.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    UtInitMemPoolTag(CPepCtrlMemPoolTag);

    PepCtrlLog("DriverEntry - Called with a registry path of \"%ws\".  (Thread: 0x%p)\n",
		       pRegistryPath->Buffer, PsGetCurrentThread());

    RtlInitUnicodeString(&DeviceName, CPepCtrlInternalDeviceName);
    RtlInitUnicodeString(&DOSName, CPepCtrlInternalDosDeviceName);

    PepCtrlLog("DriverEntry - Creating the device \"%ws\".  (Thread: 0x%p)\n", 
		       DeviceName.Buffer, PsGetCurrentThread());

    status = IoCreateDevice(pDriverObject, sizeof(TPepCtrlPortData),
                            &DeviceName, FILE_DEVICE_UNKNOWN,
                            0, TRUE, &pDeviceObject);

    if (!NT_SUCCESS(status))
    {
        PepCtrlLog("DriverEntry leaving. (Could not create the device Status: 0x%X  (Thread: 0x%p))\n",
			       status, PsGetCurrentThread());

        return status;
    }

    PepCtrlLog("DriverEntry - Creating the symbolic link of \"%ws\"  (Thread: 0x%p).\n",
		       DOSName.Buffer, PsGetCurrentThread());

    status = IoCreateSymbolicLink(&DOSName, &DeviceName);

    if (!NT_SUCCESS(status))
    {
        PepCtrlLog("DriverEntry - Deleting the device.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        IoDeleteDevice(pDeviceObject);

		PepCtrlLog("DriverEntry leaving. (Could not create the symbolic link Status: 0x%X  (Thread: 0x%p)).\n",
			      status, PsGetCurrentThread());

        return status;
    }

    PepCtrlLog("DriverEntry - Device Object pointer: 0x%p  (Thread: 0x%p)\n",
		       pDeviceObject, PsGetCurrentThread());

    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("DriverEntry - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

    RtlZeroMemory(pPortData, sizeof(TPepCtrlPortData));

    if (FALSE == PepCtrlInitPortData(pDriverObject, pDeviceObject, pRegistryPath, pPortData))
    {
        PepCtrlLog("DriverEntry - Could not initialize the port data.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        bInitialized = FALSE;
    }

    if (bInitialized == TRUE && pPortData->RegSettings.nPortType != CPepCtrlNoPortType)
    {
        PepCtrlLog("DriverEntry - Port settings found - attempting to register for plug 'n play events.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        if (PepCtrlPlugPlayRegister(pPortData->Funcs.pGetDeviceInterfaceGuidFunc(),
                                    pPortData->RegSettings.pszPortDeviceName,
                                    pPortData->pvPlugPlayData))
        {
            PepCtrlLog("DriverEntry - Successfully registered for plug 'n play events.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else
        {
            PepCtrlLog("DriverEntry - Failed to register for plug 'n play events.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            bInitialized = FALSE;
        }
    }

    if (!bInitialized)
    {
        PepCtrlLog("DriverEntry - Uninitializing the port data.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("DriverEntry - Deleting the symbolic link of \"%ws\".  (Thread: 0x%p)\n",
			       DOSName.Buffer, PsGetCurrentThread());

        status = IoDeleteSymbolicLink(&DOSName);

        if (!NT_SUCCESS(status))
        {
            PepCtrlLog("DriverEntry - Waringing: Failed to delete the symbolic link.  (0x%X)  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());
        }

        PepCtrlLog("DriverEntry - Deleting the device.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        IoDeleteDevice(pDriverObject->DeviceObject);

        PepCtrlLog("DriverEntry leaving (Failed to initialize.  (Thread: 0x%p))\n",
			       PsGetCurrentThread());

        return STATUS_UNSUCCESSFUL;
    }

    PepCtrlLog("DriverEntry - Initializing the driver object callback functions.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = lPepCtrlIrpCreate;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = lPepCtrlIrpDeviceControl;
    pDriverObject->MajorFunction[IRP_MJ_POWER] = lPepCtrlIrpPower;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = lPepCtrlIrpClose;
    pDriverObject->DriverUnload = lPepCtrlDriverUnload;

    PepCtrlLog("DriverEntry leaving.  (Finished initializing the driver  (Thread: 0x%p))\n",
		       PsGetCurrentThread());

    return STATUS_SUCCESS;
}

#pragma endregion

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
