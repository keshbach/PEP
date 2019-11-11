/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostSecurityManager.h"

#include <new>

#include "PepAppHostSecurityContext.h"

PepAppHostSecurityManager::PepAppHostSecurityManager()
{
    m_ulRefCount = 0;
}

PepAppHostSecurityManager::~PepAppHostSecurityManager()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostSecurityManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostSecurityManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostSecurityManager::QueryInterface(
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

#pragma region "IHostSecurityManager"

HRESULT STDMETHODCALLTYPE PepAppHostSecurityManager::ImpersonateLoggedOnUser(
  HANDLE hToken)
{
    return ::ImpersonateLoggedOnUser(hToken) ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSecurityManager::RevertToSelf(void)
{
    return ::RevertToSelf() ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSecurityManager::OpenThreadToken(
  DWORD dwDesiredAccess,
  BOOL bOpenAsSelf,
  HANDLE* phThreadToken)
{
    if (phThreadToken == NULL)
    {
        return E_POINTER;
    }

    return ::OpenThreadToken(::GetCurrentThread(), dwDesiredAccess, bOpenAsSelf, phThreadToken) ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSecurityManager::SetThreadToken(
  HANDLE hToken)
{
    return ::SetThreadToken(NULL, hToken) ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSecurityManager::GetSecurityContext(
  EContextType eContextType,
  IHostSecurityContext** ppSecurityContext)
{
    PepAppHostSecurityContext* pPepAppHostSecurityContext;

    if (ppSecurityContext == NULL)
    {
        return E_POINTER;
    }

    pPepAppHostSecurityContext = new (std::nothrow) PepAppHostSecurityContext(eContextType);

    if (pPepAppHostSecurityContext)
    {
        pPepAppHostSecurityContext->AddRef();

        *ppSecurityContext = pPepAppHostSecurityContext;

        return S_OK;
    }

    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostSecurityManager::SetSecurityContext(
  EContextType eContextType,
  IHostSecurityContext* pSecurityContext)
{
    return S_OK;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
