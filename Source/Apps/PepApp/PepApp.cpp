/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepApp.h"

#include "PepAppSplashDialog.h"

#include "UtPepApp.h"

#include <Includes/UtThread.inl>

#include <Includes/UtVersion.h>
#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>

#include <UtilsDevice/UtPepDevices.h>

#include <UtilsPep/UiPepCtrls.h>

#include <Hosts/PepAppHost.h>

#pragma region Constants

#define CPepAppRunningMutexName L"PepAppRunningMutex"

// Command Line Arguments definitions

#define CPluginsArgument L"/plugins"

#pragma endregion

#pragma region Typedefs

typedef BOOL (PEPAPPHOSTAPI* TPepAppHostInitializeFunc)(VOID);
typedef BOOL (PEPAPPHOSTAPI* TPepAppHostUninitializeFunc)(VOID);
typedef BOOL (PEPAPPHOSTAPI* TPepAppHostExecuteFunc)(_Out_ LPDWORD pdwExitCode);

typedef VOID (STDAPICALLTYPE *TPathRemoveExtensionWFunc)(_Inout_ LPWSTR pszPath);
typedef BOOL (STDAPICALLTYPE *TPathRemoveFileSpecWFunc)(_Inout_ LPWSTR pszPath);
typedef BOOL (STDAPICALLTYPE *TPathAppendWFunc)(_Inout_ LPWSTR pszPath, _In_ LPCWSTR pszMore);

typedef LPWSTR* (STDAPICALLTYPE *TCommandLineToArgvWFunc)(_In_ LPCWSTR pszCmdLine, _Out_ int* pNumArgs);

typedef BOOL (UTPEPDEVICESAPI *TUtPepDevicesInitializeFunc)(_In_ LPCWSTR pszPluginPath);
typedef BOOL (UTPEPDEVICESAPI *TUtPepDevicesUninitializeFunc)(VOID);

typedef VOID (UIPEPCTRLSAPI* TUiPepCtrlsInitializeFunc)(VOID);
typedef VOID (UIPEPCTRLSAPI* TUiPepCtrlsUninitializeFunc)(VOID);

#pragma endregion

#pragma region Structures

typedef struct tagTPepAppHostData
{
    HMODULE hModule;
    TPepAppHostInitializeFunc pInitialize;
    TPepAppHostUninitializeFunc pUninitialize;
    TPepAppHostExecuteFunc pExecute;
} TPepAppHostData;

typedef struct tagTPepDeviceData
{
	HMODULE hModule;
	TUtPepDevicesInitializeFunc pInitialize;
	TUtPepDevicesUninitializeFunc pUninitialize;
} TPepDeviceData;

typedef struct tagTPepCtrlsData
{
	HMODULE hModule;
	TUiPepCtrlsInitializeFunc pInitialize;
	TUiPepCtrlsUninitializeFunc pUninitialize;
} TPepCtrlsData;

typedef struct tagTSetupData
{
    HINSTANCE hInstance;
    LPCWSTR pszArguments;
    TPepAppHostData AppHostData;
	TPepDeviceData DeviceData;
	TPepCtrlsData CtrlsData;
} TSetupData;

#pragma endregion

#pragma region Global Variables

static HANDLE l_hAppRunningMutex = NULL;

#pragma endregion

#pragma region "Local Functions"

static BOOL lIsSupportedOperatingSystem(VOID)
{
    DWORDLONG nConditionMask(0);
    OSVERSIONINFOEX VersionInfoEx;

    VersionInfoEx.dwOSVersionInfoSize = sizeof(VersionInfoEx);
    VersionInfoEx.dwMajorVersion = 6; // set to 6 for vista and above
    VersionInfoEx.dwMinorVersion = 1; // set for Windows 7 and above

    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MAJORVERSION,
                                           VER_GREATER_EQUAL);
    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MINORVERSION,
                                           VER_GREATER_EQUAL);

    return ::VerifyVersionInfo(&VersionInfoEx, VER_MAJORVERSION | VER_MINORVERSION,
                               nConditionMask);
}

static BOOL lInitialize(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbAlreadyRunning)
{
    BOOL bResult = FALSE;

    *pbAlreadyRunning = FALSE;

    // Check if the application is already running

    l_hAppRunningMutex = ::CreateMutex(NULL, FALSE, CPepAppRunningMutexName);

    if (l_hAppRunningMutex)
    {
        if (::GetLastError() == ERROR_ALREADY_EXISTS)
        {
            *pbAlreadyRunning = TRUE;
        }

        bResult = TRUE;
    }
    else
    {
        if (::GetLastError() == ERROR_INVALID_HANDLE)
        {
            // name matches a different object
        }
        else if (::GetLastError() == ERROR_ACCESS_DENIED)
        {
            *pbAlreadyRunning = TRUE;

            bResult = TRUE;
        }
    }

    return bResult;
}

static VOID lUninitialize()
{
    ::CloseHandle(l_hAppRunningMutex);

    l_hAppRunningMutex = NULL;
}

static BOOL lInitializePepAppHostData(
  _In_ TPepAppHostData* pPepAppHostData)
{
	pPepAppHostData->hModule = ::LoadLibrary(L"PepAppHost.dll");

	if (pPepAppHostData->hModule == NULL)
	{
		return FALSE;
	}

	pPepAppHostData->pInitialize = (TPepAppHostInitializeFunc)::GetProcAddress(pPepAppHostData->hModule, "PepAppHostInitialize");
	pPepAppHostData->pUninitialize = (TPepAppHostUninitializeFunc)::GetProcAddress(pPepAppHostData->hModule, "PepAppHostUninitialize");
	pPepAppHostData->pExecute = (TPepAppHostExecuteFunc)::GetProcAddress(pPepAppHostData->hModule, "PepAppHostExecute");

	if (pPepAppHostData->pInitialize == NULL ||
		pPepAppHostData->pUninitialize == NULL ||
		pPepAppHostData->pExecute == NULL)
	{
		::FreeLibrary(pPepAppHostData->hModule);

		return FALSE;
	}

	return TRUE;
}

static BOOL lUninitializePepAppHostData(
  _In_ TPepAppHostData* pPepAppHostData)
{
	pPepAppHostData;

	// Cannot free the library because there is no way to unload the .NET Framework.

	//::FreeLibrary(pPepAppHostData->hModule);

	return TRUE;
}

static BOOL lInitializeDeviceData(
  _In_ TPepDeviceData* pDeviceData)
{
	pDeviceData->hModule = ::LoadLibrary(L"UtPepDevices.dll");

	if (pDeviceData->hModule == NULL)
	{
		return FALSE;
	}

	pDeviceData->pInitialize = (TUtPepDevicesInitializeFunc)::GetProcAddress(pDeviceData->hModule, "UtPepDevicesInitialize");
	pDeviceData->pUninitialize = (TUtPepDevicesUninitializeFunc)::GetProcAddress(pDeviceData->hModule, "UtPepDevicesUninitialize");

	if (pDeviceData->pInitialize == NULL ||
		pDeviceData->pUninitialize == NULL)
	{
		::FreeLibrary(pDeviceData->hModule);

		return FALSE;
	}

	return TRUE;
}

static BOOL lUninitializeDeviceData(
  _In_ TPepDeviceData* pDeviceData)
{
	::FreeLibrary(pDeviceData->hModule);

	return TRUE;
}

static BOOL lInitializeCtrlsData(
  _In_ TPepCtrlsData* pCtrlsData)
{
	pCtrlsData->hModule = ::LoadLibrary(L"UiPepCtrls.dll");

	if (pCtrlsData->hModule == NULL)
	{
		return FALSE;
	}

	pCtrlsData->pInitialize = (TUiPepCtrlsInitializeFunc)::GetProcAddress(pCtrlsData->hModule, "UiPepCtrlsInitialize");
	pCtrlsData->pUninitialize = (TUiPepCtrlsUninitializeFunc)::GetProcAddress(pCtrlsData->hModule, "UiPepCtrlsUninitialize");

	if (pCtrlsData->pInitialize == NULL ||
		pCtrlsData->pUninitialize == NULL)
	{
		::FreeLibrary(pCtrlsData->hModule);

		return FALSE;
	}

	return TRUE;
}

static BOOL lUninitializeCtrlsData(
	_In_ TPepCtrlsData* pCtrlsData)
{
	::FreeLibrary(pCtrlsData->hModule);

	return TRUE;
}

static BOOL lInitializePepDevices(
  _In_ TPepDeviceData* pDeviceData,
  _In_ LPCWSTR pszPluginPath)
{
	BOOL bResult;
	WCHAR cPluginPath[MAX_PATH];
	DWORD dwResult;
	HMODULE hModule;
	TPathRemoveExtensionWFunc pPathRemoveExtension;
	TPathRemoveFileSpecWFunc pPathRemoveFileSpec;
	TPathAppendWFunc pPathAppend;

	if (pszPluginPath)
	{
		bResult = pDeviceData->pInitialize(pszPluginPath);
	}
	else
	{
		dwResult = ::GetModuleFileName(::GetModuleHandle(NULL), cPluginPath, MArrayLen(cPluginPath));

		if (dwResult == ERROR_INSUFFICIENT_BUFFER)
		{
			return FALSE;
		}

		hModule = ::LoadLibraryW(L"shlwapi.dll");

		if (hModule == NULL)
		{
			return FALSE;
		}

		pPathRemoveExtension = (TPathRemoveExtensionWFunc)::GetProcAddress(hModule, "PathRemoveExtensionW");
		pPathRemoveFileSpec = (TPathRemoveFileSpecWFunc)::GetProcAddress(hModule, "PathRemoveFileSpecW");
		pPathAppend = (TPathAppendWFunc)::GetProcAddress(hModule, "PathAppendW");

		if (pPathRemoveExtension == NULL || pPathRemoveFileSpec == NULL || pPathAppend == NULL)
		{
			::FreeLibrary(hModule);

			return FALSE;
		}

		pPathRemoveExtension(cPluginPath);
		pPathRemoveFileSpec(cPluginPath);
		pPathAppend(cPluginPath, L"Plugins");

		::FreeLibrary(hModule);

		bResult = pDeviceData->pInitialize(cPluginPath);
	}

	return bResult;
}

static DWORD WINAPI lRunSetupThreadProc(
  _In_ LPVOID pvParameter)
{
    DWORD dwResult = 0;
    TSetupData* pSetupData = (TSetupData*)pvParameter;
	LPCWSTR pszPluginPath = NULL;
	LPWSTR* ppszArgs;
	INT nTotalArgs;
	BOOL bAlreadyRunning;
	HMODULE hModule;
	TCommandLineToArgvWFunc pCommandLineToArgv;

    if (!lIsSupportedOperatingSystem())
    {
        PepAppSplashDialogDisplayUnsupportedOS();

        return FALSE;
    }

    if (!lInitialize(&bAlreadyRunning))
    {
        PepAppSplashDialogDisplayUnknownError();

        return FALSE;
    }

    if (bAlreadyRunning)
    {
        PepAppSplashDialogDisplayAppAlreadyRunning();

        lUninitialize();

        return FALSE;
    }

	hModule = ::LoadLibraryW(L"shell32.dll");

	if (hModule == NULL)
	{
		PepAppSplashDialogDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	pCommandLineToArgv = (TCommandLineToArgvWFunc)::GetProcAddress(hModule, "CommandLineToArgvW");

	if (pCommandLineToArgv == NULL)
	{
		::FreeLibrary(hModule);

		PepAppSplashDialogDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	ppszArgs = pCommandLineToArgv(pSetupData->pszArguments, &nTotalArgs);

	::FreeModule(hModule);

	if (ppszArgs == NULL)
	{
		PepAppSplashDialogDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (nTotalArgs > 1)
	{
		if (nTotalArgs == 3)
		{
			if (::lstrcmpi(ppszArgs[1], CPluginsArgument) == 0)
			{
			    pszPluginPath = ppszArgs[2];
			}
			else
			{
				::LocalFree(ppszArgs);

				PepAppSplashDialogDisplayCommandLineHelp();

				lUninitialize();

				return FALSE;
			}
		}
		else
		{
			::LocalFree(ppszArgs);

			PepAppSplashDialogDisplayCommandLineHelp();

			lUninitialize();

			return FALSE;
		}
	}

	if (FALSE == lInitializePepAppHostData(&pSetupData->AppHostData))
	{
		::LocalFree(ppszArgs);
		
		PepAppSplashDialogDisplayUnknownError();

        lUninitialize();

        return FALSE;
    }

	if (FALSE == lInitializeDeviceData(&pSetupData->DeviceData))
	{
		lUninitializePepAppHostData(&pSetupData->AppHostData);

		::LocalFree(ppszArgs);

		PepAppSplashDialogDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == lInitializeCtrlsData(&pSetupData->CtrlsData))
	{
		lUninitializeDeviceData(&pSetupData->DeviceData);
		lUninitializePepAppHostData(&pSetupData->AppHostData);

		::LocalFree(ppszArgs);

		PepAppSplashDialogDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == lInitializePepDevices(&pSetupData->DeviceData, pszPluginPath))
	{
		lUninitializeCtrlsData(&pSetupData->CtrlsData);
		lUninitializeDeviceData(&pSetupData->DeviceData);
		lUninitializePepAppHostData(&pSetupData->AppHostData);

		::LocalFree(ppszArgs);

		PepAppSplashDialogDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	pSetupData->CtrlsData.pInitialize();

	if (FALSE == pSetupData->AppHostData.pInitialize())
	{
		pSetupData->CtrlsData.pUninitialize();
		pSetupData->DeviceData.pUninitialize();

		lUninitializeCtrlsData(&pSetupData->CtrlsData);
		lUninitializeDeviceData(&pSetupData->DeviceData);
		lUninitializePepAppHostData(&pSetupData->AppHostData);

		::LocalFree(ppszArgs);

		PepAppSplashDialogDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	::LocalFree(ppszArgs);

    PepAppSplashDialogQuitMessagePump();

    return TRUE;
}

#pragma endregion

#pragma region Public Functions

INT PepAppExecute(
  _In_ HINSTANCE hInstance,
  _In_z_ LPCWSTR pszArguments)
{
	TSetupData* pSetupData = (TSetupData*)UtAllocMem(sizeof(TSetupData));
	HANDLE hThread;
	DWORD dwThreadId, dwExitCode;
    LPCWSTR pszAppTitle, pszMessage;

	if (!PepAppSplashDialogCreate(hInstance))
    {
        pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
        pszMessage = UtPepAppAllocString(IDS_CANNOTCREATEAPPWINDOW);

        ::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

        UtPepAppFreeString(pszAppTitle);
        UtPepAppFreeString(pszMessage);

        return 1;
    }

    pSetupData->hInstance = hInstance;
    pSetupData->pszArguments = pszArguments;

    hThread = ::CreateThread(NULL, 0, lRunSetupThreadProc, pSetupData, 0, &dwThreadId);

    if (hThread == NULL)
    {
        pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
        pszMessage = UtPepAppAllocString(IDS_CANNOTCREATEINSTALLTHREAD);

        ::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

        UtPepAppFreeString(pszAppTitle);
        UtPepAppFreeString(pszMessage);

        PepAppSplashDialogDestroy();

		UtFreeMem(pSetupData);

        return 1;
    }

    UtSetThreadName(dwThreadId, "Setup Thread");

    PepAppSplashDialogMessagePump();

    ::WaitForSingleObject(hThread, INFINITE);

    ::GetExitCodeThread(hThread, &dwExitCode);

    ::CloseHandle(hThread);

    PepAppSplashDialogDestroy();

    if (dwExitCode == FALSE)
    {
        return 1;
    }

    pSetupData->AppHostData.pExecute(&dwExitCode);

	pSetupData->CtrlsData.pUninitialize();
	pSetupData->DeviceData.pUninitialize();

	lUninitializeCtrlsData(&pSetupData->CtrlsData);
	lUninitializeDeviceData(&pSetupData->DeviceData);
	lUninitializePepAppHostData(&pSetupData->AppHostData);

	UtFreeMem(pSetupData);

    lUninitialize();

    return dwExitCode;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
