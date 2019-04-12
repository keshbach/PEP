/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include "PepCtrlPortData.h"

#include "PepCtrlInit.h"

#include "PepCtrlParallelPort.h"
#include "PepCtrlUsbPort.h"
#include "PepCtrlReg.h"
#include "PepCtrlLogic.h"
#include "PepCtrlLog.h"

#include <Drivers/PepCtrlDefs.h>

#include <Utils/UtHeapDriver.h>

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlInitPortData)
#pragma alloc_text (PAGE, PepCtrlUninitPortData)
#pragma alloc_text (PAGE, PepCtrlInitPortTypeFuncs)
#endif

BOOLEAN PepCtrlInitPortData(
  IN PUNICODE_STRING pRegistryPath,
  IN TPepCtrlPortData* pPortData)
{
    BOOLEAN bResult = FALSE;
    ULONG ulPortType;

    PepCtrlLog("PepCtrlInitPortData called.\n");

    PAGED_CODE()

    RtlZeroMemory(pPortData, sizeof(TPepCtrlPortData));

    pPortData->bPortEjected = TRUE;

    ExInitializeFastMutex(&pPortData->FastMutex);

    PepCtrlInitModes(pPortData);

    PepCtrlLog("PepCtrlInitPortData - Allocating memory for the registry path.\n");

    pPortData->RegSettings.pszRegistryPath = (LPWSTR)UtAllocPagedMem((pRegistryPath->Length + 1) * sizeof(WCHAR));

    if (pPortData->RegSettings.pszRegistryPath == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData - Could not allocate memory for the registry path.\n");

        return FALSE;
    }

    RtlCopyMemory(pPortData->RegSettings.pszRegistryPath, pRegistryPath->Buffer,
                  pRegistryPath->Length * sizeof(WCHAR));

    *(pPortData->RegSettings.pszRegistryPath + pRegistryPath->Length) = 0;

    PepCtrlLog("PepCtrlInitPortData - Reading the registry settings.\n");

    if (!PepCtrlReadRegSettings(pRegistryPath, &ulPortType,
                                &pPortData->RegSettings.pszPortDeviceName))
    {
        PepCtrlLog("PepCtrlInitPortData - Could not read the registry settings.\n");

        UtFreePagedMem(pPortData->RegSettings.pszRegistryPath);

        return FALSE;
    }

    pPortData->RegSettings.nPortType = (UINT32)ulPortType;

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType ||
        pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        PepCtrlInitPortTypeFuncs(pPortData);

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("PepCtrlInitPortData - Unknown Port type retrieved from the registry.\n");

        UtFreePagedMem(pPortData->RegSettings.pszRegistryPath);
        UtFreePagedMem(pPortData->RegSettings.pszPortDeviceName);
    }

    return bResult;
}

VOID PepCtrlUninitPortData(
  IN TPepCtrlPortData* pPortData)
{
    PepCtrlLog("PepCtrlUninitPortData called.\n");

    PAGED_CODE()

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
}

VOID PepCtrlInitPortTypeFuncs(IN TPepCtrlPortData* pPortData)
{
    PepCtrlLog("PepCtrlInitPortTypeFuncs called.\n");

    PAGED_CODE()

    if (pPortData->RegSettings.nPortType == CPepCtrlParallelPortType)
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with Parallel Port functions.\n");

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocParallelPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeParallelPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitParallelPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteParallelPort;
        pPortData->Funcs.pGetDevInterfaceGuidFunc = &PepCtrlGetParallelPortDevInterfaceGuid;
    }
    else if (pPortData->RegSettings.nPortType == CPepCtrlUsbPrintPortType)
    {
        PepCtrlLog("PepCtrlInitPortTypeFuncs - Initializing with Usb Port functions.\n");

        pPortData->Funcs.pAllocPortFunc = &PepCtrlAllocUsbPort;
        pPortData->Funcs.pFreePortFunc = &PepCtrlFreeUsbPort;
        pPortData->Funcs.pReadBitPortFunc = &PepCtrlReadBitUsbPort;
        pPortData->Funcs.pWritePortFunc = &PepCtrlWriteUsbPort;
        pPortData->Funcs.pGetDevInterfaceGuidFunc = &PepCtrlGetUsbPortDevInterfaceGuid;
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
