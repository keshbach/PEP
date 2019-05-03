/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <wdmguid.h>

#include "PepCtrlPortData.h"

#include "PepCtrlPlugPlay.h"
#include "PepCtrlLog.h"

#include <Drivers/PepCtrlIOCTL.h>

static VOID lCloseThread(_In_ HANDLE hThread);
static KSTART_ROUTINE lPortArrivedThreadStart;
static KSTART_ROUTINE lPortRemovedThreadStart;
static VOID lDeviceInterfaceArrival(_In_ PDEVICE_OBJECT pDeviceObject);
static VOID lDeviceInterfaceRemoval(_In_ PDEVICE_OBJECT pDeviceObject);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lCloseThread)
#pragma alloc_text (PAGE, lPortArrivedThreadStart)
#pragma alloc_text (PAGE, lPortRemovedThreadStart)
#pragma alloc_text (PAGE, lDeviceInterfaceArrival)
#pragma alloc_text (PAGE, lDeviceInterfaceRemoval)
#pragma alloc_text (PAGE, PepCtrlPlugPlayDeviceInterfaceChange)
#pragma alloc_text (PAGE, PepCtrlPlugPlayClosePortThreads)
#endif

#pragma region "Local Functions"

static VOID lCloseThread(
  _In_ HANDLE hThread)
{
    NTSTATUS Status;
    PKTHREAD Thread;

    PAGED_CODE()

    PepCtrlLog("lCloseThread called.\n");

    PepCtrlLog("lCloseThread - Retrieving a thread object from the thread handle.\n");

    Status = ObReferenceObjectByHandle(hThread, 0, *PsThreadType,
                                       KernelMode, &Thread, NULL);

    if (NT_SUCCESS(Status))
    {
        PepCtrlLog("lCloseThread - Retrieved the thread object.\n");

        PepCtrlLog("lCloseThread - Waiting for the thread to end.\n");

        Status = KeWaitForSingleObject(Thread, Executive, KernelMode, FALSE, NULL);

        if (NT_SUCCESS(Status))
        {
            PepCtrlLog("lCloseThread - Thread has ended.\n");
        }
        else
        {
            PepCtrlLog("lCloseThread - Failed to wait for the thread to end.  (0x%X)\n", Status);
        }

        PepCtrlLog("lCloseThread - Closing the thread handle.\n");

        Status = ZwClose(hThread);

        if (NT_SUCCESS(Status))
        {
            PepCtrlLog("lCloseThread - Thread handle closed.\n");
        }
        else
        {
            PepCtrlLog("lCloseThread - Failed to close the thread handle.  (0x%X)\n", Status);
        }

        PepCtrlLog("lCloseThread - Releasing the thread object.\n");

        ObDereferenceObject(Thread);
    }
    else
    {
        PepCtrlLog("lCloseThread - Could not retrieve the thread object from the thread handle.  (0x%X)\n", Status);
    }
}

static VOID lPortArrivedThreadStart(
  _In_ PVOID pvStartContext)
{
    PDEVICE_OBJECT pDeviceObject = (PDEVICE_OBJECT)pvStartContext;
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    PIRP pIrp = NULL;
    TPepCtrlObject TmpObject;
    UINT32* pnStatusChange;

    PAGED_CODE()

    PepCtrlLog("lPortArrivedThreadStart called.\n");

    if (pPortData->bPortEjected &&
        pPortData->Funcs.pAllocPortFunc(&TmpObject, pPortData->RegSettings.pszPortDeviceName))
    {
        PepCtrlLog("lPortArrivedThreadStart - Port was acquired now saving the data.\n");

        ExAcquireFastMutexUnsafe(&pPortData->FastMutex);

        pPortData->bPortEjected = FALSE;

        pIrp = pPortData->pIrp;

        pPortData->pIrp = NULL;

        RtlCopyMemory(&pPortData->Object, &TmpObject, sizeof(TmpObject));

        ExReleaseFastMutexUnsafe(&pPortData->FastMutex);

        PepCtrlLog("lPortArrivedThreadStart - Port data was successfully saved.\n");
    }

    if (pIrp)
    {
        PepCtrlLog("lPortArrivedThreadStart - Completing status change IRP.\n");

        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceArrived;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPortArrivedThreadStart ending.\n");
}

static VOID lPortRemovedThreadStart(
  _In_ PVOID pvStartContext)
{
    PDEVICE_OBJECT pDeviceObject = (PDEVICE_OBJECT)pvStartContext;
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    PIRP pIrp = NULL;
    UINT32* pnStatusChange;

    PAGED_CODE()

    PepCtrlLog("lPortRemovedThreadStart called.\n");

    ExAcquireFastMutexUnsafe(&pPortData->FastMutex);

    if (!pPortData->bPortEjected)
    {
        pPortData->bPortEjected = TRUE;

        pPortData->Funcs.pFreePortFunc(&pPortData->Object);

        pIrp = pPortData->pIrp;

        pPortData->pIrp = NULL;
    }

    ExReleaseFastMutexUnsafe(&pPortData->FastMutex);

    if (pIrp)
    {
        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceRemoved;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPortRemovedThreadStart ending.\n");
}

static VOID lDeviceInterfaceArrival(
  _In_ PDEVICE_OBJECT pDeviceObject)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE()

    PepCtrlLog("lDeviceInterfaceArrival called.\n");

    PepCtrlPlugPlayClosePortThreads(pPortData);

    PepCtrlLog("lDeviceInterfaceArrival - Creating the port arrived thread.\n");

    /*
       The assumption is made that plug and plug notifications are sequential
       since the document says it is possible to receive two for the same event.
    */

    status = PsCreateSystemThread(&pPortData->hPortArrivedThread,
                                  DELETE | SYNCHRONIZE, NULL, NULL, NULL,
                                  &lPortArrivedThreadStart, pDeviceObject);

    if (NT_ERROR(status))
    {
        PepCtrlLog("lDeviceInterfaceArrival - Creation of the port arrived thread failed.  (Error Code: 0x%X)\n", status);
    }
}

static VOID lDeviceInterfaceRemoval(
  _In_ PDEVICE_OBJECT pDeviceObject)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE()

    PepCtrlLog("lDeviceInterfaceRemoval called.\n");

    PepCtrlPlugPlayClosePortThreads(pPortData);

    PepCtrlLog("lDeviceInterfaceRemoval - Creating the port removed thread.\n");

    /*
       The assumption is made that plug and plug notifications are sequential
       since the document says it is possible to receive two for the same event.
    */

    status = PsCreateSystemThread(&pPortData->hPortRemovedThread,
                                  DELETE | SYNCHRONIZE, NULL, NULL, NULL,
                                  &lPortRemovedThreadStart, pDeviceObject);

    if (NT_ERROR(status))
    {
        PepCtrlLog("lDeviceInterfaceRemoval - Creation of the port removed thread failed.  (Error Code: 0x%X)\n", status);
    }
}

#pragma endregion

NTSTATUS PepCtrlPlugPlayDeviceInterfaceChange(
  _In_ PVOID pvNotificationStructure,
  _In_ PVOID pvContext)
{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION pNotification;
    PDEVICE_OBJECT pDeviceObject;
    TPepCtrlPortData* pPortData;

    PAGED_CODE()

    PepCtrlLog("PepCtrlPlugPlayDeviceInterfaceChange called.\n");

    pNotification = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION)pvNotificationStructure;
    pDeviceObject = (PDEVICE_OBJECT)pvContext;
    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    if (pNotification->Version != 1)
    {
        PepCtrlLog("PepCtrlPlugPlayDeviceInterfaceChange - Received unknown notification version of %d. \n", (ULONG)pNotification->Version);

        return STATUS_SUCCESS;
    }

    PepCtrlLog("PepCtrlPlugPlayDeviceInterfaceChange - Symbolic link name \"%ws\". \n", pNotification->SymbolicLinkName->Buffer);

    if (IsEqualGUID((LPGUID)&pNotification->Event, (LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL))
    {
        PepCtrlLog("PepCtrlPlugPlayDeviceInterfaceChange - GUID_DEVICE_INTERFACE_ARRIVAL event GUID received.\n");

        lDeviceInterfaceArrival(pDeviceObject);
    }
    else if (IsEqualGUID((LPGUID)&pNotification->Event, (LPGUID)&GUID_DEVICE_INTERFACE_REMOVAL))
    {
        PepCtrlLog("PepCtrlPlugPlayDeviceInterfaceChange - GUID_DEVICE_INTERFACE_REMOVAL event GUID received.\n");

        lDeviceInterfaceRemoval(pDeviceObject);
    }
    else
    {
        PepCtrlLog("PepCtrlPlugPlayDeviceInterfaceChange - Unknown Notification Event GUID received.\n");
    }

    return STATUS_SUCCESS;
}

VOID PepCtrlPlugPlayClosePortThreads(
  _In_ TPepCtrlPortData* pPortData)
{
    HANDLE hPortArrivedThread, hPortRemovedThread;

    PAGED_CODE()

    PepCtrlLog("PepCtrlClosePortThreads called.\n");

    ExAcquireFastMutexUnsafe(&pPortData->FastMutex);

    hPortArrivedThread = pPortData->hPortArrivedThread;
    hPortRemovedThread = pPortData->hPortRemovedThread;

    pPortData->hPortArrivedThread = NULL;
    pPortData->hPortRemovedThread = NULL;

    ExReleaseFastMutexUnsafe(&pPortData->FastMutex);

    if (hPortArrivedThread)
    {
        PepCtrlLog("PepCtrlClosePortThreads - Closing port arrived thread handle.\n");

        lCloseThread(hPortArrivedThread);
    }

    if (hPortRemovedThread)
    {
        PepCtrlLog("PepCtrlClosePortThreads - Closing port removed thread handle.\n");

        lCloseThread(hPortRemovedThread);
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
