/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPALDefs.h>

static VOID lDumpTerms(
  LPCWSTR* ppszTerms)
{
#if !defined(NDEBUG)
    INT nIndex;

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        OutputDebugStringW(L"    ");
        OutputDebugStringW(ppszTerms[nIndex]);
        OutputDebugStringW(L"\n");
    }
#else
    ppszTerms;
#endif
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

static LPWSTR* lCopyTerms(
  LPCWSTR* ppszTerms)
{
    INT nTotalTerms = lGetTotalTerms(ppszTerms);
    INT nIndex, nTermLen;
    LPWSTR* ppszNewTerms;

    ppszNewTerms = (LPWSTR*)UtAllocMem((nTotalTerms + 1) * sizeof(LPWSTR));

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        nTermLen = lstrlenW(ppszTerms[nIndex]) + 1;

        ppszNewTerms[nIndex] = (LPWSTR)UtAllocMem(nTermLen * sizeof(WCHAR));

        StringCchCopy(ppszNewTerms[nIndex], nTermLen, ppszTerms[nIndex]);
    }

    ppszNewTerms[nIndex] = NULL;

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

static BOOL lIsExactTermPresent(
  LPCWSTR* ppszTerms,
  LPCWSTR pszTermMatch)
{
    INT nIndex;

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        if (lstrcmpW(ppszTerms[nIndex], pszTermMatch) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL lDoTermsMatch(
  LPCWSTR pszTerm,
  LPCWSTR pszTermMatch)
{
    INT nIndex;

    for (nIndex = 0; pszTerm[nIndex] != 0 && pszTermMatch[nIndex] != 0;
         ++nIndex)
    {
        if (pszTermMatch[nIndex] != CPALTermUnused)
        {
            if (pszTerm[nIndex] != pszTermMatch[nIndex])
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL lIsSimiliarTermPresent(
  LPCWSTR* ppszTerms,
  LPCWSTR pszTermMatch)
{
    INT nIndex;

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        if (lDoTermsMatch(ppszTerms[nIndex], pszTermMatch))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static LPWSTR* lRemoveDuplicateTerms(
  LPCWSTR* ppszTerms)
{
    LPWSTR* ppszNewTerms = (LPWSTR*)UtAllocMem(sizeof(LPWSTR));
    INT nTotalNewTerms = 1;
    INT nIndex, nTermLen;

    ppszNewTerms[0] = NULL;

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        if (lIsExactTermPresent((LPCWSTR*)ppszNewTerms, ppszTerms[nIndex]) == FALSE)
        {
            ppszNewTerms = (LPWSTR*)UtReAllocMem(ppszNewTerms,
                                                 (nTotalNewTerms + 1) *
                                                     sizeof(LPWSTR));

            nTermLen = lstrlenW(ppszTerms[nIndex]) + 1;

            ppszNewTerms[nTotalNewTerms - 1] = (LPWSTR)UtAllocMem(nTermLen * sizeof(WCHAR));

            StringCchCopyW(ppszNewTerms[nTotalNewTerms - 1], nTermLen,
                           ppszTerms[nIndex]);

            ppszNewTerms[nTotalNewTerms] = NULL;

            ++nTotalNewTerms;
        }
    }

    return ppszNewTerms;
}

static VOID lMergeSimiliarTermRow(
  LPCWSTR pszTermRHS,
  LPWSTR pszTermLHS)
{
    INT nIndex = 0;

    while (pszTermLHS[nIndex] != 0)
    {
        if (pszTermRHS[nIndex] != CPALTermUnused)
        {
            pszTermLHS[nIndex] = CPALTermUnused;
        }

        ++nIndex;
    }
}

BOOL AreSimiliarTermsPresent(
  LPCWSTR* ppszTerms,
  LPCWSTR* ppszTermsMatch)
{
    INT nIndex;

    for (nIndex = 0; ppszTermsMatch[nIndex] != NULL; ++nIndex)
    {
        if (lIsSimiliarTermPresent(ppszTerms, ppszTermsMatch[nIndex]) == FALSE)
        {
            return FALSE;
        }
    }

    return TRUE;
}

LPWSTR* AllocMergedTerms(
  LPCWSTR* ppszTerms,
  LPCWSTR* ppszTermsMatch)
{
    INT nIndex1, nIndex2;
    LPWSTR* ppszTmpTerms;
    LPWSTR* ppszFinalTerms;

    ppszTmpTerms = lCopyTerms(ppszTerms);

    for (nIndex1 = 0; ppszTermsMatch[nIndex1] != NULL; ++nIndex1)
    {
        for (nIndex2 = 0; ppszTmpTerms[nIndex2]; ++nIndex2)
        {
            if (lDoTermsMatch(ppszTmpTerms[nIndex2], ppszTermsMatch[nIndex1]))
            {
                lMergeSimiliarTermRow(ppszTermsMatch[nIndex1],
                                      ppszTmpTerms[nIndex2]);
            }
        }
    }

    ppszFinalTerms = lRemoveDuplicateTerms((LPCWSTR*)ppszTmpTerms);

    lFreeTerms(ppszTmpTerms);

    return ppszFinalTerms;
}

VOID FreeMergedTerms(
  LPWSTR* ppszTerms)
{
    lFreeTerms(ppszTerms);
}

/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/
