/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtHeapDriver_H)
#define UtHeapDriver_H

#define UTHEAPDRIVERAPI __stdcall

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID UTHEAPDRIVERAPI UtInitMemPoolTag(_In_ ULONG Tag);

_IRQL_requires_max_(APC_LEVEL)
PVOID UTHEAPDRIVERAPI UtAllocPagedMem(_In_ SIZE_T NumberOfBytes);

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(_In_ SIZE_T NumberOfBytes);

_IRQL_requires_max_(APC_LEVEL)
VOID UTHEAPDRIVERAPI UtFreePagedMem(_In_ PVOID pvMem);

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID UTHEAPDRIVERAPI UtFreeNonPagedMem(_In_ PVOID pvMem);

#endif /* !defined(UtHeapDriver_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
