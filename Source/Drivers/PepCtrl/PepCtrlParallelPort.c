/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>
#include <parallel.h>

#include <Utils/UtHeapDriver.h>

#include "PepCtrlPortData.h"
#include "PepCtrlParallelPort.h"

#include "PepCtrlLog.h"

#define CTimeoutTicks 1000000 // 100 milliseconds

#define CBusyStatusBit 0x80

/*
   Parallel Port Addresses Macros
*/

#define MDataAddress(x) (x)
#define MStatusAddress(x) (x + 1)
#define MControlAddress(x) (x + 2)

/*
   Local Functions
*/

static BOOLEAN lSetParallelPortMode(IN PDEVICE_OBJECT pPortDeviceObject);
static PPARALLEL_PORT_INFORMATION lAllocParallelPortInfo(IN PDEVICE_OBJECT pDeviceObject);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lSetParallelPortMode)
#pragma alloc_text (PAGE, lAllocParallelPortInfo)

#pragma alloc_text (PAGE, PepCtrlReadBitParallelPort)
#pragma alloc_text (PAGE, PepCtrlWriteParallelPort)
#pragma alloc_text (PAGE, PepCtrlAllocParallelPort)
#pragma alloc_text (PAGE, PepCtrlFreeParallelPort)
#pragma alloc_text (PAGE, PepCtrlGetParallelPortDevInterfaceGuid)
#endif

static GUID l_ParallelGuid = {0};

static NTSTATUS lParallelPortIoCompletion(
  IN PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp,
  IN PVOID pvContext)
{
    PepCtrlLog("lParallelPortIoCompletion called.\n");

	pDeviceObject;
	pIrp;

	KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

static BOOLEAN lSetParallelPortMode(
  IN PDEVICE_OBJECT pPortDeviceObject)
{
	BOOLEAN bResult = FALSE;
    NTSTATUS status;
    PARALLEL_PNP_INFORMATION ParallelPnpInfo;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;
	LARGE_INTEGER TimeoutInteger;

    PAGED_CODE()

    PepCtrlLog("lSetParallelPortMode called.\n");

    RtlZeroMemory(&ParallelPnpInfo, sizeof(ParallelPnpInfo));

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lSetParallelPortMode - Building IO Control Request\n");

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO,
                                         pPortDeviceObject, NULL, 0,
                                         &ParallelPnpInfo, sizeof(ParallelPnpInfo),
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        PepCtrlLog("lSetParallelPortMode - IRP could not be allocated\n");

        return FALSE;
    }

    PepCtrlLog("lSetParallelPortMode - Setting the completion routine\n");

	IoSetCompletionRoutine(pIrp, lParallelPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lSetParallelPortMode - Calling the port device driver to obtain PNP information\n");

    status = IoCallDriver(pPortDeviceObject, pIrp);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lSetParallelPortMode - Call to IoCallDriver returned status pending\n");

		TimeoutInteger.QuadPart = -CTimeoutTicks;

		status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);
		
		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("lSetParallelPortMode - Call to IoCallDriver timed out\n");

            PepCtrlLog("lSetParallelPortMode - Cancelling the IRP\n");

			IoCancelIrp(pIrp);

            PepCtrlLog("lSetParallelPortMode - Waiting for the IRP to be cancelled\n");

			KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
		}
    }

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("lSetParallelPortMode - Call to IoCallDriver succeeded\n");

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("lSetParallelPortMode - IO Status Block succeeded.\n");

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("lSetParallelPortMode - IO Status Block failed.  (Error Code: 0x%X)\n", IoStatusBlock.Status);
			}
		}
		else
		{
            PepCtrlLog("lSetParallelPortMode - Call to IoCallDriver succeeded.  (Error Code: 0x%X)\n", status);
		}
	}
	else
	{
        PepCtrlLog("lSetParallelPortMode - Call to IoCallDriver failed.  (Error Code: 0x%X)\n", status);
	}

	KeClearEvent(&Event);

    PepCtrlLog("lSetParallelPortMode - Completing the IRP.\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lSetParallelPortMode - Waiting for the IRP to complete.\n");

	KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

	if (!bResult)
	{
		return FALSE;
	}

    PepCtrlLog("lSetParallelPortMode - Checking parallel port's hardware capabilities.\n");

    if (!(ParallelPnpInfo.HardwareCapabilities & PPT_BYTE_PRESENT))
    {
        PepCtrlLog("lSetParallelPortMode - Parallel Port does not support byte mode.\n");

        return FALSE;
    }

    if (ParallelPnpInfo.CurrentMode != ECR_SPP_MODE)
    {
        PepCtrlLog("lSetParallelPortMode - Clearing the parallel port's chip mode.\n");

        status = ParallelPnpInfo.ClearChipMode(ParallelPnpInfo.Context,
                                               (UCHAR)ParallelPnpInfo.CurrentMode);

        if (!NT_SUCCESS(status))
        {
            PepCtrlLog("lSetParallelPortMode - Clearing the chip mode failed.  (Error Code: 0x%X)\n", status);

            return FALSE;
        }

        PepCtrlLog("lSetParallelPortMode - Trying to set the parallel port's chip mode.\n");

        status = ParallelPnpInfo.TrySetChipMode(ParallelPnpInfo.Context,
                                                ECR_SPP_MODE);

        if (!NT_SUCCESS(status))
        {
            PepCtrlLog("lSetParallelPortMode - Setting the chip mode failed.  (Error Code: 0x%X)\n", status);

            return FALSE;
        }
    }

    return TRUE;
}

static PPARALLEL_PORT_INFORMATION lAllocParallelPortInfo(
  IN PDEVICE_OBJECT pDeviceObject)
{
	BOOLEAN bResult = FALSE;
	PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    PIRP pIrp;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG ulParallelPortInfoLen;
	LARGE_INTEGER TimeoutInteger;

    PAGED_CODE()

    PepCtrlLog("lAllocParallelPortInfo called.\n");

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    PepCtrlLog("lAllocParallelPortInfo - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lAllocParallelPortInfo - Building IO Control Request IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO\n");

    ulParallelPortInfoLen = sizeof(PARALLEL_PORT_INFORMATION);
    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)UtAllocNonPagedMem(ulParallelPortInfoLen);

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO,
                                         pDeviceObject, NULL, 0,
                                         pParallelPortInfo, ulParallelPortInfoLen,
                                         TRUE, &Event, &IoStatusBlock);

	if (!pIrp)
	{
        PepCtrlLog("lAllocParallelPortInfo - IRP could not be allocated\n");

		UtFreeNonPagedMem(pParallelPortInfo);

		return NULL;
	}

    PepCtrlLog("lAllocParallelPortInfo - Setting the completion routine\n");

	IoSetCompletionRoutine(pIrp, lParallelPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lAllocParallelPortInfo - Calling the port device driver\n");

    status = IoCallDriver(pDeviceObject, pIrp);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver returned status pending\n");

		TimeoutInteger.QuadPart = -CTimeoutTicks;

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver timed out\n");

            PepCtrlLog("lAllocParallelPortInfo - Cancelling the IRP\n");

			IoCancelIrp(pIrp);

            PepCtrlLog("lAllocParallelPortInfo - Waiting for the IRP to be cancelled\n");

			KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
		}
    }

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver succeeded\n");

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("lAllocParallelPortInfo - IO Status Block succeeded.\n");

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("lAllocParallelPortInfo - IO Status Block failed.  (Error Code: 0x%X)\n", IoStatusBlock.Status);
			}
		}
		else
		{
            PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver succeeded.  (Error Code: 0x%X)\n", status);
		}
	}
	else
	{
        PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver failed.  (Error Code: 0x%X)\n", status);
	} 

	KeClearEvent(&Event);

    PepCtrlLog("lAllocParallelPortInfo - Completing the IRP.\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lAllocParallelPortInfo - Waiting for the IRP to complete.\n");

	KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lAllocParallelPortInfo - Finished waiting for the IRP to complete.\n");

	if (bResult)
	{
		return pParallelPortInfo;
	}

    UtFreeNonPagedMem(pParallelPortInfo);

    return NULL;
}

BOOLEAN TPEPCTRLAPI PepCtrlReadBitParallelPort(
  IN TPepCtrlObject* pObject,
  OUT PBOOLEAN pbValue)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    UCHAR ucStatus;

    PAGED_CODE()

    PepCtrlLog("PepCtrlReadBitParallelPort called.\n");

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    ucStatus = READ_PORT_UCHAR(MStatusAddress(pParallelPortInfo->Controller));

    *pbValue = (ucStatus & CBusyStatusBit) ? TRUE : FALSE;

    return TRUE;
}

BOOLEAN TPEPCTRLAPI PepCtrlWriteParallelPort(
  IN TPepCtrlObject* pObject,
  IN PUCHAR pucData,
  IN ULONG ulDataLen)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    ULONG ulIndex;

    PAGED_CODE()

    PepCtrlLog("PepCtrlWriteParallelPort called.\n");

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    for (ulIndex = 0; ulIndex < ulDataLen; ++ulIndex)
    {
    	WRITE_PORT_UCHAR(MDataAddress(pParallelPortInfo->Controller),
                         pucData[ulIndex]);
    }

    return TRUE;
}

BOOLEAN TPEPCTRLAPI PepCtrlAllocParallelPort(
  IN TPepCtrlObject* pObject,
  IN LPCWSTR pszDeviceName)
{
    NTSTATUS status;
    UNICODE_STRING DeviceName;
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;

    PAGED_CODE()

    PepCtrlLog("PepCtrlAllocParallelPort called.\n");

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    PepCtrlLog("PepCtrlAllocParallelPort - Getting Device Object pointer to \"%ws\"\n", pszDeviceName);

    status = IoGetDeviceObjectPointer(&DeviceName, GENERIC_ALL,
                                      &pObject->pPortFileObject,
                                      &pObject->pPortDeviceObject);

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("PepCtrlAllocParallelPort - Got Device Object pointer to \"%ws\"\n", pszDeviceName);

        pObject->pvObjectData = lAllocParallelPortInfo(pObject->pPortDeviceObject);

        if (!pObject->pvObjectData)
        {
            PepCtrlLog("PepCtrlAllocParallelPort - Call to lAllocParallelPortInfo failed.\n");

            goto FreeDevice;
        }

        pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

        PepCtrlLog("Port Address:        0x%X.\n", pParallelPortInfo->OriginalController);
        PepCtrlLog("System Port Address: 0x%X.\n", pParallelPortInfo->Controller);

        PepCtrlLog("PepCtrlAllocParallelPort - Trying to allocate the parallel port.\n");

		if (pParallelPortInfo->TryAllocatePort(pObject->pPortDeviceObject->DeviceExtension))
		{
            PepCtrlLog("PepCtrlAllocParallelPort - Parallel port allocated.\n");

            if (lSetParallelPortMode(pObject->pPortDeviceObject))
            {
                PepCtrlLog("PepCtrlAllocParallelPort - Parallel Port Mode set.\n");

                return TRUE;
            }
            else
            {
                PepCtrlLog("PepCtrlAllocParallelPort - Parallel Port Mode could not be changed - freeing the port.\n");

                pParallelPortInfo->FreePort(pObject->pPortDeviceObject->DeviceExtension);
            }
        }
        else
        {
            PepCtrlLog("PepCtrlAllocParallelPort - Could not allocate the parallel port\n");
        }

        UtFreeNonPagedMem(pParallelPortInfo);

FreeDevice:
        ObDereferenceObject(pObject->pPortFileObject);
    }
    else
    {
        PepCtrlLog("PepCtrlAllocParallelPort - Result of calling get device object pointer  (Error Code: 0x%X)\n", status);
    }

    pObject->pPortFileObject = NULL;
    pObject->pPortDeviceObject = NULL;
    pObject->pvObjectData = NULL;

    return FALSE;
}

BOOLEAN TPEPCTRLAPI PepCtrlFreeParallelPort(
  IN TPepCtrlObject* pObject)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;

    PAGED_CODE()

    PepCtrlLog("PepCtrlFreeParallelPort called.\n");

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    PepCtrlLog("PepCtrlFreeParallelPort - Printer port being released.\n");

    pParallelPortInfo->FreePort(pObject->pPortDeviceObject->DeviceExtension);	

    UtFreeNonPagedMem(pParallelPortInfo);

    ObDereferenceObject(pObject->pPortFileObject);

    pObject->pPortFileObject = NULL;
    pObject->pPortDeviceObject = NULL;
    pObject->pvObjectData = NULL;

    return TRUE;
}

LPGUID TPEPCTRLAPI PepCtrlGetParallelPortDevInterfaceGuid(VOID)
{
    PepCtrlLog("PepCtrlGetParallelPortDevInterfaceGuid called.\n");

    PAGED_CODE()

    RtlCopyBytes(&l_ParallelGuid, &GUID_DEVINTERFACE_PARALLEL,
                 sizeof(l_ParallelGuid));

    return &l_ParallelGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
