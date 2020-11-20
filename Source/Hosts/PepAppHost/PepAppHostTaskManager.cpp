/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostTaskManager.h"

#include <new>

#include "PepAppHostTask.h"

#include "UtPepAppHostTasks.h"
#include "UtPepAppHostUtility.h"

PepAppHostTaskManager::PepAppHostTaskManager()
{
    m_ulRefCount = 0;
    m_pCLRTaskManager = NULL;
}

PepAppHostTaskManager::~PepAppHostTaskManager()
{
    if (m_pCLRTaskManager)
    {
        m_pCLRTaskManager->Release();

        m_pCLRTaskManager = NULL;
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
	PepAppHostTask* pPepAppHostTask;

    if (ppTask == NULL)
    {
        return E_POINTER;
    }

	if (FALSE == UtPepAppHostTasksFind(::GetCurrentThreadId(), &pPepAppHostTask))
	{
		return E_FAIL;
	}

	if (pPepAppHostTask == NULL)
	{
		if (FALSE == UtPepAppHostTasksCreate(::GetCurrentThreadId(), &pPepAppHostTask))
		{
			return E_FAIL;
		}
	}

	*ppTask = pPepAppHostTask;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::CreateTask(
  DWORD dwStackSize,
  LPTHREAD_START_ROUTINE pStartAddress,
  PVOID pvParameter,
  IHostTask** ppTask)
{
    if (ppTask == NULL)
    {
        return E_POINTER;
    }

	if (FALSE == UtPepAppHostTasksCreate(dwStackSize, pStartAddress, pvParameter, ppTask))
	{
		return E_FAIL;
	}

	(*ppTask)->AddRef();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::Sleep(
  DWORD dwMilliseconds,
  DWORD dwOption)
{
	return UtPepAppHostUtilitySleep(dwMilliseconds, dwOption);
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SwitchToTask(
  DWORD dwOption)
{
    dwOption;

	::SwitchToThread();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SetUILocale(
  LCID lcid)
{
    lcid;

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::SetLocale(
  LCID lcid)
{
    lcid;

	if (!::SetThreadLocale(lcid)) {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::CallNeedsHostHook(
  SIZE_T target,
  BOOL* pbCallNeedsHostHook)
{
    target;

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
    target;

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
    guarantee;

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTaskManager::GetStackGuarantee(
  ULONG* pGuarantee)
{
	if (pGuarantee == NULL)
	{
		return E_POINTER;
	}

    *pGuarantee = 0;

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
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
