/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>
#include <parallel.h>

#include <Utils/UtHeapDriver.h>

#include "PepCtrlPortData.h"
#include "PepCtrlParallelPort.h"

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
static BOOLEAN lAllocParallelPort(IN PDEVICE_OBJECT pDeviceObject);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lSetParallelPortMode)
#pragma alloc_text (PAGE, lAllocParallelPortInfo)
#pragma alloc_text (PAGE, lAllocParallelPort)

#pragma alloc_text (PAGE, PepCtrlReadBitParallelPort)
#pragma alloc_text (PAGE, PepCtrlWriteParallelPort)
#pragma alloc_text (PAGE, PepCtrlAllocParallelPort)
#pragma alloc_text (PAGE, PepCtrlFreeParallelPort)
#pragma alloc_text (PAGE, PepCtrlGetParallelPortDevInterfaceGuid)
#endif

static GUID l_ParallelGuid = {0};

static BOOLEAN lSetParallelPortMode(
  IN PDEVICE_OBJECT pPortDeviceObject)
{
    NTSTATUS status;
    PARALLEL_PNP_INFORMATION ParallelPnpInfo;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP pIrp;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lSetParallelPortMode called.\n") );

    RtlZeroMemory(&ParallelPnpInfo, sizeof(ParallelPnpInfo));

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Building IO Control Request\n") );

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO,
                                         pPortDeviceObject, NULL, 0,
                                         &ParallelPnpInfo, sizeof(ParallelPnpInfo),
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IRP could not be allocated\n") );

        return FALSE;
    }

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Calling the port device driver to obtain PNP information\n") );

    status = IoCallDriver(pPortDeviceObject, pIrp);

    if (status == STATUS_PENDING)
    {
       	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Call to IoCallDriver returned status pending\n") );

        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

        status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(status))
    {
     	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Call to IoCallDriver failed.  (Error Code: 0x%X)\n",
                   status) );

        return FALSE;
    }

   	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Call to IoCallDriver succeeded\n") );

    if (!NT_SUCCESS(IoStatusBlock.Status))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: IO Status Block failed.  (Error Code: 0x%X)\n",
                   IoStatusBlock.Status) );

        return FALSE;
    }

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Checking parallel port's hardware capabilities.\n") );

    if (!(ParallelPnpInfo.HardwareCapabilities & PPT_BYTE_PRESENT))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Parallel Port does not support byte mode.\n") );

        return FALSE;
    }

    if (ParallelPnpInfo.CurrentMode != ECR_SPP_MODE)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Clearing the parallel port's chip mode.\n") );

        status = ParallelPnpInfo.ClearChipMode(ParallelPnpInfo.Context,
                                               (UCHAR)ParallelPnpInfo.CurrentMode);

        if (!NT_SUCCESS(status))
        {
         	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Clearing the chip mode failed.  (Error Code: 0x%X)\n",
                       status) );

            return FALSE;
        }

        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Trying to set the parallel port's chip mode.\n") );

        status = ParallelPnpInfo.TrySetChipMode(ParallelPnpInfo.Context,
                                                ECR_SPP_MODE);

        if (!NT_SUCCESS(status))
        {
         	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting the chip mode failed.  (Error Code: 0x%X)\n",
                       status) );

            return FALSE;
        }
    }

    return TRUE;
}

static PPARALLEL_PORT_INFORMATION lAllocParallelPortInfo(
  IN PDEVICE_OBJECT pDeviceObject)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    PIRP pIrp;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG ulParallelPortInfoLen;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lAllocParallelPortInfo called.\n") );

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                "PepCtrl: Initializing event\n") );

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
				"PepCtrl: Building IO Control Request IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO\n") );

    ulParallelPortInfoLen = sizeof(PARALLEL_PORT_INFORMATION);
    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)UtAllocNonPagedMem(ulParallelPortInfoLen);

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO,
                                         pDeviceObject, NULL, 0,
                                         pParallelPortInfo, ulParallelPortInfoLen,
                                         TRUE, &Event, &IoStatusBlock);

    if (pIrp)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: Calling the port device driver\n") );

        status = IoCallDriver(pDeviceObject, pIrp);

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

            goto FreePort;
        }

        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: Call to IoCallDriver succeeded\n") );

        if (!NT_SUCCESS(IoStatusBlock.Status))
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: IO Status Block failed.  (Error Code: 0x%X)\n",
                        IoStatusBlock.Status) );

            goto FreePort;
        }

        return pParallelPortInfo;
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: IRP could not be allocated\n") );
    }

FreePort:
    UtFreeNonPagedMem(pParallelPortInfo);

    return NULL;
}

static BOOLEAN lAllocParallelPort(
  IN PDEVICE_OBJECT pDeviceObject)
{
    PIRP pIrp;
    NTSTATUS status;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lAllocParallelPort called.\n") );

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                "PepCtrl: Initializing event\n") );

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
				"PepCtrl: Building IO Control Request IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE\n") );

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE,
                                         pDeviceObject, NULL, 0, NULL, 0,
                                         TRUE, &Event, &IoStatusBlock);

    if (pIrp)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: Calling the port device driver\n") );

        status = IoCallDriver(pDeviceObject, pIrp);

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

        if (!NT_SUCCESS(IoStatusBlock.Status))
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: IO Status Block failed.  (Error Code: 0x%X)\n",
                        IoStatusBlock.Status) );

            return FALSE;
        }

        return TRUE;
    }
    else
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
				    "PepCtrl: IO Control Request failed.\n") );
    }

    return FALSE;
}

BOOLEAN TPEPCTRLAPI PepCtrlReadBitParallelPort(
  IN TPepCtrlObject* pObject,
  OUT PBOOLEAN pbValue)
{
    PPARALLEL_PORT_INFORMATION pParallelPortInfo;
    UCHAR ucStatus;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: PepCtrlReadBitParallelPort called.\n") );

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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: PepCtrlWriteParallelPort called.\n") );

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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlAllocParallelPort called.\n") );

    RtlInitUnicodeString(&DeviceName, pszDeviceName);

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

        pObject->pvObjectData = lAllocParallelPortInfo(pObject->pPortDeviceObject);

        if (!pObject->pvObjectData)
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Call to lAllocParallelPortInfo failed.\n") );

            goto FreeDevice;
        }

        pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: Port Address:        0x%X.\n",
                    pParallelPortInfo->OriginalController) );
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: System Port Address: 0x%X.\n",
                    pParallelPortInfo->Controller) );

        if (lAllocParallelPort(pObject->pPortDeviceObject))
        {
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Parallel port allocated.\n") );

            if (lSetParallelPortMode(pObject->pPortDeviceObject))
            {
                KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                            "PepCtrl: Parallel Port Mode set.\n") );

                return TRUE;
            }
            else
            {
                KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                            "PepCtrl: Parallel Port Mode could not be changed - freeing the port.\n") );

                pParallelPortInfo->FreePort(pParallelPortInfo->Context);
            }
        }
        else
        {
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Could not allocate the parallel port\n") );
        }

        UtFreeNonPagedMem(pParallelPortInfo);

FreeDevice:
        ObDereferenceObject(pObject->pPortFileObject);
    }
    else
    {
    	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Result of calling get device object pointer  (Error Code: 0x%X)\n",
                   status) );
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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlFreeParallelPort called.\n") );

    pParallelPortInfo = (PPARALLEL_PORT_INFORMATION)pObject->pvObjectData;

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Printer port being released.\n") );

    pParallelPortInfo->FreePort(pParallelPortInfo->Context);

    UtFreeNonPagedMem(pParallelPortInfo);

    ObDereferenceObject(pObject->pPortFileObject);

    pObject->pPortFileObject = NULL;
    pObject->pPortDeviceObject = NULL;
    pObject->pvObjectData = NULL;

    return TRUE;
}

LPGUID TPEPCTRLAPI PepCtrlGetParallelPortDevInterfaceGuid(VOID)
{
    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlGetParallelPortDevInterfaceGuid called.\n") );

    PAGED_CODE()

    RtlCopyBytes(&l_ParallelGuid, &GUID_DEVINTERFACE_PARALLEL,
                 sizeof(l_ParallelGuid));

    return &l_ParallelGuid;
}

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
