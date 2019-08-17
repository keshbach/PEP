/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostAssemblyManager.h"

PepAppHostAssemblyManager::PepAppHostAssemblyManager()
{
    m_ulRefCount = 0;
}

PepAppHostAssemblyManager::~PepAppHostAssemblyManager()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostAssemblyManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostAssemblyManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyManager::QueryInterface(
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

#pragma region "IHostAssemblyManager"

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyManager::GetNonHostStoreAssemblies(
  ICLRAssemblyReferenceList** ppReferenceList)
{
    if (ppReferenceList == NULL)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyManager::GetAssemblyStore(
  IHostAssemblyStore** ppAssemblyStore)
{
    if (ppAssemblyStore == NULL)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
