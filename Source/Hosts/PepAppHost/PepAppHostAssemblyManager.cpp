/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostAssemblyManager.h"

#include <new>

#include "PepAppHostCLRAssemblyReferenceList.h"
#include "PepAppHostAssemblyStore.h"

PepAppHostAssemblyManager::PepAppHostAssemblyManager()
{
    m_ulRefCount = 0;
}

PepAppHostAssemblyManager::~PepAppHostAssemblyManager()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostAssemblyManager::AddRef()
{
    return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostAssemblyManager::Release()
{
    if (::InterlockedDecrement(&m_ulRefCount) == 0)
    {
        delete this;

        return 0;
    }

    return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyManager::QueryInterface(
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

#pragma region "IHostAssemblyManager"

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyManager::GetNonHostStoreAssemblies(
  ICLRAssemblyReferenceList** ppReferenceList)
{
	PepAppCLRAssemblyReferenceList* pPepAppCLRAssemblyReferenceList;
	
	if (ppReferenceList == NULL)
    {
        return E_POINTER;
    }

	pPepAppCLRAssemblyReferenceList = new (std::nothrow) PepAppCLRAssemblyReferenceList();

	if (pPepAppCLRAssemblyReferenceList)
	{
		pPepAppCLRAssemblyReferenceList->AddRef();

		*ppReferenceList = pPepAppCLRAssemblyReferenceList;

		return S_OK;
	}

	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyManager::GetAssemblyStore(
  IHostAssemblyStore** ppAssemblyStore)
{
	PepAppHostAssemblyStore* pPepAppHostAssemblyStore;

    if (ppAssemblyStore == NULL)
    {
        return E_POINTER;
    }

	pPepAppHostAssemblyStore = new (std::nothrow) PepAppHostAssemblyStore();

	if (pPepAppHostAssemblyStore)
	{
		pPepAppHostAssemblyStore->AddRef();

		*ppAssemblyStore = pPepAppHostAssemblyStore;

		return S_OK;
	}

	return E_FAIL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
