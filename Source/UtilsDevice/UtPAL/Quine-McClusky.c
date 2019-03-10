/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPALDefs.h>

typedef struct tagTMinTermData
{
    LPWSTR pszTerm;
    BOOL bUsed;
} TMinTermData;

static LPWSTR lAllocDataWithInvertedBit(
  LPCWSTR pszData,
  INT nDataBitPos)
{
    INT nNewDataLen = lstrlenW(pszData) + 1;
    LPWSTR pszNewData;

    pszNewData = (LPWSTR)UtAllocMem(sizeof(WCHAR) * nNewDataLen);

    StringCchCopyW(pszNewData, nNewDataLen, pszData);

    if (pszNewData[nDataBitPos] == CPALTermLow)
    {
        pszNewData[nDataBitPos] = CPALTermHigh;
    }
    else
    {
        pszNewData[nDataBitPos] = CPALTermLow;
    }

    return pszNewData;
}

static INT lGetTotalTerms(
  LPCWSTR* ppszTerms)
{
    INT nTotalTerms = 0;

    while (ppszTerms[nTotalTerms] != NULL)
    {
        ++nTotalTerms;
    }

    return nTotalTerms;
}

static INT lFindMatchingTerm(
  LPCWSTR pszSearchTerm,
  INT nDataStart,
  LPCWSTR* ppszTerms,
  INT nDataLen)
{
    INT nIndex;

    for (nIndex = nDataStart; nIndex < nDataLen; ++nIndex)
    {
        if (lstrcmpW(pszSearchTerm, ppszTerms[nIndex]) == 0)
        {
            return nIndex;
        }
    }

    return -1;
}

static BOOL lAreTermsSameLength(
  LPCWSTR* ppszTerms)
{
    INT nTermIndex = 1;
    INT nTermLen;

    if (ppszTerms[0])
    {
        nTermLen = lstrlenW(ppszTerms[0]);

        while (ppszTerms[nTermIndex] != NULL)
        {
            if (lstrlenW(ppszTerms[nTermIndex]) != nTermLen)
            {
                return FALSE;
            }

            ++nTermIndex;
        }

        return TRUE;
    }

    return FALSE;
}

static LPWSTR* lCopyTerms(
  LPCWSTR* ppszTerms)
{
    INT nTotalTerms = lGetTotalTerms(ppszTerms) + 1;
    INT nIndex, nTermLen;
    LPWSTR* ppszNewTerms;

    ppszNewTerms = (LPWSTR*)UtAllocMem(nTotalTerms * sizeof(LPWSTR*));

    nTermLen = lstrlenW(ppszTerms[0]) + 1;

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        ppszNewTerms[nIndex] = (LPWSTR)UtAllocMem(nTermLen * sizeof(WCHAR));

        StringCchCopyW(ppszNewTerms[nIndex], nTermLen, ppszTerms[nIndex]);
    }

    ppszNewTerms[nTotalTerms - 1] = NULL;

    return ppszNewTerms;
}

static VOID lFreeTerms(
  LPWSTR* ppszTerms)
{
    INT nIndex;

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        UtFreeMem(ppszTerms[nIndex]);
    }

    UtFreeMem(ppszTerms);
}

static VOID lRemoveDuplicateTerms(
  LPWSTR* ppszTerms,
  INT nTotalTerms)
{
    INT nIndex = 0;
    INT nNewTotalTerms = nTotalTerms;
    INT nIndexMatch;

    while (ppszTerms[nIndex] != NULL)
    {
        nIndexMatch = lFindMatchingTerm(ppszTerms[nIndex], nIndex + 1,
                                        (LPCWSTR*)ppszTerms, nTotalTerms);

        if (nIndexMatch == -1)
        {
            ++nIndex;
        }
        else
        {
            UtFreeMem(ppszTerms[nIndexMatch]);

            MoveMemory(ppszTerms + nIndexMatch, ppszTerms + nIndexMatch + 1,
                       sizeof(LPWSTR) * (nNewTotalTerms - nIndexMatch));

            --nNewTotalTerms;
        }
    }
}

static VOID lProcessTerms(
  LPCWSTR* ppszTerms,
  LPWSTR** pppszMatchedTerms,
  LPWSTR** pppszUnmatchedTerms)
{
    TMinTermData** ppMinTermData;
    LPWSTR pszInvertedDataBit;
    INT nIndex, nDataBitIndex, nTermTotal, nMatchedTermTotal, nUnmatchedTermTotal;
    INT nDataLen, nMatchIndex;

    nTermTotal = 0;
    nMatchedTermTotal = 0;
    nUnmatchedTermTotal = 0;

    *pppszMatchedTerms = (LPWSTR*)UtAllocMem(sizeof(LPWSTR*));
    *pppszUnmatchedTerms = (LPWSTR*)UtAllocMem(sizeof(LPWSTR*));

    nTermTotal = lGetTotalTerms(ppszTerms);
    nDataLen = lstrlenW(ppszTerms[0]) + 1;

    ppMinTermData = (TMinTermData**)UtAllocMem(sizeof(TMinTermData**) * nTermTotal);

    for (nIndex = 0; nIndex < nTermTotal; ++nIndex)
    {
        ppMinTermData[nIndex] = (TMinTermData*)UtAllocMem(sizeof(TMinTermData));

        ppMinTermData[nIndex]->pszTerm = (LPWSTR)UtAllocMem(sizeof(WCHAR) * nDataLen);

        StringCchCopyW(ppMinTermData[nIndex]->pszTerm, nDataLen, ppszTerms[nIndex]);

        ppMinTermData[nIndex]->bUsed = FALSE;
    }

    for (nIndex = 0; nIndex < nTermTotal - 1; ++nIndex)
    {
        for (nDataBitIndex = 0; nDataBitIndex < nDataLen; ++nDataBitIndex)
        {
            if (ppszTerms[nIndex][nDataBitIndex] != CPALTermUnused)
            {
                pszInvertedDataBit = lAllocDataWithInvertedBit(ppszTerms[nIndex],
                                                               nDataBitIndex);

                nMatchIndex = lFindMatchingTerm(pszInvertedDataBit, nIndex + 1,
                                                ppszTerms, nTermTotal);

                if (nMatchIndex != -1)
                {
                    ppMinTermData[nIndex]->bUsed = TRUE;
                    ppMinTermData[nMatchIndex]->bUsed = TRUE;

                    *pppszMatchedTerms = (LPWSTR*)UtReAllocMem(*pppszMatchedTerms,
                                             sizeof(LPWSTR*) * (nMatchedTermTotal + 2));

                    *(pszInvertedDataBit + nDataBitIndex) = CPALTermUnused;

                    (*pppszMatchedTerms)[nMatchedTermTotal] = pszInvertedDataBit;
                        
                    ++nMatchedTermTotal;
                }
                else
                {
                    UtFreeMem(pszInvertedDataBit);
                }
            }
        }
    }

    for (nIndex = 0; nIndex < nTermTotal; ++nIndex)
    {
        if (ppMinTermData[nIndex]->bUsed == FALSE)
        {
            nDataLen = lstrlenW(ppMinTermData[nIndex]->pszTerm) + 1;

            *pppszUnmatchedTerms = (LPWSTR*)UtReAllocMem(*pppszUnmatchedTerms,
                                                sizeof(LPWSTR*) * (nUnmatchedTermTotal + 2));

            (*pppszUnmatchedTerms)[nUnmatchedTermTotal] = (LPWSTR)UtAllocMem(sizeof(WCHAR) * nDataLen);
                        
            StringCchCopyW((*pppszUnmatchedTerms)[nUnmatchedTermTotal], nDataLen,
                           ppMinTermData[nIndex]->pszTerm);

            ++nUnmatchedTermTotal;
        }
    }

    (*pppszMatchedTerms)[nMatchedTermTotal] = NULL;
    (*pppszUnmatchedTerms)[nUnmatchedTermTotal] = NULL;

    lRemoveDuplicateTerms(*pppszMatchedTerms, nMatchedTermTotal);
    lRemoveDuplicateTerms(*pppszUnmatchedTerms, nUnmatchedTermTotal);

    for (nIndex = 0; nIndex < nTermTotal; ++nIndex)
    {
        UtFreeMem(ppMinTermData[nIndex]->pszTerm);
        UtFreeMem(ppMinTermData[nIndex]);
    }

    UtFreeMem(ppMinTermData);
}

LPWSTR* QM_AllocMinimizedTerms(
  LPCWSTR* ppszTerms)
{
    BOOL bQuit;
    INT nIndex, nTotalFinalTerms;
    LPWSTR* ppszWorkingTerms;
    LPWSTR* ppszMatchedTerms;
    LPWSTR* ppszUnmatchedTerms;
    LPWSTR* ppszFinalTerms;

    if (!lAreTermsSameLength(ppszTerms))
    {
        return NULL;
    }

    bQuit = FALSE;
    nTotalFinalTerms = 0;
    ppszFinalTerms = (LPWSTR*)UtAllocMem(sizeof(LPWSTR*));

    ppszWorkingTerms = lCopyTerms(ppszTerms);

    while (bQuit == FALSE)
    {
        lProcessTerms((LPCWSTR*)ppszWorkingTerms, &ppszMatchedTerms,
                      &ppszUnmatchedTerms);

        ppszFinalTerms = (LPWSTR*)UtReAllocMem(ppszFinalTerms,
                                               (nTotalFinalTerms + 1 +
                                                   lGetTotalTerms((LPCWSTR*)ppszUnmatchedTerms)) *
                                                   sizeof(LPWSTR*));

        for (nIndex = 0; ppszUnmatchedTerms[nIndex] != NULL; ++nIndex)
        {
            ppszFinalTerms[nTotalFinalTerms] = ppszUnmatchedTerms[nIndex];

            ++nTotalFinalTerms;
        }

        ppszFinalTerms[nTotalFinalTerms] = NULL;

        UtFreeMem(ppszUnmatchedTerms);

        lFreeTerms(ppszWorkingTerms);

        if (ppszMatchedTerms[0] != NULL)
        {
            ppszWorkingTerms = ppszMatchedTerms;
        }
        else
        {
            lFreeTerms(ppszMatchedTerms);

            bQuit = TRUE;
        }
    }

    return ppszFinalTerms;
}

VOID QM_FreeMinimizedTerms(
  LPWSTR* ppszTerms)
{
    lFreeTerms(ppszTerms);
}

/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/
