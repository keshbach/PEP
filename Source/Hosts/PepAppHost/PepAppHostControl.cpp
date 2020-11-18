/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostControl.h"

#include <new>

#include "PepAppHostMemoryManager.h"
#include "PepAppHostTaskManager.h"
#include "PepAppHostThreadpoolManager.h"
#include "PepAppHostIOCompletionManager.h"
#include "PepAppHostSyncManager.h"
#include "PepAppHostGCManager.h"
#include "PepAppHostAssemblyManager.h"
#include "PepAppHostPolicyManager.h"
#include "PepAppHostSecurityManager.h"

PepAppHostControl::PepAppHostControl()
{
    m_ulRefCount = 0;
}

PepAppHostControl::~PepAppHostControl()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostControl::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostControl::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostControl::QueryInterface(
  const IID& iid,
  void** ppv)
{
    iid;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    return E_NOINTERFACE;
}

#pragma endregion

#pragma region "IHostControl"

HRESULT STDMETHODCALLTYPE PepAppHostControl::GetHostManager(
  REFIID riid,
  void** ppObject)
{
    if (ppObject == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IHostMemoryManager)
    {
        PepAppHostMemoryManager* pHostMemoryManager = new (std::nothrow) PepAppHostMemoryManager();

        if (pHostMemoryManager)
        {
            pHostMemoryManager->AddRef();

            *ppObject = pHostMemoryManager;

            return S_OK;
        }

        return E_FAIL;
    }
    else if (riid == IID_IHostTaskManager)
    {
        PepAppHostTaskManager* pHostTaskManager = new (std::nothrow) PepAppHostTaskManager();

        if (pHostTaskManager)
        {
            pHostTaskManager->AddRef();

            *ppObject = pHostTaskManager;

            return S_OK;
        }

		return E_FAIL;
    }
    else if (riid == IID_IHostThreadpoolManager)
    {
        PepAppHostThreadpoolManager* pHostThreadpoolManager = new (std::nothrow) PepAppHostThreadpoolManager();

        if (pHostThreadpoolManager)
        {
            pHostThreadpoolManager->AddRef();

            *ppObject = pHostThreadpoolManager;

            return S_OK;
        }

        return E_FAIL;
    }
    else if (riid == IID_IHostIoCompletionManager)
    {
        PepAppHostIOCompletionManager* pHostIOCompletionManager = new (std::nothrow) PepAppHostIOCompletionManager();

        if (pHostIOCompletionManager)
        {
            pHostIOCompletionManager->AddRef();

            *ppObject = pHostIOCompletionManager;

            return S_OK;
        }

        return E_FAIL;
    }
    else if (riid == IID_IHostSyncManager)
    {
        PepAppHostSyncManager* pHostSyncManager = new (std::nothrow) PepAppHostSyncManager();

        if (pHostSyncManager)
        {
            pHostSyncManager->AddRef();

            *ppObject = pHostSyncManager;

            return S_OK;
        }

        return E_FAIL;
    }
    else if (riid == IID_IHostAssemblyManager)
    {
        PepAppHostAssemblyManager* pHostAssemblyManager = new (std::nothrow) PepAppHostAssemblyManager();

        if (pHostAssemblyManager)
        {
            pHostAssemblyManager->AddRef();

            *ppObject = pHostAssemblyManager;

            return S_OK;
        }

        return E_FAIL;
    }
    else if (riid == IID_IHostGCManager)
    {
        PepAppHostGCManager* pHostGCManager = new (std::nothrow) PepAppHostGCManager();

        if (pHostGCManager)
        {
            pHostGCManager->AddRef();

            *ppObject = pHostGCManager;

            return S_OK;
        }

        return E_FAIL;
    }
    else if (riid == IID_IHostPolicyManager)
    {
        PepAppHostPolicyManager* pHostPolicyManager = new (std::nothrow) PepAppHostPolicyManager();

        if (pHostPolicyManager)
        {
            pHostPolicyManager->AddRef();

            *ppObject = pHostPolicyManager;

            return S_OK;
        }

        return E_FAIL;
    }
    else if (riid == IID_IHostSecurityManager)
    {
        PepAppHostSecurityManager* pHostSecurityManager = new (std::nothrow) PepAppHostSecurityManager();

        if (pHostSecurityManager)
        {
            pHostSecurityManager->AddRef();

            *ppObject = pHostSecurityManager;

            return S_OK;
        }

        return E_FAIL;
    }

    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE PepAppHostControl::SetAppDomainManager(
  DWORD dwAppDomainID,
  IUnknown* pUnkAppDomainManager)
{
    dwAppDomainID;
    pUnkAppDomainManager;

    return E_NOTIMPL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
