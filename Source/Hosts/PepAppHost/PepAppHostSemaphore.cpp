/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostSemaphore.h"

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
  DWORD option)
{
	DWORD dwResult;

    if (m_hSemaphore)
    {
		if (option & WAIT_ALERTABLE)
		{
			dwResult = ::WaitForSingleObjectEx(m_hSemaphore, dwMilliseconds, TRUE);
		}
		else
		{
			dwResult = ::WaitForSingleObject(m_hSemaphore, dwMilliseconds);
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
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
