/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
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

static DRIVER_CANCEL lDeviceControlCancelIrpRoutine;

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

#pragma region "Local Functions"

static VOID lDeviceControlCancelIrpRoutine(
  _In_ _Out_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp)
{
	TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lDeviceControlCancelIrpRoutine entering.\n");

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

    PepCtrlLog("lDeviceControlCancelIrpRoutine leaving.\n");
}

#pragma endregion

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetProgrammerMode called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetVccMode called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetPinPulseMode called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetVppMode called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetAddress called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_GetData called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetData called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_TriggerProgram called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetOutputEnable called.\n");

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

	return Status;
}

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

	PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_GetPortStatus called.\n");

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

	return Status;
}

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

		IoSetCancelRoutine(pIrp, lDeviceControlCancelIrpRoutine);

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

    PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_GetSettings entering.\n");

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

        PepCtrlLog("PepCtrlDeviceControl_GetSettings leaving (buffer too small).\n");

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

    PepCtrlLog("PepCtrlDeviceControl_GetSettings leaving (settings retrieved).\n");

    return Status;
}

NTSTATUS PepCtrlDeviceControl_SetSettings(
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

    PAGED_CODE()

    PepCtrlLog("PepCtrlDeviceControl_SetSettings entering.\n");

    if (ulInBufLen < sizeof(TPepCtrlPortSettings))
    {
        Status = STATUS_BUFFER_TOO_SMALL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_SetSettings leaving (Buffer too small.)\n");

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

        PepCtrlLog("PepCtrlDeviceControl_SetSettings leaving (Invalid port type.)\n");

        return Status;
    }

    if (pPortSettings->nPortType != CPepCtrlNoPortType)
    {
        PepCtrlLog("PepCtrlDeviceControl_SetSettings - Validating the port data.\n");

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

            PepCtrlLog("PepCtrlDeviceControl_SetSettings leaving (Port device name not null terminated.)\n");

            return Status;
        }

        if (ulDeviceNameLen == 1)
        {
            Status = STATUS_UNSUCCESSFUL;

            pIrp->IoStatus.Status = Status;

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);

            PepCtrlLog("PepCtrlDeviceControl_SetSettings leaving (Port device name is empty.)\n");

            return Status;
        }
    }
    else
    {
        ulDeviceNameLen = 1;
    }

    if (pPortData->RegSettings.nPortType != CPepCtrlNoPortType)
    {
        PepCtrlLog("PepCtrlDeviceControl_SetSettings releasing the existing device.\n");

        pPortData->nState = CPepCtrlStateChangePortSettings;

        if (PepCtrlPlugPlayUnregister(pPortData->pvPlugPlayData))
        {
            PepCtrlLog("PepCtrlDeviceControl_SetSettings - Plug and Play notification unregistered.\n");
        }
        else 
        {
            PepCtrlLog("PepCtrlDeviceControl_SetSettings - Could not unregister the Plug and Play notification.\n");
        }

        pPortData->nState = CPepCtrlStateDeviceControl;

        pPortData->RegSettings.nPortType = CPepCtrlNoPortType;
    }
    else
    {
        PepCtrlLog("PepCtrlDeviceControl_SetSettings no existing device to release.\n");
    }

    if (pPortData->RegSettings.pszPortDeviceName)
    {
        PepCtrlLog("PepCtrlDeviceControl_SetSettings - Deleting the memory allocated for the port's device name.\n");

        UtFreePagedMem(pPortData->RegSettings.pszPortDeviceName);

        pPortData->RegSettings.pszPortDeviceName = NULL;
    }

    RtlInitUnicodeString(&RegistryPath, pPortData->RegSettings.pszRegistryPath);

    PepCtrlLog("PepCtrlDeviceControl_SetSettings - saving the new registry settings.\n");

    if (!PepCtrlWriteRegSettings(&RegistryPath, pPortSettings->nPortType,
                                 pPortSettings->cPortDeviceName))
    {
        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_SetSettings leaving (Could not save the new registry settings.)\n");

        return Status;
    }

    pszDeviceName = (LPWSTR)UtAllocPagedMem(ulDeviceNameLen * sizeof(WCHAR));

    if (pszDeviceName == NULL)
    {
        Status = STATUS_UNSUCCESSFUL;

        pIrp->IoStatus.Status = Status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        PepCtrlLog("PepCtrlDeviceControl_SetSettings leaving (Could not allocate memory for the port's device name.)\n");

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

        PepCtrlLog("PepCtrlDeviceControl_SetSettings - Registering for plug and Play notification.\n");

        if (PepCtrlPlugPlayRegister(pPortData->Funcs.pGetDeviceInterfaceGuidFunc(),
                                    pPortData->RegSettings.pszPortDeviceName,
                                    pPortData->pvPlugPlayData))
        {
            PepCtrlLog("PepCtrlDeviceControl_SetSettings - Plug and Play notification registered.\n");
        }
        else
        {
            PepCtrlLog("PepCtrlDeviceControl_SetSettings - Plug and Play notification failed to register.\n");
        }
    }

    Status = STATUS_SUCCESS;

    pIrp->IoStatus.Status = Status;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlDeviceControl_SetSettings leaving.\n");

    return Status;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
