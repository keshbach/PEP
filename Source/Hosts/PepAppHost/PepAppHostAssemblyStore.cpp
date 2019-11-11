/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostAssemblyStore.h"

PepAppHostAssemblyStore::PepAppHostAssemblyStore()
{
	m_ulRefCount = 0;
}

PepAppHostAssemblyStore::~PepAppHostAssemblyStore()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppHostAssemblyStore::AddRef()
{
	return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppHostAssemblyStore::Release()
{
	if (::InterlockedDecrement(&m_ulRefCount) == 0)
	{
		delete this;

		return 0;
	}

	return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyStore::QueryInterface(
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

#pragma region "IHostAssemblyStore"

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyStore::ProvideAssembly(
  AssemblyBindInfo* pBindInfo,
  UINT64* pAssemblyId,
  UINT64* pContext,
  IStream** ppStmAssemblyImage,
  IStream** ppStmPDB)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PepAppHostAssemblyStore::ProvideModule(
  ModuleBindInfo* pBindInfo,
  DWORD* pdwModuleId,
  IStream** ppStmModuleImage,
  IStream** ppStmPDB)
{
	return E_NOTIMPL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
