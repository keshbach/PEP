/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostAutoEvent.h"

#include "UtPepAppHostUtility.h"

PepAppHostAutoEvent::PepAppHostAutoEvent(
  SIZE_T Cookie)
{
    m_ulRefCount = 0;
    m_Cookie = Cookie;

    m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

PepAppHostAutoEvent::~PepAppHostAutoEvent()
{
    if (m_hEvent)
    {
        ::CloseHandle(m_hEvent);

        m_hEvent = NULL;
    }
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostAutoEvent::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostAutoEvent::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostAutoEvent::QueryInterface(
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

#pragma region "IHostAutoEvent"

HRESULT STDMETHODCALLTYPE PepAppHostAutoEvent::Wait(
  DWORD dwMilliseconds,
  DWORD dwOption)
{
    if (m_hEvent)
    {
		return UtPepAppHostUtilityWait(m_hEvent, dwMilliseconds, dwOption);
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostAutoEvent::Set(void)
{
    if (m_hEvent)
    {
        if (::SetEvent(m_hEvent))
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
