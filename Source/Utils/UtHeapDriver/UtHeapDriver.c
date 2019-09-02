/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <Utils/UtHeapDriver.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, UtInitMemPoolTag)
#pragma alloc_text (PAGE, UtAllocPagedMem)
#pragma alloc_text (PAGE, UtFreePagedMem)
#endif

#pragma region "Local Variables"

static ULONG l_MemPoolTag = 0;
static POOL_TYPE l_NonPagedPoolType = NonPagedPool;

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID UTHEAPDRIVERAPI UtInitMemPoolTag(
  _In_ ULONG Tag)
{
    RTL_OSVERSIONINFOW VersionInfo;

    PAGED_CODE()

    l_MemPoolTag = Tag;

    VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);

    if (STATUS_SUCCESS == RtlGetVersion(&VersionInfo))
    {
        if (VersionInfo.dwMajorVersion > 6)
        {
            l_NonPagedPoolType = NonPagedPoolNx;
        }
        else if (VersionInfo.dwMajorVersion == 6 && VersionInfo.dwMinorVersion >= 2)
        {
            l_NonPagedPoolType = NonPagedPoolNx;
        }
    }
}

_IRQL_requires_max_(APC_LEVEL)
PVOID UTHEAPDRIVERAPI UtAllocPagedMem(
  _In_ SIZE_T NumberOfBytes)
{
    PAGED_CODE()
        
    return ExAllocatePoolWithTag(PagedPool, NumberOfBytes, l_MemPoolTag);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(
  _In_ SIZE_T NumberOfBytes)
{
    return ExAllocatePoolWithTag(l_NonPagedPoolType, NumberOfBytes, l_MemPoolTag);
}

_IRQL_requires_max_(APC_LEVEL)
VOID UTHEAPDRIVERAPI UtFreePagedMem(
  _In_ PVOID pvMem)
{
    PAGED_CODE()

    ExFreePoolWithTag(pvMem, l_MemPoolTag);
}

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID UTHEAPDRIVERAPI UtFreeNonPagedMem(
  _In_ PVOID pvMem)
{
    ExFreePoolWithTag(pvMem, l_MemPoolTag);
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
