/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <Utils/UtHeapDriver.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, UtInitMemPoolTag)
#pragma alloc_text (PAGE, UtAllocPagedMem)
#pragma alloc_text (PAGE, UtFreePagedMem)
#endif

typedef PVOID (NTAPI* TExAllocatePool2Func)(_In_ POOL_FLAGS Flags, _In_ SIZE_T NumberOfBytes, _In_ ULONG Tag);

#pragma region "Local Variables"

static ULONG l_MemPoolTag = 0;
static POOL_TYPE l_NonPagedPoolType = NonPagedPool;

static TExAllocatePool2Func l_pExAllocatePool2 = NULL;

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID UTHEAPDRIVERAPI UtInitMemPoolTag(
  _In_ ULONG Tag)
{
    RTL_OSVERSIONINFOW VersionInfo;
    UNICODE_STRING mmRoutineName;

    PAGED_CODE()

    l_MemPoolTag = Tag;

    VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);

    if (STATUS_SUCCESS == RtlGetVersion(&VersionInfo))
    {
        // Check for Windows 8 or above (pool type used for ExAllocatePool only)

        if (VersionInfo.dwMajorVersion > 6)
        {
            l_NonPagedPoolType = NonPagedPoolNx;
        }
        else if (VersionInfo.dwMajorVersion == 6 && VersionInfo.dwMinorVersion >= 2)
        {
            // Windows 8 or above

            l_NonPagedPoolType = NonPagedPoolNx;
        }

        // Check for Windows 10 or above

        if (VersionInfo.dwMajorVersion >= 10)
        {
            RtlInitUnicodeString(&mmRoutineName, L"ExAllocatePool2");

            l_pExAllocatePool2 = (TExAllocatePool2Func)MmGetSystemRoutineAddress(&mmRoutineName);
        }
    }
}

_IRQL_requires_max_(APC_LEVEL)
PVOID UTHEAPDRIVERAPI UtAllocPagedMem(
  _In_ SIZE_T NumberOfBytes)
{
    POOL_FLAGS PoolFlags;

    PAGED_CODE()

    if (l_pExAllocatePool2)
    {
        PoolFlags = POOL_FLAG_PAGED;

#if !defined(NDEBUG)
        PoolFlags |= POOL_FLAG_SPECIAL_POOL;
#endif

        return l_pExAllocatePool2(PoolFlags, NumberOfBytes, l_MemPoolTag);
    }

#pragma warning (disable:4996)
    return ExAllocatePoolWithTag(PagedPool, NumberOfBytes, l_MemPoolTag);
#pragma warning (default:4996)
}

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(
  _In_ SIZE_T NumberOfBytes)
{
    POOL_FLAGS PoolFlags;

    if (l_pExAllocatePool2)
    {
        PoolFlags = POOL_FLAG_NON_PAGED;

#if !defined(NDEBUG)
        PoolFlags |= POOL_FLAG_SPECIAL_POOL;
#endif

        return l_pExAllocatePool2(PoolFlags, NumberOfBytes, l_MemPoolTag);
    }

#pragma warning (disable:4996)
    return ExAllocatePoolWithTag(l_NonPagedPoolType, NumberOfBytes, l_MemPoolTag);
#pragma warning (default:4996)
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
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/
