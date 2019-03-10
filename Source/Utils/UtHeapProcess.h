/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtHeapProcess_H)
#define UtHeapProcess_H

#include <Includes/UtExternC.h>

#define UTHEAPPROCESSAPI __stdcall

MExternC LPVOID UTHEAPPROCESSAPI UtAllocMem(DWORD dwLen);

MExternC LPVOID UTHEAPPROCESSAPI UtReAllocMem(LPVOID pvMem, DWORD dwLen);

MExternC VOID UTHEAPPROCESSAPI UtFreeMem(LPVOID pvMem);

#endif /* end of UtHeapProcess_H */

/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/
