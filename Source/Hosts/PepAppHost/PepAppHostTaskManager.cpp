/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostTaskManager.h"

#include <new>

#include "PepAppHostTask.h"

PepAppHostTaskManager::PepAppHostTaskManager()
{
    m_ulRefCount = 0;
    m_pCLRTaskManager = NULL;
    m_pDummyHostTask = new (std::nothrow) PepAppHostTask();
}

PepAppHostTaskManager::~PepAppHostTaskManager()
{
    if (m_pCLRTaskManager)
    {
        m_pCLRTaskManager->Release();

        m_pCLRTaskManager = NULL;
    }

    if (m_pDummyHostTask)
    {
        delete m_pDummyHostTask;

        m_pDummyHostTask = NULL;
    }
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostTaskManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostTaskManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::QueryInterface(
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

#pragma region "IHostTaskManager"

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::GetCurrentTask(
  IHostTask** ppTask)
{
    if (ppTask == NULL)
    {
        return E_POINTER;
    }

    *ppTask = m_pDummyHostTask;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::CreateTask(
  DWORD dwStackSize,
  LPTHREAD_START_ROUTINE pStartAddress,
  PVOID pvParameter,
  IHostTask** ppTask)
{
    PepAppHostTask* pPepAppHostTask;

    if (ppTask == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostTask = new (std::nothrow) PepAppHostTask(dwStackSize, pStartAddress, pvParameter);

    if (pPepAppHostTask)
    {
        pPepAppHostTask->AddRef();

        *ppTask = pPepAppHostTask;

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::Sleep(
  DWORD dwMilliseconds,
  DWORD option)
{
    ::Sleep(dwMilliseconds);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SwitchToTask(
  DWORD option)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SetUILocale(
  LCID lcid)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SetLocale(
  LCID lcid)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::CallNeedsHostHook(
  SIZE_T target,
  BOOL* pbCallNeedsHostHook)
{
    if (pbCallNeedsHostHook == NULL)
    {
        return E_POINTER;
    }

    *pbCallNeedsHostHook = FALSE;
    
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::LeaveRuntime(
  SIZE_T target)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::EnterRuntime(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::ReverseLeaveRuntime(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::ReverseEnterRuntime(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::BeginDelayAbort(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::EndDelayAbort(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::BeginThreadAffinity(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::EndThreadAffinity(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SetStackGuarantee(
  ULONG guarantee)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::GetStackGuarantee(
  ULONG* pGuarantee)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SetCLRTaskManager(
  ICLRTaskManager* pManager)
{
    if (m_pCLRTaskManager)
    {
        m_pCLRTaskManager->Release();
    }

    m_pCLRTaskManager = pManager;

    m_pCLRTaskManager->AddRef();

    return S_OK;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
