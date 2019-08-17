/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostPolicyManager.h"

PepAppHostPolicyManager::PepAppHostPolicyManager()
{
    m_ulRefCount = 0;
}

PepAppHostPolicyManager::~PepAppHostPolicyManager()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostPolicyManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostPolicyManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostPolicyManager::QueryInterface(
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

#pragma region "IHostPolicyManager"

HRESULT STDMETHODCALLTYPE PepAppHostPolicyManager::OnDefaultAction(
  EClrOperation operation,
  EPolicyAction action)
{
    operation;
    action;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostPolicyManager::OnTimeout(
  EClrOperation operation,
  EPolicyAction action)
{
    operation;
    action;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PepAppHostPolicyManager::OnFailure(
  EClrFailure failure,
  EPolicyAction action)
{
    failure;
    action;

    return S_OK;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
