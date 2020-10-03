/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include <Utils/UtHeapDriver.h>

#include <Drivers/PepCtrlDefs.h>
#include <Drivers/PepCtrlIOCTL.h>

#include <UtilsPep/UtPepLogic.h>

#include "PepCtrlPortData.h"

#include "PepCtrlDeviceControl.h"

#include "PepCtrlPlugPlay.h"

#include "PepCtrlInit.h"

#include "PepCtrlReg.h"

#include "PepCtrlLog.h"

#include "PepCtrlHelper.h"

static DRIVER_CANCEL lDeviceControlCancelIrpRoutine;

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetProgrammerMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetVccMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetPinPulseMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetVppMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetAddress)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetAddressWithDelay)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_GetData)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetData)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_TriggerProgram)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetOutputEnable)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_GetDeviceStatus)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_DeviceNotification)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_GetPortSettings)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetPortSettings)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetDelaySettings)
#endif

#pragma region "Local Functions"

_IRQL_requires_min_(DISPATCH_LEVEL)
static VOID lDeviceControlCancelIrpRoutine(
  _In_ _Out_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp)
{
    BOOLEAN bQuit = FALSE;
	TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lDeviceControlCancelIrpRoutine entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    while (!bQuit)
    {
        if (ExTryToAcquireFastMutex(&pPortData->FastMutex))
        {
            switch (pPortData->nState)
            {
                case CPepCtrlStateRunning:
                    if (pPortData->pIrp == pIrp)
                    {
                        PepCtrlLog("lDeviceControlCancelIrpRoutine - Clearing out the existing device notification IRP.  (Thread: 0x%p)\n",
							       PsGetCurrentThread());

                        pPortData->pIrp = NULL;
                    }

                    bQuit = TRUE;
                    break;
                case CPepCtrlStateUnloading:
                case CPepCtrlStateDeviceArrived:
                case CPepCtrlStateDeviceRemoved:
                    PepCtrlLog("lDeviceControlCancelIrpRoutine - ERROR: Invalid state of \"%s\".  (Thread: 0x%p)\n",
						       PepCtrlHelperTranslateState(pPortData->nState),
						       PsGetCurrentThread());

                    bQuit = TRUE;
                    break;
                case CPepCtrlStateDeviceControl:
                case CPepCtrlStateChangePortSettings:
                    break;
                default:
                    PepCtrlLog("lDeviceControlCancelIrpRoutine - ERROR: Unknown state of \"%s\".  (Thread: 0x%p)\n",
						       PepCtrlHelperTranslateState(pPortData->nState),
						       PsGetCurrentThread());

                    bQuit = TRUE;
                    break;
            }

            ExReleaseFastMutex(&pPortData->FastMutex);
        }
        else
        {
            switch (pPortData->nState)
            {
                case CPepCtrlStateUnloading:
                    bQuit = TRUE;
                    break;
                case CPepCtrlStateRunning:
                case CPepCtrlStateDeviceControl:
                case CPepCtrlStateChangePortSettings:
                    break;
                case CPepCtrlStateDeviceArrived:
                case CPepCtrlStateDeviceRemoved:
                    PepCtrlLog("lDeviceControlCancelIrpRoutine - ERROR: Invalid state of \"%s\".  (Thread: 0x%p)\n",
						       PepCtrlHelperTranslateState(pPortData->nState),
						       PsGetCurrentThread());

                    bQuit = TRUE;
                    break;
                default:
                    PepCtrlLog("lDeviceControlCancelIrpRoutine - ERROR: Unknown state of \"%s\".  (Thread: 0x%p)\n",
						       PepCtrlHelperTranslateState(pPortData->nState),
						       PsGetCurrentThread());

                    bQuit = TRUE;
                    break;
            }
        }
    }

	pIrp->IoStatus.Status = STATUS_CANCELLED;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lDeviceControlCancelIrpRoutine leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
}

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetProgrammerMode(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetProgrammerMode entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulInBufLen == sizeof(UINT32))
		{
			if (UtPepLogicSetProgrammerMode(&pPortData->LogicData, *((PUINT32)pvInBuf)))
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

	PepCtrlLog("PepCtrlDeviceControl_SetProgrammerMode leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetVccMode(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetVccMode entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulInBufLen == sizeof(UINT32))
		{
			if (UtPepLogicSetVccMode(&pPortData->LogicData, *((PUINT32)pvInBuf)))
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

	PepCtrlLog("PepCtrlDeviceControl_SetVccMode leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetPinPulseMode(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetPinPulseMode entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulInBufLen == sizeof(UINT32))
		{
			if (UtPepLogicSetPinPulseMode(&pPortData->LogicData, *((PUINT32)pvInBuf)))
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

	PepCtrlLog("PepCtrlDeviceControl_SetPinPulseMode leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetVppMode(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetVppMode entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulInBufLen == sizeof(UINT32))
		{
			if (UtPepLogicSetVppMode(&pPortData->LogicData, *((PUINT32)pvInBuf)))
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

	PepCtrlLog("PepCtrlDeviceControl_SetVppMode leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetAddress(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetAddress entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulInBufLen == sizeof(UINT32))
		{
			if (UtPepLogicSetAddress(&pPortData->LogicData, *(UINT32*)pvInBuf))
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

	PepCtrlLog("PepCtrlDeviceControl_SetAddress leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetAddressWithDelay(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	TPepCtrlAddressWithDelay* pAddressWithDelay;

	pvOutBuf;
	ulOutBufLen;

	PepCtrlLog("PepCtrlDeviceControl_SetAddressWithDelay entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

	if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
	{
		if (ulInBufLen == sizeof(TPepCtrlAddressWithDelay))
		{
			pAddressWithDelay = (TPepCtrlAddressWithDelay*)pvInBuf;

			if (UtPepLogicSetAddressWithDelay(&pPortData->LogicData,
				                              pAddressWithDelay->nAddress,
				                              pAddressWithDelay->nDelayNanoseconds))
			{
				Status = STATUS_SUCCESS;
			}
		}
		else
		{
			if (ulOutBufLen < sizeof(TPepCtrlAddressWithDelay))
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

	PepCtrlLog("PepCtrlDeviceControl_SetAddressWithDelay leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_GetData(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvInBuf;
	ulInBufLen;

    PepCtrlLog("PepCtrlDeviceControl_GetData entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulOutBufLen == sizeof(UCHAR))
		{
			if (UtPepLogicGetData(&pPortData->LogicData, (UCHAR*)pvOutBuf))
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

	PepCtrlLog("PepCtrlDeviceControl_GetData leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetData(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetData entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulInBufLen == sizeof(UCHAR))
		{
			if (UtPepLogicSetData(&pPortData->LogicData, *((PUCHAR)pvInBuf)))
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

	PepCtrlLog("PepCtrlDeviceControl_SetData leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_TriggerProgram(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	BOOLEAN bProgramSuccess = FALSE;

	pvInBuf;
	ulInBufLen;

    PepCtrlLog("PepCtrlDeviceControl_TriggerProgram entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulOutBufLen == sizeof(UINT32))
		{
			if (UtPepLogicTriggerProgram(&pPortData->LogicData, &bProgramSuccess))
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

	PepCtrlLog("PepCtrlDeviceControl_TriggerProgram leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetOutputEnable(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetOutputEnable entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData))
    {
		if (ulInBufLen == sizeof(UINT32))
		{
			if (UtPepLogicSetOutputEnable(&pPortData->LogicData, *((PUINT32)pvInBuf)))
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

	PepCtrlLog("PepCtrlDeviceControl_SetOutputEnable leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_GetDeviceStatus(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvInBuf;
	ulInBufLen;

    PepCtrlLog("PepCtrlDeviceControl_GetPortStatus entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

	if (ulOutBufLen == sizeof(UINT32))
	{
        *((UINT32*)pvOutBuf) = (PepCtrlPlugPlayIsDevicePresent(pPortData->pvPlugPlayData)) ? CPepCtrlDevicePresent : CPepCtrlDeviceNotPresent;

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

	PepCtrlLog("PepCtrlDeviceControl_GetPortStatus leaving.  (Thread: 0x%p)\n");

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_DeviceNotification(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvInBuf;
	ulInBufLen;
	pvOutBuf;
	ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_DeviceNotification entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

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
        PepCtrlLog("PepCtrlDeviceControl_DeviceNotification - Saving off the IRP.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		pPortData->pIrp = pIrp;

		IoMarkIrpPending(pIrp);

		IoSetCancelRoutine(pIrp, lDeviceControlCancelIrpRoutine);

		Status = STATUS_PENDING;

		pIrp->IoStatus.Status = Status;
	}
	else
	{
		PepCtrlLog("PepCtrlDeviceControl_DeviceNotification - IRP already present, cannot save IRP off.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		pIrp->IoStatus.Status = Status;

		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	}

	PepCtrlLog("PepCtrlDeviceControl_DeviceNotification leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_GetPortSettings(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData, 
  _In_ const PVOID pvInBuf, 
  _In_ ULONG ulInBufLen, 
  _Out_ PVOID pvOutBuf, 
  _In_ ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    size_t PortDeviceNameLen = 0;
    UINT32 Length;
    TPepCtrlPortSettings* pPortSettings;

    pvInBuf;
    ulInBufLen;

    PepCtrlLog("PepCtrlDeviceControl_GetPortSettings entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    Length = sizeof(UINT32);

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType ||
        pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        RtlStringCchLengthW(pPortData->RegSettings.pszPortDeviceName, NTSTRSAFE_MAX_CCH, &PortDeviceNameLen);

        Length += (((UINT32)PortDeviceNameLen + 1) * sizeof(WCHAR));
    }

    if (ulOutBufLen < Length)
    {
        Status = STATUS_BUFFER_TOO_SMALL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_GetPortSettings leaving.  (Buffer too small)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return Status;
    }

    pPortSettings = (TPepCtrlPortSettings*)pvOutBuf;

    pPortSettings->nPortType = pPortData->RegSettings.nPortType;

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType ||
        pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        RtlStringCchCopyW(pPortSettings->cPortDeviceName, 
                          PortDeviceNameLen + 1,
                          pPortData->RegSettings.pszPortDeviceName);
    }
    else
    {
        pPortSettings->cPortDeviceName[0] = 0;
    }

    Status = STATUS_SUCCESS;

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = ulOutBufLen;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlDeviceControl_GetPortSettings leaving.  (Settings retrieved)  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetPortSettings(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    TPepCtrlPortSettings* pPortSettings;
    ULONG ulBufferLen, ulDeviceNameLen;
    BOOLEAN bDeviceNameValid;
    UNICODE_STRING RegistryPath;
    LPWSTR pszDeviceName;

    pIrp;
    pvOutBuf;
    ulOutBufLen;

    PepCtrlLog("PepCtrlDeviceControl_SetPortSettings entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    if (ulInBufLen < sizeof(TPepCtrlPortSettings))
    {
        Status = STATUS_BUFFER_TOO_SMALL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings leaving.  (Buffer too small)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return Status;
    }

    pPortSettings = (TPepCtrlPortSettings*)pvInBuf;

    if (pPortSettings->nPortType != CPepCtrlNoPortType &&
        pPortSettings->nPortType != CPepCtrlParallelPortType &&
        pPortSettings->nPortType != CPepCtrlUsbPrintPortType)
    {
        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings leaving.  (Invalid port type)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return Status;
    }

    if (pPortSettings->nPortType != CPepCtrlNoPortType)
    {
        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - Validating the port data.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        ulBufferLen = ulInBufLen - UFIELD_OFFSET(TPepCtrlPortSettings, cPortDeviceName);
        ulDeviceNameLen = 0;
        bDeviceNameValid = FALSE;

        for (ULONG ulIndex = 0; !bDeviceNameValid && ulIndex < ulBufferLen; ++ulIndex)
        {
            if (pPortSettings->cPortDeviceName[ulIndex] == 0)
            {
                bDeviceNameValid = TRUE;
            }

            ++ulDeviceNameLen;
        }

        if (!bDeviceNameValid)
        {
            Status = STATUS_UNSUCCESSFUL;

            pIrp->IoStatus.Status = Status;

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);

            PepCtrlLog("PepCtrlDeviceControl_SetPortSettings leaving.  (Port device name not null terminated)  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            return Status;
        }

        if (ulDeviceNameLen == 1)
        {
            Status = STATUS_UNSUCCESSFUL;

            pIrp->IoStatus.Status = Status;

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);

            PepCtrlLog("PepCtrlDeviceControl_SetPortSettings leaving.  (Port device name is empty)  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            return Status;
        }
    }
    else
    {
        ulDeviceNameLen = 1;
    }

    pPortData->nState = CPepCtrlStateChangePortSettings;

    if (pPortData->RegSettings.nPortType != CPepCtrlNoPortType)
    {
        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings releasing the existing device.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        if (PepCtrlPlugPlayUnregister(pPortData->pvPlugPlayData))
        {
            PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - Plug and Play notification unregistered.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else 
        {
            PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - Could not unregister the Plug and Play notification.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }

        pPortData->RegSettings.nPortType = CPepCtrlNoPortType;
    }
    else
    {
        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings no existing device to release.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    if (pPortData->RegSettings.pszPortDeviceName)
    {
        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - Deleting the memory allocated for the port's device name.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        UtFreePagedMem(pPortData->RegSettings.pszPortDeviceName);

        pPortData->RegSettings.pszPortDeviceName = NULL;
    }

    RtlInitUnicodeString(&RegistryPath, pPortData->RegSettings.pszRegistryPath);

    PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - saving the new registry settings.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (!PepCtrlWriteRegSettings(&RegistryPath, pPortSettings->nPortType,
                                 pPortSettings->cPortDeviceName))
    {
        pPortData->nState = CPepCtrlStateDeviceControl;

        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings leaving.  (Could not save the new registry settings)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return Status;
    }

    pszDeviceName = (LPWSTR)UtAllocPagedMem(ulDeviceNameLen * sizeof(WCHAR));

    if (pszDeviceName == NULL)
    {
        pPortData->nState = CPepCtrlStateDeviceControl;

        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings leaving.  (Could not allocate memory for the port's device name)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return Status;
    }

    if (pPortSettings->nPortType != CPepCtrlNoPortType)
    {
        RtlStringCbCopyW(pszDeviceName, ulDeviceNameLen * sizeof(WCHAR),
                         pPortSettings->cPortDeviceName);
    }
    else
    {
        *pszDeviceName = 0;
    }

    pPortData->RegSettings.nPortType = pPortSettings->nPortType;
    pPortData->RegSettings.pszPortDeviceName = pszDeviceName;

    if (pPortSettings->nPortType != CPepCtrlNoPortType)
    {
        PepCtrlInitPortTypeFuncs(pPortData);

        PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - Registering for plug and Play notification.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        if (PepCtrlPlugPlayRegister(pPortData->Funcs.pGetDeviceInterfaceGuidFunc(),
                                    pPortData->RegSettings.pszPortDeviceName,
                                    pPortData->pvPlugPlayData))
        {
            PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - Plug and Play notification registered.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else
        {
            PepCtrlLog("PepCtrlDeviceControl_SetPortSettings - Plug and Play notification failed to register.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
    }

    pPortData->nState = CPepCtrlStateDeviceControl;

    Status = STATUS_SUCCESS;

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlDeviceControl_SetPortSettings leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetDelaySettings(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_ PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	TPepCtrlDelaySettings* pDelaySettings;

	pIrp;
	pvOutBuf;
	ulOutBufLen;

	PepCtrlLog("PepCtrlDeviceControl_SetDelaySettings entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

	if (ulInBufLen < sizeof(TPepCtrlDelaySettings))
	{
		Status = STATUS_BUFFER_TOO_SMALL;

		pIrp->IoStatus.Status = Status;

		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		PepCtrlLog("PepCtrlDeviceControl_SetDelaySettings leaving.  (Buffer too small)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		return Status;
	}

	if (ulInBufLen > sizeof(TPepCtrlDelaySettings))
	{
		Status = STATUS_ARRAY_BOUNDS_EXCEEDED;

		pIrp->IoStatus.Status = Status;

		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		PepCtrlLog("PepCtrlDeviceControl_SetDelaySettings leaving.  (Buffer too large)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		return Status;
	}

	pDelaySettings = (TPepCtrlDelaySettings*)pvInBuf;

	if (UtPepLogicSetDelays(&pPortData->LogicData,
		                    pDelaySettings->nChipEnableNanoseconds,
		                    pDelaySettings->nOutputEnableNanoseconds))
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_UNSUCCESSFUL;
	}

	pIrp->IoStatus.Status = Status;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	PepCtrlLog("PepCtrlDeviceControl_SetDelaySettings leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return Status;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
