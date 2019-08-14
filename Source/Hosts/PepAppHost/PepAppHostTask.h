/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostTask : public IHostTask
{
private:
    ULONG m_ulRefCount;
    ICLRTask* m_pCLRTask;
    HANDLE m_hThread;

public:
    PepAppHostTask();
    PepAppHostTask(DWORD dwStackSize, LPTHREAD_START_ROUTINE pStartAddress, PVOID pvParameter);
    virtual ~PepAppHostTask();

private:
    PepAppHostTask(const PepAppHostTask&);
    PepAppHostTask& operator = (const PepAppHostTask&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostTask
public:
    virtual HRESULT STDMETHODCALLTYPE Start(void);
    virtual HRESULT STDMETHODCALLTYPE Alert(void);
    virtual HRESULT STDMETHODCALLTYPE Join(DWORD dwMilliseconds, DWORD option);
    virtual HRESULT STDMETHODCALLTYPE SetPriority(int newPriority);
    virtual HRESULT STDMETHODCALLTYPE GetPriority(int* pPriority);
    virtual HRESULT STDMETHODCALLTYPE SetCLRTask(ICLRTask* pCLRTask);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
