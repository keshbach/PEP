/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostSemaphore : public IHostSemaphore
{
private:
    ULONG m_ulRefCount;
    HANDLE m_hSemaphore;

public:
    PepAppHostSemaphore(DWORD dwInitial, DWORD dwMax);
    virtual ~PepAppHostSemaphore();

private:
    PepAppHostSemaphore();
    PepAppHostSemaphore(const PepAppHostSemaphore&);
    PepAppHostSemaphore& operator = (const PepAppHostSemaphore&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostSemaphore
public:
    virtual HRESULT STDMETHODCALLTYPE Wait(DWORD dwMilliseconds, DWORD option);
    virtual HRESULT STDMETHODCALLTYPE ReleaseSemaphore(LONG lReleaseCount, LONG* plPreviousCount);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
