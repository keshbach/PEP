/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

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

typedef struct tagTDeviceControlCancelWorkItemData
{
	PIO_WORKITEM pWorkItem;
	PIRP pIrp;
} TDeviceControlCancelWorkItemData;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#pragma endregion

static DRIVER_CANCEL lDeviceControlCancelIrp;

static IO_WORKITEM_ROUTINE lDeviceControlCancelWorkItem;

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lDeviceControlCancelWorkItem)

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
#pragma alloc_text (PAGE, PepCtrlDeviceControl_DebugWritePortData)
#endif

#pragma region "Local Functions"

_Use_decl_annotations_
static VOID lDeviceControlCancelIrp(
  _Inout_ PDEVICE_OBJECT pDeviceObject,
  _Inout_ _IRQL_uses_cancel_ PIRP pIrp)
{
	TDeviceControlCancelWorkItemData* pDeviceControlCancelWorkItemData;

	IoReleaseCancelSpinLock(pIrp->CancelIrql);

	pDeviceControlCancelWorkItemData = (TDeviceControlCancelWorkItemData*)UtAllocNonPagedMem(sizeof(TDeviceControlCancelWorkItemData));

	if (pDeviceControlCancelWorkItemData)
	{
		pDeviceControlCancelWorkItemData->pWorkItem = IoAllocateWorkItem(pDeviceObject);

		if (pDeviceControlCancelWorkItemData->pWorkItem)
		{
			pDeviceControlCancelWorkItemData->pIrp = pIrp;

			IoQueueWorkItem(pDeviceControlCancelWorkItemData->pWorkItem, lDeviceControlCancelWorkItem,
			                DelayedWorkQueue, pDeviceControlCancelWorkItemData);
		}
		else
		{
			UtFreeNonPagedMem(pDeviceControlCancelWorkItemData);
		}
	}
}

_Use_decl_annotations_
static void lDeviceControlCancelWorkItem(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_opt_ PVOID pvContext)
{
	BOOLEAN bQuit = FALSE;
	TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
	TDeviceControlCancelWorkItemData* pDeviceControlCancelWorkItemData = (TDeviceControlCancelWorkItemData*)pvContext;
	INT32 nOriginalState;
	PIRP pOriginalPortDataIrp;

	PepCtrlLog("lDeviceControlCancelWorkItem entering.  (Thread: 0x%p)\n",
               PsGetCurrentThread());

	PAGED_CODE()

	if (pvContext == NULL)
	{
		PepCtrlLog("lDeviceControlCancelWorkItem leaving.  (pvContext is null.  Thread: 0x%p)\n",
			       PsGetCurrentThread());

		return;
    }

	while (!bQuit)
	{
		if (ExTryToAcquireFastMutex(&pPortData->FastMutex))
		{
			nOriginalState = pPortData->nState;

			switch (pPortData->nState)
			{
				case CPepCtrlStateRunning:
					pOriginalPortDataIrp = pPortData->pIrp;

					if (pPortData->pIrp == pDeviceControlCancelWorkItemData->pIrp)
					{
						pPortData->pIrp = NULL;
					}

					bQuit = TRUE;
					break;
				case CPepCtrlStateUnloading:
				case CPepCtrlStateDeviceArrived:
				case CPepCtrlStateDeviceRemoved:
					bQuit = TRUE;
					break;
				case CPepCtrlStateDeviceControl:
				case CPepCtrlStateChangePortSettings:
					break;
				default:
					bQuit = TRUE;
					break;
			}

			ExReleaseFastMutex(&pPortData->FastMutex);

			switch (nOriginalState)
			{
    			case CPepCtrlStateRunning:
					PepCtrlLog("lDeviceControlCancelWorkItem - Clearing out the existing device notification IRP.  (Thread: 0x%p)\n",
						       PsGetCurrentThread());

					pDeviceControlCancelWorkItemData->pIrp->IoStatus.Status = STATUS_CANCELLED;
					pDeviceControlCancelWorkItemData->pIrp->IoStatus.Information = 0;

					IoCompleteRequest(pDeviceControlCancelWorkItemData->pIrp, IO_NO_INCREMENT);
					break;
				case CPepCtrlStateUnloading:
				case CPepCtrlStateDeviceArrived:
				case CPepCtrlStateDeviceRemoved:
					PepCtrlLog("lDeviceControlCancelWorkItem - ERROR: Invalid state of \"%s\".  (Thread: 0x%p)\n",
						       PepCtrlHelperTranslateState(pPortData->nState), PsGetCurrentThread());
					break;
				case CPepCtrlStateDeviceControl:
				case CPepCtrlStateChangePortSettings:
					break;
				default:
					PepCtrlLog("lDeviceControlCancelWorkItem - ERROR: Unknown state of \"%s\".  (Thread: 0x%p)\n",
					           PepCtrlHelperTranslateState(pPortData->nState), PsGetCurrentThread());
					break;
			}
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
					PepCtrlLog("lDeviceControlCancelWorkItem - ERROR: Invalid state of \"%s\".  (Thread: 0x%p)\n",
						       PepCtrlHelperTranslateState(pPortData->nState), PsGetCurrentThread());

					bQuit = TRUE;
					break;
				default:
					PepCtrlLog("lDeviceControlCancelWorkItem - ERROR: Unknown state of \"%s\".  (Thread: 0x%p)\n",
						       PepCtrlHelperTranslateState(pPortData->nState), PsGetCurrentThread());

					bQuit = TRUE;
					break;
			}
		}
	}

	IoFreeWorkItem(pDeviceControlCancelWorkItemData->pWorkItem);

	UtFreeNonPagedMem(pDeviceControlCancelWorkItemData);

	PepCtrlLog("lDeviceControlCancelWorkItem leaving.  (Thread: 0x%p)\n",
               PsGetCurrentThread());
}

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetProgrammerMode(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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

	PepCtrlLog("PepCtrlDeviceControl_GetPortStatus leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread);

	return Status;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_DeviceNotification(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	pvInBuf;
	ulInBufLen;
	pvOutBuf;

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

	*((UINT32*)pvOutBuf) = 0; // Suppress C6101 warning

    if (pPortData->pIrp == NULL)
	{
        PepCtrlLog("PepCtrlDeviceControl_DeviceNotification - Saving off the IRP.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		pPortData->pIrp = pIrp;

		IoMarkIrpPending(pIrp);

		IoSetCancelRoutine(pIrp, lDeviceControlCancelIrp);

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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    size_t PortDeviceNameLen = 0;
    size_t Length;
    TPepCtrlPortSettings* pPortSettings;

    pvInBuf;
    ulInBufLen;

    PepCtrlLog("PepCtrlDeviceControl_GetPortSettings entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    Length = sizeof(Length);

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType ||
        pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        RtlStringCchLengthW(pPortData->RegSettings.pszPortDeviceName, NTSTRSAFE_MAX_CCH, &PortDeviceNameLen);

        Length += ((PortDeviceNameLen + 1) * sizeof(WCHAR));
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
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

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_DebugWritePortData(
  _In_ PIRP pIrp,
  _In_ TPepCtrlPortData* pPortData,
  _In_ const PVOID pvInBuf,
  _In_ ULONG ulInBufLen,
  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
  _In_ ULONG ulOutBufLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	UCHAR* pcData;

	pIrp;
	pvOutBuf;
	ulOutBufLen;

	PepCtrlLog("PepCtrlDeviceControl_DebugWritePortData entering.  (Thread: 0x%p)\n",
               PsGetCurrentThread());

	PAGED_CODE()

    if (ulInBufLen < sizeof(UCHAR))
	{
		Status = STATUS_BUFFER_TOO_SMALL;

		pIrp->IoStatus.Status = Status;

		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		PepCtrlLog("PepCtrlDeviceControl_DebugWritePortData leaving.  (Buffer too small)  (Thread: 0x%p)\n",
                   PsGetCurrentThread());

		return Status;
	}

	if (ulInBufLen > sizeof(UCHAR))
	{
		Status = STATUS_ARRAY_BOUNDS_EXCEEDED;

		pIrp->IoStatus.Status = Status;

		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		PepCtrlLog("PepCtrlDeviceControl_DebugWritePortData leaving.  (Buffer too large)  (Thread: 0x%p)\n",
                   PsGetCurrentThread());

		return Status;
	}

	pcData = (UCHAR*)pvInBuf;

	if (pPortData->Funcs.pWritePortFunc(&pPortData->Object, pcData, sizeof(UCHAR), 0))
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_UNSUCCESSFUL;
	}

	pIrp->IoStatus.Status = Status;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	PepCtrlLog("PepCtrlDeviceControl_DebugWritePortData leaving.  (Thread: 0x%p)\n",
               PsGetCurrentThread());

	return Status;
}

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
