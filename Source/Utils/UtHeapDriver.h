/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtHeapDriver_H)
#define UtHeapDriver_H

#define UTHEAPDRIVERAPI __stdcall

VOID UTHEAPDRIVERAPI UtInitMemPoolTag(_In_ ULONG Tag);

PVOID UTHEAPDRIVERAPI UtAllocPagedMem(_In_ SIZE_T NumberOfBytes);
PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(_In_ SIZE_T NumberOfBytes);

VOID UTHEAPDRIVERAPI UtFreePagedMem(_In_ PVOID pvMem);
VOID UTHEAPDRIVERAPI UtFreeNonPagedMem(_In_ PVOID pvMem);

#endif /* !defined(UtHeapDriver_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
