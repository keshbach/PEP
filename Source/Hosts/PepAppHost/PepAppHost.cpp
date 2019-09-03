/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Hosts/PepAppHost.h>

#include "PepAppHostControl.h"

#include <new>

#include "PepAppActionOnCLREvent.h"

#pragma region "Constants"

#define CNetFrameworkRuntimeVersion L"v4.0.30319"

#define CPepAppHostNetLibraryName L"PepAppHostNet.dll"

#define CPepAppHostNetExecuteInAppDomainFuncName "PepAppHostNetExecuteInAppDomain"

#pragma endregion

#pragma region "Type Defs"

typedef HRESULT (__stdcall* TPepAppHostNetExecuteInAppDomainFunc)(void* cookie);

#pragma endregion

#pragma region "Structures"

typedef struct tagTPepAppHostData
{
    BOOL bCOMInitialized;
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

    HMODULE hHostNetLibrary;
    TPepAppHostNetExecuteInAppDomainFunc pPepAppHostNetExecuteInAppDomain;
} TPepAppHostData;

#pragma endregion

#pragma region "Local Variables"

static TPepAppHostData l_PepAppHostData;

#pragma endregion

#pragma region "Local Functions"

static BOOL lUninitialize(_In_ TPepAppHostData* pPepAppHostData);

static BOOL lInitialize(
  _In_ TPepAppHostData* pPepAppHostData)
{
    if (S_OK != ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))
    {
        return FALSE;
    }

    pPepAppHostData->bCOMInitialized = TRUE;

    if (S_OK != ::CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost,
                                    (LPVOID*)&pPepAppHostData->pCLRMetaHost))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    if (S_OK != pPepAppHostData->pCLRMetaHost->GetRuntime(CNetFrameworkRuntimeVersion, IID_ICLRRuntimeInfo,
                                                          (LPVOID*)&pPepAppHostData->pCLRRuntimeInfo))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    if (S_OK != pPepAppHostData->pCLRRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost,
                                                               (LPVOID*)&pPepAppHostData->pCLRRuntimeHost))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    pPepAppHostData->pPepAppHostControl = new (std::nothrow) PepAppHostControl();

    if (pPepAppHostData->pPepAppHostControl == NULL)
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    pPepAppHostData->pPepAppHostControl->AddRef();

    if (S_OK != pPepAppHostData->pCLRRuntimeHost->SetHostControl(pPepAppHostData->pPepAppHostControl))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    if (S_OK != pPepAppHostData->pCLRRuntimeHost->GetCLRControl(&pPepAppHostData->pCLRControl))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    if (S_OK != pPepAppHostData->pCLRControl->GetCLRManager(IID_ICLROnEventManager,
                                                            (LPVOID*)&pPepAppHostData->pCLROnEventManager))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    pPepAppHostData->pPepAppActionOnCLREvent = new (std::nothrow) PepAppActionOnCLREvent();

    if (pPepAppHostData->pPepAppActionOnCLREvent == NULL)
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    pPepAppHostData->pPepAppActionOnCLREvent->AddRef();

    pPepAppHostData->pCLROnEventManager->RegisterActionOnEvent(Event_DomainUnload, pPepAppHostData->pPepAppActionOnCLREvent);
    pPepAppHostData->pCLROnEventManager->RegisterActionOnEvent(Event_ClrDisabled, pPepAppHostData->pPepAppActionOnCLREvent);
    pPepAppHostData->pCLROnEventManager->RegisterActionOnEvent(Event_MDAFired, pPepAppHostData->pPepAppActionOnCLREvent);
    pPepAppHostData->pCLROnEventManager->RegisterActionOnEvent(Event_StackOverflow, pPepAppHostData->pPepAppActionOnCLREvent);
    //pPepAppHostData->pCLROnEventManager->RegisterActionOnEvent(MaxClrEvent, pPepAppHostData->pPepAppActionOnCLREvent);

    if (S_OK != pPepAppHostData->pCLRControl->GetCLRManager(IID_ICLRPolicyManager,
                                                            (LPVOID*)&pPepAppHostData->pCLRPolicyManager))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    pPepAppHostData->pCLRPolicyManager->SetUnhandledExceptionPolicy(eHostDeterminedPolicy);

    if (S_OK != pPepAppHostData->pCLRControl->GetCLRManager(IID_ICLRHostProtectionManager,
                                                            (LPVOID*)&pPepAppHostData->pCLRHostProtectionManager))
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    pPepAppHostData->pCLRHostProtectionManager->SetProtectedCategories(eNoChecks);







    if (S_OK != pPepAppHostData->pCLRRuntimeHost->Start())
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    pPepAppHostData->bRuntimeStarted = TRUE;

    pPepAppHostData->hHostNetLibrary = ::LoadLibrary(CPepAppHostNetLibraryName);

    if (pPepAppHostData->hHostNetLibrary)
    {
        pPepAppHostData->pPepAppHostNetExecuteInAppDomain = (TPepAppHostNetExecuteInAppDomainFunc)::GetProcAddress(pPepAppHostData->hHostNetLibrary, CPepAppHostNetExecuteInAppDomainFuncName);
    }

    if (pPepAppHostData->hHostNetLibrary == NULL ||
        pPepAppHostData->pPepAppHostNetExecuteInAppDomain == NULL)
    {
        lUninitialize(pPepAppHostData);

        return FALSE;
    }

    return TRUE;
}

static BOOL lUninitialize(
  _In_ TPepAppHostData* pPepAppHostData)
{
    if (pPepAppHostData->bRuntimeStarted)
    {
        pPepAppHostData->pCLRRuntimeHost->Stop();

        pPepAppHostData->bRuntimeStarted = FALSE;
    }

    if (pPepAppHostData->hHostNetLibrary)
    {
        ::FreeLibrary(pPepAppHostData->hHostNetLibrary);

        pPepAppHostData->hHostNetLibrary = NULL;
        pPepAppHostData->pPepAppHostNetExecuteInAppDomain = NULL;
    }

    if (pPepAppHostData->pCLRHostProtectionManager)
    {
        pPepAppHostData->pCLRHostProtectionManager->Release();

        pPepAppHostData->pCLRHostProtectionManager = NULL;
    }

    if (pPepAppHostData->pCLRPolicyManager)
    {
        pPepAppHostData->pCLRPolicyManager->Release();

        pPepAppHostData->pCLRPolicyManager = NULL;
    }

    if (pPepAppHostData->pPepAppActionOnCLREvent)
    {
        pPepAppHostData->pCLROnEventManager->UnregisterActionOnEvent(Event_DomainUnload, pPepAppHostData->pPepAppActionOnCLREvent);
        pPepAppHostData->pCLROnEventManager->UnregisterActionOnEvent(Event_ClrDisabled, pPepAppHostData->pPepAppActionOnCLREvent);
        pPepAppHostData->pCLROnEventManager->UnregisterActionOnEvent(Event_MDAFired, pPepAppHostData->pPepAppActionOnCLREvent);
        pPepAppHostData->pCLROnEventManager->UnregisterActionOnEvent(Event_StackOverflow, pPepAppHostData->pPepAppActionOnCLREvent);
        //pPepAppHostData->pCLROnEventManager->UnregisterActionOnEvent(MaxClrEvent, pPepAppHostData->pPepAppActionOnCLREvent);

        pPepAppHostData->pPepAppActionOnCLREvent->Release();

        pPepAppHostData->pPepAppActionOnCLREvent = NULL;
    }

    if (pPepAppHostData->pCLROnEventManager)
    {
        pPepAppHostData->pCLROnEventManager->Release();

        pPepAppHostData->pCLROnEventManager = NULL;
    }

    if (pPepAppHostData->pCLRControl)
    {
        pPepAppHostData->pCLRControl->Release();

        pPepAppHostData->pCLRControl = NULL;
    }

    if (pPepAppHostData->pPepAppHostControl)
    {
        pPepAppHostData->pPepAppHostControl->Release();

        pPepAppHostData->pPepAppHostControl = NULL;
    }

    if (pPepAppHostData->pCLRRuntimeHost)
    {
        pPepAppHostData->pCLRRuntimeHost->Release();

        pPepAppHostData->pCLRRuntimeHost = NULL;
    }

    if (pPepAppHostData->pCLRRuntimeInfo)
    {
        pPepAppHostData->pCLRRuntimeInfo->Release();

        pPepAppHostData->pCLRRuntimeInfo = NULL;
    }

    if (pPepAppHostData->pCLRMetaHost)
    {
        pPepAppHostData->pCLRMetaHost->Release();

        pPepAppHostData->pCLRMetaHost = NULL;
    }

    if (pPepAppHostData->bCOMInitialized)
    {
        ::CoUninitialize();

        pPepAppHostData->bCOMInitialized = FALSE;
    }

    return TRUE;
}

#pragma endregion

#pragma region "Public Functions"

MExternC BOOL PEPAPPHOSTAPI PepAppHostInitialize(VOID)
{
    ::ZeroMemory(&l_PepAppHostData, sizeof(l_PepAppHostData));

    return TRUE;
}

MExternC BOOL PEPAPPHOSTAPI PepAppHostUninitialize(VOID)
{
    return TRUE;
}

MExternC BOOL PEPAPPHOSTAPI PepAppHostExecute(
  _Out_ LPDWORD pdwExitCode)
{
    *pdwExitCode = 0;

    lInitialize(&l_PepAppHostData);



    
    DWORD dwAppDomainId = 0;

    l_PepAppHostData.pCLRRuntimeHost->GetCurrentAppDomainId(&dwAppDomainId);

    l_PepAppHostData.pCLRRuntimeHost->ExecuteInAppDomain(dwAppDomainId, l_PepAppHostData.pPepAppHostNetExecuteInAppDomain /*lExecuteInAppDomain*/, NULL);
    




    /*
    pCLRRuntimeHost->ExecuteInDefaultAppDomain(L"C:\\git\\PEP\\Source\\bin\\Debug\\x86\\PepAppNet.exe",
                                               L"Pep.Application.Startup",
                                               L"Test",
                                               L"",
                                               pdwExitCode);
    */



    lUninitialize(&l_PepAppHostData);





    *pdwExitCode = (DWORD)-1;

    return TRUE;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
