/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include <SetupPep/Includes/UtPepUtils.inl>

#include <Includes/UtExtractResource.inl>

#include <Config/UtPepCtrlCfg.h>

#include <Utils/UtHeap.h>

#pragma region Constants

#define CLogMessagePrefix L"PepCustomAction: "

#define CUtPepCtrlCfgDllFileName L"UtPepCtrlCfg.dll"

#define CPepCtrlSysFileName L"PepCtrl.sys"

#define CCustomActionDataName L"CustomActionData"

#define CDriverPathName L"Driver"

#pragma endregion

#pragma region Function Definitions

typedef BOOL (UTPEPCTRLCFGAPI* TUtPepCtrlCfgInitialize)(VOID);
typedef BOOL (UTPEPCTRLCFGAPI* TUtPepCtrlCfgUninitialize)(VOID);
typedef BOOL (UTPEPCTRLCFGAPI* TUtPepCtrlCfgInstallDriver)(__in LPCWSTR pszFile, __in TUtPepCtrlCfgMsgFunc pMsgFunc);
typedef BOOL (UTPEPCTRLCFGAPI* TUtPepCtrlCfgUninstallDriver)(__in TUtPepCtrlCfgMsgFunc pMsgFunc);
typedef BOOL (UTPEPCTRLCFGAPI* TUtPepCtrlCfgStartDriver)(__in TUtPepCtrlCfgMsgFunc pMsgFunc);
typedef BOOL (UTPEPCTRLCFGAPI* TUtPepCtrlCfgStopDriver)(__in TUtPepCtrlCfgMsgFunc pMsgFunc);

#pragma endregion

#pragma region Structures

typedef struct TCustomActionData
{
	LPTSTR pszProductPath;
} TCustomActionData;

#pragma endregion

#pragma region Local Variables

static MSIHANDLE l_hInstall = NULL;
static TCHAR l_cTempPath[MAX_PATH] = {0};
static TCHAR l_cUtPepCtrlDllFileName[MAX_PATH] = {0};
static HMODULE l_hUtPepCtrl = NULL;

#pragma endregion

HMODULE UtGetInstance(VOID);

#pragma region Local Functions

static BOOL lWriteMsiLogMessage(
  __in MSIHANDLE hInstall,
  __in LPCWSTR pszMessage)
{
    BOOL bResult(FALSE);
    MSIHANDLE hRecord(NULL);
    LPWSTR pszNewMessage;
    DWORD dwNewMessageLen;

    dwNewMessageLen = lstrlen(CLogMessagePrefix) + lstrlen(pszMessage) + 1;

	pszNewMessage = (LPWSTR)UtAllocMem(dwNewMessageLen * sizeof(WCHAR));

    if (pszNewMessage == NULL)
    {
        return FALSE;
    }

    hRecord = MsiCreateRecord(1);

    if (hRecord == NULL)
    {
		UtFreeMem(pszNewMessage);

        return FALSE;
    }

    if (ERROR_SUCCESS == MsiRecordSetString(hRecord, 1, pszMessage) &&
        ERROR_SUCCESS == MsiRecordSetString(hRecord, 0, CLogMessagePrefix L"[1]") &&
        ERROR_SUCCESS == MsiFormatRecord(hInstall, hRecord, pszNewMessage, &dwNewMessageLen))
    {
        bResult = TRUE;
    }

    MsiProcessMessage(hInstall, INSTALLMESSAGE_INFO, hRecord);

    MsiCloseHandle(hRecord);

	UtFreeMem(pszNewMessage);

    return bResult;
}

static LPWSTR lAllocCustomActionDataValue(
  __in MSIHANDLE hInstall)
{
	DWORD dwValueLen = 0;
	LPWSTR pszValue, pszTmpValue;

	lWriteMsiLogMessage(hInstall, L"lAllocCustomActionDataValue");

	pszValue = (LPWSTR)UtAllocMem(dwValueLen * sizeof(WCHAR));

	if (ERROR_MORE_DATA != ::MsiGetProperty(hInstall, CCustomActionDataName, pszValue, &dwValueLen))
	{
		lWriteMsiLogMessage(hInstall, L"Failed to retrieve the length of the custom action data");

		UtFreeMem(pszValue);

		return NULL;
	}

	++dwValueLen;

	pszTmpValue = (LPWSTR)UtReAllocMem(pszValue, dwValueLen * sizeof(WCHAR));

	if (pszTmpValue == NULL)
	{
        lWriteMsiLogMessage(hInstall, L"Failed to allocate memory for the custom action data");

        UtFreeMem(pszValue);

		return NULL;
	}

    pszValue = pszTmpValue;

	if (ERROR_SUCCESS != ::MsiGetProperty(hInstall, CCustomActionDataName, pszValue, &dwValueLen))
	{
		lWriteMsiLogMessage(hInstall, L"Failed to retrieve the custom action data");

		UtFreeMem(pszValue);

		return NULL;
	}

	lWriteMsiLogMessage(hInstall, pszValue);
    lWriteMsiLogMessage(hInstall, L"Successfully retrieved the custom action data");

	return pszValue;
}

static TCustomActionData* lAllocCustomActionData(
  __in MSIHANDLE hInstall)
{
	LPTSTR pszCustomActionDataValue;
	TCustomActionData* pCustomActionData;
	INT nDataLen;

	lWriteMsiLogMessage(hInstall, L"lAllocCustomActionData");

	pszCustomActionDataValue = lAllocCustomActionDataValue(hInstall);

	if (pszCustomActionDataValue == NULL)
	{
		return NULL;
	}

	pCustomActionData = (TCustomActionData*)UtAllocMem(sizeof(TCustomActionData));

	if (pCustomActionData == NULL)
	{
		UtFreeMem(pszCustomActionDataValue);

		return NULL;
	}

	// Extract product path

	nDataLen = ::lstrlen(pszCustomActionDataValue) + 1;

	pCustomActionData->pszProductPath = (LPTSTR)UtAllocMem(nDataLen * sizeof(TCHAR));

	if (pCustomActionData->pszProductPath == NULL)
	{
		UtFreeMem(pszCustomActionDataValue);
		UtFreeMem(pCustomActionData);

		return NULL;
	}

	::StringCchCopy(pCustomActionData->pszProductPath, nDataLen, pszCustomActionDataValue);

	return pCustomActionData;
}

static void lFreeCustomActionData(
  __in TCustomActionData* pCustomActionData)
{
	UtFreeMem(pCustomActionData->pszProductPath);
	UtFreeMem(pCustomActionData);
}

static VOID UTPEPCTRLCFGAPI lUtPepCtrlCfgMsg(
  __in LPCWSTR pszMsg)
{
    lWriteMsiLogMessage(l_hInstall, L"lUtPepCtrlCfgMsg");
    lWriteMsiLogMessage(l_hInstall, pszMsg);
}

static VOID lClearGlobals(VOID)
{
    lWriteMsiLogMessage(l_hInstall, L"lClearGlobals");

    l_hInstall = NULL;
    l_cTempPath[0] = 0;
    l_cUtPepCtrlDllFileName[0] = 0;
    l_hUtPepCtrl = NULL;
}

static BOOL lRollback(
  __in MSIHANDLE hInstall)
{
    lWriteMsiLogMessage(hInstall, L"lRollback");

    if (l_hUtPepCtrl)
    {
        lWriteMsiLogMessage(hInstall, L"Unloading the UtPepCtrl.dll file");

        if (!::FreeLibrary(l_hUtPepCtrl))
        {
            lWriteMsiLogMessage(hInstall, L"UtPepCtrl.dll file could not be unloaded");

            lClearGlobals();

            return FALSE;
        }
    }

    if (l_cUtPepCtrlDllFileName[0])
    {
        lWriteMsiLogMessage(hInstall, L"Removing the UtPepCtrl.dll file");

        if (!::DeleteFile(l_cUtPepCtrlDllFileName))
        {
            lWriteMsiLogMessage(hInstall, L"UtPepCtrl.dll file could not be removed");

            lClearGlobals();

            return FALSE;
        }
    }

    if (l_cTempPath[0])
    {
        lWriteMsiLogMessage(hInstall, L"Removing the temporary path");

        if (!::RemoveDirectory(l_cTempPath))
        {
            lWriteMsiLogMessage(hInstall, L"Could not remove the temporary path");

            lClearGlobals();

            return FALSE;
        }
    }

    lClearGlobals();

    return TRUE;
}

static BOOL lInitialize(
  __in MSIHANDLE hInstall)
{
    TUtPepCtrlCfgInitialize pUtPepCtrlCfgInitialize;
	BOOL bResult;

    lWriteMsiLogMessage(hInstall, L"lInitialize");

    lWriteMsiLogMessage(hInstall, L"Generating a temporary path");

    if (!UtGenerateTempDirectoryName(l_cTempPath, sizeof(l_cTempPath) / sizeof(l_cTempPath[0])))
    {
        lWriteMsiLogMessage(hInstall, L"Could not generate a temporary path");

        lClearGlobals();

        return FALSE;
    }

    lWriteMsiLogMessage(hInstall, L"Creating the temporary path");

    if (!::CreateDirectory(l_cTempPath, NULL))
    {
        lWriteMsiLogMessage(hInstall, L"Could not create the temporary path");

        lClearGlobals();

        return FALSE;
    }

    lWriteMsiLogMessage(hInstall, L"Extracting the UtPepCtrl.dll file");

    ::PathCombine(l_cUtPepCtrlDllFileName, l_cTempPath, CUtPepCtrlCfgDllFileName);

    lWriteMsiLogMessage(hInstall, l_cUtPepCtrlDllFileName);

    if (!UtExtractResource(UtGetInstance(), RT_BINARY, IDR_UTPEPCTRLCFGDLL, l_cUtPepCtrlDllFileName))
    {
        lWriteMsiLogMessage(hInstall, L"UtPepCtrl.dll file could not be extracted");

        bResult = lRollback(hInstall);

		return bResult;
    }

    lWriteMsiLogMessage(hInstall, L"Loading the UtPepCtrl.dll file");

    l_hUtPepCtrl = ::LoadLibrary(l_cUtPepCtrlDllFileName);

    if (l_hUtPepCtrl == NULL)
    {
        lWriteMsiLogMessage(hInstall, L"The UtPepCtrl.dll file could not be loaded");

        bResult = lRollback(hInstall);

		return bResult;
    }

    lWriteMsiLogMessage(hInstall, L"Initializing the UtPepCtrl.dll file");

    pUtPepCtrlCfgInitialize = (TUtPepCtrlCfgInitialize)::GetProcAddress(l_hUtPepCtrl, "UtPepCtrlCfgInitialize");

    if (!pUtPepCtrlCfgInitialize || !pUtPepCtrlCfgInitialize())
    {
        lWriteMsiLogMessage(hInstall, L"Could not initialize the UtPepCtrl.dll file");

        bResult = lRollback(hInstall);

		return bResult;
    }

    l_hInstall = hInstall;

    return TRUE;
}

static BOOL lUninitialize(VOID)
{
    TUtPepCtrlCfgUninitialize pUtPepCtrlCfgUninitialize;
	BOOL bResult;

    lWriteMsiLogMessage(l_hInstall, L"lUninitialize");

    if (l_hUtPepCtrl)
    {
        lWriteMsiLogMessage(l_hInstall, L"Uninitializing the UtPepCtrl.dll file");

        pUtPepCtrlCfgUninitialize = (TUtPepCtrlCfgUninitialize)::GetProcAddress(l_hUtPepCtrl, "UtPepCtrlCfgUninitialize");

        if (!pUtPepCtrlCfgUninitialize || !pUtPepCtrlCfgUninitialize())
        {
            lWriteMsiLogMessage(l_hInstall, L"Could not uninitialize the UtPepCtrl.dll file");

            lRollback(l_hInstall);

            return FALSE;
        }
    }

    bResult = lRollback(l_hInstall);

	return bResult;
}

static BOOL lCreateDriverDirectory(
   __in LPCWSTR pszDriverPath)
{
	lWriteMsiLogMessage(l_hInstall, L"lCreateDriverDirectory");
	lWriteMsiLogMessage(l_hInstall, pszDriverPath);

	if (!::CreateDirectory(pszDriverPath, NULL))
	{
		lWriteMsiLogMessage(l_hInstall, L"Could not create the driver directory");

		return FALSE;
	}

	lWriteMsiLogMessage(l_hInstall, L"Driver directory created");

	return TRUE;
}


static BOOL lRemoveDriverDirectory(
  __in LPCWSTR pszDriverPath)
{
	lWriteMsiLogMessage(l_hInstall, L"lRemoveDriverDirectory");
	lWriteMsiLogMessage(l_hInstall, pszDriverPath);

	if (!::RemoveDirectory(pszDriverPath))
	{
		lWriteMsiLogMessage(l_hInstall, L"Could not delete the driver directory");

		return FALSE;
	}

	lWriteMsiLogMessage(l_hInstall, L"Driver directory deleted");

	return TRUE;
}

static BOOL lCreateDriverFile(
  __in LPCWSTR pszDriverFile)
{
	lWriteMsiLogMessage(l_hInstall, L"lCreateDriverFile");
	lWriteMsiLogMessage(l_hInstall, pszDriverFile);

	if (!UtExtractResource(UtGetInstance(), RT_BINARY,
                           UtIsWindows64Present() ? IDR_PEPCTRLSYS64 : IDR_PEPCTRLSYS,
                           pszDriverFile))
	{
		lWriteMsiLogMessage(l_hInstall, L"Could not extract the PepCtrl.sys driver file");

		return FALSE;
	}

	lWriteMsiLogMessage(l_hInstall, L"Driver file created");

	return TRUE;
}

static BOOL lRemoveDriverFile(
  __in LPCWSTR pszDriverFile)
{
    lWriteMsiLogMessage(l_hInstall, L"lRemoveDriverFile");
	lWriteMsiLogMessage(l_hInstall, pszDriverFile);

    if (!::DeleteFile(pszDriverFile))
    {
        lWriteMsiLogMessage(l_hInstall, L"Could not delete the driver file");

        return FALSE;
    }

	lWriteMsiLogMessage(l_hInstall, L"Driver file deleted");

    return TRUE;
}

static BOOL lInstallDriver(
  __in LPCWSTR pszProductPath)
{
	WCHAR cDriverPath[MAX_PATH], cDriverFile[MAX_PATH];
    TUtPepCtrlCfgInstallDriver pUtPepCtrlCfgInstallDriver;
    TUtPepCtrlCfgUninstallDriver pUtPepCtrlCfgUninstallDriver;
    TUtPepCtrlCfgStartDriver pUtPepCtrlCfgStartDriver;

    lWriteMsiLogMessage(l_hInstall, L"lInstallDriver");

	::PathCombine(cDriverPath, pszProductPath, CDriverPathName);

	if (!lCreateDriverDirectory(cDriverPath))
	{
		return FALSE;
	}

	::PathCombine(cDriverFile, cDriverPath, CPepCtrlSysFileName);

	if (!lCreateDriverFile(cDriverFile))
	{
		lRemoveDriverDirectory(cDriverPath);

		return FALSE;
	}

    pUtPepCtrlCfgInstallDriver = (TUtPepCtrlCfgInstallDriver)::GetProcAddress(l_hUtPepCtrl, "UtPepCtrlCfgInstallDriver");
    pUtPepCtrlCfgUninstallDriver = (TUtPepCtrlCfgUninstallDriver)::GetProcAddress(l_hUtPepCtrl, "UtPepCtrlCfgUninstallDriver");
    pUtPepCtrlCfgStartDriver = (TUtPepCtrlCfgStartDriver)::GetProcAddress(l_hUtPepCtrl, "UtPepCtrlCfgStartDriver");

    if (!pUtPepCtrlCfgInstallDriver || !pUtPepCtrlCfgUninstallDriver ||
        !pUtPepCtrlCfgStartDriver)
    {
        lWriteMsiLogMessage(l_hInstall, L"Could not find exported functions");

		lRemoveDriverFile(cDriverFile);
		lRemoveDriverDirectory(cDriverPath);

        return FALSE;
    }

    lWriteMsiLogMessage(l_hInstall, L"Installing the PepCtrl.sys driver file (Calling UtPepCtrlCfgInstallDriver)");

    if (!pUtPepCtrlCfgInstallDriver(cDriverFile, lUtPepCtrlCfgMsg))
    {
        lWriteMsiLogMessage(l_hInstall, L"Could not install the PepCtrl.sys driver file");

		lRemoveDriverFile(cDriverFile);
		lRemoveDriverDirectory(cDriverPath);

        return FALSE;
    }

    lWriteMsiLogMessage(l_hInstall, L"Setting the driver configuration settings (Calling UtPepCtrlCfgSetPortSettings)");

/*    lWriteMsiLogMessage(l_hInstall, L"Attempting to start the driver (Calling UtPepCtrlCfgStartDriver)");

	if (pUtPepCtrlCfgStartDriver(lUtPepCtrlCfgMsg))
	{
		lWriteMsiLogMessage(l_hInstall, L"Driver started");
	}
	else
	{
		lWriteMsiLogMessage(l_hInstall, L"Driver failed to start");
	}*/

    return TRUE;
}

static BOOL lRemoveDriver(
  __in LPCWSTR pszProductPath)
{
    TUtPepCtrlCfgUninstallDriver pUtPepCtrlCfgUninstallDriver;
	WCHAR cDriverPath[MAX_PATH], cDriverFile[MAX_PATH];

    lWriteMsiLogMessage(l_hInstall, L"lRemoveDriver");

    pUtPepCtrlCfgUninstallDriver = (TUtPepCtrlCfgUninstallDriver)::GetProcAddress(l_hUtPepCtrl, "UtPepCtrlCfgUninstallDriver");

    if (!pUtPepCtrlCfgUninstallDriver)
    {
        lWriteMsiLogMessage(l_hInstall, L"Could not find exported functions");

        return FALSE;
    }

    lWriteMsiLogMessage(l_hInstall, L"Uninstalling the PepCtrl.sys driver (Calling UtPepCtrlCfgUninstallDriver)");

    if (!pUtPepCtrlCfgUninstallDriver(lUtPepCtrlCfgMsg))
    {
        lWriteMsiLogMessage(l_hInstall, L"Could not uninstall the PepCtrl.sys driver");

        return FALSE;
    }

	::PathCombine(cDriverPath, pszProductPath, CDriverPathName);
	::PathCombine(cDriverFile, cDriverPath, CPepCtrlSysFileName);

	if (!lRemoveDriverFile(cDriverFile))
	{
		return FALSE;
	}

	if (!lRemoveDriverDirectory(cDriverPath))
	{
		return FALSE;
	}

	return TRUE;
}

#pragma endregion

#pragma region Exported Function

extern "C"
{

UINT __stdcall InstallDriverCustomAction(
  __in MSIHANDLE hInstall)
{
	if (UtInitHeap())
	{
		lWriteMsiLogMessage(hInstall, L"InstallDriverCustomAction");

		UtUninitHeap();
	}	

    return ERROR_SUCCESS;
}

UINT __stdcall InstallCommitDriverCustomAction(
  __in MSIHANDLE hInstall)
{
	TCustomActionData* pCustomActionData;

	if (!UtInitHeap())
	{
		return ERROR_INSTALL_FAILURE;
	}

    lWriteMsiLogMessage(hInstall, L"InstallCommitDriverCustomAction");

    if (!lInitialize(hInstall))
    {
		UtUninitHeap();
		
		return ERROR_INSTALL_FAILURE;
    }

    lWriteMsiLogMessage(hInstall, L"Retrieving the CustomActionData");

	pCustomActionData = lAllocCustomActionData(hInstall);

    if (pCustomActionData == NULL)
    {
        lWriteMsiLogMessage(hInstall, L"Could not retrieve the CustomActionData");

        lUninitialize();

		UtUninitHeap();

        return ERROR_INSTALL_FAILURE;
    }

    lWriteMsiLogMessage(hInstall, L"Attempting to install the PepCtrl.sys driver file");

    if (!lInstallDriver(pCustomActionData->pszProductPath))
    {
        lWriteMsiLogMessage(hInstall, L"Could not install the PepCtrl.sys driver file");

		lFreeCustomActionData(pCustomActionData);

        lUninitialize();

		UtUninitHeap();

        return ERROR_INSTALL_FAILURE;
    }

	lFreeCustomActionData(pCustomActionData);

    if (!lUninitialize())
    {
		UtUninitHeap();
		
		return ERROR_INSTALL_FAILURE;
    }

	UtUninitHeap();

    return ERROR_SUCCESS;
}

UINT __stdcall InstallRollbackDriverCustomAction(
  __in MSIHANDLE hInstall)
{
	if (UtInitHeap())
	{
		lWriteMsiLogMessage(hInstall, L"InstallRollbackDriverCustomAction");

		UtUninitHeap();
	}

    return ERROR_SUCCESS;
}

UINT __stdcall RemoveDriverCustomAction(
  __in MSIHANDLE hInstall)
{
	if (UtInitHeap())
	{
		lWriteMsiLogMessage(hInstall, L"RemoveDriverCustomAction");

		UtUninitHeap();
	}

    return ERROR_SUCCESS;
}

UINT __stdcall RemoveCommitDriverCustomAction(
  __in MSIHANDLE hInstall)
{
	TCustomActionData* pCustomActionData;

	if (!UtInitHeap())
	{
		return ERROR_INSTALL_FAILURE;
	}
	
	lWriteMsiLogMessage(hInstall, L"RemoveCommitDriverCustomAction");

    if (!lInitialize(hInstall))
    {
		UtUninitHeap();
		
		return ERROR_INSTALL_FAILURE;
    }

	lWriteMsiLogMessage(hInstall, L"Retrieving the CustomActionData");

	pCustomActionData = lAllocCustomActionData(hInstall);

	if (pCustomActionData == NULL)
	{
		lWriteMsiLogMessage(hInstall, L"Could not retrieve the CustomActionData");

		lUninitialize();

		UtUninitHeap();

		return ERROR_INSTALL_FAILURE;
	}

    if (!lRemoveDriver(pCustomActionData->pszProductPath))
    {
		lWriteMsiLogMessage(hInstall, L"Could not remove the driver");

		lFreeCustomActionData(pCustomActionData);
		
		lUninitialize();

		UtUninitHeap();

        return ERROR_INSTALL_FAILURE;
    }

	lFreeCustomActionData(pCustomActionData);

    if (!lUninitialize())
    {
		UtUninitHeap();
		
		return ERROR_INSTALL_FAILURE;
    }

	UtUninitHeap();

    return ERROR_SUCCESS;
}

UINT __stdcall RemoveRollbackDriverCustomAction(
  __in MSIHANDLE hInstall)
{
	if (UtInitHeap())
	{
	    lWriteMsiLogMessage(hInstall, L"RemoveRollbackDriverCustomAction");

		UtUninitHeap();
    }

    return ERROR_SUCCESS;
}

};

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
