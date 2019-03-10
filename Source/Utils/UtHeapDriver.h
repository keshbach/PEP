/***************************************************************************/
/*  Copyright (C) 2006-2012 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtHeapDriver_H)
#define UtHeapDriver_H

#define UTHEAPDRIVERAPI __stdcall

VOID UTHEAPDRIVERAPI UtInitMemPoolTag(IN ULONG Tag);

PVOID UTHEAPDRIVERAPI UtAllocPagedMem(IN SIZE_T NumberOfBytes);
PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(IN SIZE_T NumberOfBytes);

VOID UTHEAPDRIVERAPI UtFreePagedMem(IN PVOID pvMem);
VOID UTHEAPDRIVERAPI UtFreeNonPagedMem(IN PVOID pvMem);

#endif /* !defined(UtHeapDriver_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2012 Kevin Eshbach                                  */
/***************************************************************************/
