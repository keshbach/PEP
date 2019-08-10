/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Hosts/PepAppHost.h>

#include "PepAppHostControl.h"

#include <new>

#pragma region "Public Functions"

MExternC BOOL PEPAPPHOSTAPI PepAppHostInitialize(VOID)
{
    return TRUE;
}

MExternC BOOL PEPAPPHOSTAPI PepAppHostUninitialize(VOID)
{
    return TRUE;
}

MExternC BOOL PEPAPPHOSTAPI PepAppHostExecute(
  _Out_ LPDWORD pdwExitCode)
{
    ICLRMetaHost* pCLRMetaHost = NULL;
    ICLRRuntimeInfo* pCLRRuntimeInfo = NULL;
    ICLRRuntimeHost* pCLRRuntimeHost = NULL;
    PepAppHostControl* pPepAppHostControl = NULL;

    *pdwExitCode = 0;

    if (S_OK != ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))
    {
        return FALSE;
    }

    if (S_OK != ::CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&pCLRMetaHost))
    {
        ::CoUninitialize();

        return FALSE;
    }

    if (S_OK != pCLRMetaHost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, (LPVOID*)&pCLRRuntimeInfo))
    {
        pCLRRuntimeInfo->Release();

        ::CoUninitialize();

        return FALSE;
    }

    if (S_OK != pCLRRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (LPVOID*)&pCLRRuntimeHost))
    {
        pCLRRuntimeInfo->Release();
        pCLRMetaHost->Release();

        ::CoUninitialize();

        return FALSE;
    }

    pPepAppHostControl = new (std::nothrow) PepAppHostControl();

    if (pPepAppHostControl == NULL)
    {
        pCLRRuntimeHost->Release();
        pCLRRuntimeInfo->Release();
        pCLRMetaHost->Release();

        ::CoUninitialize();

        return FALSE;
    }

    pPepAppHostControl->AddRef();

    if (S_OK != pCLRRuntimeHost->SetHostControl(pPepAppHostControl))
    {
        pPepAppHostControl->Release();
        pCLRRuntimeHost->Release();
        pCLRRuntimeInfo->Release();
        pCLRMetaHost->Release();

        ::CoUninitialize();

        return FALSE;
    }

    if (S_OK != pCLRRuntimeHost->Start())
    {
        pPepAppHostControl->Release();
        pCLRRuntimeHost->Release();
        pCLRRuntimeInfo->Release();
        pCLRMetaHost->Release();

        ::CoUninitialize();

        return FALSE;
    }

    pCLRRuntimeHost->ExecuteInDefaultAppDomain(L"C:\\git\\PEP\\Source\\bin\\Debug\\x86\\PepAppNet.exe",
                                               L"Pep.Application.Startup",
                                               L"Test",
                                               L"",
                                               pdwExitCode);

    pCLRRuntimeHost->Stop();

    pPepAppHostControl->Release();
    pCLRRuntimeHost->Release();
    pCLRRuntimeInfo->Release();
    pCLRMetaHost->Release();

    ::CoUninitialize();

    *pdwExitCode = (DWORD)-1;

    return TRUE;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
