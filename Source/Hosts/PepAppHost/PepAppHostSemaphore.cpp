/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostSemaphore.h"

#include "UtPepAppHostUtility.h"

PepAppHostSemaphore::PepAppHostSemaphore(
  DWORD dwInitial,
  DWORD dwMax)
{
    m_ulRefCount = 0;

    m_hSemaphore = ::CreateSemaphore(NULL, dwInitial, dwMax, NULL);
}

PepAppHostSemaphore::~PepAppHostSemaphore()
{
    if (m_hSemaphore)
    {
        ::CloseHandle(m_hSemaphore);

        m_hSemaphore = NULL;
    }
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostSemaphore::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostSemaphore::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostSemaphore::QueryInterface(
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

#pragma region "IHostSemaphore"

HRESULT STDMETHODCALLTYPE PepAppHostSemaphore::Wait(
  DWORD dwMilliseconds,
  DWORD dwOption)
{
    if (m_hSemaphore)
    {
		return UtPepAppHostUtilityWait(m_hSemaphore, dwMilliseconds, dwOption);
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSemaphore::ReleaseSemaphore(
    LONG lReleaseCount,
    LONG* plPreviousCount)
{
    if (m_hSemaphore)
    {
        if (::ReleaseSemaphore(m_hSemaphore, lReleaseCount, plPreviousCount))
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
