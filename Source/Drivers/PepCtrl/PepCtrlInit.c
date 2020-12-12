/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

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

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(_In_ PVOID pvContext, _Out_ PBOOLEAN pbValue);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(_In_ PVOID pvContext, _In_ PUCHAR pucData, _In_ ULONG ulDataLen, _In_ ULONG ulWaitNanoSeconds);

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

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(
  _In_ PVOID pvContext,
  _Out_ PBOOLEAN pbValue)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pvContext;
	BOOLEAN bResult;

	PepCtrlLog("lPepLogicReadBitPort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lPepLogicReadBitPort - Port Data pointer: 0x%p  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

	bResult = pPortData->Funcs.pReadBitPortFunc(&pPortData->Object, pbValue);

	PepCtrlLog("lPepLogicReadBitPort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(
  _In_ PVOID pvContext,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen,
  _In_ ULONG ulDelayNanoSeconds)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pvContext;
	BOOLEAN bResult;

	PepCtrlLog("lPepLogicWritePort entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("lPepLogicWritePort - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

	bResult = pPortData->Funcs.pWritePortFunc(&pPortData->Object, pucData, ulDataLen, ulDelayNanoSeconds);

	PepCtrlLog("lPepLogicWritePort leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

	return bResult;
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

    PepCtrlLog("lPortDeviceArrived entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    pPepCtrlObject = &pPortData->Object;

    PepCtrlLog("lPortDeviceArrived - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());
    PepCtrlLog("lPortDeviceArrived - Control Object pointer: 0x%p.  (Thread: 0x%p)\n",
               pPepCtrlObject, PsGetCurrentThread());

    PepCtrlLog("lPortDeviceArrived - Attempting to allocate the port.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (pPortData->Funcs.pAllocPortFunc(pPepCtrlObject, pPortData->RegSettings.pszPortDeviceName))
    {
        PepCtrlLog("lPortDeviceArrived - Successfully allocated the port.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        pIrp = pPortData->pIrp;

        pPortData->pIrp = NULL;

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("lPortDeviceArrived - Failed to allocate the port.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    if (pIrp)
    {
        PepCtrlLog("lPortDeviceArrived - Completing status change IRP.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceArrived;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPortDeviceArrived leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static VOID lPortDeviceRemoved(
  _In_ TPepCtrlPortData* pPortData)
{
    PIRP pIrp = NULL;
    UINT32* pnStatusChange;
    TPepCtrlObject* pPepCtrlObject;

    PepCtrlLog("lPortDeviceRemoved entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    pPepCtrlObject = &pPortData->Object;

    PepCtrlLog("lPortDeviceRemoved - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());
    PepCtrlLog("lPortDeviceRemoved - Control Object pointer: 0x%p.  (Thread: 0x%p)\n",
               pPepCtrlObject, PsGetCurrentThread());

    PepCtrlLog("lPortDeviceRemoved - Attempting to free the port.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (pPortData->Funcs.pFreePortFunc(pPepCtrlObject))
    {
        PepCtrlLog("lPortDeviceRemoved - Successfully freed the port.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }
    else
    {
        PepCtrlLog("lPortDeviceRemoved - Failed to free the port.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    pIrp = pPortData->pIrp;

    pPortData->pIrp = NULL;

    if (pIrp)
    {
        PepCtrlLog("lPortDeviceRemoved - Completing status change IRP.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        IoSetCancelRoutine(pIrp, NULL);

        pnStatusChange = (UINT32*)pIrp->AssociatedIrp.SystemBuffer;

        *pnStatusChange = CPepCtrlDeviceRemoved;

        pIrp->IoStatus.Information = sizeof(UINT32);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    PepCtrlLog("lPortDeviceRemoved leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
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
    INT32 nPreviousState;

    PepCtrlLog("lPepPlugPlayDeviceArrived entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lPepPlugPlayDeviceArrived - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

    while (!bQuit)
    {
        if (ExTryToAcquireFastMutex(&pPortData->FastMutex))
        {
            nPreviousState = pPortData->nState;

            if (pPortData->nState == CPepCtrlStateRunning)
            {
                pPortData->nState = CPepCtrlStateDeviceArrived;

                bCallPortArrivedFunc = TRUE;
            }

            ExReleaseFastMutex(&pPortData->FastMutex);

            switch (nPreviousState)
            {
                case CPepCtrlStateRunning:
                    break;
                case CPepCtrlStateDeviceControl:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unsupported state of CPepCtrlStateDeviceControl.  (Thread: 0x%p)\n",
                               PsGetCurrentThread());
                    break;
                case CPepCtrlStateDeviceArrived:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unsupported state of CPepCtrlStateDeviceArrived.  (Thread: 0x%p)\n",
                               PsGetCurrentThread());
                    break;
                case CPepCtrlStateDeviceRemoved:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unsupported state of CPepCtrlStateDeviceRemoved.  (Thread: 0x%p)\n",
                               PsGetCurrentThread());
                    break;
                case CPepCtrlStateUnloading:
                case CPepCtrlStateChangePortSettings:
                    break;
                default:
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unknown state of %d.  (Thread: 0x%p)\n",
                               nPreviousState, PsGetCurrentThread());
                    break;
            }

            if (bCallPortArrivedFunc)
            {
                bResult = lPortDeviceArrived(pPortData);

                ExAcquireFastMutex(&pPortData->FastMutex);

                nPreviousState = pPortData->nState;

                pPortData->nState = CPepCtrlStateRunning;

                ExReleaseFastMutex(&pPortData->FastMutex);

                if (nPreviousState != CPepCtrlStateDeviceArrived)
                {
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: State should be CPepCtrlStateDeviceArrived not %d.  (Thread: 0x%p)\n",
                               nPreviousState, PsGetCurrentThread());
                }
            }

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
                    PepCtrlLog("lPepPlugPlayDeviceArrived - ERROR: Unknown state of %d.  (Thread: 0x%p)\n",
                               pPortData->nState, PsGetCurrentThread());

                    bQuit = TRUE;
                    break;
            }
        }
    }

    PepCtrlLog("lPepPlugPlayDeviceArrived leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

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
    INT32 nOriginalState, nPreviousState;

    PepCtrlLog("lPepPlugPlayDeviceRemoved entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    pPortData = (TPepCtrlPortData*)pDeviceObject->DeviceExtension;

    PepCtrlLog("lPepPlugPlayDeviceRemoved - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

    while (!bQuit)
    {
        if (ExTryToAcquireFastMutex(&pPortData->FastMutex))
        {
            nOriginalState = pPortData->nState;
            nPreviousState = pPortData->nState;

            switch (pPortData->nState)
            {
                case CPepCtrlStateRunning:
                case CPepCtrlStateUnloading:
                    pPortData->nState = CPepCtrlStateDeviceRemoved;

                    bCallPortRemovedFunc = TRUE;
                    break;
            }

            ExReleaseFastMutex(&pPortData->FastMutex);

            switch (nPreviousState)
            {
                case CPepCtrlStateRunning:
                case CPepCtrlStateUnloading:
                    break;
                case CPepCtrlStateDeviceControl:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unsupported state of CPepCtrlStateDeviceControl.  (Thread: 0x%p)\n",
                               PsGetCurrentThread());
                    break;
                case CPepCtrlStateDeviceArrived:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unsupported state of CPepCtrlStateDeviceArrived.  (Thread: 0x%p)\n",
                               PsGetCurrentThread());
                    break;
                case CPepCtrlStateDeviceRemoved:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unsupported state of CPepCtrlStateDeviceRemoved.  (Thread: 0x%p)\n",
                               PsGetCurrentThread());
                    break;
                case CPepCtrlStateChangePortSettings:
                    break;
                default:
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unknown state of %d.  (Thread: 0x%p)\n",
                               nPreviousState, PsGetCurrentThread());
                    break;
            }

            if (bCallPortRemovedFunc)
            {
                lPortDeviceRemoved(pPortData);

                ExAcquireFastMutex(&pPortData->FastMutex);

                nPreviousState = pPortData->nState;

                pPortData->nState = nOriginalState;

                ExReleaseFastMutex(&pPortData->FastMutex);

                if (nPreviousState != CPepCtrlStateDeviceRemoved)
                {
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: State should be CPepCtrlStateDeviceRemoved not %d.  (Thread: 0x%p)\n",
                               nPreviousState, PsGetCurrentThread());
                }
            }

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
                    PepCtrlLog("lPepPlugPlayDeviceRemoved - ERROR: Unknown state of %d.  (Thread: 0x%p)\n",
                               pPortData->nState, PsGetCurrentThread());
                    break;
            }
        }
    }

    PepCtrlLog("lPepPlugPlayDeviceRemoved leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
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

    PepCtrlLog("PepCtrlInitPortData entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlInitPortData - Driver Object pointer: 0x%p.  (Thread: 0x%p)\n",
               pDriverObject, PsGetCurrentThread());
    PepCtrlLog("PepCtrlInitPortData - Device Object pointer: 0x%p.  (Thread: 0x%p)\n",
               pDeviceObject, PsGetCurrentThread());
    PepCtrlLog("PepCtrlInitPortData - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

    RtlZeroMemory(pPortData, sizeof(TPepCtrlPortData));

    PepCtrlLog("PepCtrlInitPortData - Calling UtPepLogicAllocLogicContext.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pPortData->pDriverObject = pDriverObject;
    pPortData->pDeviceObject = pDeviceObject;
    pPortData->nState = CPepCtrlStateRunning;
    pPortData->LogicData.pvLogicContext = UtPepLogicAllocLogicContext();

    if (pPortData->LogicData.pvLogicContext == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData leaving.  (Call to UtPepLogicAllocLogicContext failed.)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    pPortData->LogicData.pvDeviceContext = pPortData;
    pPortData->LogicData.pReadBitPortFunc = lPepLogicReadBitPort;
    pPortData->LogicData.pWritePortFunc = lPepLogicWritePort;
    pPortData->LogicData.pLogFunc = lPepLogicLog;

    ExInitializeFastMutex(&pPortData->FastMutex);

    PepCtrlLog("PepCtrlInitPortData - Allocating memory for the registry path.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pPortData->RegSettings.pszRegistryPath = (LPWSTR)UtAllocPagedMem(pRegistryPath->Length + sizeof(WCHAR));

    if (pPortData->RegSettings.pszRegistryPath == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData - Failed to allocate memory for the registry path.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("PepCtrlInitPortData leaving.  (Could not allocate memory for the registry path.)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("PepCtrlInitPortData - Memory allocated for the registry path.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    RtlCopyMemory(pPortData->RegSettings.pszRegistryPath, pRegistryPath->Buffer,
                  pRegistryPath->Length + sizeof(WCHAR));

    PepCtrlLog("PepCtrlInitPortData - Reading the registry settings.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    if (!PepCtrlReadRegSettings(pRegistryPath, &ulPortType,
                                &pPortData->RegSettings.pszPortDeviceName))
    {
        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("PepCtrlInitPortData leaving.  (Could not read the registry settings.)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    pPortData->RegSettings.nPortType = (UINT32)ulPortType;

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType ||
        pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        switch (pPortData->RegSettings.nPortType)
        {
            case CPepCtrlParallelPortType:
                PepCtrlLog("PepCtrlInitPortData - Parallel Port type retrieved from the registry.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
                break;
            case CPepCtrlUsbPrintPortType:
                PepCtrlLog("PepCtrlInitPortData - USB Print Port type retrieved from the registry.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
                break;
            default:
                PepCtrlLog("PepCtrlInitPortData - Unknown Port type retrieved from the registry.  (Thread: 0x%p)\n",
					       PsGetCurrentThread());
                break;
        }

        PepCtrlLog("PepCtrlInitPortData - Port Device Name \"%ws\" retrieved from the registry.  (Thread: 0x%p)\n",
			       pPortData->RegSettings.pszPortDeviceName,
			       PsGetCurrentThread());

        PepCtrlInitPortTypeFuncs(pPortData);
    }
    else
    {
        PepCtrlLog("PepCtrlInitPortData - No Port type retrieved from the registry.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());
    }

    PepCtrlLog("PepCtrlInitPortData - Attempting to allocate the plug 'n play data.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    pPortData->pvPlugPlayData = PepCtrlPlugPlayAlloc(pPortData->pDriverObject,
                                                     pPortData->pDeviceObject,
                                                     lPepPlugPlayDeviceArrived,
                                                     lPepPlugPlayDeviceRemoved);

    if (pPortData->pvPlugPlayData == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData - Failed to allocate the plug 'n play data.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        PepCtrlUninitPortData(pPortData);

        PepCtrlLog("PepCtrlInitPortData leaving.  (Could not allocate memory for the Plug and Play data.)  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        return FALSE;
    }

    PepCtrlLog("PepCtrlInitPortData - Successfully allocated the plug 'n play data.  (Pointer: 0x%p)  (Thread: 0x%p)\n",
               pPortData->pvPlugPlayData, PsGetCurrentThread());

    PepCtrlLog("PepCtrlInitPortData leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlUninitPortData(
  _In_ TPepCtrlPortData* pPortData)
{
    PepCtrlLog("PepCtrlUninitPortData entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlUninitPortData - Port Data pointer: 0x%p.  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

    if (pPortData->pvPlugPlayData)
    {
        PepCtrlLog("PepCtrlUninitPortData - Deleting the memory allocated for the Plug and Play data.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        PepCtrlPlugPlayFree(pPortData->pvPlugPlayData);
    }

    if (pPortData->RegSettings.pszPortDeviceName)
    {
        PepCtrlLog("PepCtrlUninitPortData - Deleting the memory allocated for the port's device name.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        UtFreePagedMem(pPortData->RegSettings.pszPortDeviceName);
    }

    if (pPortData->RegSettings.pszRegistryPath)
    {
        PepCtrlLog("PepCtrlUninitPortData - Deleting the memory allocated for the registry path.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        UtFreePagedMem(pPortData->RegSettings.pszRegistryPath);
    }

    if (pPortData->LogicData.pvLogicContext)
    {
        PepCtrlLog("PepCtrlUninitPortData - Calling UtPepLogicFreeLogicContext.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        UtPepLogicFreeLogicContext(pPortData->LogicData.pvLogicContext);
    }

    PepCtrlLog("PepCtrlUninitPortData leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlInitPortTypeFuncs(
  _In_ TPepCtrlPortData* pPortData)
{
    PepCtrlLog("PepCtrlInitPortTypeFuncs entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlInitPortTypeFuncs - Port Data pointer: 0x%p  (Thread: 0x%p)\n",
               pPortData, PsGetCurrentThread());

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType)
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with Parallel Port functions.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocParallelPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeParallelPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitParallelPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteParallelPort;
        pPortData->Funcs.pGetDeviceInterfaceGuidFunc = &PepCtrlGetParallelPortDevInterfaceGuid;
    }
    else if (pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with Usb Port functions.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocUsbPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeUsbPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitUsbPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteUsbPort;
        pPortData->Funcs.pGetDeviceInterfaceGuidFunc = &PepCtrlGetUsbPortDevInterfaceGuid;
    }
    else
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with no functions.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        pPortData->Funcs.pAllocPortFunc = NULL;
        pPortData->Funcs.pFreePortFunc = NULL;
        pPortData->Funcs.pReadBitPortFunc = NULL;
        pPortData->Funcs.pWritePortFunc = NULL;
        pPortData->Funcs.pGetDeviceInterfaceGuidFunc = NULL;
    }

    PepCtrlLog("PepCtrlInitPortTypeFuncs leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
