/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>
#include <usbprint.h>

#include "PepCtrlPortData.h"
#include "PepCtrlUsbPort.h"
#include "UsbPrintGuid.h"

#include "PepCtrlLog.h"

#define CTimeoutTicks 1000000 // 100 milliseconds

/*
   Local Functions
*/

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PepCtrlReadBitUsbPort)
#pragma alloc_text (PAGE, PepCtrlWriteUsbPort)
#pragma alloc_text (PAGE, PepCtrlAllocUsbPort)
#pragma alloc_text (PAGE, PepCtrlFreeUsbPort)
#pragma alloc_text (PAGE, PepCtrlGetUsbPortDevInterfaceGuid)
#endif

static GUID l_UsbPrintGuid = {0};

static NTSTATUS lUsbPortIoCompletion(
  IN PDEVICE_OBJECT pDeviceObject,
  IN PIRP pIrp,
  IN PVOID pvContext)
{
    PepCtrlLog("lUsbPortIoCompletion called.\n");

    pDeviceObject;
	pIrp;

	KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

BOOLEAN TPEPCTRLAPI PepCtrlReadBitUsbPort(
  IN TPepCtrlObject* pObject,
  OUT PUCHAR pucStatus)
{
	BOOLEAN bResult = FALSE;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;
	LARGE_INTEGER TimeoutInteger;

    PAGED_CODE()

    PepCtrlLog("PepCtrlReadBitUsbPort called.\n");

    PepCtrlLog("PepCtrlReadBitUsbPort - Initializing event\n");

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PepCtrlLog("PepCtrlReadBitUsbPort - Building Device IO Control Request\n");

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_GET_LPT_STATUS,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, pucStatus, sizeof(*pucStatus),
                                         TRUE, &Event, &IoStatusBlock);

	if (!pIrp)
	{
        PepCtrlLog("PepCtrlReadBitUsbPort - IRP could not be allocated\n");

		return FALSE;
	}

    PepCtrlLog("PepCtrlReadBitUsbPort - Setting the completion routine\n");

	IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("PepCtrlReadBitUsbPort - Calling the port device driver\n");

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver returned status pending\n");

		TimeoutInteger.QuadPart = -CTimeoutTicks;

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver timed out\n");

            PepCtrlLog("PepCtrlReadBitUsbPort - Cancelling the IRP\n");

			IoCancelIrp(pIrp);

            PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IRP to be cancelled\n");

			KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
		}
    }

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("PepCtrlReadBitUsbPort - Call to IoCallDriver succeeded\n");

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("PepCtrlReadBitUsbPort - IO Status Block succeeded.\n");

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("PepCtrlReadBitUsbPort - IO Status Block failed.  (Error Code: 0x%X)\n",
					       IoStatusBlock.Status);
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

	KeClearEvent(&Event);

    PepCtrlLog("PepCtrlReadBitUsbPort - Completing the IRP.\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlReadBitUsbPort - Waiting for the IRP to complete.\n");

	KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    PepCtrlLog("PepCtrlReadBitUsbPort - Finished waiting for the IRP to complete.\n");

    return bResult;
}

BOOLEAN TPEPCTRLAPI PepCtrlWriteUsbPort(
  IN TPepCtrlObject* pObject,
  IN PUCHAR pucData,
  IN ULONG ulDataLen)
{
	BOOLEAN bResult = FALSE;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER OffsetInteger, TimeoutInteger;
    PIRP pIrp;

    PAGED_CODE()

    PepCtrlLog("PepCtrlWriteUsbPort called.\n");

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
        PepCtrlLog("PepCtrlWriteUsbPort - IRP could not be allocated\n");

		return FALSE;
	}

    PepCtrlLog("PepCtrlWriteUsbPort - Setting the completion routine\n");

	IoSetCompletionRoutine(pIrp, lUsbPortIoCompletion, &Event, TRUE, TRUE, TRUE);

    PepCtrlLog("PepCtrlWriteUsbPort - Calling the port device driver\n");

    status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

    if (status == STATUS_PENDING)
    {
        PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver returned status pending\n");

		TimeoutInteger.QuadPart = -CTimeoutTicks;

        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeoutInteger);

		if (status == STATUS_TIMEOUT)
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver timed out\n");

            PepCtrlLog("PepCtrlWriteUsbPort - Cancelling the IRP\n");

			IoCancelIrp(pIrp);

            PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IRP to be cancelled\n");

			KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
		}
	}

	if (NT_SUCCESS(status))
	{
		if (status == STATUS_SUCCESS)
		{
            PepCtrlLog("PepCtrlWriteUsbPort - Call to IoCallDriver succeeded\n");

			if (NT_SUCCESS(IoStatusBlock.Status))
			{
                PepCtrlLog("PepCtrlWriteUsbPort - IO Status Block succeeded.\n");

				bResult = TRUE;
			}
			else
			{
                PepCtrlLog("PepCtrlWriteUsbPort - IO Status Block failed.  (Error Code: 0x%X)\n",
					       IoStatusBlock.Status);
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

	KeClearEvent(&Event);

    PepCtrlLog("PepCtrlWriteUsbPort - Completing the IRP.\n");

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    PepCtrlLog("PepCtrlWriteUsbPort - Waiting for the IRP to complete.\n");

	KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    return bResult;
}

BOOLEAN TPEPCTRLAPI PepCtrlAllocUsbPort(
  IN TPepCtrlObject* pObject,
  IN LPCWSTR pszDeviceName)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DeviceName;

    PAGED_CODE()

    PepCtrlLog("PepCtrlAllocUsbPort called.\n");

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                               NULL, NULL);

    PepCtrlLog("PepCtrlAllocUsbPort - Getting Device Object pointer to \"%ws\"\n", pszDeviceName);

    status = IoGetDeviceObjectPointer(&DeviceName, GENERIC_ALL,
                                      &pObject->pPortFileObject,
                                      &pObject->pPortDeviceObject);

    if (NT_SUCCESS(status))
    {
        PepCtrlLog("PepCtrlAllocUsbPort - Got Device Object pointer to \"%ws\"\n", pszDeviceName);

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("PepCtrlAllocUsbPort - Result of calling get device object pointer  (Error Code: 0x%X)\n", status);
    }

    return bResult;
}

BOOLEAN TPEPCTRLAPI PepCtrlFreeUsbPort(
  IN TPepCtrlObject* pObject)
{
    PAGED_CODE()

    PepCtrlLog("PepCtrlFreeUsbPort called.\n");

    PepCtrlLog("PepCtrlFreeUsbPort - USB Printer port being released.\n");

    ObDereferenceObject(pObject->pPortFileObject);

    return TRUE;
}

LPGUID TPEPCTRLAPI PepCtrlGetUsbPortDevInterfaceGuid(VOID)
{
    PAGED_CODE()

    PepCtrlLog("PepCtrlGetUsbPortDevInterfaceGuid called.\n");

    RtlCopyBytes(&l_UsbPrintGuid, &GUID_DEVINTERFACE_USBPRINT,
                 sizeof(l_UsbPrintGuid));

    return &l_UsbPrintGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
