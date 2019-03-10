/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>
#include <usbprint.h>

#include "PepCtrlPortData.h"
#include "PepCtrlUsbPort.h"
#include "UsbPrintGuid.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PepCtrlReadBitUsbPort)
#pragma alloc_text (PAGE, PepCtrlWriteUsbPort)
#pragma alloc_text (PAGE, PepCtrlAllocUsbPort)
#pragma alloc_text (PAGE, PepCtrlFreeUsbPort)
#pragma alloc_text (PAGE, PepCtrlGetUsbPortDevInterfaceGuid)
#endif

static GUID l_UsbPrintGuid = {0};

BOOLEAN TPEPCTRLAPI PepCtrlReadBitUsbPort(
  IN TPepCtrlObject* pObject,
  OUT PUCHAR pucStatus)
{
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlReadBitUsbPort called.\n") );

 	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Initializing event\n") );

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			   "PepCtrl: Building IO Control Request\n") );

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_USBPRINT_GET_LPT_STATUS,
                                         pObject->pPortDeviceObject,
                                         NULL, 0, pucStatus, sizeof(*pucStatus),
                                         FALSE, &Event, &IoStatusBlock);

    if (pIrp)
    {
      	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Calling the port device driver\n") );

        status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

        if (status == STATUS_PENDING)
        {
         	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Call to IoCallDriver returned status pending\n") );

            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            status = STATUS_SUCCESS;
        }

        if (NT_ERROR(status))
        {
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Call to IoCallDriver failed.  (Error Code: 0x%X)\n",
                       status) );

            return FALSE;
        }

    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Call to IoCallDriver succeeded\n") );

        if (NT_SUCCESS(IoStatusBlock.Status))
        {
            return TRUE;
        }

    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IO Status Block failed.  (Error Code: 0x%X)\n",
                   IoStatusBlock.Status) );
    }
    else
    {
    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IRP could not be allocated\n") );
    }

    return FALSE;
}

BOOLEAN TPEPCTRLAPI PepCtrlWriteUsbPort(
  IN TPepCtrlObject* pObject,
  IN PUCHAR pucData,
  IN ULONG ulDataLen)
{
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER OffsetInteger;
    PIRP pIrp;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlWriteUsbPort called.\n") );

 	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Initializing event\n") );

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			   "PepCtrl: Initializing offset large integer\n") );

    OffsetInteger.QuadPart = 0;

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			   "PepCtrl: Building IO Control Request\n") );

    pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE,
                                        pObject->pPortDeviceObject,
                                        pucData, ulDataLen, &OffsetInteger,
                                        &Event, &IoStatusBlock);

    if (pIrp)
    {
      	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Calling the port device driver\n") );

        status = IoCallDriver(pObject->pPortDeviceObject, pIrp);

        if (status == STATUS_PENDING)
        {
         	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Call to IoCallDriver returned status pending\n") );

            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            status = STATUS_SUCCESS;
        }

        if (NT_ERROR(status))
        {
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Call to IoCallDriver failed.  (Error Code: 0x%X)\n",
                       status) );

            return FALSE;
        }

    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Call to IoCallDriver succeeded\n") );

        if (NT_SUCCESS(IoStatusBlock.Status))
        {
            return TRUE;
        }

    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IO Status Block failed.  (Error Code: 0x%X)\n",
                   IoStatusBlock.Status) );
    }
    else
    {
    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IRP could not be allocated\n") );
    }

    return FALSE;
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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlAllocUsbPort called.\n") );

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                               NULL, NULL);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Getting Device Object pointer to \"%ws\"\n", pszDeviceName) );

    status = IoGetDeviceObjectPointer(&DeviceName, GENERIC_ALL,
                                      &pObject->pPortFileObject,
                                      &pObject->pPortDeviceObject);

    if (NT_SUCCESS(status))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Got Device Object pointer to \"%ws\"\n",
                   pszDeviceName) );

        bResult = TRUE;
    }
    else
    {
    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Result of calling get device object pointer  (Error Code: 0x%X)\n",
                   status) );
    }

    return bResult;
}

BOOLEAN TPEPCTRLAPI PepCtrlFreeUsbPort(
  IN TPepCtrlObject* pObject)
{
    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlFreeUsbPort called.\n") );

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: USB Printer port being released.\n") );

    ObDereferenceObject(pObject->pPortFileObject);

    return TRUE;
}

LPGUID TPEPCTRLAPI PepCtrlGetUsbPortDevInterfaceGuid(VOID)
{
    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlGetUsbPortDevInterfaceGuid called.\n") );

    RtlCopyBytes(&l_UsbPrintGuid, &GUID_DEVINTERFACE_USBPRINT,
                 sizeof(l_UsbPrintGuid));

    return &l_UsbPrintGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
