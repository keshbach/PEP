/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostTask : public IHostTask
{
private:
    ULONG m_ulRefCount;
    ICLRTask* m_pCLRTask;
    HANDLE m_hThread;
	DWORD m_dwThreadId;
	LPTHREAD_START_ROUTINE m_pStartAddress;
	PVOID m_pvParameter;

public:
	PepAppHostTask(DWORD dwThreadId);
    PepAppHostTask(DWORD dwStackSize, LPTHREAD_START_ROUTINE pStartAddress, PVOID pvParameter);
    virtual ~PepAppHostTask();

	inline DWORD GetThreadId() { return m_dwThreadId;  }

private:
	PepAppHostTask();
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
    virtual HRESULT STDMETHODCALLTYPE Join(DWORD dwMilliseconds, DWORD dwOption);
    virtual HRESULT STDMETHODCALLTYPE SetPriority(int newPriority);
    virtual HRESULT STDMETHODCALLTYPE GetPriority(int* pPriority);
    virtual HRESULT STDMETHODCALLTYPE SetCLRTask(ICLRTask* pCLRTask);

private:
	static DWORD WINAPI ThreadTask(_In_ LPVOID pvParameter);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
