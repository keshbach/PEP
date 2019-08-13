/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostMalloc : public IHostMalloc
{
private:
    ULONG m_ulRefCount;
    HANDLE m_hHeap;

public:
    PepAppHostMalloc(BOOL bThreadSafe, BOOL bExecutable);
    virtual ~PepAppHostMalloc();

private:
    PepAppHostMalloc();
    PepAppHostMalloc(const PepAppHostMalloc&);
    PepAppHostMalloc& operator = (const PepAppHostMalloc&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostMalloc
public:
    virtual HRESULT STDMETHODCALLTYPE Alloc(SIZE_T cbSize, EMemoryCriticalLevel eCriticalLevel, void** ppvMem);
    virtual HRESULT STDMETHODCALLTYPE DebugAlloc(SIZE_T cbSize, EMemoryCriticalLevel eCriticalLevel, char* pszFileName, int iLineNo, _Outptr_result_maybenull_ void** ppvMem);
    virtual HRESULT STDMETHODCALLTYPE Free(void* pvMem);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
