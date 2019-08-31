/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include "PepCtrlPortData.h"

#include "PepCtrlInit.h"

#include "PepCtrlParallelPort.h"
#include "PepCtrlUsbPort.h"
#include "PepCtrlReg.h"
#include "PepCtrlPlugPlay.h"
#include "PepCtrlLog.h"

#include <Drivers/PepCtrlDefs.h>

#include <Utils/UtHeapDriver.h>

#include <UtilsPep/UtPepLogic.h>

#include <Includes/UtMacros.h>

static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(_In_ PVOID pvContext, _Out_ PBOOLEAN pbValue);
static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(_In_ PVOID pvContext, _In_ PUCHAR pucData, _In_ ULONG ulDataLen);
static VOID __cdecl lPepLogicLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lPortDeviceArrived(_In_ TPepCtrlPortData* pPortData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID lPortDeviceRemoved(_In_ TPepCtrlPortData* pPortData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN TPEPCTRLPLUGPLAYAPI lPepPlugPlayDeviceArrived(_In_ PDEVICE_OBJECT pDeviceObject);

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID TPEPCTRLPLUGPLAYAPI lPepPlugPlayDeviceRemoved(_In_ PDEVICE_OBJECT pDeviceObject);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lPepLogicReadBitPort)
#pragma alloc_text (PAGE, lPepLogicWritePort)
#pragma alloc_text (PAGE, lPortDeviceArrived)
#pragma alloc_text (PAGE, lPortDeviceRemoved)
#pragma alloc_text (PAGE, lPepPlugPlayDeviceArrived)
#pragma alloc_text (PAGE, lPepPlugPlayDeviceRemoved)

#pragma alloc_text (PAGE, PepCtrlInitPortData)
#pragma alloc_text (PAGE, PepCtrlUninitPortData)
#pragma alloc_text (PAGE, PepCtrlInitPortTypeFuncs)
#endif

#pragma region "Local Functions"

static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(
  _In_ PVOID pvContext,
  _Out_ PBOOLEAN pbValue)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pvContext;

    PAGED_CODE()

    PepCtrlLog("lPepLogicReadBitPort - Port Data pointer: 0x%p\n",
               pPortData);

    return pPortData->Funcs.pReadBitPortFunc(&pPortData->Object, pbValue);
}

static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(
  _In_ PVOID pvContext,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pvContext;

    PAGED_CODE()

    PepCtrlLog("lPepLogicWritePort - Port Data pointer: 0x%p\n",
               pPortData);

    return pPortData->Funcs.pWritePortFunc(&pPortData->Object, pucData, ulDataLen);
}

static VOID __cdecl lPepLogicLog(
  _In_z_ _Printf_format_string_ PCSTR pszFormat,
  ...)
{
    va_list arguments;

    va_start(arguments, pszFormat);

    PepCtrlLogV(pszFormat, arguments);

    va_end(arguments);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lPortDeviceArrived(
  _In_ TPepCtrlPortData* pPortData)
{
    BOOLEAN bResult = FALSE;
    PIRP pIrp = NULL;
    UINT32* pnStatusChange;
    TPepCtrlObject* pPepCtrlObject;

    PepCtrlLog("lPortDeviceArrived entering.\n");

    PAGED_CODE()

    pPepCtrlObject = &pPortData->Object;

    PepCtrlLog("lPortDeviceArrived - Port Data pointer: 0x%p\n",
               pPortData);
    PepCtrlLog("lPortDeviceArrived - Control Object pointer: 0x%p\n",
               pPepCtrlObject);

    PepCtrlLog("lPortDeviceArrived - Attempting to allocate the port.\n");

    if (pPortData->Funcs.pAllocPortFunc(pPepCtrlObject, pPortData->RegSettings.pszPortDeviceName))
    {
        PepCtrlLog("lPortDeviceArrived - Successfully allocated the port.\n");

        pIrp = pPortData->pIrp;

        pPortData->pIrp = NULL;

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lPortDeviceArrived - Failed to allocate the port.\n");
    }

    if (pIrp)
    {
        PepCtrlLog("lPortDeviceArrived - Completing status change IRP.\n");

        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceArrived;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPortDeviceArrived leaving.\n");

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID lPortDeviceRemoved(
  _In_ TPepCtrlPortData* pPortData)
{
    PIRP pIrp = NULL;
    UINT32* pnStatusChange;
    TPepCtrlObject* pPepCtrlObject;

    PepCtrlLog("lPortDeviceRemoved entering.\n");

    PAGED_CODE()

    pPepCtrlObject = &pPortData->Object;

    PepCtrlLog("lPortDeviceRemoved - Port Data pointer: 0x%p\n",
               pPortData);
    PepCtrlLog("lPortDeviceRemoved - Control Object pointer: 0x%p\n",
               pPepCtrlObject);

    PepCtrlLog("lPortDeviceRemoved - Attempting to free the port.\n");

    if (pPortData->Funcs.pFreePortFunc(pPepCtrlObject))
    {
        PepCtrlLog("lPortDeviceRemoved - Successfully freed the port.\n");
    }
    else
    {
        PepCtrlLog("lPortDeviceRemoved - Failed to free the port.\n");
    }

    pIrp = pPortData->pIrp;

    pPortData->pIrp = NULL;

    if (pIrp)
    {
        PepCtrlLog("lPortDeviceRemoved - Completing status change IRP.\n");

        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceRemoved;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPortDeviceRemoved leaving.\n");
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN TPEPCTRLPLUGPLAYAPI lPepPlugPlayDeviceArrived(
  _In_ PDEVICE_OBJECT pDeviceObject)
{
    BOOLEAN bResult = FALSE;
    BOOLEAN bQuit = FALSE;
    BOOLEAN bCallPortArrivedFunc = FALSE;
    TPepCtrlPortData* pPortData;
    LARGE_INTEGER Interval;

    PepCtrlLog("lPepPlugPlayDeviceArrived entering.\n");

    PAGED_CODE()

    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lPepPlugPlayDeviceArrived - Port Data pointer: 0x%p\n",
               pPortData);

    while (!bQuit)
    {
        if (ExTryToAcquireFastMutex(&pPortData->FastMutex))
        {
            switch (pPortData->nState)
            {
                case CPepCtrlStateRunning:
                    pPortData->nState = CPepCtrlStateDeviceArrived;

                    bCallPortArrivedFunc = TRUE;
                    break;
                case CPepCtrlStateDeviceControl:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unsupported state of CPepCtrlStateDeviceControl\n");
                    break;
                case CPepCtrlStateDeviceArrived:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unsupported state of CPepCtrlStateDeviceArrived\n");
                    break;
                case CPepCtrlStateDeviceRemoved:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unsupported state of CPepCtrlStateDeviceRemoved\n");
                    break;
                case CPepCtrlStateUnloading:
                case CPepCtrlStateChangePortSettings:
                    break;
                default:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unknown state of %d\n",
                               pPortData->nState);
                    break;
            }

            ExReleaseFastMutex(&pPortData->FastMutex);

            if (bCallPortArrivedFunc)
            {
                bResult = lPortDeviceArrived(pPortData);
            }

            ExAcquireFastMutex(&pPortData->FastMutex);

            if (pPortData->nState != CPepCtrlStateDeviceArrived)
            {
                PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: State should be CPepCtrlStateDeviceArrived not %d\n",
                           pPortData->nState);
            }

            pPortData->nState = CPepCtrlStateRunning;

            ExReleaseFastMutex(&pPortData->FastMutex);

            bQuit = TRUE;
        }
        else
        {
            switch (pPortData->nState)
            {
                case CPepCtrlStateRunning:
                case CPepCtrlStateDeviceControl:
                    Interval.QuadPart = -1;

                    KeDelayExecutionThread(KernelMode, FALSE, &Interval);
                    break;
                case CPepCtrlStateUnloading:
                case CPepCtrlStateChangePortSettings:
                case CPepCtrlStateDeviceArrived:
                case CPepCtrlStateDeviceRemoved:
                    bQuit = TRUE;
                    break;
                default:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unknown state of %d\n",
                               pPortData->nState);

                    bQuit = TRUE;
                    break;
            }
        }
    }

    PepCtrlLog("lPepPlugPlayDeviceArrived leaving.\n");

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID TPEPCTRLPLUGPLAYAPI lPepPlugPlayDeviceRemoved(
  _In_ PDEVICE_OBJECT pDeviceObject)
{
    BOOLEAN bQuit = FALSE;
    BOOLEAN bCallPortRemovedFunc = FALSE;
    TPepCtrlPortData* pPortData;
    LARGE_INTEGER Interval;

    PepCtrlLog("lPepPlugPlayDeviceRemoved entering.\n");

    PAGED_CODE()

    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lPepPlugPlayDeviceRemoved - Port Data pointer: 0x%p\n",
               pPortData);

    while (!bQuit)
    {
        if (ExTryToAcquireFastMutex(&pPortData->FastMutex))
        {
            switch (pPortData->nState)
            {
                case CPepCtrlStateRunning:
                    pPortData->nState = CPepCtrlStateDeviceRemoved;

                    bCallPortRemovedFunc = TRUE;
                    break;
                case CPepCtrlStateDeviceControl:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unsupported state of CPepCtrlStateDeviceControl\n");
                    break;
                case CPepCtrlStateDeviceArrived:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unsupported state of CPepCtrlStateDeviceArrived\n");
                    break;
                case CPepCtrlStateDeviceRemoved:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unsupported state of CPepCtrlStateDeviceRemoved\n");
                    break;
                case CPepCtrlStateUnloading:
                case CPepCtrlStateChangePortSettings:
                    break;
                default:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unknown state of %d\n",
                               pPortData->nState);
                    break;
            }

            ExReleaseFastMutex(&pPortData->FastMutex);

            if (bCallPortRemovedFunc)
            {
                lPortDeviceRemoved(pPortData);
            }

            ExAcquireFastMutex(&pPortData->FastMutex);

            if (pPortData->nState != CPepCtrlStateDeviceRemoved)
            {
                PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: State should be CPepCtrlStateDeviceRemoved not %d\n",
                          pPortData->nState);
            }

            pPortData->nState = CPepCtrlStateRunning;

            ExReleaseFastMutex(&pPortData->FastMutex);

            bQuit = TRUE;
        }
        else
        {
            switch (pPortData->nState)
            {
                case CPepCtrlStateRunning:
                case CPepCtrlStateDeviceControl:
                    Interval.QuadPart = -1;

                    KeDelayExecutionThread(KernelMode, FALSE, &Interval);
                    break;
                case CPepCtrlStateUnloading:
                case CPepCtrlStateChangePortSettings:
                    lPortDeviceRemoved(pPortData);

                    bQuit = TRUE;
                    break;
                case CPepCtrlStateDeviceArrived:
                case CPepCtrlStateDeviceRemoved:
                    bQuit = TRUE;
                    break;
                default:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unknown state of %d\n",
                               pPortData->nState);
                    break;
            }
        }
    }

    PepCtrlLog("lPepPlugPlayDeviceRemoved leaving.\n");
}

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlInitPortData(
  _In_ PDRIVER_OBJECT pDriverObject,
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PUNICODE_STRING pRegistryPath,
  _In_ TPepCtrlPortData* pPortData)
{
    ULONG ulPortType;

    PepCtrlLog("PepCtrlInitPortData entering.\n");

    PAGED_CODE()

    PepCtrlLog("PepCtrlInitPortData - Driver Object pointer: 0x%p\n",
               pDriverObject);
    PepCtrlLog("PepCtrlInitPortData - Device Object pointer: 0x%p\n",
               pDeviceObject);
    PepCtrlLog("PepCtrlInitPortData - Port Data pointer: 0x%p\n",
               pPortData);

    RtlZeroMemory(pPortData, sizeof(TPepCtrlPortData));

    PepCtrlLog("PepCtrlInitPortData - Calling UtPepLogicAllocLogicContext.\n");

    pPortData->pDriverObject = pDriverObject;
    pPortData->pDeviceObject = pDeviceObject;
    pPortData->nState = CPepCtrlStateRunning;
    pPortData->LogicData.pvLogicContext = UtPepLogicAllocLogicContext();

    if (pPortData->LogicData.pvLogicContext == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData leaving (Call to UtPepLogicAllocLogicContext failed.)\n");

        return FALSE;
    }

    pPortData->LogicData.pvDeviceContext = pPortData;
    pPortData->LogicData.pReadBitPortFunc = lPepLogicReadBitPort;
    pPortData->LogicData.pWritePortFunc = lPepLogicWritePort;
    pPortData->LogicData.pLogFunc = lPepLogicLog;

    ExInitializeFastMutex(&pPortData->FastMutex);

    PepCtrlLog("PepCtrlInitPortData - Allocating memory for the registry path.\n");

    pPortData->RegSettings.pszRegistryPath = (LPWSTR)UtAllocPagedMem(pRegistryPath->Length + sizeof(WCHAR));

    if (pPortData->RegSettings.pszRegistryPath == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData - Failed to allocate memory for the registry path.\n");

        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("PepCtrlInitPortData leaving (Could not allocate memory for the registry path.)\n");

        return FALSE;
    }

    PepCtrlLog("PepCtrlInitPortData - Memory allocated for the registry path.\n");

    RtlCopyMemory(pPortData->RegSettings.pszRegistryPath, pRegistryPath->Buffer,
                  pRegistryPath->Length + sizeof(WCHAR));

    PepCtrlLog("PepCtrlInitPortData - Reading the registry settings.\n");

    if (!PepCtrlReadRegSettings(pRegistryPath, &ulPortType,
                                &pPortData->RegSettings.pszPortDeviceName))
    {
        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("PepCtrlInitPortData leaving (Could not read the registry settings.)\n");

        return FALSE;
    }

    pPortData->RegSettings.nPortType = (UINT32)ulPortType;

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType ||
        pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        switch (pPortData->RegSettings.nPortType)
        {
            case CPepCtrlParallelPortType:
                PepCtrlLog("PepCtrlInitPortData - Parallel Port type retrieved from the registry.\n");
                break;
            case CPepCtrlUsbPrintPortType:
                PepCtrlLog("PepCtrlInitPortData - USB Print Port type retrieved from the registry.\n");
                break;
            default:
                PepCtrlLog("PepCtrlInitPortData - Unknown Port type retrieved from the registry.\n");
                break;
        }

        PepCtrlLog("PepCtrlInitPortData - Port Device Name \"%ws\" retrieved from the registry.\n", pPortData->RegSettings.pszPortDeviceName);

        PepCtrlInitPortTypeFuncs(pPortData);
    }
    else
    {
        PepCtrlLog("PepCtrlInitPortData - No Port type retrieved from the registry.\n");
    }

    PepCtrlLog("PepCtrlInitPortData - Attempting to allocate the plug 'n play data.\n");

    pPortData->pvPlugPlayData = PepCtrlPlugPlayAlloc(pPortData->pDriverObject,
                                                     pPortData->pDeviceObject,
                                                     lPepPlugPlayDeviceArrived,
                                                     lPepPlugPlayDeviceRemoved);

    if (pPortData->pvPlugPlayData == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData - Failed to allocate the plug 'n play data.\n");

        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("PepCtrlInitPortData leaving (Could not allocate memory for the Plug and Play data.)\n");

        return FALSE;
    }

    PepCtrlLog("PepCtrlInitPortData - Successfully allocated the plug 'n play data.  (Pointer: 0x%p)\n",
               pPortData->pvPlugPlayData);

    PepCtrlLog("PepCtrlInitPortData leaving.\n");

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlUninitPortData(
  _In_ TPepCtrlPortData* pPortData)
{
    PepCtrlLog("PepCtrlUninitPortData enter.\n");

    PAGED_CODE()

    PepCtrlLog("PepCtrlUninitPortData - Port Data pointer: 0x%p\n",
               pPortData);

    if (pPortData->pvPlugPlayData)
    {
        PepCtrlLog("PepCtrlUninitPortData - Deleting the memory allocated for the Plug and Play data.\n");

        PepCtrlPlugPlayFree(pPortData->pvPlugPlayData);
    }

    if (pPortData->RegSettings.pszPortDeviceName)
    {
        PepCtrlLog("PepCtrlUninitPortData - Deleting the memory allocated for the port's device name.\n");

        UtFreePagedMem(pPortData->RegSettings.pszPortDeviceName);
    }

    if (pPortData->RegSettings.pszRegistryPath)
    {
        PepCtrlLog("PepCtrlUninitPortData - Deleting the memory allocated for the registry path.\n");

        UtFreePagedMem(pPortData->RegSettings.pszRegistryPath);
    }

    if (pPortData->LogicData.pvLogicContext)
    {
        PepCtrlLog("PepCtrlUninitPortData - Calling UtPepLogicFreeLogicContext.\n");

        UtPepLogicFreeLogicContext(pPortData->LogicData.pvLogicContext);
    }

    PepCtrlLog("PepCtrlUninitPortData leaving.\n");
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlInitPortTypeFuncs(
  _In_ TPepCtrlPortData* pPortData)
{
    PepCtrlLog("PepCtrlInitPortTypeFuncs entering.\n");

    PAGED_CODE()

    PepCtrlLog("PepCtrlInitPortTypeFuncs - Port Data pointer: 0x%p\n",
               pPortData);

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType)
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with Parallel Port functions.\n");

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocParallelPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeParallelPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitParallelPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteParallelPort;
        pPortData->Funcs.pGetDeviceInterfaceGuidFunc = &PepCtrlGetParallelPortDevInterfaceGuid;
    }
    else if (pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with Usb Port functions.\n");

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocUsbPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeUsbPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitUsbPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteUsbPort;
        pPortData->Funcs.pGetDeviceInterfaceGuidFunc = &PepCtrlGetUsbPortDevInterfaceGuid;
    }
    else
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with no functions.\n");

        pPortData->Funcs.pAllocPortFunc = NULL;
        pPortData->Funcs.pFreePortFunc = NULL;
        pPortData->Funcs.pReadBitPortFunc = NULL;
        pPortData->Funcs.pWritePortFunc = NULL;
        pPortData->Funcs.pGetDeviceInterfaceGuidFunc = NULL;
    }

    PepCtrlLog("PepCtrlInitPortTypeFuncs leaving.\n");
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
