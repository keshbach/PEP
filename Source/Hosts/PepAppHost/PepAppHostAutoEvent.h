/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostAutoEvent : public IHostAutoEvent
{
private:
    ULONG m_ulRefCount;
    SIZE_T m_Cookie;
    HANDLE m_hEvent;

public:
    PepAppHostAutoEvent(SIZE_T Cookie);
    virtual ~PepAppHostAutoEvent();

private:
    PepAppHostAutoEvent();
    PepAppHostAutoEvent(const PepAppHostAutoEvent&);
    PepAppHostAutoEvent& operator = (const PepAppHostAutoEvent&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostAutoEvent
public:
    virtual HRESULT STDMETHODCALLTYPE Wait(DWORD dwMilliseconds, DWORD option);
    virtual HRESULT STDMETHODCALLTYPE Set(void);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
