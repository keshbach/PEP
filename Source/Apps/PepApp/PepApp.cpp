/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepApp.h"

#include "PepAppSplashWindow.h"

#include "UtPepApp.h"

#include <Includes/UtThread.inl>

#include <Includes/UtVersion.h>
#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>

#include <UtilsDevice/UtPepDevices.h>
#include <UtilsDevice/UtPAL.h>

#include <UtilsPep/UiPepCtrls.h>

#include <Hosts/PepAppHost.h>

#pragma region Constants

#define CPepAppRunningMutexName L"PepAppRunningMutex"

// Command Line Arguments definitions

#define CPluginsArgument L"/plugins"
#define CDisableDPIArgument L"/disabledpi"

#pragma endregion

#pragma region Typedefs

typedef BOOL (WINAPI* TSetProcessDPIAware)(VOID);

typedef HRESULT (STDAPICALLTYPE* TSetProcessDpiAwareness)(PROCESS_DPI_AWARENESS value);

typedef BOOL (WINAPI* TSetProcessDpiAwarenessContext)(DPI_AWARENESS_CONTEXT value);

typedef BOOL (PEPAPPHOSTAPI* TPepAppHostInitializeFunc)(VOID);
typedef BOOL (PEPAPPHOSTAPI* TPepAppHostUninitializeFunc)(VOID);
typedef BOOL (PEPAPPHOSTAPI* TPepAppHostExecuteFunc)(_Out_ LPDWORD pdwExitCode);

typedef VOID (STDAPICALLTYPE *TPathRemoveExtensionWFunc)(_Inout_ LPWSTR pszPath);
typedef BOOL (STDAPICALLTYPE *TPathRemoveFileSpecWFunc)(_Inout_ LPWSTR pszPath);
typedef BOOL (STDAPICALLTYPE *TPathAppendWFunc)(_Inout_ LPWSTR pszPath, _In_ LPCWSTR pszMore);

typedef BOOL (UTPEPDEVICESAPI *TUtPepDevicesInitializeFunc)(_In_ LPCWSTR pszPluginPath);
typedef BOOL (UTPEPDEVICESAPI *TUtPepDevicesUninitializeFunc)(VOID);

typedef BOOL(UTPALAPI* TUtPALInitializeFunc)(VOID);
typedef BOOL(UTPALAPI* TUtPALUninitializeFunc)(VOID);

typedef VOID (UIPEPCTRLSAPI* TUiPepCtrlsInitializeFunc)(VOID);
typedef VOID (UIPEPCTRLSAPI* TUiPepCtrlsUninitializeFunc)(VOID);

#pragma endregion

#pragma region Structures

typedef struct tagTPepAppHostModuleData
{
    HMODULE hModule;
    TPepAppHostInitializeFunc pInitialize;
    TPepAppHostUninitializeFunc pUninitialize;
    TPepAppHostExecuteFunc pExecute;
} TPepAppHostModuleData;

typedef struct tagTPepDeviceModuleData
{
	HMODULE hModule;
	TUtPepDevicesInitializeFunc pInitialize;
	TUtPepDevicesUninitializeFunc pUninitialize;
} TPepDeviceModuleData;

typedef struct tagTPALModuleData
{
	HMODULE hModule;
	TUtPALInitializeFunc pInitialize;
	TUtPALUninitializeFunc pUninitialize;
} TPALModuleData;

typedef struct tagTPepCtrlsModuleData
{
	HMODULE hModule;
	TUiPepCtrlsInitializeFunc pInitialize;
	TUiPepCtrlsUninitializeFunc pUninitialize;
} TPepCtrlsModuleData;

typedef struct tagTPepAppData
{
    HINSTANCE hInstance;
	LPCWSTR pszPluginPath;
    TPepAppHostModuleData AppHostModuleData;
	TPepDeviceModuleData DeviceModuleData;
	TPALModuleData PALModuleData;
	TPepCtrlsModuleData CtrlsModuleData;
} TPepAppData;

#pragma endregion

#pragma region Global Variables

static HANDLE l_hAppRunningMutex = NULL;

#pragma endregion

#pragma region "Local Functions"

static void lDisplayUnsupportedOS()
{
	LPCWSTR pszAppTitle, pszMessage;

	pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
	pszMessage = UtPepAppAllocString(IDS_UNSUPPORTEDWINDOWSVERSION);

	::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

	UtPepAppFreeString(pszAppTitle);
	UtPepAppFreeString(pszMessage);
}

static VOID lDisplayCommandLineHelp()
{
	LPCWSTR pszAppTitle, pszMessage;

    pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
	pszMessage = UtPepAppAllocString(IDS_COMMANDLINEHELP);

	::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

	UtPepAppFreeString(pszAppTitle);
	UtPepAppFreeString(pszMessage);
}

static void lDisplayCannotCreateAppWindow()
{
	LPCWSTR pszAppTitle, pszMessage;

	pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
	pszMessage = UtPepAppAllocString(IDS_CANNOTCREATEAPPWINDOW);

	::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

	UtPepAppFreeString(pszAppTitle);
	UtPepAppFreeString(pszMessage);
}

static void lDisplayCannotCreateWorkerThread()
{
	LPCWSTR pszAppTitle, pszMessage;

	pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
	pszMessage = UtPepAppAllocString(IDS_CANNOTCREATEWORKERTHREAD);

	::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

	UtPepAppFreeString(pszAppTitle);
	UtPepAppFreeString(pszMessage);
}

static VOID lEnableDPIAwareness(VOID)
{
	HMODULE hModule;
	TSetProcessDPIAware pSetProcessDPIAware;
	TSetProcessDpiAwarenessContext pSetProcessDpiAwarenessContext;
	TSetProcessDpiAwareness pSetProcessDpiAwareness;

	// Check for Windows 10

    hModule = ::LoadLibraryW(L"user32.dll");

	pSetProcessDpiAwarenessContext = (TSetProcessDpiAwarenessContext)::GetProcAddress(hModule, "SetProcessDpiAwarenessContext");

	if (pSetProcessDpiAwarenessContext)
	{
		if (!pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
		{
			pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
		}
	
        ::FreeLibrary(hModule);

        return;
    }

    ::FreeLibrary(hModule);

	// Check for Windows 8.1

	hModule = ::LoadLibraryW(L"shcore.dll");

	pSetProcessDpiAwareness = (TSetProcessDpiAwareness)::GetProcAddress(hModule, "SetProcessDpiAwareness");

	if (pSetProcessDpiAwareness)
	{
        pSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

		::FreeLibrary(hModule);

		return;
	}

	::FreeLibrary(hModule);

	// Check for Vista

	hModule = ::LoadLibraryW(L"user32.dll");

	pSetProcessDPIAware = (TSetProcessDPIAware)::GetProcAddress(hModule, "SetProcessDPIAware");

	if (pSetProcessDPIAware)
	{
		pSetProcessDPIAware();
	}

	::FreeLibrary(hModule);
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

static BOOL lInitializePepAppHostModuleData(
  _In_ TPepAppHostModuleData* pPepAppHostModuleData)
{
	pPepAppHostModuleData->hModule = ::LoadLibrary(L"PepAppHost.dll");

	if (pPepAppHostModuleData->hModule == NULL)
	{
		return FALSE;
	}

	pPepAppHostModuleData->pInitialize = (TPepAppHostInitializeFunc)::GetProcAddress(pPepAppHostModuleData->hModule, "PepAppHostInitialize");
	pPepAppHostModuleData->pUninitialize = (TPepAppHostUninitializeFunc)::GetProcAddress(pPepAppHostModuleData->hModule, "PepAppHostUninitialize");
	pPepAppHostModuleData->pExecute = (TPepAppHostExecuteFunc)::GetProcAddress(pPepAppHostModuleData->hModule, "PepAppHostExecute");

	if (pPepAppHostModuleData->pInitialize == NULL ||
		pPepAppHostModuleData->pUninitialize == NULL ||
		pPepAppHostModuleData->pExecute == NULL)
	{
		::FreeLibrary(pPepAppHostModuleData->hModule);

		return FALSE;
	}

	return TRUE;
}

static BOOL lUninitializePepAppHostModuleData(
  _In_ TPepAppHostModuleData* pPepAppHostModuleData)
{
	pPepAppHostModuleData;

	// Cannot free the library because there is no way to unload the .NET Framework.

	//::FreeLibrary(pPepAppHostData->hModule);

	return TRUE;
}

static BOOL lInitializeDeviceModuleData(
  _In_ TPepDeviceModuleData* pDeviceModuleData)
{
	pDeviceModuleData->hModule = ::LoadLibrary(L"UtPepDevices.dll");

	if (pDeviceModuleData->hModule == NULL)
	{
		return FALSE;
	}

	pDeviceModuleData->pInitialize = (TUtPepDevicesInitializeFunc)::GetProcAddress(pDeviceModuleData->hModule, "UtPepDevicesInitialize");
	pDeviceModuleData->pUninitialize = (TUtPepDevicesUninitializeFunc)::GetProcAddress(pDeviceModuleData->hModule, "UtPepDevicesUninitialize");

	if (pDeviceModuleData->pInitialize == NULL ||
		pDeviceModuleData->pUninitialize == NULL)
	{
		::FreeLibrary(pDeviceModuleData->hModule);

		return FALSE;
	}

	return TRUE;
}

static BOOL lUninitializeDeviceModuleData(
  _In_ TPepDeviceModuleData* pDeviceModuleData)
{
	::FreeLibrary(pDeviceModuleData->hModule);

	return TRUE;
}

static BOOL lInitializePALModuleData(
  _In_ TPALModuleData* pPALModuleData)
{
	pPALModuleData->hModule = ::LoadLibrary(L"UtPAL.dll");

	if (pPALModuleData->hModule == NULL)
	{
		return FALSE;
	}

	pPALModuleData->pInitialize = (TUtPALInitializeFunc)::GetProcAddress(pPALModuleData->hModule, "UtPALInitialize");
	pPALModuleData->pUninitialize = (TUtPALUninitializeFunc)::GetProcAddress(pPALModuleData->hModule, "UtPALUninitialize");

	if (pPALModuleData->pInitialize == NULL ||
		pPALModuleData->pUninitialize == NULL)
	{
		::FreeLibrary(pPALModuleData->hModule);

		return FALSE;
	}

	return TRUE;
}

static BOOL lUninitializePALModuleData(
	_In_ TPALModuleData* pPALModuleData)
{
	::FreeLibrary(pPALModuleData->hModule);

	return TRUE;
}

static BOOL lInitializeCtrlsModuleData(
  _In_ TPepCtrlsModuleData* pCtrlsModuleData)
{
	pCtrlsModuleData->hModule = ::LoadLibrary(L"UiPepCtrls.dll");

	if (pCtrlsModuleData->hModule == NULL)
	{
		return FALSE;
	}

	pCtrlsModuleData->pInitialize = (TUiPepCtrlsInitializeFunc)::GetProcAddress(pCtrlsModuleData->hModule, "UiPepCtrlsInitialize");
	pCtrlsModuleData->pUninitialize = (TUiPepCtrlsUninitializeFunc)::GetProcAddress(pCtrlsModuleData->hModule, "UiPepCtrlsUninitialize");

	if (pCtrlsModuleData->pInitialize == NULL ||
		pCtrlsModuleData->pUninitialize == NULL)
	{
		::FreeLibrary(pCtrlsModuleData->hModule);

		return FALSE;
	}

	return TRUE;
}

static BOOL lUninitializeCtrlsModuleData(
	_In_ TPepCtrlsModuleData* pCtrlsModuleData)
{
	::FreeLibrary(pCtrlsModuleData->hModule);

	return TRUE;
}

static BOOL lInitializePepDevices(
  _In_ TPepDeviceModuleData* pDeviceModuleData,
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
		bResult = pDeviceModuleData->pInitialize(pszPluginPath);
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

		bResult = pDeviceModuleData->pInitialize(cPluginPath);
	}

	return bResult;
}

static VOID PEPAPPSPLASHWINDOWEXECUTEAPI lExecuteInitializeControls(
  _In_ PVOID pvData)
{
	TPepCtrlsModuleData* pCtrlsModuleData = (TPepCtrlsModuleData*)pvData;

	pCtrlsModuleData->pInitialize();
}

static VOID PEPAPPSPLASHWINDOWEXECUTEAPI lExecuteUninitializeControls(
  _In_ PVOID pvData)
{
	TPepCtrlsModuleData* pCtrlsModuleData = (TPepCtrlsModuleData*)pvData;

	pCtrlsModuleData->pUninitialize();
}

static DWORD WINAPI lRunSetupThreadProc(
  _In_ LPVOID pvParameter)
{
	TPepAppData* pPepAppData = (TPepAppData*)pvParameter;
	BOOL bAlreadyRunning;

    if (!lInitialize(&bAlreadyRunning))
    {
        PepAppSplashWindowDisplayUnknownError();

        return FALSE;
    }

    if (bAlreadyRunning)
    {
        PepAppSplashWindowDisplayAppAlreadyRunning();

        lUninitialize();

        return FALSE;
    }

	if (FALSE == lInitializePepAppHostModuleData(&pPepAppData->AppHostModuleData))
	{
		PepAppSplashWindowDisplayUnknownError();

        lUninitialize();

        return FALSE;
    }

	if (FALSE == lInitializePALModuleData(&pPepAppData->PALModuleData))
	{
		lUninitializePepAppHostModuleData(&pPepAppData->AppHostModuleData);

		PepAppSplashWindowDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == pPepAppData->PALModuleData.pInitialize())
	{
		lUninitializePALModuleData(&pPepAppData->PALModuleData);
		lUninitializePepAppHostModuleData(&pPepAppData->AppHostModuleData);

		PepAppSplashWindowDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == lInitializeDeviceModuleData(&pPepAppData->DeviceModuleData))
	{
		pPepAppData->PALModuleData.pUninitialize();

		lUninitializePALModuleData(&pPepAppData->PALModuleData);
		lUninitializePepAppHostModuleData(&pPepAppData->AppHostModuleData);

		PepAppSplashWindowDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == lInitializeCtrlsModuleData(&pPepAppData->CtrlsModuleData))
	{
		lUninitializeDeviceModuleData(&pPepAppData->DeviceModuleData);

		pPepAppData->PALModuleData.pUninitialize();

		lUninitializePALModuleData(&pPepAppData->PALModuleData);
		lUninitializePepAppHostModuleData(&pPepAppData->AppHostModuleData);

		PepAppSplashWindowDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == lInitializePepDevices(&pPepAppData->DeviceModuleData, pPepAppData->pszPluginPath))
	{
		lUninitializeCtrlsModuleData(&pPepAppData->CtrlsModuleData);
		lUninitializeDeviceModuleData(&pPepAppData->DeviceModuleData);

		pPepAppData->PALModuleData.pUninitialize();

		lUninitializePALModuleData(&pPepAppData->PALModuleData);
		lUninitializePepAppHostModuleData(&pPepAppData->AppHostModuleData);

		PepAppSplashWindowDisplayPluginsLoadFailed();

		lUninitialize();

		return FALSE;
	}

	PepAppSplashWindowExecute(lExecuteInitializeControls, &pPepAppData->CtrlsModuleData);

	if (FALSE == pPepAppData->AppHostModuleData.pInitialize())
	{
		PepAppSplashWindowExecute(lExecuteUninitializeControls, &pPepAppData->CtrlsModuleData);

		pPepAppData->DeviceModuleData.pUninitialize();

		lUninitializeCtrlsModuleData(&pPepAppData->CtrlsModuleData);
		lUninitializeDeviceModuleData(&pPepAppData->DeviceModuleData);

		pPepAppData->PALModuleData.pUninitialize();

		lUninitializePALModuleData(&pPepAppData->PALModuleData);
		lUninitializePepAppHostModuleData(&pPepAppData->AppHostModuleData);

		PepAppSplashWindowDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

    PepAppSplashWindowQuitMessagePump();

    return TRUE;
}

#pragma endregion

#pragma region Public Functions

INT PepAppExecute(
  _In_ HINSTANCE hInstance,
  _In_ INT nTotalArgs,
  _In_z_ LPWSTR* ppszArgs)
{
	TPepAppData* pPepAppData = (TPepAppData*)UtAllocMem(sizeof(TPepAppData));
	HANDLE hThread;
	DWORD dwThreadId, dwExitCode;
	INT nArgIndex;
	BOOL bDisableDPI, bDisplayHelp;

	if (!IsWindows7OrGreater())
	{
		lDisplayUnsupportedOS();

		return 1;
	}

	pPepAppData = (TPepAppData*)UtAllocMem(sizeof(TPepAppData));

	::ZeroMemory(pPepAppData, sizeof(TPepAppData));

	nArgIndex = 1;
	bDisableDPI = FALSE;
	bDisplayHelp = FALSE;

	while (nArgIndex < nTotalArgs && bDisplayHelp == FALSE)
	{
		if (::lstrcmpi(ppszArgs[nArgIndex], CPluginsArgument) == 0)
		{
			++nArgIndex;

			if (nArgIndex < nTotalArgs)
			{
				pPepAppData->pszPluginPath = ppszArgs[nArgIndex];

				++nArgIndex;
			}
			else
			{
				bDisplayHelp = TRUE;
			}
		}
		else if (::lstrcmpi(ppszArgs[nArgIndex], CDisableDPIArgument) == 0)
		{
			bDisableDPI = TRUE;

			++nArgIndex;
		}
		else 
		{
			bDisplayHelp = TRUE;
		}
	}

	if (bDisplayHelp == TRUE)
	{
		UtFreeMem(pPepAppData);

		lDisplayCommandLineHelp();

		return 1;
	}

	if (bDisableDPI == FALSE)
	{
		lEnableDPIAwareness();
	}

	if (!PepAppSplashWindowCreate(hInstance))
    {
		lDisplayCannotCreateAppWindow();

		UtFreeMem(pPepAppData);

        return 1;
    }

    pPepAppData->hInstance = hInstance;

    hThread = ::CreateThread(NULL, 0, lRunSetupThreadProc, pPepAppData, 0, &dwThreadId);

    if (hThread == NULL)
    {
		lDisplayCannotCreateWorkerThread();

        PepAppSplashWindowDestroy(hInstance);

		UtFreeMem(pPepAppData);

        return 1;
    }

    UtSetThreadName(dwThreadId, "App Thread");

    PepAppSplashWindowMessagePump();

    ::WaitForSingleObject(hThread, INFINITE);

    ::GetExitCodeThread(hThread, &dwExitCode);

    ::CloseHandle(hThread);

    PepAppSplashWindowDestroy(hInstance);

    if (dwExitCode == FALSE)
    {
		UtFreeMem(pPepAppData);
		
		return 1;
    }

    pPepAppData->AppHostModuleData.pExecute(&dwExitCode);

	pPepAppData->CtrlsModuleData.pUninitialize();
	pPepAppData->DeviceModuleData.pUninitialize();
	pPepAppData->PALModuleData.pUninitialize();

	lUninitializeCtrlsModuleData(&pPepAppData->CtrlsModuleData);
	lUninitializeDeviceModuleData(&pPepAppData->DeviceModuleData);
	lUninitializePALModuleData(&pPepAppData->PALModuleData);
	lUninitializePepAppHostModuleData(&pPepAppData->AppHostModuleData);

	UtFreeMem(pPepAppData);

    lUninitialize();

    return dwExitCode;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
