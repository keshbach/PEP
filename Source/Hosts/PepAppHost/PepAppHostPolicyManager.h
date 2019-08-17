/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostPolicyManager : public IHostPolicyManager
{
private:
    ULONG m_ulRefCount;

public:
    PepAppHostPolicyManager();
    virtual ~PepAppHostPolicyManager();

private:
    PepAppHostPolicyManager(const PepAppHostPolicyManager&);
    PepAppHostPolicyManager& operator = (const PepAppHostPolicyManager&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostPolicyManager
public:
    virtual HRESULT STDMETHODCALLTYPE OnDefaultAction(EClrOperation operation, EPolicyAction action);
    virtual HRESULT STDMETHODCALLTYPE OnTimeout(EClrOperation operation, EPolicyAction action);
    virtual HRESULT STDMETHODCALLTYPE OnFailure(EClrFailure failure, EPolicyAction action);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
