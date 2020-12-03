/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

#include <ntddk.h>

#include <ntstrsafe.h>
#include <usbprint.h>

#include "PepCtrlPortData.h"
#include "PepCtrlUsbPort.h"

#include "PepCtrlLog.h"

#include <Includes/UtMacros.h>

#pragma region "Constants"

#define CTimeoutMs 100

#pragma endregion

/*
   Local Functions
*/

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lResetUsbPort(_In_ TPepCtrlObject* pObject);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadDeviceId(_In_ TPepCtrlObject* pObject);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, lResetUsbPort)
#pragma alloc_text (PAGE, lReadDeviceId)

#pragma alloc_text (PAGE, PepCtrlReadBitUsbPort)
#pragma alloc_text (PAGE, PepCtrlWriteUsbPort)
#pragma alloc_text (PAGE, PepCtrlAllocUsbPort)
#pragma alloc_text (PAGE, PepCtrlFreeUsbPort)
#pragma alloc_text (PAGE, PepCtrlGetUsbPortDevInterfaceGuid)
#endif

#pragma region "Local Variables"

#pragma data_seg("PAGEDATA")
#pragma bss_seg("PAGEBSS")

static GUID l_UsbPrintGuid;

#pragma data_seg()
#pragma bss_seg()

#pragma endregion

#pragma region "Local Functions"

_IRQL_requires_max_(DISPATCH_LEVEL)
static NTSTATUS lUsbPortIoCompletion(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp,
  _In_ PVOID pvContext)
{
    PepCtrlLog("lUsbPortIoCompletion entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pDeviceObject;
	pIrp;

    PepCtrlLog("lUsbPortIoCompletion - setting the event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

    PepCtrlLog("lUsbPortIoCompletion - finished setting the event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PepCtrlLog("lUsbPortIoCompletion leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return STATUS_MORE_PROCESSING_REQUIRED;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lResetUsbPort(
  _In_ TPepCtrlObject* pObject)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;
    LARGE_INTEGER TimeoutInteger;

    PepCtrlLog("lResetUsbPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    PepCtrlLog("lResetUsbPort - Initializing event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lResetUsbPort - Building Device IO Control Request.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_SOFT_RESET,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, NULL, 0,
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        PepCtrlLog("lResetUsbPort leaving (IRP could not be allocated).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("lResetUsbPort - Setting the completion routine.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lResetUsbPort - Calling IoCallDriver.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("lResetUsbPort - Finished calling IoCallDriver (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lResetUsbPort - Call to IoCallDriver returned status pending.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("lResetUsbPort - Waiting for the IoCallDriver event to be set.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lResetUsbPort - Finished waiting for the IoCallDriver event to be set  (Error Code: 0x%X).  (Thread: 0x%p)\n",
                   status, PsGetCurrentThread());

        if (status == STATUS_TIMEOUT)
        {
            PepCtrlLog("lResetUsbPort - Call to IoCallDriver timed out.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            PepCtrlLog("lResetUsbPort - Cancelling the IRP.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("lResetUsbPort - IRP cancel routine found and called.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
            else
            {
                PepCtrlLog("lResetUsbPort - IRP cancel bit set.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }

            PepCtrlLog("lResetUsbPort - Waiting for the IRP to be cancelled.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lResetUsbPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());

            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
        if (status == STATUS_SUCCESS)
        {
            PepCtrlLog("lResetUsbPort - Call to IoCallDriver succeeded  (IO Status Block: 0x%X).  (Thread: 0x%p)\n",
				       IoStatusBlock.Status, PsGetCurrentThread());

            if (NT_SUCCESS(IoStatusBlock.Status))
            {
                PepCtrlLog("lResetUsbPort - IO Status Block succeeded.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("lResetUsbPort - IO Status Block failed.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
        }
        else
        {
            PepCtrlLog("lResetUsbPort - Call to IoCallDriver succeeded.  (Error Code: 0x%X)  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());
        }
    }
    else
    {
        PepCtrlLog("lResetUsbPort - Call to IoCallDriver failed.  (Error Code: 0x%X)  (Thread: 0x%p)\n",
                   status, PsGetCurrentThread());
    }

    PepCtrlLog("lResetUsbPort - Completing the IRP.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lResetUsbPort - Waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lResetUsbPort - Finished waiting for the IRP to complete  (Error Code: 0x%X)  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    PepCtrlLog("lResetUsbPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadDeviceId(
  _In_ TPepCtrlObject* pObject)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;
    LARGE_INTEGER TimeoutInteger;
    UINT8 data[50];

    PepCtrlLog("lReadDeviceId entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    PepCtrlLog("lReadDeviceId - Initializing event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lReadDeviceId - Building Device IO Control Request.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_GET_1284_ID,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, data, sizeof(data),
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        PepCtrlLog("lReadDeviceId leaving.  (IRP could not be allocated)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("lReadDeviceId - Setting the completion routine.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lReadDeviceId - Calling IoCallDriver.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("lReadDeviceId - Finished calling IoCallDriver (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lReadDeviceId - Call to IoCallDriver returned status pending.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("lReadDeviceId - Waiting for the IoCallDriver event to be set.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lReadDeviceId - Finished waiting for the IoCallDriver event to be set (Error Code: 0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());

        if (status == STATUS_TIMEOUT)
        {
            PepCtrlLog("lReadDeviceId - Call to IoCallDriver timed out.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            PepCtrlLog("lReadDeviceId - Cancelling the IRP.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("lReadDeviceId - IRP cancel routine found and called.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
            else
            {
                PepCtrlLog("lReadDeviceId - IRP cancel bit set.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }

            PepCtrlLog("lReadDeviceId - Waiting for the IRP to be cancelled.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lReadDeviceId - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());

            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
        if (status == STATUS_SUCCESS)
        {
            PepCtrlLog("lReadDeviceId - Call to IoCallDriver succeeded  (IO Status Block: 0x%X).  (Thread: 0x%p)\n",
				       IoStatusBlock.Status, PsGetCurrentThread());

            if (NT_SUCCESS(IoStatusBlock.Status))
            {
                PepCtrlLog("lReadDeviceId - IO Status Block succeeded.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("lReadDeviceId - IO Status Block failed.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
        }
        else
        {
            PepCtrlLog("lReadDeviceId - Call to IoCallDriver succeeded.  (Error Code: 0x%X)  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());
        }
    }
    else
    {
        PepCtrlLog("lReadDeviceId - Call to IoCallDriver failed.  (Error Code: 0x%X)  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
    }

    PepCtrlLog("lReadDeviceId - Completing the IRP.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lReadDeviceId - Waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lReadDeviceId - Finished waiting for the IRP to complete  (Error Code: 0x%X)  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    PepCtrlLog("lReadDeviceId leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlReadBitUsbPort(
  _In_ TPepCtrlObject* pObject,
  _Out_ PBOOLEAN pbValue)
{
	BOOLEAN bResult = FALSE;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;
	LARGE_INTEGER TimeoutInteger;

    PepCtrlLog("PepCtrlReadBitUsbPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	PAGED_CODE()

    PepCtrlLog("PepCtrlReadBitUsbPort - Initializing event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("PepCtrlReadBitUsbPort - Building Device IO Control Request. (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_GET_LPT_STATUS,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, pbValue, sizeof(*pbValue),
                                         TRUE, &Event, &IoStatusBlock);

	if (!pIrp)
	{
        PepCtrlLog("PepCtrlReadBitUsbPort leaving (IRP could not be allocated).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		return FALSE;
	}

    PepCtrlLog("PepCtrlReadBitUsbPort - Setting the completion routine.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("PepCtrlReadBitUsbPort - Calling IoCallDriver.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("PepCtrlReadBitUsbPort - Finished calling IoCallDriver  (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver returned status pending.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IoCallDriver event to be set.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("PepCtrlReadBitUsbPort - Finished waiting for the IoCallDriver event to be set  (Error Code: 0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver timed out.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            PepCtrlLog("PepCtrlReadBitUsbPort - Cancelling the IRP.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("PepCtrlReadBitUsbPort - IRP cancel routine found and called.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
            else
            {
                PepCtrlLog("PepCtrlReadBitUsbPort - IRP cancel bit set.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }

            PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IRP to be cancelled.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

			status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        
            PepCtrlLog("PepCtrlReadBitUsbPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());

            status = STATUS_UNSUCCESSFUL;
        }
    }

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver succeeded  (IO Status Block: 0x%X).  (Thread: 0x%p)\n",
				       IoStatusBlock.Status, PsGetCurrentThread());

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("PepCtrlReadBitUsbPort - IO Status Block succeeded.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("PepCtrlReadBitUsbPort - IO Status Block failed.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
			}
        }
		else
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver succeeded.  (Error Code: 0x%X)  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());
		}
	}
	else
	{
        PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver failed.  (Error Code: 0x%X)  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
	}

    PepCtrlLog("PepCtrlReadBitUsbPort - Completing the IRP.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("PepCtrlReadBitUsbPort - Finished waiting for the IRP to complete  (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    PepCtrlLog("PepCtrlReadBitUsbPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlWriteUsbPort(
  _In_ TPepCtrlObject* pObject,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen,
  _In_ ULONG ulWaitNanoSeconds)
{
	BOOLEAN bResult = FALSE;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER OffsetInteger, TimeoutInteger;
    PIRP pIrp;

	PepCtrlLog("PepCtrlWriteUsbPort entering.  (Thread: 0x%p)\n",
               PsGetCurrentThread());

	PAGED_CODE()

	// TODO: Implement support for waiting.
	ulWaitNanoSeconds;

    PepCtrlLog("PepCtrlWriteUsbPort - Initializing event.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("PepCtrlWriteUsbPort - Initializing offset large integer.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    OffsetInteger.QuadPart = 0;

    PepCtrlLog("PepCtrlWriteUsbPort - Building IO Control Request.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE,
                                        pObject->pPortDeviceObject,
                                        pucData, ulDataLen, &OffsetInteger,
                                        &Event, &IoStatusBlock);

	if (!pIrp) 
	{
        PepCtrlLog("PepCtrlWriteUsbPort leaving (IRP could not be allocated).  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

		return FALSE;
	}

    PepCtrlLog("PepCtrlWriteUsbPort - Setting the completion routine.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("PepCtrlWriteUsbPort - Calling the port device driver.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("PepCtrlWriteUsbPort - Finished calling the port device driver  (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver returned status pending.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IoCallDriver event to be set.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("PepCtrlWriteUsbPort - Finished waiting for the IoCallDriver event to be set  (Error Code: 0x%X).  (Thread: 0x%p)\n",
                   status, PsGetCurrentThread());

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver timed out.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            PepCtrlLog("PepCtrlWriteUsbPort - Cancelling the IRP.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

			if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("PepCtrlWriteUsbPort - IRP cancel routine found and called.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }
            else
            {
                PepCtrlLog("PepCtrlWriteUsbPort - IRP cancel bit set.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
            }

            PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IRP to be cancelled.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

			status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("PepCtrlWriteUsbPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());

            status = STATUS_UNSUCCESSFUL;
        }
	}

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver succeeded  (IO Status Block: 0x%X).  (Thread: 0x%p)\n",
				       IoStatusBlock.Status, PsGetCurrentThread());

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("PepCtrlWriteUsbPort - IO Status Block succeeded.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("PepCtrlWriteUsbPort - IO Status Block failed.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
			}
		}
		else
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver succeeded  (Error Code: 0x%X).  (Thread: 0x%p)\n",
				       status, PsGetCurrentThread());
		}
	}
	else
	{
        PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver failed  (Error Code: 0x%X).  (Thread: 0x%p)\n",
			       status, PsGetCurrentThread());
	}

    PepCtrlLog("PepCtrlWriteUsbPort - Completing the IRP.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IRP to complete.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("PepCtrlWriteUsbPort - Finished waiting for the IRP to complete  (Error Code: 0x%X).  (Thread: 0x%p)\n",
		       status, PsGetCurrentThread());

    PepCtrlLog("PepCtrlWriteUsbPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlAllocUsbPort(
  _In_ TPepCtrlObject* pObject,
  _In_ LPCWSTR pszDeviceName)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING DeviceName;

	PepCtrlLog("PepCtrlAllocUsbPort entering.  (Thread: 0x%p)\n",
               PsGetCurrentThread());

    PAGED_CODE()

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    PepCtrlLog("PepCtrlAllocUsbPort - Getting Device Object pointer to \"%ws\".  (Thread: 0x%p)\n",
		       pszDeviceName, PsGetCurrentThread());

    status = IoGetDeviceObjectPointer(&DeviceName, GENERIC_ALL,
                                      &pObject->pPortFileObject,
                                      &pObject->pPortDeviceObject);

    PepCtrlLog("PepCtrlAllocUsbPort - Finished getting Device Object pointer to \"%ws\"  (Error Code: 0x%X).  (Thread: 0x%p)\n",
               pszDeviceName, status, PsGetCurrentThread());

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("PepCtrlAllocUsbPort - Got Device Object pointer to \"%ws\".  (Thread: 0x%p)\n",
			       pszDeviceName, PsGetCurrentThread());

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("PepCtrlAllocUsbPort - Failed to get device object pointer.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("PepCtrlAllocUsbPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlFreeUsbPort(
  _In_ TPepCtrlObject* pObject)
{
	PepCtrlLog("PepCtrlFreeUsbPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
	
	PAGED_CODE()

    PepCtrlLog("PepCtrlFreeUsbPort - USB Printer port being released.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    ObDereferenceObject(pObject->pPortFileObject);

    PepCtrlLog("PepCtrlFreeUsbPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
LPGUID TPEPCTRLAPI PepCtrlGetUsbPortDevInterfaceGuid(VOID)
{
    PepCtrlLog("PepCtrlGetUsbPortDevInterfaceGuid entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    RtlCopyBytes(&l_UsbPrintGuid, &GUID_DEVINTERFACE_USBPRINT,
                 sizeof(l_UsbPrintGuid));

    PepCtrlLog("PepCtrlGetUsbPortDevInterfaceGuid leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return &l_UsbPrintGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
