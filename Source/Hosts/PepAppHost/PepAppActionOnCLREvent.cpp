/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppActionOnCLREvent.h"

PepAppActionOnCLREvent::PepAppActionOnCLREvent()
{
    m_ulRefCount = 0;
}

PepAppActionOnCLREvent::~PepAppActionOnCLREvent()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppActionOnCLREvent::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppActionOnCLREvent::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppActionOnCLREvent::QueryInterface(
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

#pragma region "IActionOnCLREvent"

HRESULT STDMETHODCALLTYPE PepAppActionOnCLREvent::OnEvent(
  EClrEvent event,
  PVOID pvData)
{
    return S_OK;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
