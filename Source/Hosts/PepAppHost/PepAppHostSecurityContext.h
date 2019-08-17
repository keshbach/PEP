/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostSecurityContext : public IHostSecurityContext
{
private:
    ULONG m_ulRefCount;
    EContextType m_ContextType;

public:
    PepAppHostSecurityContext(EContextType ContextType);
    virtual ~PepAppHostSecurityContext();

private:
    PepAppHostSecurityContext();
    PepAppHostSecurityContext(const PepAppHostSecurityContext&);
    PepAppHostSecurityContext& operator = (const PepAppHostSecurityContext&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostSecurityContext
public:
    virtual HRESULT STDMETHODCALLTYPE Capture(IHostSecurityContext** ppClonedContext);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
