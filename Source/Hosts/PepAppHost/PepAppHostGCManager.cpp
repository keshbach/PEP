/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostGCManager.h"

PepAppHostGCManager::PepAppHostGCManager()
{
    m_ulRefCount = 0;
}

PepAppHostGCManager::~PepAppHostGCManager()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostGCManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostGCManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostGCManager::QueryInterface(
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

#pragma region "IHostGCManager"

HRESULT STDMETHODCALLTYPE PepAppHostGCManager::ThreadIsBlockingForSuspension(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostGCManager::SuspensionStarting(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostGCManager::SuspensionEnding(DWORD dwGeneration)
{
    dwGeneration;

    return S_OK;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
