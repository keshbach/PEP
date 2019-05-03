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
#include "PepCtrlLog.h"

#include <Drivers/PepCtrlDefs.h>

#include <Utils/UtHeapDriver.h>

#include <UtilsPep/UtPepLogic.h>

static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(_In_ PVOID pvContext, _Out_ PBOOLEAN pbValue);
static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(_In_ PVOID pvContext, _In_ PUCHAR pucData, _In_ ULONG ulDataLen);
static VOID __cdecl lPepLogicLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, lPepLogicReadBitPort)
#pragma alloc_text (PAGE, lPepLogicWritePort)

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

    return pPortData->Funcs.pReadBitPortFunc(&pPortData->Object, pbValue);
}

static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(
  _In_ PVOID pvContext,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen)
{
    TPepCtrlPortData* pPortData = (TPepCtrlPortData*)pvContext;

    PAGED_CODE()

    return pPortData->Funcs.pWritePortFunc(&pPortData->Object, pucData, ulDataLen);
}

static VOID __cdecl lPepLogicLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...)
{
    va_list arguments;

    va_start(arguments, pszFormat);

    PepCtrlLog(pszFormat, arguments);

    va_end(arguments);
}

#pragma endregion

BOOLEAN PepCtrlInitPortData(
  _In_ PUNICODE_STRING pRegistryPath,
  _In_ TPepCtrlPortData* pPortData)
{
    BOOLEAN bResult = FALSE;
    ULONG ulPortType;

    PepCtrlLog("PepCtrlInitPortData called.\n");

    PAGED_CODE()

    RtlZeroMemory(pPortData, sizeof(TPepCtrlPortData));

    PepCtrlLog("PepCtrlInitPortData - Calling UtPepLogicAllocLogicContext.\n");

    pPortData->LogicData.pvLogicContext = UtPepLogicAllocLogicContext();

    if (pPortData->LogicData.pvLogicContext == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData - Call to UtPepLogicAllocLogicContext failed.\n");

        return FALSE;
    }

    pPortData->LogicData.pvDeviceContext = pPortData;
    pPortData->LogicData.pReadBitPortFunc = lPepLogicReadBitPort;
    pPortData->LogicData.pWritePortFunc = lPepLogicWritePort;
    pPortData->LogicData.pLogFunc = lPepLogicLog;
    pPortData->bPortEjected = TRUE;

    ExInitializeFastMutex(&pPortData->FastMutex);

    PepCtrlLog("PepCtrlInitPortData - Allocating memory for the registry path.\n");

    pPortData->RegSettings.pszRegistryPath = (LPWSTR)UtAllocPagedMem((pRegistryPath->Length + 1) * sizeof(WCHAR));

    if (pPortData->RegSettings.pszRegistryPath == NULL)
    {
        PepCtrlLog("PepCtrlInitPortData - Could not allocate memory for the registry path.\n");

        UtPepLogicFreeLogicContext(pPortData->LogicData.pvLogicContext);

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

        UtPepLogicFreeLogicContext(pPortData->LogicData.pvLogicContext);

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

        UtPepLogicFreeLogicContext(pPortData->LogicData.pvLogicContext);
    }

    return bResult;
}

VOID PepCtrlUninitPortData(
  _In_ TPepCtrlPortData* pPortData)
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

    if (pPortData->LogicData.pvLogicContext)
    {
        PepCtrlLog("PepCtrlUninitPortData - Calling UtPepLogicFreeLogicContext.\n");

        UtPepLogicFreeLogicContext(pPortData->LogicData.pvLogicContext);
    }
}

VOID PepCtrlInitPortTypeFuncs(
  _In_ TPepCtrlPortData* pPortData)
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
