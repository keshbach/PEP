/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
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

typedef struct tagTPepAppData
{
    HINSTANCE hInstance;
	LPCWSTR pszPluginPath;
    TPepAppHostData AppHostData;
	TPepDeviceData DeviceData;
	TPepCtrlsData CtrlsData;
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

static VOID PEPAPPSPLASHWINDOWEXECUTEAPI lExecuteInitializeControls(
  _In_ PVOID pvData)
{
	TPepCtrlsData* pCtrlsData = (TPepCtrlsData*)pvData;

	pCtrlsData->pInitialize();
}

static VOID PEPAPPSPLASHWINDOWEXECUTEAPI lExecuteUninitializeControls(
  _In_ PVOID pvData)
{
	TPepCtrlsData* pCtrlsData = (TPepCtrlsData*)pvData;

	pCtrlsData->pUninitialize();
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

	if (FALSE == lInitializePepAppHostData(&pPepAppData->AppHostData))
	{
		PepAppSplashWindowDisplayUnknownError();

        lUninitialize();

        return FALSE;
    }

	if (FALSE == lInitializeDeviceData(&pPepAppData->DeviceData))
	{
		lUninitializePepAppHostData(&pPepAppData->AppHostData);

		PepAppSplashWindowDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == lInitializeCtrlsData(&pPepAppData->CtrlsData))
	{
		lUninitializeDeviceData(&pPepAppData->DeviceData);
		lUninitializePepAppHostData(&pPepAppData->AppHostData);

		PepAppSplashWindowDisplayUnknownError();

		lUninitialize();

		return FALSE;
	}

	if (FALSE == lInitializePepDevices(&pPepAppData->DeviceData, pPepAppData->pszPluginPath))
	{
		lUninitializeCtrlsData(&pPepAppData->CtrlsData);
		lUninitializeDeviceData(&pPepAppData->DeviceData);
		lUninitializePepAppHostData(&pPepAppData->AppHostData);

		PepAppSplashWindowDisplayPluginsLoadFailed();

		lUninitialize();

		return FALSE;
	}

	PepAppSplashWindowExecute(lExecuteInitializeControls, &pPepAppData->CtrlsData);

	if (FALSE == pPepAppData->AppHostData.pInitialize())
	{
		PepAppSplashWindowExecute(lExecuteUninitializeControls, &pPepAppData->CtrlsData);

		pPepAppData->DeviceData.pUninitialize();

		lUninitializeCtrlsData(&pPepAppData->CtrlsData);
		lUninitializeDeviceData(&pPepAppData->DeviceData);
		lUninitializePepAppHostData(&pPepAppData->AppHostData);

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

    pPepAppData->AppHostData.pExecute(&dwExitCode);

	pPepAppData->CtrlsData.pUninitialize();
	pPepAppData->DeviceData.pUninitialize();

	lUninitializeCtrlsData(&pPepAppData->CtrlsData);
	lUninitializeDeviceData(&pPepAppData->DeviceData);
	lUninitializePepAppHostData(&pPepAppData->AppHostData);

	UtFreeMem(pPepAppData);

    lUninitialize();

    return dwExitCode;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
