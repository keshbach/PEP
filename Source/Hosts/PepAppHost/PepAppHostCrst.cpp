/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostCrst.h"

PepAppHostCrst::PepAppHostCrst()
{
    m_ulRefCount = 0;

    ::InitializeCriticalSection(&m_CriticalSection);

    m_bValid = TRUE;
}

PepAppHostCrst::PepAppHostCrst(
  DWORD dwSpinCount)
{
    m_ulRefCount = 0;

    m_bValid = ::InitializeCriticalSectionAndSpinCount(&m_CriticalSection, dwSpinCount);
}

PepAppHostCrst::~PepAppHostCrst()
{
    if (m_bValid)
    {
        ::DeleteCriticalSection(&m_CriticalSection);
    }
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostCrst::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostCrst::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostCrst::QueryInterface(
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

#pragma region "IHostCrst"

HRESULT STDMETHODCALLTYPE PepAppHostCrst::Enter(
  DWORD option)
{
    option;

    if (m_bValid)
    {
        ::EnterCriticalSection(&m_CriticalSection);

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostCrst::Leave(void)
{
    if (m_bValid)
    {
        ::LeaveCriticalSection(&m_CriticalSection);

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostCrst::TryEnter(
  DWORD option,
  BOOL* pbSucceeded)
{
    option;

    if (pbSucceeded == NULL)
    {
        return E_POINTER;
    }

    if (m_bValid)
    {
        *pbSucceeded = ::TryEnterCriticalSection(&m_CriticalSection);

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostCrst::SetSpinCount(
  DWORD dwSpinCount)
{
    if (m_bValid)
    {
        ::SetCriticalSectionSpinCount(&m_CriticalSection, dwSpinCount);

        return S_OK;
    }

    return E_FAIL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
