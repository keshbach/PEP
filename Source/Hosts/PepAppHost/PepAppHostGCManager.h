/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostGCManager : public IHostGCManager
{
private:
    ULONG m_ulRefCount;

public:
    PepAppHostGCManager();
    virtual ~PepAppHostGCManager();

private:
    PepAppHostGCManager(const PepAppHostGCManager&);
    PepAppHostGCManager& operator = (const PepAppHostGCManager&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostGCManager
public:
    virtual HRESULT STDMETHODCALLTYPE ThreadIsBlockingForSuspension(void);
    virtual HRESULT STDMETHODCALLTYPE SuspensionStarting(void);
    virtual HRESULT STDMETHODCALLTYPE SuspensionEnding(DWORD dwGeneration);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
