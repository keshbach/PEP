/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppActionOnCLREvent : public IActionOnCLREvent
{
private:
    ULONG m_ulRefCount;

public:
    PepAppActionOnCLREvent();
    virtual ~PepAppActionOnCLREvent();

private:
    PepAppActionOnCLREvent(const PepAppActionOnCLREvent&);
    PepAppActionOnCLREvent& operator = (const PepAppActionOnCLREvent&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IActionOnCLREvent
public:
    virtual HRESULT STDMETHODCALLTYPE OnEvent(EClrEvent event, PVOID pvData);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
