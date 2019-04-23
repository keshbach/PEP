/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include <Drivers/PepCtrlDefs.h>
#include <Drivers/PepCtrlIOCTL.h>

#include "PepCtrlPortData.h"

#include "PepCtrlDeviceControl.h"

#include "PepCtrlLogic.h"
#include "PepCtrlLog.h"

static DRIVER_CANCEL lCancelIrpRoutine;

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetProgrammerMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetVccMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetPinPulseMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetVppMode)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetAddress)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_GetData)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetData)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_TriggerProgram)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetOutputEnable)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_GetDeviceStatus)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_DeviceNotification)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_GetSettings)
#pragma alloc_text (PAGE, PepCtrlDeviceControl_SetSettings)
#endif

static VOID lCancelIrpRoutine(
  IN OUT PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp)
{
	TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lCancelIrpRoutine called.\n");

	IoReleaseCancelSpinLock(pIrp->CancelIrql);

    ExAcquireFastMutexUnsafe(&pPortData->FastMutex);

	if (pPortData->pIrp == pIrp)
	{
		pPortData->pIrp = NULL;
	}

	ExReleaseFastMutexUnsafe(&pPortData->FastMutex);

	pIrp->IoStatus.Status = STATUS_CANCELLED;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}

NTSTATUS PepCtrlDeviceControl_SetProgrammerMode(
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

    PepCtrlLog("PepCtrlDeviceControl_SetProgrammerMode called.\n");

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

NTSTATUS PepCtrlDeviceControl_SetVccMode(
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

    PepCtrlLog("PepCtrlDeviceControl_SetVccMode called.\n");

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

NTSTATUS PepCtrlDeviceControl_SetPinPulseMode(
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

    PepCtrlLog("PepCtrlDeviceControl_SetPinPulseMode called.\n");

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

NTSTATUS PepCtrlDeviceControl_SetVppMode(
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

    PepCtrlLog("PepCtrlDeviceControl_SetVppMode called.\n");

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

NTSTATUS PepCtrlDeviceControl_SetAddress(
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

    PepCtrlLog("PepCtrlDeviceControl_SetAddress called.\n");

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

NTSTATUS PepCtrlDeviceControl_GetData(
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

    PepCtrlLog("PepCtrlDeviceControl_GetData called.\n");

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

NTSTATUS PepCtrlDeviceControl_SetData(
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

    PepCtrlLog("PepCtrlDeviceControl_SetData called.\n");

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

NTSTATUS PepCtrlDeviceControl_TriggerProgram(
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

    PepCtrlLog("PepCtrlDeviceControl_TriggerProgram called.\n");

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

NTSTATUS PepCtrlDeviceControl_SetOutputEnable(
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

    PepCtrlLog("PepCtrlDeviceControl_SetOutputEnable called.\n");

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

NTSTATUS PepCtrlDeviceControl_GetDeviceStatus(
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

    PepCtrlLog("PepCtrlDeviceControl_GetPortStatus called.\n");

	if (ulOutBufLen == sizeof(UINT32))
	{
		*((UINT32*)pvOutBuf) = pPortData->bPortEjected ? CPepCtrlDeviceNotPresent : CPepCtrlDevicePresent;

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

NTSTATUS PepCtrlDeviceControl_DeviceNotification(
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

    PepCtrlLog("PepCtrlDeviceControl_DeviceNotification called.\n");

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
        PepCtrlLog("PepCtrlDeviceControl_DeviceNotification - Saving off the IRP.\n");

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

NTSTATUS PepCtrlDeviceControl_GetSettings(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData, 
  IN const PVOID pvInBuf, 
  IN ULONG ulInBufLen, 
  OUT PVOID pvOutBuf, 
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    size_t PortDeviceNameLen;
    UINT32 Length;

    pvInBuf;
    ulInBufLen;

    PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_GetSettings called.\n");

    RtlStringCchLengthW(pPortData->RegSettings.pszPortDeviceName, NTSTRSAFE_MAX_CCH, &PortDeviceNameLen);

    Length = sizeof(UINT32) + (((UINT32)PortDeviceNameLen + 1) * sizeof(WCHAR));

    if (ulOutBufLen == sizeof(UINT32))
    {
        *((UINT32*)pvOutBuf) = Length;

        pIrp->IoStatus.Information = ulOutBufLen;

        Status = STATUS_SUCCESS;
    }
    else if (ulOutBufLen >= Length)
    {
        *((UINT32*)pvOutBuf) = pPortData->RegSettings.nPortType;

        RtlStringCchCopyW((LPWSTR)(((UINT32*)pvOutBuf) + 1), PortDeviceNameLen + 1,
                          pPortData->RegSettings.pszPortDeviceName);

        pIrp->IoStatus.Information = Length;

        Status = STATUS_SUCCESS;
    }
    else 
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS PepCtrlDeviceControl_SetSettings(
  IN PIRP pIrp,
  IN TPepCtrlPortData* pPortData,
  IN const PVOID pvInBuf,
  IN ULONG ulInBufLen,
  OUT PVOID pvOutBuf,
  IN ULONG ulOutBufLen)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    pIrp;
    pPortData;
    pvInBuf;
    ulInBufLen;
    pvOutBuf;
    ulOutBufLen;

    PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetSettings called.\n");

    // verify the settings appear to be correct

    if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
    {
    }

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
