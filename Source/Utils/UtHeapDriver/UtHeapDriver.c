/***************************************************************************/
/*  Copyright (C) 2006-2012 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <Utils/UtHeapDriver.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, UtInitMemPoolTag)
#endif

static ULONG l_MemPoolTag = 0;

VOID UTHEAPDRIVERAPI UtInitMemPoolTag(
  IN ULONG Tag)
{
    PAGED_CODE()

    l_MemPoolTag = Tag;
}

PVOID UTHEAPDRIVERAPI UtAllocPagedMem(
  IN SIZE_T NumberOfBytes)
{
    return ExAllocatePoolWithTag(PagedPool, NumberOfBytes, l_MemPoolTag);
}

PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(
  IN SIZE_T NumberOfBytes)
{
    return ExAllocatePoolWithTag(NonPagedPool, NumberOfBytes, l_MemPoolTag);
}

VOID UTHEAPDRIVERAPI UtFreePagedMem(
  IN PVOID pvMem)
{
    ExFreePoolWithTag(pvMem, l_MemPoolTag | PROTECTED_POOL);
}

VOID UTHEAPDRIVERAPI UtFreeNonPagedMem(
  IN PVOID pvMem)
{
    ExFreePoolWithTag(pvMem, l_MemPoolTag | PROTECTED_POOL);
}

/***************************************************************************/
/*  Copyright (C) 2006-2012 Kevin Eshbach                                  */
/***************************************************************************/
