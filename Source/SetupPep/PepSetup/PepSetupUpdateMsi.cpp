/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepSetupUpdateMsi.h"

#include "PepSetupResources.h"

#include "UtPepSetup.h"

#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>

#include <Config/UtListPorts.h>

#pragma region Constants

#define CTotalParallelPortsQueryStatement L"SELECT * FROM Property WHERE Property = 'TotalParallelPorts'"
#define CParallelPortDevObjNameQueryStatement L"SELECT * FROM Property WHERE Property = 'ParallelPortDevObjName'"
#define CPortTextQueryStatement L"SELECT * FROM Control WHERE Dialog_ = 'ConfigPortDlg' AND Control = 'PortText'"

#define CPortTextControlTextStart L"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033{\\fonttbl{\\f0\\froman Tms Rmn;}}\\viewkind4\\uc1\\pardf0\\fs20 "
#define CPortTextControlTextEnd L"\r\n\\par}"

#define CDisableDriverEnforcementWin7Msg \
L"{\\par}{\\par\\b Note:}" \
L"{\\par After turning on your computer immediately press the \"F8\" key and select the \"Disable Driver Signature Enforcement\" option from the \"Advanced Boot Options\" screen.}"

#define CDisableDriverEnforcementWin8xMsg \
L"{\\par}{\\par\\b Note:}" \
L"{\\par Driver signature enforcement must be disabled by using the following steps.}" \
L"{\\par}" \
L"{\\par}" \
L"\\viewkind4\\uc1" \
L"\\pard" \
L"{ \\pntext 1.\\tab }" \
L"{\\*\\pn\\pnlvlbody\\pnf0\\pnindent0\\pnstart1\\pndec{\\pntxta.}}" \
L"\\fi-360\\li360\\sl240\\slmult1 After your computer has started hold down the \"Shift\" key while clicking the \"Restart\" option.\\par" \
L"{ \\pntext 2.\\tab }Select the \"Troubleshoot\" tile on the \"Choose an option\" screen that appears.\\par" \
L"{ \\pntext 3.\\tab }Next select the \"Advanced options\" tile.\\par" \
L"{ \\pntext 4.\\tab }Next select the \"Startup Settings\" tile.\\par" \
L"{ \\pntext 5.\\tab }Click the \"Restart\" button to restart your PC into the \"Startup Settings\" screen.\\par" \
L"{ \\pntext 6.\\tab }Type \"7\" at the Startup Settings screen to activate the \"Disable driver signature enforcement\" option.\\par" \
L"}"

#define CDisableDriverEnforcementWin10Msg \
L"{\\par}{\\par\\b Note:}" \
L"{\\par Driver signature enforcement must be disabled by using the following steps after your computer has started.}" \
L"{\\par}" \
L"{\\par}" \
L"\\viewkind4\\uc1" \
L"\\pard" \
L"{ \\pntext 1.\\tab }" \
L"{\\*\\pn\\pnlvlbody\\pnf0\\pnindent0\\pnstart1\\pndec{\\pntxta.}}" \
L"\\fi-360\\li360\\sl240\\slmult1 Click the \"Start\" menu and select \"Settings\".\\par" \
L"{ \\pntext 2.\\tab }Select the \"Update and Security\" option.\\par" \
L"{ \\pntext 3.\\tab }Next select the \"Recovery\" option.\\par" \
L"{ \\pntext 4.\\tab }Next select the \"Restart now\" under \"Advanced Startup\".\\par" \
L"{ \\pntext 5.\\tab }Next select the \"Troubleshoot\" option.\\par" \
L"{ \\pntext 6.\\tab }Next select the \"Advanced options\" option.\\par" \
L"{ \\pntext 7.\\tab }Next select the \"Startup Settings\" option.\\par" \
L"{ \\pntext 8.\\tab }Next select the \"Restart\" option.\\par" \
L"{ \\pntext 9.\\tab }Type \"7\" or \"F7\" at the Startup Settings screen to activate the \"Disable driver signature enforcement\" option.\\par" \
L"}"

#define CNoParallelPortsFoundMsg L"No parallel ports were found.  Please add a parallel port and then restart the installation."

#define COneParallelPortFoundMsgStart L"The \""
#define COneParallelPortFoundMsgEnd L"\" parallel port will be used."

#define CMultiParallelPortsFoundMsgStart L"Multiple parallel ports were found but the \""
#define CMultiParallelPortsFoundMsgEnd L"\" will be used.  If this is wrong change the parallel port being used with the CfgPepCtrl tool after the installation completes."

#pragma endregion

#pragma region Function Definitions

typedef BOOL(UTLISTPORTSAPI* TUtListPortsInitialize)(VOID);
typedef BOOL(UTLISTPORTSAPI* TUtListPortsUninitialize)(VOID);
typedef BOOL(UTLISTPORTSAPI* TUtListPortsGetLptPortCount)(__out LPINT pnCount);
typedef BOOL(UTLISTPORTSAPI* TUtListPortsGetLptPortData)(__in INT nIndex, __in INT nPortData, __out_opt LPWSTR pszData, __out LPINT pnDataLen);

#pragma endregion

#pragma region Structures

typedef struct tagTParallelPort
{
    LPWSTR pszParallelPortName;
    LPWSTR pszDeviceObjectName;
} TParallelPort;

typedef struct tagTParallelPortData
{
    INT nTotalPorts;
    TParallelPort ParallelPorts[1];
} TParallelPortData;

#pragma endregion

#pragma region Local Functions

static LPCWSTR lGetDisableDriverEnforcementMsg()
{
    DWORDLONG nConditionMask(0);
    OSVERSIONINFOEX VersionInfoEx;

    // Check for Windows 10 (internally version is 10.0)

    VersionInfoEx.dwOSVersionInfoSize = sizeof(VersionInfoEx);
    VersionInfoEx.dwMajorVersion = 10; // set for Windows 10 and above
    VersionInfoEx.dwMinorVersion = 0;

    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MAJORVERSION,
                                           VER_GREATER_EQUAL);
    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MINORVERSION,
                                           VER_GREATER_EQUAL);

    if (::VerifyVersionInfo(&VersionInfoEx, VER_MAJORVERSION | VER_MINORVERSION,
                            nConditionMask))
    {
        return CDisableDriverEnforcementWin10Msg;
    }

    // Check for Windows 8.x (internally version is 6.2 for Windows 8 and 6.3 for Windows 8.1)

    VersionInfoEx.dwOSVersionInfoSize = sizeof(VersionInfoEx);
    VersionInfoEx.dwMajorVersion = 6; // set to 6 for Windows 7 and above
    VersionInfoEx.dwMinorVersion = 2; // set for Windows 8 and above

    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MAJORVERSION,
                                           VER_GREATER_EQUAL);
    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MINORVERSION,
                                           VER_GREATER_EQUAL);

    if (::VerifyVersionInfo(&VersionInfoEx, VER_MAJORVERSION | VER_MINORVERSION,
                            nConditionMask))
    {
        return CDisableDriverEnforcementWin8xMsg;
    }

    // Have Windows 7 (internally version is 6.1)

    return CDisableDriverEnforcementWin7Msg;
}

static VOID lFreeParallelPortData(
  _In_ TParallelPortData* pParallelPortData)
{
    for (INT nIndex = 0; nIndex < pParallelPortData->nTotalPorts; ++nIndex)
    {
        if (pParallelPortData->ParallelPorts[nIndex].pszParallelPortName)
        {
            UtFreeMem(pParallelPortData->ParallelPorts[nIndex].pszParallelPortName);
        }

        if (pParallelPortData->ParallelPorts[nIndex].pszDeviceObjectName)
        {
            UtFreeMem(pParallelPortData->ParallelPorts[nIndex].pszDeviceObjectName);
        }
    }

    UtFreeMem(pParallelPortData);
}

static BOOL lAllocParallelPortData(
  _Out_writes_to_ptr_(ppParallelPortData) TParallelPortData** ppParallelPortData)
{
    BOOL bResult = FALSE;
    HMODULE hModule;
    INT nCount, nDataLen;
    TUtListPortsInitialize pUtListPortsInitialize;
    TUtListPortsUninitialize pUtListPortsUninitialize;
    TUtListPortsGetLptPortCount pUtListPortsGetLptPortCount;
    TUtListPortsGetLptPortData pUtListPortsGetLptPortData;

    *ppParallelPortData = NULL;

    hModule = ::LoadLibrary(PepSetupGetListPortsFile());

    if (hModule == NULL)
    {
        return NULL;
    }

    pUtListPortsInitialize = (TUtListPortsInitialize)GetProcAddress(hModule, "UtListPortsInitialize");
    pUtListPortsUninitialize = (TUtListPortsUninitialize)GetProcAddress(hModule, "UtListPortsUninitialize");
    pUtListPortsGetLptPortCount = (TUtListPortsGetLptPortCount)GetProcAddress(hModule, "UtListPortsGetLptPortCount");
    pUtListPortsGetLptPortData = (TUtListPortsGetLptPortData)GetProcAddress(hModule, "UtListPortsGetLptPortData");

    if (pUtListPortsInitialize == NULL || pUtListPortsUninitialize == NULL ||
        pUtListPortsGetLptPortCount == NULL || pUtListPortsGetLptPortData == NULL)
    {
        goto End;
    }

    if (!pUtListPortsInitialize())
    {
        goto End;
    }

    if (!pUtListPortsGetLptPortCount(&nCount))
    {
        goto UninitializeEnd;
    }

    nDataLen = sizeof(TParallelPortData) + ((nCount - 1) * sizeof(TParallelPort));

    *ppParallelPortData = (TParallelPortData*)UtAllocMem(nDataLen);

    if (*ppParallelPortData == NULL)
    {
        goto UninitializeEnd;
    }

    ::ZeroMemory(*ppParallelPortData, nDataLen);

    (*ppParallelPortData)->nTotalPorts = nCount;

    for (INT nIndex = 0; nIndex < nCount; ++nIndex)
    {
        // Retrieve the parallel port's friendly name

        if (!pUtListPortsGetLptPortData(nIndex, epdLocation, NULL, &nDataLen))
        {
            lFreeParallelPortData(*ppParallelPortData);

            *ppParallelPortData = NULL;

            goto UninitializeEnd;
        }

        (*ppParallelPortData)->ParallelPorts[nIndex].pszParallelPortName = (LPWSTR)UtAllocMem(nDataLen * sizeof(WCHAR));

        if ((*ppParallelPortData)->ParallelPorts[nIndex].pszParallelPortName == NULL)
        {
            lFreeParallelPortData(*ppParallelPortData);

            *ppParallelPortData = NULL;

            goto UninitializeEnd;
        }

        if (!pUtListPortsGetLptPortData(nIndex, epdLocation, (*ppParallelPortData)->ParallelPorts[nIndex].pszParallelPortName, &nDataLen))
        {
            lFreeParallelPortData(*ppParallelPortData);

            *ppParallelPortData = NULL;

            goto UninitializeEnd;
        }

        // Retrieve the parallel port's physical device object name

        if (!pUtListPortsGetLptPortData(nIndex, epdPhysicalDeviceObjectName, NULL, &nDataLen))
        {
            lFreeParallelPortData(*ppParallelPortData);

            *ppParallelPortData = NULL;

            goto UninitializeEnd;
        }

        (*ppParallelPortData)->ParallelPorts[nIndex].pszDeviceObjectName = (LPWSTR)UtAllocMem(nDataLen * sizeof(WCHAR));

        if ((*ppParallelPortData)->ParallelPorts[nIndex].pszDeviceObjectName == NULL)
        {
            lFreeParallelPortData(*ppParallelPortData);

            *ppParallelPortData = NULL;

            goto UninitializeEnd;
        }

        if (!pUtListPortsGetLptPortData(nIndex, epdPhysicalDeviceObjectName, (*ppParallelPortData)->ParallelPorts[nIndex].pszDeviceObjectName, &nDataLen))
        {
            lFreeParallelPortData(*ppParallelPortData);

            *ppParallelPortData = NULL;

            goto UninitializeEnd;
        }
    }

    bResult = TRUE;

UninitializeEnd:
    pUtListPortsUninitialize();

End:
    ::FreeLibrary(hModule);

    return bResult;
}

static LPWSTR lAllocPortTextControlText_NoPorts()
{
    INT nTextLen(1);
    LPWSTR pszText;

    nTextLen += ::lstrlen(CPortTextControlTextStart);
    nTextLen += ::lstrlen(CPortTextControlTextEnd);
    nTextLen += ::lstrlen(CNoParallelPortsFoundMsg);

    pszText = (LPWSTR)UtAllocMem(nTextLen * sizeof(WCHAR));

    if (pszText == NULL)
    {
        return NULL;
    }

    ::StringCchCopy(pszText, nTextLen, CPortTextControlTextStart);
    ::StringCchCat(pszText, nTextLen, CNoParallelPortsFoundMsg);
    ::StringCchCat(pszText, nTextLen, CPortTextControlTextEnd);

    return pszText;
}

static LPWSTR lAllocPortTextControlText_OnePort(
  _In_z_ LPCWSTR pszParallelPort)
{
    INT nTextLen(1);
    LPWSTR pszText;

    nTextLen += ::lstrlen(CPortTextControlTextStart);
    nTextLen += ::lstrlen(CPortTextControlTextEnd);
    nTextLen += ::lstrlen(pszParallelPort);
    nTextLen += ::lstrlen(COneParallelPortFoundMsgStart);
    nTextLen += ::lstrlen(COneParallelPortFoundMsgEnd);

    if (UtPepSetupIsWindows64Present())
    {
        nTextLen += ::lstrlen(lGetDisableDriverEnforcementMsg());
    }

    pszText = (LPWSTR)UtAllocMem(nTextLen * sizeof(WCHAR));

    if (pszText == NULL)
    {
        return NULL;
    }

    ::StringCchCopy(pszText, nTextLen, CPortTextControlTextStart);
    ::StringCchCat(pszText, nTextLen, COneParallelPortFoundMsgStart);
    ::StringCchCat(pszText, nTextLen, pszParallelPort);
    ::StringCchCat(pszText, nTextLen, COneParallelPortFoundMsgEnd);

    if (UtPepSetupIsWindows64Present())
    {
        ::StringCchCat(pszText, nTextLen, lGetDisableDriverEnforcementMsg());
    }

    ::StringCchCat(pszText, nTextLen, CPortTextControlTextEnd);

    return pszText;
}

static LPWSTR lAllocPortTextControlText_MultiPorts(
  _In_z_ LPCWSTR pszSelectedParallelPort)
{
    INT nTextLen(1);
    LPWSTR pszText;

    nTextLen += ::lstrlen(CPortTextControlTextStart);
    nTextLen += ::lstrlen(CPortTextControlTextEnd);
    nTextLen += ::lstrlen(pszSelectedParallelPort);
    nTextLen += ::lstrlen(CMultiParallelPortsFoundMsgStart);
    nTextLen += ::lstrlen(CMultiParallelPortsFoundMsgEnd);
    nTextLen += ::lstrlen(lGetDisableDriverEnforcementMsg());

    pszText = (LPWSTR)UtAllocMem(nTextLen * sizeof(WCHAR));

    if (pszText == NULL)
    {
        return NULL;
    }

    ::StringCchCopy(pszText, nTextLen, CPortTextControlTextStart);
    ::StringCchCat(pszText, nTextLen, CMultiParallelPortsFoundMsgStart);
    ::StringCchCat(pszText, nTextLen, pszSelectedParallelPort);
    ::StringCchCat(pszText, nTextLen, CMultiParallelPortsFoundMsgEnd);

    if (UtPepSetupIsWindows64Present())
    {
        ::StringCchCat(pszText, nTextLen, lGetDisableDriverEnforcementMsg());
    }

    ::StringCchCat(pszText, nTextLen, CPortTextControlTextEnd);

    return pszText;
}

static BOOL lUpdateParallelPortMessage(
  _In_ MSIHANDLE hDatabase,
  _In_z_ LPCWSTR pszParallelPort,
  _In_ INT nParallelPortCount)
{
    BOOL bResult(FALSE);
    MSIHANDLE hView(NULL), hRecord(NULL);
    LPWSTR pszControlText;

    if (nParallelPortCount == 0)
    {
        pszControlText = lAllocPortTextControlText_NoPorts();
    }
    else
    {
        if (nParallelPortCount == 1)
        {
            pszControlText = lAllocPortTextControlText_OnePort(pszParallelPort);
        }
        else
        {
            pszControlText = lAllocPortTextControlText_MultiPorts(pszParallelPort);
        }
    }

    if (pszControlText != NULL &&
        ERROR_SUCCESS == ::MsiDatabaseOpenView(hDatabase, CPortTextQueryStatement, &hView) &&
        ERROR_SUCCESS == ::MsiViewExecute(hView, 0) &&
        ERROR_SUCCESS == ::MsiViewFetch(hView, &hRecord) &&
        ERROR_SUCCESS == ::MsiRecordSetString(hRecord, 10, pszControlText) &&
        ERROR_SUCCESS == ::MsiViewModify(hView, MSIMODIFY_UPDATE, hRecord))
    {
        bResult = TRUE;
    }

    if (pszControlText)
    {
        UtFreeMem(pszControlText);
    }

    if (hRecord)
    {
        MsiCloseHandle(hRecord);
    }

    if (hView)
    {
        MsiCloseHandle(hView);
    }

    return bResult;
}

static BOOL lUpdateParallelPortData(
  _In_ MSIHANDLE hDatabase,
  _In_ const TParallelPortData* pParallelPortData)
{
    BOOL bResult(FALSE);
    MSIHANDLE hView(NULL), hRecord(NULL);
    WCHAR cNumber[10];
    LPWSTR pszDevObjName;

    if (pParallelPortData->nTotalPorts > 0)
    {
        if (!lUpdateParallelPortMessage(hDatabase, pParallelPortData->ParallelPorts[0].pszParallelPortName,
            pParallelPortData->nTotalPorts))
        {
            return FALSE;
        }
    }
    else
    {
        if (!lUpdateParallelPortMessage(hDatabase, L"", 0))
        {
            return FALSE;
        }
    }

    // Update the total parallel ports property

    ::StringCchPrintf(cNumber, MArrayLen(cNumber), L"%d", pParallelPortData->nTotalPorts);

    if (ERROR_SUCCESS == ::MsiDatabaseOpenView(hDatabase, CTotalParallelPortsQueryStatement, &hView) &&
        ERROR_SUCCESS == ::MsiViewExecute(hView, 0) &&
        ERROR_SUCCESS == ::MsiViewFetch(hView, &hRecord) &&
        ERROR_SUCCESS == ::MsiRecordSetString(hRecord, 2, cNumber) &&
        ERROR_SUCCESS == ::MsiViewModify(hView, MSIMODIFY_UPDATE, hRecord))
    {
        bResult = TRUE;
    }

    if (hView)
    {
        ::MsiCloseHandle(hView);

        hView = NULL;
    }

    if (hRecord)
    {
        ::MsiCloseHandle(hRecord);

        hRecord = NULL;
    }

    if (bResult == FALSE)
    {
        return FALSE;
    }

    // Update the parallel port device object name property

    bResult = FALSE;

    if (pParallelPortData->nTotalPorts > 0)
    {
        pszDevObjName = pParallelPortData->ParallelPorts[0].pszDeviceObjectName;
    }
    else
    {
        pszDevObjName = (LPWSTR)L" ";
    }

    if (ERROR_SUCCESS == ::MsiDatabaseOpenView(hDatabase, CParallelPortDevObjNameQueryStatement, &hView) &&
        ERROR_SUCCESS == ::MsiViewExecute(hView, 0) &&
        ERROR_SUCCESS == ::MsiViewFetch(hView, &hRecord) &&
        ERROR_SUCCESS == ::MsiRecordSetString(hRecord, 2, pszDevObjName) &&
        ERROR_SUCCESS == ::MsiViewModify(hView, MSIMODIFY_UPDATE, hRecord))
    {
        bResult = TRUE;
    }

    if (hView)
    {
        ::MsiCloseHandle(hView);
    }

    if (hRecord)
    {
        ::MsiCloseHandle(hRecord);
    }

    return bResult;
}

#pragma endregion

#pragma region Public Functions

BOOL PepSetupUpdateMsi(
  _In_opt_z_ LPCWSTR pszMsiFile)
{
    BOOL bResult(TRUE);
    TParallelPortData* pParallelPortData = NULL;
    MSIHANDLE hDatabase;

    if (!lAllocParallelPortData(&pParallelPortData))
    {
        return FALSE;
    }

    if (ERROR_SUCCESS != ::MsiOpenDatabase(pszMsiFile, MSIDBOPEN_TRANSACT, &hDatabase))
    {
        lFreeParallelPortData(pParallelPortData);

        return FALSE;
    }

    if (!lUpdateParallelPortData(hDatabase, pParallelPortData))
    {
        bResult = FALSE;
    }

    if (ERROR_SUCCESS != ::MsiDatabaseCommit(hDatabase))
    {
        bResult = FALSE;
    }

    if (ERROR_SUCCESS != ::MsiCloseHandle(hDatabase))
    {
        bResult = FALSE;
    }

    lFreeParallelPortData(pParallelPortData);

    return bResult;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
