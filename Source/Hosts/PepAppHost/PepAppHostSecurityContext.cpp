/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostSecurityContext.h"

#include <new>

PepAppHostSecurityContext::PepAppHostSecurityContext(
  EContextType ContextType)
{
    m_ulRefCount = 0;
    m_ContextType = ContextType;
}

PepAppHostSecurityContext::~PepAppHostSecurityContext()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostSecurityContext::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostSecurityContext::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostSecurityContext::QueryInterface(
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

#pragma region "IHostSecurityContext"

HRESULT STDMETHODCALLTYPE PepAppHostSecurityContext::Capture(
  IHostSecurityContext** ppClonedContext)
{
    PepAppHostSecurityContext* pPepAppHostSecurityContext;

    if (ppClonedContext == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostSecurityContext = new (std::nothrow) PepAppHostSecurityContext(m_ContextType);

    if (pPepAppHostSecurityContext)
    {
        pPepAppHostSecurityContext->AddRef();

        *ppClonedContext = pPepAppHostSecurityContext;

        return S_OK;
    }

    return E_FAIL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
