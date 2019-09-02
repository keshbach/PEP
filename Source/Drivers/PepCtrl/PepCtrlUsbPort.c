/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

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
    PepCtrlLog("lUsbPortIoCompletion entering.\n");

    pDeviceObject;
	pIrp;

    PepCtrlLog("lUsbPortIoCompletion - setting the event.\n");

	KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

    PepCtrlLog("lUsbPortIoCompletion - finished setting the event.\n");

    PepCtrlLog("lUsbPortIoCompletion leaving.\n");

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

    PAGED_CODE()

    PepCtrlLog("lResetUsbPort entering.\n");

    PepCtrlLog("lResetUsbPort - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lResetUsbPort - Building Device IO Control Request\n");

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_SOFT_RESET,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, NULL, 0,
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        PepCtrlLog("lResetUsbPort leaving (IRP could not be allocated)\n");

        return FALSE;
    }

    PepCtrlLog("lResetUsbPort - Setting the completion routine\n");

    IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lResetUsbPort - Calling IoCallDriver\n");

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("lResetUsbPort - Finished calling IoCallDriver  (Error Code: 0x%X)\n", status);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lResetUsbPort - Call to IoCallDriver returned status pending\n");

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("lResetUsbPort - Waiting for the IoCallDriver event to be set\n");

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lResetUsbPort - Finished waiting for the IoCallDriver event to be set  (Error Code: 0x%X)\n", status);

        if (status == STATUS_TIMEOUT)
        {
            PepCtrlLog("lResetUsbPort - Call to IoCallDriver timed out\n");

            PepCtrlLog("lResetUsbPort - Cancelling the IRP\n");

            if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("lResetUsbPort - IRP cancel routine found and called\n");
            }
            else
            {
                PepCtrlLog("lResetUsbPort - IRP cancel bit set\n");
            }

            PepCtrlLog("lResetUsbPort - Waiting for the IRP to be cancelled\n");

            status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lResetUsbPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X)\n", status);

            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
        if (status == STATUS_SUCCESS)
        {
            PepCtrlLog("lResetUsbPort - Call to IoCallDriver succeeded  (IO Status Block: 0x%X)\n", IoStatusBlock.Status);

            if (NT_SUCCESS(IoStatusBlock.Status))
            {
                PepCtrlLog("lResetUsbPort - IO Status Block succeeded.\n");

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("lResetUsbPort - IO Status Block failed.\n");
            }
        }
        else
        {
            PepCtrlLog("lResetUsbPort - Call to IoCallDriver succeeded.  (Error Code: 0x%X)\n", status);
        }
    }
    else
    {
        PepCtrlLog("lResetUsbPort - Call to IoCallDriver failed.  (Error Code: 0x%X)\n", status);
    }

    PepCtrlLog("lResetUsbPort - Completing the IRP.\n");

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lResetUsbPort - Waiting for the IRP to complete.\n");

    status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lResetUsbPort - Finished waiting for the IRP to complete  (Error Code: 0x%X)\n", status);

    PepCtrlLog("lResetUsbPort leaving.\n");

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

    PAGED_CODE()

    PepCtrlLog("lReadDeviceId entering.\n");

    PepCtrlLog("lReadDeviceId - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("lReadDeviceId - Building Device IO Control Request\n");

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_GET_1284_ID,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, data, sizeof(data),
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        PepCtrlLog("lReadDeviceId leaving (IRP could not be allocated)\n");

        return FALSE;
    }

    PepCtrlLog("lReadDeviceId - Setting the completion routine\n");

    IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("lReadDeviceId - Calling IoCallDriver\n");

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("lReadDeviceId - Finished calling IoCallDriver  (Error Code: 0x%X)\n", status);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("lReadDeviceId - Call to IoCallDriver returned status pending\n");

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("lReadDeviceId - Waiting for the IoCallDriver event to be set\n");

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("lReadDeviceId - Finished waiting for the IoCallDriver event to be set  (Error Code: 0x%X)\n", status);

        if (status == STATUS_TIMEOUT)
        {
            PepCtrlLog("lReadDeviceId - Call to IoCallDriver timed out\n");

            PepCtrlLog("lReadDeviceId - Cancelling the IRP\n");

            if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("lReadDeviceId - IRP cancel routine found and called\n");
            }
            else
            {
                PepCtrlLog("lReadDeviceId - IRP cancel bit set\n");
            }

            PepCtrlLog("lReadDeviceId - Waiting for the IRP to be cancelled\n");

            status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("lReadDeviceId - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X)\n", status);

            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
        if (status == STATUS_SUCCESS)
        {
            PepCtrlLog("lReadDeviceId - Call to IoCallDriver succeeded  (IO Status Block: 0x%X)\n", IoStatusBlock.Status);

            if (NT_SUCCESS(IoStatusBlock.Status))
            {
                PepCtrlLog("lReadDeviceId - IO Status Block succeeded.\n");

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("lReadDeviceId - IO Status Block failed.\n");
            }
        }
        else
        {
            PepCtrlLog("lReadDeviceId - Call to IoCallDriver succeeded.  (Error Code: 0x%X)\n", status);
        }
    }
    else
    {
        PepCtrlLog("lReadDeviceId - Call to IoCallDriver failed.  (Error Code: 0x%X)\n", status);
    }

    PepCtrlLog("lReadDeviceId - Completing the IRP.\n");

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("lReadDeviceId - Waiting for the IRP to complete.\n");

    status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("lReadDeviceId - Finished waiting for the IRP to complete  (Error Code: 0x%X)\n", status);

    PepCtrlLog("lReadDeviceId leaving.\n");

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

    PAGED_CODE()

    PepCtrlLog("PepCtrlReadBitUsbPort entering.\n");

    PepCtrlLog("PepCtrlReadBitUsbPort - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("PepCtrlReadBitUsbPort - Building Device IO Control Request\n");

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_GET_LPT_STATUS,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, pbValue, sizeof(*pbValue),
                                         TRUE, &Event, &IoStatusBlock);

	if (!pIrp)
	{
        PepCtrlLog("PepCtrlReadBitUsbPort leaving (IRP could not be allocated)\n");

		return FALSE;
	}

    PepCtrlLog("PepCtrlReadBitUsbPort - Setting the completion routine\n");

	IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("PepCtrlReadBitUsbPort - Calling IoCallDriver\n");

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("PepCtrlReadBitUsbPort - Finished calling IoCallDriver  (Error Code: 0x%X)\n", status);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver returned status pending\n");

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IoCallDriver event to be set\n");

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("PepCtrlReadBitUsbPort - Finished waiting for the IoCallDriver event to be set  (Error Code: 0x%X)\n", status);

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver timed out\n");

            PepCtrlLog("PepCtrlReadBitUsbPort - Cancelling the IRP\n");

            if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("PepCtrlReadBitUsbPort - IRP cancel routine found and called\n");
            }
            else
            {
                PepCtrlLog("PepCtrlReadBitUsbPort - IRP cancel bit set\n");
            }

            PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IRP to be cancelled\n");

			status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        
            PepCtrlLog("PepCtrlReadBitUsbPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X)\n", status);

            status = STATUS_UNSUCCESSFUL;
        }
    }

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver succeeded  (IO Status Block: 0x%X)\n", IoStatusBlock.Status);

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("PepCtrlReadBitUsbPort - IO Status Block succeeded.\n");

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("PepCtrlReadBitUsbPort - IO Status Block failed.\n");
			}
        }
		else
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver succeeded.  (Error Code: 0x%X)\n", status);
		}
	}
	else
	{
        PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver failed.  (Error Code: 0x%X)\n", status);
	}

    PepCtrlLog("PepCtrlReadBitUsbPort - Completing the IRP.\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IRP to complete.\n");

	status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("PepCtrlReadBitUsbPort - Finished waiting for the IRP to complete  (Error Code: 0x%X)\n", status);

    PepCtrlLog("PepCtrlReadBitUsbPort leaving.\n");

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlWriteUsbPort(
  _In_ TPepCtrlObject* pObject,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen)
{
	BOOLEAN bResult = FALSE;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER OffsetInteger, TimeoutInteger;
    PIRP pIrp;

    PAGED_CODE()

    PepCtrlLog("PepCtrlWriteUsbPort entering.\n");

    PepCtrlLog("PepCtrlWriteUsbPort - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("PepCtrlWriteUsbPort - Initializing offset large integer\n");

    OffsetInteger.QuadPart = 0;

    PepCtrlLog("PepCtrlWriteUsbPort - Building IO Control Request\n");

    pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE,
                                        pObject->pPortDeviceObject,
                                        pucData, ulDataLen, &OffsetInteger,
                                        &Event, &IoStatusBlock);

	if (!pIrp) 
	{
        PepCtrlLog("PepCtrlWriteUsbPort leaving (IRP could not be allocated)\n");

		return FALSE;
	}

    PepCtrlLog("PepCtrlWriteUsbPort - Setting the completion routine\n");

	IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("PepCtrlWriteUsbPort - Calling the port device driver\n");

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    PepCtrlLog("PepCtrlWriteUsbPort - Finished calling the port device driver  (Error Code: 0x%X)\n", status);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver returned status pending\n");

        TimeoutInteger.QuadPart = MMillisecondsToRelativeTime(CTimeoutMs);

        PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IoCallDriver event to be set\n");

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

        PepCtrlLog("PepCtrlWriteUsbPort - Finished waiting for the IoCallDriver event to be set  (Error Code: 0x%X)\n", status);

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver timed out\n");

            PepCtrlLog("PepCtrlWriteUsbPort - Cancelling the IRP\n");

			if (IoCancelIrp(pIrp))
            {
                PepCtrlLog("PepCtrlWriteUsbPort - IRP cancel routine found and called\n");
            }
            else
            {
                PepCtrlLog("PepCtrlWriteUsbPort - IRP cancel bit set\n");
            }

            PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IRP to be cancelled\n");

			status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            PepCtrlLog("PepCtrlWriteUsbPort - Finished waiting for the IRP to be cancelled  (Error Code: 0x%X)\n", status);

            status = STATUS_UNSUCCESSFUL;
        }
	}

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver succeeded  (IO Status Block: 0x%X)\n", IoStatusBlock.Status);

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("PepCtrlWriteUsbPort - IO Status Block succeeded.\n");

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("PepCtrlWriteUsbPort - IO Status Block failed.\n");
			}
		}
		else
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver succeeded.  (Error Code: 0x%X)\n", status);
		}
	}
	else
	{
        PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver failed.  (Error Code: 0x%X)\n", status);
	}

    PepCtrlLog("PepCtrlWriteUsbPort - Completing the IRP.\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IRP to complete.\n");

	status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("PepCtrlWriteUsbPort - Finished waiting for the IRP to complete  (Error Code: 0x%X)\n", status);

    PepCtrlLog("PepCtrlWriteUsbPort leaving.\n");

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

    PAGED_CODE()

    PepCtrlLog("PepCtrlAllocUsbPort entering.\n");

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    PepCtrlLog("PepCtrlAllocUsbPort - Getting Device Object pointer to \"%ws\"\n", pszDeviceName);

    status = IoGetDeviceObjectPointer(&DeviceName, GENERIC_ALL,
                                      &pObject->pPortFileObject,
                                      &pObject->pPortDeviceObject);

    PepCtrlLog("PepCtrlAllocUsbPort - Finished getting Device Object pointer to \"%ws\"  (Error Code: 0x%X)\n",
               pszDeviceName, status);

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("PepCtrlAllocUsbPort - Got Device Object pointer to \"%ws\"\n", pszDeviceName);

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("PepCtrlAllocUsbPort - Failed to get device object pointer\n");
    }

    PepCtrlLog("PepCtrlAllocUsbPort leaving.\n");

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlFreeUsbPort(
  _In_ TPepCtrlObject* pObject)
{
    PAGED_CODE()

    PepCtrlLog("PepCtrlFreeUsbPort entering.\n");

    PepCtrlLog("PepCtrlFreeUsbPort - USB Printer port being released.\n");

    ObDereferenceObject(pObject->pPortFileObject);

    PepCtrlLog("PepCtrlFreeUsbPort leaving.\n");

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
LPGUID TPEPCTRLAPI PepCtrlGetUsbPortDevInterfaceGuid(VOID)
{
    PepCtrlLog("PepCtrlGetUsbPortDevInterfaceGuid entering.\n");

    PAGED_CODE()

    RtlCopyBytes(&l_UsbPrintGuid, &GUID_DEVINTERFACE_USBPRINT,
                 sizeof(l_UsbPrintGuid));

    PepCtrlLog("PepCtrlGetUsbPortDevInterfaceGuid leaving.\n");

    return &l_UsbPrintGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
