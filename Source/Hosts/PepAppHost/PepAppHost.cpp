/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2023 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Hosts/PepAppHost.h>
#include <Hosts/PepAppHostData.h>

#include <Utils/UtHeapProcess.h>

#include "PepAppHostControl.h"

#include <new>

#include "PepAppActionOnCLREvent.h"

#include "UtPepAppHostTasks.h"

#pragma region "Constants"

#define CNetFrameworkRuntimeVersion L"v4.0.30319"

#define CUxThemeLibraryName L"UxTheme.dll"

#define CBufferedPaintInitFuncName "BufferedPaintInit"
#define CBufferedPaintUnInitFuncName "BufferedPaintUnInit"

#define CPepAppHostNetLibraryName L"PepAppHostNet.dll"

#define CPepAppHostNetExecuteInAppDomainFuncName "PepAppHostNetExecuteInAppDomain"

#pragma endregion

#pragma region "Type Defs"

typedef HRESULT(STDAPICALLTYPE* TBufferedPaintInitFunc)(VOID);
typedef HRESULT(STDAPICALLTYPE* TBufferedPaintUnInitFunc)(VOID);

typedef HRESULT (__stdcall* TPepAppHostNetExecuteInAppDomainFunc)(void* cookie);

#pragma endregion

#pragma region "Structures"

typedef struct tagTPepAppHostRuntimeData
{
    BOOL bCOMInitialized;
    BOOL bBufferPaintInitialized;
	BOOL bPepAppHostTasksInitialized;
    BOOL bRuntimeStarted;
    ICLRMetaHost* pCLRMetaHost;
    ICLRRuntimeInfo* pCLRRuntimeInfo;
    ICLRRuntimeHost* pCLRRuntimeHost;
    ICLRControl* pCLRControl;
    ICLROnEventManager* pCLROnEventManager;
    ICLRPolicyManager* pCLRPolicyManager;
    ICLRHostProtectionManager* pCLRHostProtectionManager;
    PepAppHostControl* pPepAppHostControl;
    PepAppActionOnCLREvent* pPepAppActionOnCLREvent;

    HMODULE hUxThemeLibrary;
    TBufferedPaintInitFunc pBufferedPaintInit;
    TBufferedPaintUnInitFunc pBufferedPaintUnInit;

    HMODULE hHostNetLibrary;
    TPepAppHostNetExecuteInAppDomainFunc pPepAppHostNetExecuteInAppDomain;
} TPepAppHostRuntimeData;

#pragma endregion

#pragma region "Local Variables"

static TPepAppHostRuntimeData l_PepAppHostRuntimeData;

static TPepAppHostData l_PepAppHostData;

#pragma endregion

#pragma region "Local Functions"

static BOOL lUninitialize(_In_ TPepAppHostRuntimeData* pPepAppHostRuntimeData);

static BOOL lInitialize(
  _In_ TPepAppHostRuntimeData* pPepAppHostRuntimeData)
{
    if (S_OK != ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))
    {
        return FALSE;
    }

    pPepAppHostRuntimeData->bCOMInitialized = TRUE;

    if (::IsWindowsVistaOrGreater())
    {
        pPepAppHostRuntimeData->hUxThemeLibrary = ::LoadLibrary(CUxThemeLibraryName);

        if (pPepAppHostRuntimeData->hUxThemeLibrary)
        {
            pPepAppHostRuntimeData->pBufferedPaintInit = (TBufferedPaintInitFunc)::GetProcAddress(pPepAppHostRuntimeData->hUxThemeLibrary, CBufferedPaintInitFuncName);
            pPepAppHostRuntimeData->pBufferedPaintUnInit = (TBufferedPaintUnInitFunc)::GetProcAddress(pPepAppHostRuntimeData->hUxThemeLibrary, CBufferedPaintUnInitFuncName);
        }

        if (pPepAppHostRuntimeData->hUxThemeLibrary == NULL ||
            pPepAppHostRuntimeData->pBufferedPaintInit == NULL ||
            pPepAppHostRuntimeData->pBufferedPaintUnInit == NULL)
        {
            lUninitialize(pPepAppHostRuntimeData);

            return FALSE;
        }

        if (S_OK != pPepAppHostRuntimeData->pBufferedPaintInit())
        {
            lUninitialize(pPepAppHostRuntimeData);

            return FALSE;
        }

        pPepAppHostRuntimeData->bBufferPaintInitialized = TRUE;
    }

	if (FALSE == UtPepAppHostTasksInitialize())
	{
		lUninitialize(pPepAppHostRuntimeData);

		return FALSE;
	}

	pPepAppHostRuntimeData->bPepAppHostTasksInitialized = TRUE;

    if (S_OK != ::CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost,
                                    (LPVOID*)&pPepAppHostRuntimeData->pCLRMetaHost))
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    if (S_OK != pPepAppHostRuntimeData->pCLRMetaHost->GetRuntime(CNetFrameworkRuntimeVersion,
                                                                 IID_ICLRRuntimeInfo,
                                                                 (LPVOID*)&pPepAppHostRuntimeData->pCLRRuntimeInfo))
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    if (S_OK != pPepAppHostRuntimeData->pCLRRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost,
                                                                      IID_ICLRRuntimeHost,
                                                                      (LPVOID*)&pPepAppHostRuntimeData->pCLRRuntimeHost))
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    pPepAppHostRuntimeData->pPepAppHostControl = new (std::nothrow) PepAppHostControl();

    if (pPepAppHostRuntimeData->pPepAppHostControl == NULL)
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    pPepAppHostRuntimeData->pPepAppHostControl->AddRef();

	if (S_OK != pPepAppHostRuntimeData->pCLRRuntimeHost->GetCLRControl(&pPepAppHostRuntimeData->pCLRControl))
	{
		lUninitialize(pPepAppHostRuntimeData);

		return FALSE;
	}

/*
	HRESULT hResult;

	hResult = pPepAppHostRuntimeData->pCLRControl->SetAppDomainManagerType(L"C:\\git\\PEP\\Source\\bin\\Debug\\x86\\PepAppHostNet.dll",
	                                                                       L"Pep.Application.PepAppHostNetAppDomainManager");

	if (hResult != S_OK)
	{
		::OutputDebugString(L"error");
	}
*/

    if (S_OK != pPepAppHostRuntimeData->pCLRRuntimeHost->SetHostControl(pPepAppHostRuntimeData->pPepAppHostControl))
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    if (S_OK != pPepAppHostRuntimeData->pCLRControl->GetCLRManager(IID_ICLROnEventManager,
                                                                   (LPVOID*)&pPepAppHostRuntimeData->pCLROnEventManager))
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    pPepAppHostRuntimeData->pPepAppActionOnCLREvent = new (std::nothrow) PepAppActionOnCLREvent();

    if (pPepAppHostRuntimeData->pPepAppActionOnCLREvent == NULL)
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    pPepAppHostRuntimeData->pPepAppActionOnCLREvent->AddRef();

    pPepAppHostRuntimeData->pCLROnEventManager->RegisterActionOnEvent(Event_DomainUnload, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
    pPepAppHostRuntimeData->pCLROnEventManager->RegisterActionOnEvent(Event_ClrDisabled, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
    pPepAppHostRuntimeData->pCLROnEventManager->RegisterActionOnEvent(Event_MDAFired, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
    pPepAppHostRuntimeData->pCLROnEventManager->RegisterActionOnEvent(Event_StackOverflow, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
    //pPepAppHostRuntimeData->pCLROnEventManager->RegisterActionOnEvent(MaxClrEvent, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);

    if (S_OK != pPepAppHostRuntimeData->pCLRControl->GetCLRManager(IID_ICLRPolicyManager,
                                                                   (LPVOID*)&pPepAppHostRuntimeData->pCLRPolicyManager))
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    pPepAppHostRuntimeData->pCLRPolicyManager->SetUnhandledExceptionPolicy(eHostDeterminedPolicy);

    if (S_OK != pPepAppHostRuntimeData->pCLRControl->GetCLRManager(IID_ICLRHostProtectionManager,
                                                                   (LPVOID*)&pPepAppHostRuntimeData->pCLRHostProtectionManager))
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    pPepAppHostRuntimeData->pCLRHostProtectionManager->SetProtectedCategories(eNoChecks);

    if (S_OK != pPepAppHostRuntimeData->pCLRRuntimeHost->Start())
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    pPepAppHostRuntimeData->bRuntimeStarted = TRUE;

    pPepAppHostRuntimeData->hHostNetLibrary = ::LoadLibrary(CPepAppHostNetLibraryName);

    if (pPepAppHostRuntimeData->hHostNetLibrary)
    {
        pPepAppHostRuntimeData->pPepAppHostNetExecuteInAppDomain = (TPepAppHostNetExecuteInAppDomainFunc)::GetProcAddress(pPepAppHostRuntimeData->hHostNetLibrary, CPepAppHostNetExecuteInAppDomainFuncName);
    }

    if (pPepAppHostRuntimeData->hHostNetLibrary == NULL ||
        pPepAppHostRuntimeData->pPepAppHostNetExecuteInAppDomain == NULL)
    {
        lUninitialize(pPepAppHostRuntimeData);

        return FALSE;
    }

    return TRUE;
}

static BOOL lUninitialize(
  _In_ TPepAppHostRuntimeData* pPepAppHostRuntimeData)
{
    if (pPepAppHostRuntimeData->bRuntimeStarted)
    {
        pPepAppHostRuntimeData->pCLRRuntimeHost->Stop();

        pPepAppHostRuntimeData->bRuntimeStarted = FALSE;
    }

    if (pPepAppHostRuntimeData->hHostNetLibrary)
    {
        ::FreeLibrary(pPepAppHostRuntimeData->hHostNetLibrary);

        pPepAppHostRuntimeData->hHostNetLibrary = NULL;
        pPepAppHostRuntimeData->pPepAppHostNetExecuteInAppDomain = NULL;
    }

    if (pPepAppHostRuntimeData->pCLRHostProtectionManager)
    {
        pPepAppHostRuntimeData->pCLRHostProtectionManager->Release();

        pPepAppHostRuntimeData->pCLRHostProtectionManager = NULL;
    }

    if (pPepAppHostRuntimeData->pCLRPolicyManager)
    {
        pPepAppHostRuntimeData->pCLRPolicyManager->Release();

        pPepAppHostRuntimeData->pCLRPolicyManager = NULL;
    }

    if (pPepAppHostRuntimeData->pPepAppActionOnCLREvent)
    {
        pPepAppHostRuntimeData->pCLROnEventManager->UnregisterActionOnEvent(Event_DomainUnload, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
        pPepAppHostRuntimeData->pCLROnEventManager->UnregisterActionOnEvent(Event_ClrDisabled, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
        pPepAppHostRuntimeData->pCLROnEventManager->UnregisterActionOnEvent(Event_MDAFired, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
        pPepAppHostRuntimeData->pCLROnEventManager->UnregisterActionOnEvent(Event_StackOverflow, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);
        //pPepAppHostRuntimeData->pCLROnEventManager->UnregisterActionOnEvent(MaxClrEvent, pPepAppHostRuntimeData->pPepAppActionOnCLREvent);

        pPepAppHostRuntimeData->pPepAppActionOnCLREvent->Release();

        pPepAppHostRuntimeData->pPepAppActionOnCLREvent = NULL;
    }

    if (pPepAppHostRuntimeData->pCLROnEventManager)
    {
        pPepAppHostRuntimeData->pCLROnEventManager->Release();

        pPepAppHostRuntimeData->pCLROnEventManager = NULL;
    }

    if (pPepAppHostRuntimeData->pCLRControl)
    {
        pPepAppHostRuntimeData->pCLRControl->Release();

        pPepAppHostRuntimeData->pCLRControl = NULL;
    }

    if (pPepAppHostRuntimeData->pPepAppHostControl)
    {
        pPepAppHostRuntimeData->pPepAppHostControl->Release();

        pPepAppHostRuntimeData->pPepAppHostControl = NULL;
    }

    if (pPepAppHostRuntimeData->pCLRRuntimeHost)
    {
        pPepAppHostRuntimeData->pCLRRuntimeHost->Release();

        pPepAppHostRuntimeData->pCLRRuntimeHost = NULL;
    }

    if (pPepAppHostRuntimeData->pCLRRuntimeInfo)
    {
        pPepAppHostRuntimeData->pCLRRuntimeInfo->Release();

        pPepAppHostRuntimeData->pCLRRuntimeInfo = NULL;
    }

    if (pPepAppHostRuntimeData->pCLRMetaHost)
    {
        pPepAppHostRuntimeData->pCLRMetaHost->Release();

        pPepAppHostRuntimeData->pCLRMetaHost = NULL;
    }

	if (pPepAppHostRuntimeData->bPepAppHostTasksInitialized == TRUE)
	{
		UtPepAppHostTasksUninitialize();

		pPepAppHostRuntimeData->bPepAppHostTasksInitialized = FALSE;
	}

    if (pPepAppHostRuntimeData->bBufferPaintInitialized)
    {
        pPepAppHostRuntimeData->pBufferedPaintUnInit();

        ::FreeLibrary(pPepAppHostRuntimeData->hUxThemeLibrary);

        pPepAppHostRuntimeData->hUxThemeLibrary = NULL;
        pPepAppHostRuntimeData->pBufferedPaintInit = NULL;
        pPepAppHostRuntimeData->pBufferedPaintUnInit = NULL;

        pPepAppHostRuntimeData->bBufferPaintInitialized = FALSE;
    }

    if (pPepAppHostRuntimeData->bCOMInitialized)
    {
        ::CoUninitialize();

        pPepAppHostRuntimeData->bCOMInitialized = FALSE;
    }

    return TRUE;
}

#pragma endregion

#pragma region "Public Functions"

MExternC BOOL PEPAPPHOSTAPI PepAppHostInitialize(VOID)
{
    ::ZeroMemory(&l_PepAppHostRuntimeData, sizeof(l_PepAppHostRuntimeData));
    ::ZeroMemory(&l_PepAppHostData, sizeof(l_PepAppHostData));

    return TRUE;
}

MExternC BOOL PEPAPPHOSTAPI PepAppHostUninitialize(VOID)
{
    return TRUE;
}

MExternC BOOL PEPAPPHOSTAPI PepAppHostExecute(
  _In_ BOOL bUseParallelPort,
  _Out_ LPDWORD pdwExitCode)
{
    DWORD dwAppDomainId = 0;

    *pdwExitCode = 0;

	if (FALSE == lInitialize(&l_PepAppHostRuntimeData))
	{
		return FALSE;
	}

    l_PepAppHostData.bUseParallelPort = bUseParallelPort;

    l_PepAppHostRuntimeData.pCLRRuntimeHost->GetCurrentAppDomainId(&dwAppDomainId);

    l_PepAppHostRuntimeData.pCLRRuntimeHost->ExecuteInAppDomain(dwAppDomainId, l_PepAppHostRuntimeData.pPepAppHostNetExecuteInAppDomain, &l_PepAppHostData);

    lUninitialize(&l_PepAppHostRuntimeData);

    *pdwExitCode = l_PepAppHostData.dwExitCode;

    return TRUE;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2023 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
