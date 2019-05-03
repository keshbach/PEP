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

static PPARALLEL_PORT_INFORMATION lAllocParallelPortInfo(_In_ PDEVICE_OBJECT pDeviceObject);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lAllocParallelPortInfo)

#pragma alloc_text (PAGE, PepCtrlReadBitParallelPort)
#pragma alloc_text (PAGE, PepCtrlWriteParallelPort)
#pragma alloc_text (PAGE, PepCtrlAllocParallelPort)
#pragma alloc_text (PAGE, PepCtrlFreeParallelPort)
#pragma alloc_text (PAGE, PepCtrlGetParallelPortDevInterfaceGuid)
#endif

static GUID l_ParallelGuid = {0};

static NTSTATUS lParallelPortIoCompletion(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp,
  _In_ PVOID pvContext)
{
    PepCtrlLog("lParallelPortIoCompletion called.\n");

	pDeviceObject;
	pIrp;

    PepCtrlLog("lParallelPortIoCompletion - setting the event.\n");

	KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

    PepCtrlLog("lParallelPortIoCompletion - finished setting the event.\n");

    PepCtrlLog("lParallelPortIoCompletion finished.\n");

	return STATUS_MORE_PROCESSING_REQUIRED;
}

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
		return pParallelPortInfo;
	}

    UtFreeNonPagedMem(pParallelPortInfo);

    return NULL;
}

BOOLEAN TPEPCTRLAPI PepCtrlReadBitParallelPort(
  _In_ TPepCtrlObject* pObject,
  _Out_ PBOOLEAN pbValue)
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
  _In_ TPepCtrlObject* pObject,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen)
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
  _In_ TPepCtrlObject* pObject,
  _In_ LPCWSTR pszDeviceName)
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

    PepCtrlLog("PepCtrlAllocParallelPort - Finished getting Device Object pointer to \"%ws\" (Error Code: 0x%X)\n",
               pszDeviceName, status);

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

            return TRUE;
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
        PepCtrlLog("PepCtrlAllocParallelPort - Result of calling get device object pointer\n");
    }

    pObject->pPortFileObject = NULL;
    pObject->pPortDeviceObject = NULL;
    pObject->pvObjectData = NULL;

    return FALSE;
}

BOOLEAN TPEPCTRLAPI PepCtrlFreeParallelPort(
  _In_ TPepCtrlObject* pObject)
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
