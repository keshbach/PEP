/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostManualEvent.h"

PepAppHostManualEvent::PepAppHostManualEvent(
  BOOL bInitialState,
  SIZE_T Cookie)
{
    m_ulRefCount = 0;

    m_Cookie = Cookie;

    m_hEvent = ::CreateEvent(NULL, TRUE, bInitialState, NULL);
}

PepAppHostManualEvent::~PepAppHostManualEvent()
{
    if (m_hEvent)
    {
        ::CloseHandle(m_hEvent);

        m_hEvent = NULL;
    }
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostManualEvent::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostManualEvent::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostManualEvent::QueryInterface(
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

#pragma region "IHostManualEvent"

HRESULT STDMETHODCALLTYPE PepAppHostManualEvent::Wait(
  DWORD dwMilliseconds,
  DWORD option)
{
	DWORD dwResult;

    if (m_hEvent)
    {
		if (option & WAIT_ALERTABLE)
		{
			dwResult = ::WaitForSingleObjectEx(m_hEvent, dwMilliseconds, TRUE);
		}
		else
		{
			dwResult = ::WaitForSingleObject(m_hEvent, dwMilliseconds);
		}

		switch (dwResult)
        {
            case WAIT_OBJECT_0:
                return S_OK;
			case WAIT_ABANDONED:
				return HOST_E_ABANDONED;
			case WAIT_IO_COMPLETION:
				return HOST_E_INTERRUPTED;
            case WAIT_TIMEOUT:
                return HOST_E_TIMEOUT;
        }
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostManualEvent::Reset(void)
{
    if (m_hEvent)
    {
        if (::ResetEvent(m_hEvent))
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostManualEvent::Set(void)
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
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
