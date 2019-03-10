/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <Utils/UtHeapProcess.h>

LPVOID UTHEAPPROCESSAPI UtAllocMem(
  DWORD dwLen)
{
    return HeapAlloc(GetProcessHeap(), 0, dwLen);
}

LPVOID UTHEAPPROCESSAPI UtReAllocMem(
  LPVOID pvMem,
  DWORD dwLen)
{
    return HeapReAlloc(GetProcessHeap(), 0, pvMem, dwLen);
}

VOID UTHEAPPROCESSAPI UtFreeMem(
  LPVOID pvMem)
{
    HeapFree(GetProcessHeap(), 0, pvMem);
}

/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/
