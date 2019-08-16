/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostThreadpoolManager.h"

PepAppHostThreadpoolManager::PepAppHostThreadpoolManager()
{
    m_ulRefCount = 0;
}

PepAppHostThreadpoolManager::~PepAppHostThreadpoolManager()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostThreadpoolManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostThreadpoolManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostThreadpoolManager::QueryInterface(
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

#pragma region "IHostThreadpoolManager"

HRESULT STDMETHODCALLTYPE PepAppHostThreadpoolManager::QueueUserWorkItem(
  LPTHREAD_START_ROUTINE pFunction, 
  PVOID pvContext,
  ULONG Flags)
{
    pFunction;
    pvContext;
    Flags;

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostThreadpoolManager::SetMaxThreads(
  DWORD dwMaxWorkerThreads)
{
    dwMaxWorkerThreads;

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostThreadpoolManager::GetMaxThreads(
  DWORD* pdwMaxWorkerThreads)
{
    if (pdwMaxWorkerThreads == NULL)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostThreadpoolManager::GetAvailableThreads(
  DWORD* pdwAvailableWorkerThreads)
{
    if (pdwAvailableWorkerThreads == NULL)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostThreadpoolManager::SetMinThreads(
  DWORD dwMinIOCompletionThreads)
{
    dwMinIOCompletionThreads;

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostThreadpoolManager::GetMinThreads(
  DWORD* pdwMinIOCompletionThreads)
{
    if (pdwMinIOCompletionThreads == NULL)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
