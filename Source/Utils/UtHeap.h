/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtHeap_H)
#define UtHeap_H

#include <Includes/UtExternC.h>

#define UTHEAPAPI __stdcall

MExternC BOOL UTHEAPAPI UtInitHeap(VOID);

MExternC BOOL UTHEAPAPI UtUninitHeap(VOID);

MExternC LPVOID UTHEAPAPI UtAllocMem(DWORD dwLen);

MExternC LPVOID UTHEAPAPI UtReAllocMem(LPVOID pvMem, DWORD dwLen);

MExternC VOID UTHEAPAPI UtFreeMem(LPVOID pvMem);

#endif /* end of UtHeap_H */

/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/
