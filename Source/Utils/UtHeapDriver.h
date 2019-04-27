/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtHeapDriver_H)
#define UtHeapDriver_H

#include <Includes/UtExternC.h>

#define UTHEAPDRIVERAPI __stdcall

MExternC VOID UTHEAPDRIVERAPI UtInitMemPoolTag(IN ULONG Tag);

MExternC PVOID UTHEAPDRIVERAPI UtAllocPagedMem(IN SIZE_T NumberOfBytes);
MExternC PVOID UTHEAPDRIVERAPI UtAllocNonPagedMem(IN SIZE_T NumberOfBytes);

MExternC VOID UTHEAPDRIVERAPI UtFreePagedMem(IN PVOID pvMem);
MExternC VOID UTHEAPDRIVERAPI UtFreeNonPagedMem(IN PVOID pvMem);

#endif /* !defined(UtHeapDriver_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
