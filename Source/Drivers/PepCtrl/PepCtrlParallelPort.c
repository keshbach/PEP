/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

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
    PepCtrlLog("lParallelPortIoCompletion entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	pDeviceObject;
	pIrp;

    PepCtrlLog("lParallelPortIoCompletion - Setting the event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

    PepCtrlLog("lParallelPortIoCompletion - Finished setting the event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PepCtrlLog("lParallelPortIoCompletion leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

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

    PepCtrlLog("lAllocParallelPortInfo entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    PepCtrlLog("lAllocParallelPortInfo - Initializing event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lAllocParallelPortInfo - Building IO Control Request IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    ulParallelPortInfoLen = sizeof(PARALLEL_PORT_INFORMATION);

    PepCtrlLog("lAllocParallelPortInfo - Allocating memory for the PARALLEL_PORT_INFORMATION structure (Total Bytes: 0x%X).  (Thread: 0x%p)\n",
               ulParallelPortInfoLen, PsGetCurrentThread());

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)UtAllocPagedMem(ulParallelPortInfoLen);

    if (pParallelPortInfo == NULL)
    {
        PepCtrlLog("lAllocParallelPortInfo leaving.  (Could not allocate memory for the PARALLEL_PORT_INFORMATION structure)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return NULL;
    }

    PepCtrlLog("lAllocParallelPortInfo - Memory allocated for the PARALLEL_PORT_INFORMATION structure.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    RtlZeroMemory(pParallelPortInfo, ulParallelPortInfoLen);

    PepCtrlLog("lAllocParallelPortInfo - Allocating the IO Control Request IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO,
                                         pDeviceObject, NULL, 0,
                                         pParallelPortInfo, ulParallelPortInfoLen,
                                         TRUE, &Event, &IoStatusBlock);

	if (!pIrp)
	{
        PepCtrlLog("lAllocParallelPortInfo - Freeing memory allocated for the PARALLEL_PORT_INFORMATION structure.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		UtFreePagedMem(pParallelPortInfo);

        PepCtrlLog("lAllocParallelPortInfo leaving.  (IRP could not be allocated)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		return NULL;
	}

    PepCtrlLog("lAllocParallelPortInfo - Setting the completion routine.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	IoSetCompletionRoutine(pIrp, lParallelPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lAllocParallelPortInfo - Calling the port device driver.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = IoCallDriver(pDeviceObject, pIrp);

    PepCtrlLog("lAllocParallelPortInfo - Finished calling IoCallDriver (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver returned status pending.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lAllocParallelPortInfo - Finished waiting for the IRP to complete (Error Code: 0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver timed out.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            PepCtrlLog("lAllocParallelPortInfo - Cancelling the IRP.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

			IoCancelIrp(pIrp);

            PepCtrlLog("lAllocParallelPortInfo - Waiting for the IRP to be cancelled.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

			status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lAllocParallelPortInfo - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());

            status = STATUS_UNSUCCESSFUL;
        }
    }

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver succeeded.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("lAllocParallelPortInfo - IO Status Block succeeded.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("lAllocParallelPortInfo - IO Status Block failed  (Error Code: 0x%X).  (Thread: 0x%p)\n",
					       IoStatusBlock.Status, PsGetCurrentThread());
			}
		}
		else
		{
            PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver succeeded  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());
		}
	}
	else
	{
        PepCtrlLog("lAllocParallelPortInfo - Call to IoCallDriver failed  (Error Code: 0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
	} 

    PepCtrlLog("lAllocParallelPortInfo - Completing the IRP.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lAllocParallelPortInfo - Waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lAllocParallelPortInfo - Finished waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	if (bResult)
	{
        PepCtrlLog("lAllocParallelPortInfo leaving (Parallel port allocated).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		return pParallelPortInfo;
	}

    PepCtrlLog("lAllocParallelPortInfo - Freeing memory allocated for the PARALLEL_PORT_INFORMATION structure.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    UtFreePagedMem(pParallelPortInfo);

    PepCtrlLog("lAllocParallelPortInfo leaving  (Parallel port could not be allocated).  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

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

    PepCtrlLog("lAllocParallelPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    PepCtrlLog("lAllocParallelPort - Initializing event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    
    PepCtrlLog("lAllocParallelPort - Allocating IO Control Request IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE,
                                         pDeviceObject, NULL, 0, NULL, 0,
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        PepCtrlLog("lAllocParallelPort leaving (IRP could not be allocated).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("lAllocParallelPort - Setting the completion routine.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoSetCompletionRoutine(pIrp, lParallelPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lAllocParallelPort - Calling the port device driver.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = IoCallDriver(pDeviceObject, pIrp);

    PepCtrlLog("lAllocParallelPort - Finished calling IoCallDriver (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lAllocParallelPort - Call to IoCallDriver returned status pending.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lAllocParallelPort - Finished waiting for the IRP to be completed (Error Code: 0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());

        if (status == STATUS_TIMEOUT)
        {
            PepCtrlLog("lAllocParallelPort - Call to IoCallDriver timed out.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            PepCtrlLog("lAllocParallelPort - Cancelling the IRP.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            IoCancelIrp(pIrp);

            PepCtrlLog("lAllocParallelPort - Waiting for the IRP to be cancelled.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lAllocParallelPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());

            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
        if (status == STATUS_SUCCESS)
        {
            PepCtrlLog("lAllocParallelPort - Call to IoCallDriver succeeded.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            if (NT_SUCCESS(IoStatusBlock.Status))
            {
                PepCtrlLog("lAllocParallelPort - IO Status Block succeeded.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("lAllocParallelPort - IO Status Block failed  (Error Code: 0x%X).  (Thread: 0x%p)\n",
					       IoStatusBlock.Status, PsGetCurrentThread());
            }
        }
        else
        {
            PepCtrlLog("lAllocParallelPort - Call to IoCallDriver succeeded  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());
        }
    }
    else
    {
        PepCtrlLog("lAllocParallelPort - Call to IoCallDriver failed  (Error Code: 0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
    }

    PepCtrlLog("lAllocParallelPort - Completing the IRP.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lAllocParallelPort - Waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lAllocParallelPort - Finished waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PepCtrlLog("lAllocParallelPortInfo leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

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

    PepCtrlLog("PepCtrlReadBitParallelPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    ucStatus = READ_PORT_UCHAR(MStatusAddress(pParallelPortInfo->Controller));

    *pbValue = (ucStatus & CBusyStatusBit) ? TRUE : FALSE;

    PepCtrlLog("PepCtrlReadBitParallelPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

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

    PepCtrlLog("PepCtrlWriteParallelPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

	Interval.QuadPart = ulWaitNanoSeconds;

    for (ulIndex = 0; ulIndex < ulDataLen; ++ulIndex)
    {
    	WRITE_PORT_UCHAR(MDataAddress(pParallelPortInfo->Controller),
                         pucData[ulIndex]);

		if (!UtSleep(&Interval))
		{
			PepCtrlLog("PepCtrlWriteParallelPort - Sleep failed.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
		}
	}

    PepCtrlLog("PepCtrlWriteParallelPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

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

    PepCtrlLog("PepCtrlAllocParallelPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    PepCtrlLog("PepCtrlAllocParallelPort - Getting Device Object pointer to \"%ws\".  (Thread: 0x%p)\n",
		       pszDeviceName, PsGetCurrentThread());

    status = IoGetDeviceObjectPointer(&DeviceName, GENERIC_ALL,
                                      &pObject->pPortFileObject,
                                      &pObject->pPortDeviceObject);

    PepCtrlLog("PepCtrlAllocParallelPort - Finished getting Device Object pointer to \"%ws\"  (Error Code: 0x%X).  (Thread: 0x%p)\n",
               pszDeviceName, status, PsGetCurrentThread());

    if (status != STATUS_SUCCESS)
    {
        PepCtrlLog("PepCtrlAllocParallelPort leaving  (Could not get Device Object pointer).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("PepCtrlAllocParallelPort - Attempting to allocate the parallel port.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (!lAllocParallelPort(pObject->pPortDeviceObject))
    {
        ObDereferenceObject(pObject->pPortFileObject);

        pObject->pPortFileObject = NULL;
        pObject->pPortDeviceObject = NULL;

        PepCtrlLog("PepCtrlAllocParallelPort leaving  (Failed to allocate the parallel port).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("PepCtrlAllocParallelPort - Parallel port allocated.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PepCtrlLog("PepCtrlAllocParallelPort - Attempting to allocate the parallel port information.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pObject->pvObjectData = lAllocParallelPortInfo(pObject->pPortDeviceObject);

    if (!pObject->pvObjectData)
    {
        ObDereferenceObject(pObject->pPortFileObject);

        pObject->pPortFileObject = NULL;
        pObject->pPortDeviceObject = NULL;

        PepCtrlLog("PepCtrlAllocParallelPort leaving  (Failed to allocate the parallel port information).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("PepCtrlAllocParallelPort - Successfully allocated the parallel port information.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

#if defined(AMD64)
	PepCtrlLog("Port Address:        0x%p.\n", (PVOID)(pParallelPortInfo->OriginalController.QuadPart));
#elif defined(i386)
	PepCtrlLog("Port Address:        0x%p.\n", (PVOID)(pParallelPortInfo->OriginalController.LowPart));
#else
#error Unknown CPU architecture
#endif

    PepCtrlLog("System Port Address: 0x%p.\n", pParallelPortInfo->Controller);

    PepCtrlLog("PepCtrlAllocParallelPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlFreeParallelPort(
  _In_ TPepCtrlObject* pObject)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;

    PepCtrlLog("PepCtrlFreeParallelPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    PepCtrlLog("PepCtrlFreeParallelPort - Printer port to be freed.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pParallelPortInfo->FreePort(pObject->pPortDeviceObject->DeviceExtension);	

    PepCtrlLog("PepCtrlFreeParallelPort - Printer port freed.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    UtFreePagedMem(pParallelPortInfo);

    ObDereferenceObject(pObject->pPortFileObject);

    pObject->pPortFileObject = NULL;
    pObject->pPortDeviceObject = NULL;
    pObject->pvObjectData = NULL;

    PepCtrlLog("PepCtrlFreeParallelPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
LPGUID TPEPCTRLAPI PepCtrlGetParallelPortDevInterfaceGuid(VOID)
{
    PepCtrlLog("PepCtrlGetParallelPortDevInterfaceGuid entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    RtlCopyBytes(&l_ParallelGuid, &GUID_DEVINTERFACE_PARALLEL,
                 sizeof(l_ParallelGuid));

    PepCtrlLog("PepCtrlGetParallelPortDevInterfaceGuid leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return &l_ParallelGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
