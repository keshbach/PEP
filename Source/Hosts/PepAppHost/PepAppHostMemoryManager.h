/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostMemoryManager : public IHostMemoryManager
{
private:
    ULONG m_ulRefCount;
    ICLRMemoryNotificationCallback* m_pMemoryNotificationCallback;

public:
    PepAppHostMemoryManager();
    virtual ~PepAppHostMemoryManager();

private:
    PepAppHostMemoryManager(const PepAppHostMemoryManager&);
    PepAppHostMemoryManager& operator = (const PepAppHostMemoryManager&);

// IUnknown
public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);

// IHostMemoryManager
public:
    virtual HRESULT STDMETHODCALLTYPE CreateMalloc(DWORD dwMallocType, IHostMalloc** ppMalloc);
    virtual HRESULT STDMETHODCALLTYPE VirtualAlloc(void* pvAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect, EMemoryCriticalLevel eCriticalLevel, void** ppvMem);
    virtual HRESULT STDMETHODCALLTYPE VirtualFree(void* pvAddress, SIZE_T dwSize, DWORD dwFreeType);
    virtual HRESULT STDMETHODCALLTYPE VirtualQuery(void* pvAddress, void* pvBuffer, SIZE_T dwLength, SIZE_T* pResult);
    virtual HRESULT STDMETHODCALLTYPE VirtualProtect(void* pvAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD* pflOldProtect);
    virtual HRESULT STDMETHODCALLTYPE GetMemoryLoad(DWORD* pdwMemoryLoad, SIZE_T* pAvailableBytes);
    virtual HRESULT STDMETHODCALLTYPE RegisterMemoryNotificationCallback(ICLRMemoryNotificationCallback* pCallback);
    virtual HRESULT STDMETHODCALLTYPE NeedsVirtualAddressSpace(void* pvStartAddress, SIZE_T size);
    virtual HRESULT STDMETHODCALLTYPE AcquiredVirtualAddressSpace(void* pvStartAddress, SIZE_T size);
    virtual HRESULT STDMETHODCALLTYPE ReleasedVirtualAddressSpace(void* pvStartAddress);
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
