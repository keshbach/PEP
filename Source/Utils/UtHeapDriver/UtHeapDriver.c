/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <Utils/UtHeapDriver.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, UtInitMemPoolTag)
#endif

static ULONG l_MemPoolTag = 0;

VOID UTHEAPDRIVERAPI UtInitMemPoolTag(
  _In_ ULONG Tag)
{
    PAGED_CODE()

    l_MemPoolTag = Tag;
}

PVOID UTHEAPDRIVERAPI UtAllocPagedMem(
  _In_ SIZE_T NumberOfBytes)
{
    return ExAllocatePoolWithTag(PagedPool, NumberOfBytes, l_MemPoolTag);
}

PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(
  _In_ SIZE_T NumberOfBytes)
{
    POOL_TYPE PoolType = NonPagedPool;
    RTL_OSVERSIONINFOW VersionInfo;

    VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
    
    if (STATUS_SUCCESS == RtlGetVersion(&VersionInfo))
    {
        if (VersionInfo.dwMajorVersion > 6)
        {
            PoolType = NonPagedPoolNx;
        }
        else if (VersionInfo.dwMajorVersion == 6 && VersionInfo.dwMinorVersion >= 2)
        {
            PoolType = NonPagedPoolNx;
        }
    }

    return ExAllocatePoolWithTag(PoolType, NumberOfBytes, l_MemPoolTag);
}

VOID UTHEAPDRIVERAPI UtFreePagedMem(
  _In_ PVOID pvMem)
{
    ExFreePoolWithTag(pvMem, l_MemPoolTag);
}

VOID UTHEAPDRIVERAPI UtFreeNonPagedMem(
  _In_ PVOID pvMem)
{
    ExFreePoolWithTag(pvMem, l_MemPoolTag);
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
