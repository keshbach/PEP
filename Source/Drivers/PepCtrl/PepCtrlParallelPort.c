/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>
#include <parallel.h>

#include <Utils/UtHeapDriver.h>
#include <Utils/UtSleepDriver.h>

#include <Includes/UtMacros.h>

#include "PepCtrlPortData.h"
#include "PepCtrlParallelPort.h"

#include "PepCtrlLog.h"

#pragma region "Constants"

#define CTimeoutMs 5000

#define CBusyStatusBit 0x80

#pragma endregion

#pragma region "Macros"

/*
   Parallel Port Addresses Macros
*/

#define MDataAddress(x) (x)
#define MStatusAddress(x) (x + 1)
#define MControlAddress(x) (x + 2)

#pragma endregion

/*
   Local Functions
*/

_IRQL_requires_max_(PASSIVE_LEVEL)
static PPARALLEL_PORT_INFORMATION lAllocParallelPortInfo(_In_ PDEVICE_OBJECT pDeviceObject);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lAllocParallelPort(_In_ PDEVICE_OBJECT pDeviceObject);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lAllocParallelPortInfo)
#pragma alloc_text (PAGE, lAllocParallelPort)

#pragma alloc_text (PAGE, PepCtrlReadBitParallelPort)
#pragma alloc_text (PAGE, PepCtrlWriteParallelPort)
#pragma alloc_text (PAGE, PepCtrlAllocParallelPort)
#pragma alloc_text (PAGE, PepCtrlFreeParallelPort)
#pragma alloc_text (PAGE, PepCtrlGetParallelPortDevInterfaceGuid)
#endif

#pragma region "Local Variables"

#pragma data_seg("PAGEDATA")
#pragma bss_seg("PAGEBSS")

static GUID l_ParallelGuid;

#pragma data_seg()
#pragma bss_seg()

#pragma endregion

#pragma region "Local Functions"

_IRQL_requires_max_(DISPATCH_LEVEL)
static NTSTATUS lParallelPortIoCompletion(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp,
  _In_ PVOID pvContext)
{
    PepCtrlLog("lParallelPortIoCompletion entering.\n");

	pDeviceObject;
	pIrp;

    PepCtrlLog("lParallelPortIoCompletion - setting the event.\n");

	KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

    PepCtrlLog("lParallelPortIoCompletion - finished setting the event.\n");

    PepCtrlLog("lParallelPortIoCompletion leaving.\n");

	return STATUS_MORE_PROCESSING_REQUIRED;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static PPARALLEL_PORT_INFORMATION lAllocParallelPortInfo(
  _In_ PDEVICE_OBJECT pDeviceObject)
{
	BOOLEAN bResult = FALSE;
	PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    PIRP pIrp;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG ulParallelPortInfoLen;
	LARGE_INTEGER TimeoutInteger;

    PepCtrlLog("lAllocParallelPortInfo entering.\n");

    PAGED_CODE()

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    PepCtrlLog("lAllocParallelPortInfo - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lAllocParallelPortInfo - Building IO Control Request IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO\n");

    ulParallelPortInfoLen = sizeof(PARALLEL_PORT_INFORMATION);

    PepCtrlLog("lAllocParallelPortInfo - Allocating memory for the PARALLEL_PORT_INFORMATION structure (Total Bytes: 0x%X)\n",
               ulParallelPortInfoLen);

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)UtAllocPagedMem(ulParallelPortInfoLen);

    if (pParallelPortInfo == NULL)
    {
        PepCtrlLog("lAllocParallelPortInfo leaving (Could not allocate memory for the PARALLEL_PORT_INFORMATION structure)\n");

        return NULL;
    }

    PepCtrlLog("lAllocParallelPortInfo - Memory allocated for the PARALLEL_PORT_INFORMATION structure\n");

    RtlZeroMemory(pParallelPortInfo, ulParallelPortInfoLen);

    PepCtrlLog("lAllocParallelPortInfo - Allocating the IO Control Request IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO\n");

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO,
                                         pDeviceObject, NULL, 0,
                                         pParallelPortInfo, ulParallelPortInfoLen,
                                         TRUE, &Event, &IoStatusBlock);

	if (!pIrp)
	{
        PepCtrlLog("lAllocParallelPortInfo - Freeing memory allocated for the PARALLEL_PORT_INFORMATION structure\n");

		UtFreePagedMem(pParallelPortInfo);

        PepCtrlLog("lAllocParallelPortInfo leaving (IRP could not be allocated)\n");

		return NULL;
	}

    PepCtrlLog("lAllocParallelPortInfo - Setting the completion routine\n");

	IoSetCompletionRoutine(pIrp, lParallelPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lAllocParallelPortInfo - Calling the port device driver\n");

    status = IoCallDriver(pDeviceObject, pIrp);

    PepCtrlLog("lAllocParallelPortInfo - Finished calling IoCallDriver (Error Code: 0x%X)\n", status);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver returned status pending\n");

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lAllocParallelPortInfo - Finished waiting for the IRP to complete (Error Code: 0x%X)\n", status);

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver timed out\n");

            PepCtrlLog("lAllocParallelPortInfo - Cancelling the IRP\n");

			IoCancelIrp(pIrp);

            PepCtrlLog("lAllocParallelPortInfo - Waiting for the IRP to be cancelled\n");

			status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lAllocParallelPortInfo - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X)\n", status);

            status = STATUS_UNSUCCESSFUL;
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

    PepCtrlLog("lAllocParallelPortInfo - Completing the IRP.\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lAllocParallelPortInfo - Waiting for the IRP to complete.\n");

	KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lAllocParallelPortInfo - Finished waiting for the IRP to complete.\n");

	if (bResult)
	{
        PepCtrlLog("lAllocParallelPortInfo leaving (Parallel port allocated.)\n");

		return pParallelPortInfo;
	}

    PepCtrlLog("lAllocParallelPortInfo - Freeing memory allocated for the PARALLEL_PORT_INFORMATION structure\n");

    UtFreePagedMem(pParallelPortInfo);

    PepCtrlLog("lAllocParallelPortInfo leaving (Parallel port could not be allocated).\n");

    return NULL;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lAllocParallelPort(
  _In_ PDEVICE_OBJECT pDeviceObject)
{
    BOOLEAN bResult = FALSE;
    PIRP pIrp;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER TimeoutInteger;

    PepCtrlLog("lAllocParallelPort entering\n");

    PAGED_CODE()

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    PepCtrlLog("lAllocParallelPort - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    
    PepCtrlLog("lAllocParallelPort - Allocating IO Control Request IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE\n");

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE,
                                         pDeviceObject, NULL, 0, NULL, 0,
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        PepCtrlLog("lAllocParallelPort leaving (IRP could not be allocated)\n");

        return FALSE;
    }

    PepCtrlLog("lAllocParallelPort - Setting the completion routine\n");

    IoSetCompletionRoutine(pIrp, lParallelPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lAllocParallelPort - Calling the port device driver\n");

    status = IoCallDriver(pDeviceObject, pIrp);

    PepCtrlLog("lAllocParallelPort - Finished calling IoCallDriver (Error Code: 0x%X)\n", status);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lAllocParallelPort - Call to IoCallDriver returned status pending\n");

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lAllocParallelPort - Finished waiting for the IRP to be completed (Error Code: 0x%X)\n", status);

        if (status == STATUS_TIMEOUT)
        {
            PepCtrlLog("lAllocParallelPort - Call to IoCallDriver timed out\n");

            PepCtrlLog("lAllocParallelPort - Cancelling the IRP\n");

            IoCancelIrp(pIrp);

            PepCtrlLog("lAllocParallelPort - Waiting for the IRP to be cancelled\n");

            status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lAllocParallelPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X)\n", status);

            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
        if (status == STATUS_SUCCESS)
        {
            PepCtrlLog("lAllocParallelPort - Call to IoCallDriver succeeded\n");

            if (NT_SUCCESS(IoStatusBlock.Status))
            {
                PepCtrlLog("lAllocParallelPort - IO Status Block succeeded.\n");

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("lAllocParallelPort - IO Status Block failed.  (Error Code: 0x%X)\n", IoStatusBlock.Status);
            }
        }
        else
        {
            PepCtrlLog("lAllocParallelPort - Call to IoCallDriver succeeded.  (Error Code: 0x%X)\n", status);
        }
    }
    else
    {
        PepCtrlLog("lAllocParallelPort - Call to IoCallDriver failed.  (Error Code: 0x%X)\n", status);
    }

    PepCtrlLog("lAllocParallelPort - Completing the IRP.\n");

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lAllocParallelPort - Waiting for the IRP to complete.\n");

    KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lAllocParallelPort - Finished waiting for the IRP to complete.\n");

    PepCtrlLog("lAllocParallelPortInfo leaving.\n");

    return bResult;
}

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlReadBitParallelPort(
  _In_ TPepCtrlObject* pObject,
  _Out_ PBOOLEAN pbValue)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    UCHAR ucStatus;

    PAGED_CODE()

    PepCtrlLog("PepCtrlReadBitParallelPort entering.\n");

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    ucStatus = READ_PORT_UCHAR(MStatusAddress(pParallelPortInfo->Controller));

    *pbValue = (ucStatus & CBusyStatusBit) ? TRUE : FALSE;

    PepCtrlLog("PepCtrlReadBitParallelPort leaving.\n");

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlWriteParallelPort(
  _In_ TPepCtrlObject* pObject,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen,
  _In_ ULONG ulWaitNanoSeconds)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    ULONG ulIndex;
	LARGE_INTEGER Interval;

    PepCtrlLog("PepCtrlWriteParallelPort entering.\n");

    PAGED_CODE()

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

	Interval.QuadPart = ulWaitNanoSeconds;

    for (ulIndex = 0; ulIndex < ulDataLen; ++ulIndex)
    {
    	WRITE_PORT_UCHAR(MDataAddress(pParallelPortInfo->Controller),
                         pucData[ulIndex]);

		if (!UtSleep(&Interval))
		{
			PepCtrlLog("PepCtrlWriteParallelPort - Sleep failed.\n");
		}
	}

    PepCtrlLog("PepCtrlWriteParallelPort leaving.\n");

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlAllocParallelPort(
  _In_ TPepCtrlObject* pObject,
  _In_ LPCWSTR pszDeviceName)
{
    NTSTATUS status;
    UNICODE_STRING DeviceName;
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;

    PepCtrlLog("PepCtrlAllocParallelPort entering.\n");

    PAGED_CODE()

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    PepCtrlLog("PepCtrlAllocParallelPort - Getting Device Object pointer to \"%ws\"\n", pszDeviceName);

    status = IoGetDeviceObjectPointer(&DeviceName, GENERIC_ALL,
                                      &pObject->pPortFileObject,
                                      &pObject->pPortDeviceObject);

    PepCtrlLog("PepCtrlAllocParallelPort - Finished getting Device Object pointer to \"%ws\" (Error Code: 0x%X)\n",
               pszDeviceName, status);

    if (status != STATUS_SUCCESS)
    {
        PepCtrlLog("PepCtrlAllocParallelPort leaving (Could not get Device Object pointer).\n");

        return FALSE;
    }

    PepCtrlLog("PepCtrlAllocParallelPort - Attempting to allocate the parallel port.\n");

    if (!lAllocParallelPort(pObject->pPortDeviceObject))
    {
        ObDereferenceObject(pObject->pPortFileObject);

        pObject->pPortFileObject = NULL;
        pObject->pPortDeviceObject = NULL;

        PepCtrlLog("PepCtrlAllocParallelPort leaving (Failed to allocate the parallel port.)\n");

        return FALSE;
    }

    PepCtrlLog("PepCtrlAllocParallelPort - Parallel port allocated.\n");

    PepCtrlLog("PepCtrlAllocParallelPort - Attempting to allocate the parallel port information.\n");

    pObject->pvObjectData = lAllocParallelPortInfo(pObject->pPortDeviceObject);

    if (!pObject->pvObjectData)
    {
        ObDereferenceObject(pObject->pPortFileObject);

        pObject->pPortFileObject = NULL;
        pObject->pPortDeviceObject = NULL;

        PepCtrlLog("PepCtrlAllocParallelPort leaving (Failed to allocate the parallel port information.)\n");

        return FALSE;
    }

    PepCtrlLog("PepCtrlAllocParallelPort - Successfully allocated the parallel port information.\n");

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    PepCtrlLog("Port Address:        0x%X.\n", pParallelPortInfo->OriginalController);
    PepCtrlLog("System Port Address: 0x%X.\n", pParallelPortInfo->Controller);

    PepCtrlLog("PepCtrlAllocParallelPort leaving.\n");

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlFreeParallelPort(
  _In_ TPepCtrlObject* pObject)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;

    PepCtrlLog("PepCtrlFreeParallelPort entering.\n");

    PAGED_CODE()

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    PepCtrlLog("PepCtrlFreeParallelPort - Printer port to be freed.\n");

    pParallelPortInfo->FreePort(pObject->pPortDeviceObject->DeviceExtension);	

    PepCtrlLog("PepCtrlFreeParallelPort - Printer port freed.\n");

    UtFreePagedMem(pParallelPortInfo);

    ObDereferenceObject(pObject->pPortFileObject);

    pObject->pPortFileObject = NULL;
    pObject->pPortDeviceObject = NULL;
    pObject->pvObjectData = NULL;

    PepCtrlLog("PepCtrlFreeParallelPort leaving.\n");

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
LPGUID TPEPCTRLAPI PepCtrlGetParallelPortDevInterfaceGuid(VOID)
{
    PepCtrlLog("PepCtrlGetParallelPortDevInterfaceGuid entering.\n");

    PAGED_CODE()

    RtlCopyBytes(&l_ParallelGuid, &GUID_DEVINTERFACE_PARALLEL,
                 sizeof(l_ParallelGuid));

    PepCtrlLog("PepCtrlGetParallelPortDevInterfaceGuid leaving.\n");

    return &l_ParallelGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
