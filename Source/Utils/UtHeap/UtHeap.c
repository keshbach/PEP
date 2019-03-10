/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <Utils/UtHeap.h>

static HANDLE l_hHeap = NULL;

BOOL UTHEAPAPI UtInitHeap(VOID)
{
    if (l_hHeap == NULL)
    {
        l_hHeap = HeapCreate(0, 0, 0);

        if (l_hHeap)
        {
            HeapSetInformation(l_hHeap, HeapEnableTerminationOnCorruption, NULL, 0);

            return TRUE;
        }
    }

    return FALSE;
}

BOOL UTHEAPAPI UtUninitHeap(VOID)
{
    BOOL bResult = FALSE;

    if (l_hHeap)
    {
        bResult = HeapDestroy(l_hHeap);

        l_hHeap = NULL;
    }

    return bResult;
}

LPVOID UTHEAPAPI UtAllocMem(
  DWORD dwLen)
{
    return HeapAlloc(l_hHeap, 0, dwLen);
}

LPVOID UTHEAPAPI UtReAllocMem(
  LPVOID pvMem,
  DWORD dwLen)
{
    return HeapReAlloc(l_hHeap, 0, pvMem, dwLen);
}

VOID UTHEAPAPI UtFreeMem(
  LPVOID pvMem)
{
    HeapFree(l_hHeap, 0, pvMem);
}

/***************************************************************************/
/*  Copyright (C) 2008-2012 Kevin Eshbach                                  */
/***************************************************************************/
