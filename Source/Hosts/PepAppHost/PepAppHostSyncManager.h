/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostSyncManager : public IHostSyncManager
{
private:
    ULONG m_ulRefCount;
    ICLRSyncManager* m_pCLRSyncManager;

public:
    PepAppHostSyncManager();
    virtual ~PepAppHostSyncManager();

private:
    PepAppHostSyncManager(const PepAppHostSyncManager&);
    PepAppHostSyncManager& operator = (const PepAppHostSyncManager&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostSyncManager
public:
    virtual HRESULT STDMETHODCALLTYPE SetCLRSyncManager(ICLRSyncManager* pManager);
    virtual HRESULT STDMETHODCALLTYPE CreateCrst(IHostCrst** ppCrst);
    virtual HRESULT STDMETHODCALLTYPE CreateCrstWithSpinCount(DWORD dwSpinCount, IHostCrst** ppCrst);
    virtual HRESULT STDMETHODCALLTYPE CreateAutoEvent(IHostAutoEvent** ppEvent);
    virtual HRESULT STDMETHODCALLTYPE CreateManualEvent(BOOL bInitialState, IHostManualEvent** ppEvent);
    virtual HRESULT STDMETHODCALLTYPE CreateMonitorEvent(SIZE_T Cookie, IHostAutoEvent** ppEvent);
    virtual HRESULT STDMETHODCALLTYPE CreateRWLockWriterEvent(SIZE_T Cookie, IHostAutoEvent** ppEvent);
    virtual HRESULT STDMETHODCALLTYPE CreateRWLockReaderEvent(BOOL bInitialState, SIZE_T Cookie, IHostManualEvent** ppEvent);
    virtual HRESULT STDMETHODCALLTYPE CreateSemaphore(DWORD dwInitial, DWORD dwMax, IHostSemaphore** ppSemaphore);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
