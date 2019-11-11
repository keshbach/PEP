/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostCLRAssemblyReferenceList.h"

PepAppCLRAssemblyReferenceList::PepAppCLRAssemblyReferenceList()
{
	m_ulRefCount = 0;
}

PepAppCLRAssemblyReferenceList::~PepAppCLRAssemblyReferenceList()
{
}

#pragma region "IUnknown"

ULONG STDMETHODCALLTYPE PepAppCLRAssemblyReferenceList::AddRef()
{
	return ::InterlockedIncrement(&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE PepAppCLRAssemblyReferenceList::Release()
{
	if (::InterlockedDecrement(&m_ulRefCount) == 0)
	{
		delete this;

		return 0;
	}

	return m_ulRefCount;
}

HRESULT STDMETHODCALLTYPE PepAppCLRAssemblyReferenceList::QueryInterface(
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

#pragma region "ICLRAssemblyReferenceList"

HRESULT STDMETHODCALLTYPE PepAppCLRAssemblyReferenceList::IsStringAssemblyReferenceInList(
  LPCWSTR pwzAssemblyName)
{
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE PepAppCLRAssemblyReferenceList::IsAssemblyReferenceInList(
  IUnknown* pUnknown)
{
	IAssemblyName* pAssemblyName = NULL;
	IReferenceIdentity* pReferenceIdentity = NULL;

	if (pUnknown->QueryInterface(&pAssemblyName) == S_OK)
	{
		pAssemblyName->Release();

		return S_OK;
	}

	if (pUnknown->QueryInterface(&pReferenceIdentity) == S_OK)
	{
		pReferenceIdentity->Release();
	}

	return E_FAIL;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
