/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostSyncManager.h"

#include <new>

#include "PepAppHostManualEvent.h"
#include "PepAppHostAutoEvent.h"
#include "PepAppHostSemaphore.h"
#include "PepAppHostCrst.h"

PepAppHostSyncManager::PepAppHostSyncManager()
{
    m_ulRefCount = 0;
    m_pCLRSyncManager = NULL;
}

PepAppHostSyncManager::~PepAppHostSyncManager()
{
    if (m_pCLRSyncManager)
    {
        m_pCLRSyncManager->Release();

        m_pCLRSyncManager = NULL;
    }
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostSyncManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostSyncManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::QueryInterface(
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

#pragma region "IHostSyncManager"

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::SetCLRSyncManager(
   ICLRSyncManager* pManager)
{
    if (m_pCLRSyncManager)
    {
        m_pCLRSyncManager->Release();
    }

    m_pCLRSyncManager = pManager;

    m_pCLRSyncManager->AddRef();
    
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateCrst(
  IHostCrst** ppCrst)
{
    PepAppHostCrst* pPepAppHostCrst;

    if (ppCrst == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostCrst = new (std::nothrow) PepAppHostCrst();

    if (pPepAppHostCrst)
    {
        *ppCrst = pPepAppHostCrst;

        pPepAppHostCrst->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateCrstWithSpinCount(
  DWORD dwSpinCount,
  IHostCrst** ppCrst)
{
    PepAppHostCrst* pPepAppHostCrst;

    if (ppCrst == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostCrst = new (std::nothrow) PepAppHostCrst(dwSpinCount);

    if (pPepAppHostCrst)
    {
        *ppCrst = pPepAppHostCrst;

        pPepAppHostCrst->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateAutoEvent(
  IHostAutoEvent** ppEvent)
{
    PepAppHostAutoEvent* pPepAppHostAutoEvent;

    if (ppEvent == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostAutoEvent = new (std::nothrow) PepAppHostAutoEvent(0);

    if (pPepAppHostAutoEvent)
    {
        *ppEvent = pPepAppHostAutoEvent;

        pPepAppHostAutoEvent->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateManualEvent(
  BOOL bInitialState,
  IHostManualEvent** ppEvent)
{
    PepAppHostManualEvent* pPepAppHostManualEvent;

    if (ppEvent == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostManualEvent = new (std::nothrow) PepAppHostManualEvent(bInitialState, 0);

    if (pPepAppHostManualEvent)
    {
        *ppEvent = pPepAppHostManualEvent;

        pPepAppHostManualEvent->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateMonitorEvent(
  SIZE_T Cookie,
  IHostAutoEvent** ppEvent)
{
    PepAppHostAutoEvent* pPepAppHostAutoEvent;

    if (ppEvent == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostAutoEvent = new (std::nothrow) PepAppHostAutoEvent(Cookie);

    if (pPepAppHostAutoEvent)
    {
        *ppEvent = pPepAppHostAutoEvent;

        pPepAppHostAutoEvent->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateRWLockWriterEvent(
  SIZE_T Cookie,
  IHostAutoEvent** ppEvent)
{
    PepAppHostAutoEvent* pPepAppHostAutoEvent;

    if (ppEvent == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostAutoEvent = new (std::nothrow) PepAppHostAutoEvent(Cookie);

    if (pPepAppHostAutoEvent)
    {
        *ppEvent = pPepAppHostAutoEvent;

        pPepAppHostAutoEvent->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateRWLockReaderEvent(
  BOOL bInitialState,
  SIZE_T Cookie,
  IHostManualEvent** ppEvent)
{
    PepAppHostManualEvent* pPepAppHostManualEvent;

    if (ppEvent == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostManualEvent = new (std::nothrow) PepAppHostManualEvent(bInitialState, Cookie);

    if (pPepAppHostManualEvent)
    {
        *ppEvent = pPepAppHostManualEvent;

        pPepAppHostManualEvent->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSyncManager::CreateSemaphore(
  DWORD dwInitial,
  DWORD dwMax,
  IHostSemaphore** ppSemaphore)
{
    PepAppHostSemaphore* pPepAppHostSemaphore;

    if (ppSemaphore == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostSemaphore = new (std::nothrow) PepAppHostSemaphore(dwInitial, dwMax);

    if (pPepAppHostSemaphore)
    {
        *ppSemaphore = pPepAppHostSemaphore;

        pPepAppHostSemaphore->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
