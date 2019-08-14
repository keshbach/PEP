/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostControl.h"

#include <new>

#include "PepAppHostMemoryManager.h"
#include "PepAppHostTaskManager.h"

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
    }
    else if (riid == IID_IHostIoCompletionManager)
    {
    }
    else if (riid == IID_IHostSyncManager)
    {
    }
    else if (riid == IID_IHostAssemblyManager)
    {
    }
    else if (riid == IID_IHostGCManager)
    {
    }
    else if (riid == IID_IHostPolicyManager)
    {
    }
    else if (riid == IID_IHostSecurityManager)
    {
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
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
