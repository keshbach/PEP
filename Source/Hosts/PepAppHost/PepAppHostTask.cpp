/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostTask.h"

PepAppHostTask::PepAppHostTask()
{
    m_ulRefCount = 0;
    m_pCLRTask = NULL;
    m_hThread = NULL;
}

PepAppHostTask::PepAppHostTask(
  DWORD dwStackSize,
  LPTHREAD_START_ROUTINE pStartAddress,
  PVOID pvParameter)
{
    m_ulRefCount = 0;
    m_pCLRTask = NULL;
    m_hThread = ::CreateThread(NULL, 0, pStartAddress, pvParameter, CREATE_SUSPENDED, NULL);
}

PepAppHostTask::~PepAppHostTask()
{
    if (m_hThread)
    {
        ::CloseHandle(m_hThread);
    }

    if (m_pCLRTask)
    {
        m_pCLRTask->Release();

        m_pCLRTask = NULL;
    }
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostTask::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostTask::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostTask::QueryInterface(
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

#pragma region "IHostTask"

HRESULT STDMETHODCALLTYPE PepAppHostTask::Start(void)
{
    if (m_hThread)
    {
        if (::ResumeThread(m_hThread))
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTask::Alert(void)
{
    if (m_hThread)
    {
        return S_OK;
    }

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTask::Join(
  DWORD dwMilliseconds,
  DWORD option)
{
    if (m_hThread)
    {
        switch (::WaitForSingleObject(m_hThread, dwMilliseconds))
        {
            case WAIT_OBJECT_0:
                return S_OK;
            case WAIT_TIMEOUT:
                return HOST_E_TIMEOUT;
        }
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTask::SetPriority(
  int newPriority)
{
    if (m_hThread)
    {
        if (::SetThreadPriority(m_hThread, newPriority))
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTask::GetPriority(
  int* pPriority)
{
    if (m_hThread)
    {
        *pPriority = ::GetThreadPriority(m_hThread);

        if (*pPriority != THREAD_PRIORITY_ERROR_RETURN)
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostTask::SetCLRTask(
  ICLRTask* pCLRTask)
{
    if (m_pCLRTask)
    {
        m_pCLRTask->Release();
    }

    m_pCLRTask = pCLRTask;

    m_pCLRTask->AddRef();

    return S_OK;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
