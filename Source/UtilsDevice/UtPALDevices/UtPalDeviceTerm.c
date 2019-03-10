/***************************************************************************/
/*  Copyright (C) 2007-2012 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <assert.h>

#include <UtilsDevice/UtPepDevices.h>

#include "UtPalDeviceAdapter.h"
#include "UtPalDeviceTerm.h"
#include "UtPalDevicePin.h"

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPAL.h>
#include <UtilsDevice/UtPALDefs.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#define CEquationSep L" & "
#define CEndOfLine L"\r\n"

#define COutputEnableVcc L"Vcc"

static BOOL lIsPinDataOn(
  UINT nPin,
  ULONG ulData)
{
    static UINT nPins[] = {11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    ULONG ulIndex, ulTmpData;

    for (ulIndex = 0; ulIndex < MArrayLen(nPins); ++ulIndex)
    {
        if (nPins[ulIndex] == nPin)
        {
            ulTmpData = 1 << ulIndex;

            return ((ulData & ulTmpData) == ulTmpData ? TRUE : FALSE);
        }
    }

    assert(0);

    return FALSE;
}

static BOOL lIsPinOutputActive(
  UINT nPin,
  ULONG ulActiveData)
{
    static UINT nPins[] = {11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    ULONG ulIndex, ulTmpActiveData;

    for (ulIndex = 0; ulIndex < MArrayLen(nPins); ++ulIndex)
    {
        if (nPins[ulIndex] == nPin)
        {
            ulTmpActiveData = 1 << ulIndex;

            return ((ulActiveData & ulTmpActiveData) == ulTmpActiveData ? TRUE : FALSE);
        }
    }

    assert(0);

    return FALSE;
}

LPWSTR* UtPALDeviceAllocOutputEnableTerms(
  const ULONG* pulInputData,
  const TPALDeviceAdapterData* pOutputData,
  ULONG ulOutputDataLen,
  UINT* pnInputPins,
  ULONG ulInputPinsLen,
  UINT nOutputPin)
{
    LPWSTR pszTerm;
    LPWSTR* ppszTerms;
    ULONG ulTotalTerms, ulIndex, ulInputIndex, ulTermLen, ulTmpIndex;

    ppszTerms = (LPWSTR*)UtAllocMem(sizeof(LPWSTR*));
    ulTotalTerms = 0;

    for (ulIndex = 0; ulIndex < ulOutputDataLen; ++ulIndex)
    {
        if (lIsPinOutputActive(nOutputPin, pOutputData[ulIndex].wPinActiveData))
        {
            ulTermLen = ulInputPinsLen + 1;
            pszTerm = (LPWSTR)UtAllocMem(sizeof(WCHAR) * ulTermLen);

            ppszTerms = (LPWSTR*)UtReAllocMem(ppszTerms, sizeof(LPWSTR*) * (ulTotalTerms + 2));

            ppszTerms[ulTotalTerms] = pszTerm;

            ++ulTotalTerms;

            *pszTerm = 0;

            for (ulInputIndex = 0; ulInputIndex < ulInputPinsLen; ++ulInputIndex)
            {
                ulTmpIndex = 1 << UtPALDeviceGetDevicePinIndex(pnInputPins[ulInputIndex]);

                StringCchCatW(pszTerm, ulTermLen,
                              (pulInputData[ulIndex] & ulTmpIndex) ? L"1" : L"0");
            }
        }
    }

    ppszTerms[ulTotalTerms] = NULL;

    return ppszTerms;
}

VOID UtPALDeviceFreeOutputEnableTerms(
  LPWSTR* ppszTerms)
{
    ULONG ulIndex;

    for (ulIndex = 0; ppszTerms[ulIndex] != NULL; ++ulIndex)
    {
        UtFreeMem(ppszTerms[ulIndex]);
    }

    UtFreeMem(ppszTerms);
}

LPWSTR* UtPALDeviceAllocDataTerms(
  const ULONG* pulInputData,
  const TPALDeviceAdapterData* pOutputData,
  ULONG ulOutputDataLen,
  UINT* pnInputPins,
  ULONG ulInputPinsLen,
  UINT nOutputPin,
  BOOL bPinDataOn)
{
    LPWSTR pszTerm;
    LPWSTR* ppszTerms;
    ULONG ulTotalTerms, ulIndex, ulInputIndex, ulTermLen, ulTmpIndex;

    ppszTerms = (LPWSTR*)UtAllocMem(sizeof(LPWSTR*));
    ulTotalTerms = 0;

    for (ulIndex = 0; ulIndex < ulOutputDataLen; ++ulIndex)
    {
        if (bPinDataOn == lIsPinDataOn(nOutputPin, pOutputData[ulIndex].wOutputData))
        {
            ulTermLen = ulInputPinsLen + 1;
            pszTerm = (LPWSTR)UtAllocMem(sizeof(WCHAR) * ulTermLen);

            ppszTerms = (LPWSTR*)UtReAllocMem(ppszTerms, sizeof(LPWSTR*) * (ulTotalTerms + 2));

            ppszTerms[ulTotalTerms] = pszTerm;

            ++ulTotalTerms;

            *pszTerm = 0;

            for (ulInputIndex = 0; ulInputIndex < ulInputPinsLen; ++ulInputIndex)
            {
                ulTmpIndex = 1 << UtPALDeviceGetDevicePinIndex(pnInputPins[ulInputIndex]);

                StringCchCatW(pszTerm, ulTermLen,
                              (pulInputData[ulIndex] & ulTmpIndex) ? L"1" : L"0");
            }
        }
    }

    ppszTerms[ulTotalTerms] = NULL;

    return ppszTerms;
}

VOID UtPALDeviceFreeDataTerms(
  LPWSTR* ppszTerms)
{
    ULONG ulIndex;

    for (ulIndex = 0; ppszTerms[ulIndex] != NULL; ++ulIndex)
    {
        UtFreeMem(ppszTerms[ulIndex]);
    }

    UtFreeMem(ppszTerms);
}

LPWSTR UtPALDeviceAllocMinimizedTermsToEquations(
  UINT* pnInputPins,
  UINT nInputPinsLen,
  LPCWSTR* ppszMinimizedTerms)
{
    ULONG ulIndex, ulInputPinIndex;
    WCHAR cPinNum[10];
    BOOL bTermAdded;
    LPWSTR pszEquations;
    ULONG ulEquationsLen;

    pszEquations = (LPWSTR)UtAllocMem(sizeof(WCHAR));
    ulEquationsLen = 1;

    *pszEquations = 0;

    for (ulIndex = 0; ppszMinimizedTerms[ulIndex] != NULL; ++ulIndex)
    {
        bTermAdded = FALSE;

        for (ulInputPinIndex = 0; ulInputPinIndex < nInputPinsLen;
             ++ulInputPinIndex)
        {
            switch (ppszMinimizedTerms[ulIndex][ulInputPinIndex])
            {
                case CPALTermLow:
                    StringCchPrintfW(cPinNum, MArrayLen(cPinNum), L"~%d",
                                     pnInputPins[ulInputPinIndex]);
                    break;
                case CPALTermHigh:
                    StringCchPrintfW(cPinNum, MArrayLen(cPinNum), L"%d",
                                     pnInputPins[ulInputPinIndex]);
                    break;
                case CPALTermUnused:
                    cPinNum[0] = 0;
                    break;
                default:
                    assert(0);
                    cPinNum[0] = 0;
                    break;
            }

            if (cPinNum[0] != 0)
            {
                if (bTermAdded)
                {
                    ulEquationsLen += lstrlenW(CEquationSep);

                    pszEquations = (LPWSTR)UtReAllocMem(pszEquations,
                                                        ulEquationsLen * sizeof(WCHAR));

                    StringCchCatW(pszEquations, ulEquationsLen, CEquationSep);
                }

                ulEquationsLen += lstrlenW(cPinNum);

                pszEquations = (LPWSTR)UtReAllocMem(pszEquations,
                                                    ulEquationsLen * sizeof(WCHAR));

                StringCchCatW(pszEquations, ulEquationsLen, cPinNum);

                bTermAdded = TRUE;
            }
        }

        ulEquationsLen += lstrlenW(CEndOfLine);

        pszEquations = (LPWSTR)UtReAllocMem(pszEquations,
                                            ulEquationsLen * sizeof(WCHAR));

        StringCchCatW(pszEquations, ulEquationsLen, CEndOfLine);
    }

    return pszEquations;
}

LPWSTR UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(
  UINT* pnInputPins,
  UINT nInputPinsLen,
  LPCWSTR* ppszMinimizedTerms,
  UINT nOutputPin,
  BOOL bOutputPinHigh)
{
    ULONG ulIndex, ulInputPinIndex;
    WCHAR cPinNum[10], cOutputPinNum[10];
    LPWSTR pszEquations;
    ULONG ulEquationsLen;

    pszEquations = (LPWSTR)UtAllocMem(sizeof(WCHAR));
    ulEquationsLen = 1;

    *pszEquations = 0;

    if (bOutputPinHigh)
    {
        StringCchPrintfW(cOutputPinNum, MArrayLen(cOutputPinNum), L"%d",
                         nOutputPin);
    }
    else
    {
        StringCchPrintfW(cOutputPinNum, MArrayLen(cOutputPinNum), L"~%d",
                         nOutputPin);
    }

    for (ulIndex = 0; ppszMinimizedTerms[ulIndex] != NULL; ++ulIndex)
    {
        ulEquationsLen += lstrlenW(cOutputPinNum);

        pszEquations = (LPWSTR)UtReAllocMem(pszEquations,
                                            ulEquationsLen * sizeof(WCHAR));

        StringCchCatW(pszEquations, ulEquationsLen, cOutputPinNum);

        for (ulInputPinIndex = 0; ulInputPinIndex < nInputPinsLen;
             ++ulInputPinIndex)
        {
            switch (ppszMinimizedTerms[ulIndex][ulInputPinIndex])
            {
                case CPALTermLow:
                    StringCchPrintfW(cPinNum, MArrayLen(cPinNum), L"~%d",
                                     pnInputPins[ulInputPinIndex]);
                    break;
                case CPALTermHigh:
                    StringCchPrintfW(cPinNum, MArrayLen(cPinNum), L"%d",
                                     pnInputPins[ulInputPinIndex]);
                    break;
                case CPALTermUnused:
                    cPinNum[0] = 0;
                    break;
                default:
                    assert(0);
                    cPinNum[0] = 0;
                    break;
            }

            if (cPinNum[0] != 0)
            {
                ulEquationsLen += lstrlenW(CEquationSep);

                pszEquations = (LPWSTR)UtReAllocMem(pszEquations,
                                                    ulEquationsLen * sizeof(WCHAR));

                StringCchCatW(pszEquations, ulEquationsLen, CEquationSep);

                ulEquationsLen += lstrlenW(cPinNum);

                pszEquations = (LPWSTR)UtReAllocMem(pszEquations,
                                                    ulEquationsLen * sizeof(WCHAR));

                StringCchCatW(pszEquations, ulEquationsLen, cPinNum);
            }
        }

        ulEquationsLen += lstrlenW(CEndOfLine);

        pszEquations = (LPWSTR)UtReAllocMem(pszEquations,
                                            ulEquationsLen * sizeof(WCHAR));

        StringCchCatW(pszEquations, ulEquationsLen, CEndOfLine);
    }

    return pszEquations;
}

LPWSTR UtPALDeviceAllocOutputEnableAlwaysOnEquation()
{
    LPWSTR pszEquation;
    INT nEquationLen;

    nEquationLen = lstrlenW(COutputEnableVcc) + 1;

    pszEquation = (LPWSTR)UtAllocMem(nEquationLen * sizeof(WCHAR));

    StringCchCopyW(pszEquation, nEquationLen, COutputEnableVcc);

    return pszEquation;
}

ULONG UtPALDeviceGetTotalTerms(
  LPCWSTR* ppszTerms)
{
    ULONG ulTotal = 0;
    ULONG ulIndex;

    for (ulIndex = 0; ppszTerms[ulIndex] != NULL; ++ulIndex)
    {
        ++ulTotal;
    }

    return ulTotal;
}

BOOL UtPALDeviceTranslateMinimizedOutputEnableTerms(
  UINT* pnInputPins,
  UINT nInputPinsCount,
  UINT nOutputPin,
  LPWSTR* ppszMinimizedTerms,
  const TPALData* pPALData,
  LPBYTE pbyFuseData,
  ULONG ulFuseDataLen)
{
    ULONG ulTotalTerms, ulFuseRow, ulFuseColumn, ulInputPinIndex;
    UINT nInputPin;

    if (FALSE == UtPALIsOutputEnablePresent(nOutputPin, pPALData))
    {
        return FALSE;
    }

    ulTotalTerms = UtPALDeviceGetTotalTerms(ppszMinimizedTerms);

    if (ulTotalTerms > 1)
    {
        return FALSE;
    }

    ulFuseRow = UtPALOutputEnableFuseRow(nOutputPin, pPALData);

    if (ulTotalTerms == 0)
    {
        UtPALRestoreFuseRow(ulFuseRow, pPALData, pbyFuseData, ulFuseDataLen);

        return TRUE;
    }

    for (ulInputPinIndex = 0; ulInputPinIndex < nInputPinsCount;
         ++ulInputPinIndex)
    {
        nInputPin = pnInputPins[ulInputPinIndex];

        switch (ppszMinimizedTerms[0][ulInputPinIndex])
        {
            case CPALTermLow:
                ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, TRUE,
                                                      pPALData);

                UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                              pbyFuseData, ulFuseDataLen);
                break;
            case CPALTermHigh:
                ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, FALSE,
                                                      pPALData);

                UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                              pbyFuseData, ulFuseDataLen);
                break;
            case CPALTermUnused:
                ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, TRUE,
                                                      pPALData);

                UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                              pbyFuseData, ulFuseDataLen);

                ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, FALSE,
                                                      pPALData);

                UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                              pbyFuseData, ulFuseDataLen);
                break;
            default:
                assert(0);
                break;
        }
    }

    return TRUE;
}

BOOL UtPALDeviceTranslateMinimizedOutputEnableTermsWithOutputPin(
  UINT* pnInputPins,
  UINT nInputPinsCount,
  UINT nOutputPin,
  LPWSTR* ppszMinimizedTerms,
  UINT nMergeOutputPin,
  const TPALData* pPALData,
  LPBYTE pbyFuseData,
  ULONG ulFuseDataLen)
{
    ULONG ulFuseRow, ulFuseColumn;

    if (!UtPALDeviceTranslateMinimizedOutputEnableTerms(pnInputPins, nInputPinsCount,
                                                        nOutputPin, ppszMinimizedTerms,
                                                        pPALData, pbyFuseData, ulFuseDataLen))
    {
        return FALSE;
    }

    ulFuseRow = UtPALOutputEnableFuseRow(nOutputPin, pPALData);
    ulFuseColumn = UtPALFuseColumnFromPin(nMergeOutputPin, FALSE, pPALData);

    UtPALRestoreFuse(ulFuseRow, ulFuseColumn, pPALData, pbyFuseData,
                     ulFuseDataLen);

    return TRUE;
}

BOOL UtPALDeviceTranslateMinimizedDataTerms(
  UINT* pnInputPins,
  UINT nInputPinsCount,
  UINT nOutputPin,
  LPWSTR* ppszMinimizedTerms,
  const TPALData* pPALData,
  LPBYTE pbyFuseData,
  ULONG ulFuseDataLen)
{
    ULONG ulInputPinIndex, ulTotalTerms, ulTermIndex, ulPinTermCount;
    ULONG ulFuseRow, ulFuseColumn;
    UINT nInputPin;

    ulTotalTerms = UtPALDeviceGetTotalTerms(ppszMinimizedTerms);
    ulPinTermCount = UtPALTermCount(nOutputPin, pPALData);

    if (ulTotalTerms > ulPinTermCount)
    {
        return FALSE;
    }

    for (ulTermIndex = 0; ulTermIndex < ulTotalTerms; ++ulTermIndex)
    {
        ulFuseRow = UtPALTermRowToFuseRow(nOutputPin, ulTermIndex, pPALData);

        for (ulInputPinIndex = 0; ulInputPinIndex < nInputPinsCount;
             ++ulInputPinIndex)
        {
            nInputPin = pnInputPins[ulInputPinIndex];

            switch (ppszMinimizedTerms[ulTermIndex][ulInputPinIndex])
            {
                case CPALTermLow:
                    ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, TRUE,
                                                          pPALData);

                    UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                                  pbyFuseData, ulFuseDataLen);
                    break;
                case CPALTermHigh:
                    ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, FALSE,
                                                          pPALData);

                    UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                                  pbyFuseData, ulFuseDataLen);
                    break;
                case CPALTermUnused:
                    ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, TRUE,
                                                          pPALData);

                    UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                                  pbyFuseData, ulFuseDataLen);

                    ulFuseColumn = UtPALFuseColumnFromPin(nInputPin, FALSE,
                                                          pPALData);

                    UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                                  pbyFuseData, ulFuseDataLen);
                    break;
                default:
                    assert(0);
                    break;
            }
        }
    }

    /* Take any unused term rows and restore all fuses in them. */

    for (ulTermIndex = ulTotalTerms; ulTermIndex < ulPinTermCount; ++ulTermIndex)
    {
        ulFuseRow = UtPALTermRowToFuseRow(nOutputPin, ulTermIndex, pPALData);

        UtPALRestoreFuseRow(ulFuseRow, pPALData, pbyFuseData, ulFuseDataLen);
    }

    return TRUE;
}

BOOL UtPALDeviceTranslateMinimizedDataTermsWithOutputPin(
  UINT* pnInputPins,
  UINT nInputPinsCount,
  UINT nOutputPin,
  LPWSTR* ppszMinimizedTerms,
  UINT nMergeOutputPin,
  const TPALData* pPALData,
  LPBYTE pbyFuseData,
  ULONG ulFuseDataLen)
{
    ULONG ulTotalTerms, ulTermIndex;
    ULONG ulFuseRow, ulFuseColumn;

    ulTotalTerms = UtPALDeviceGetTotalTerms(ppszMinimizedTerms);

    if (!UtPALDeviceTranslateMinimizedDataTerms(pnInputPins, nInputPinsCount,
                                                nOutputPin, ppszMinimizedTerms,
                                                pPALData, pbyFuseData,
                                                ulFuseDataLen))
    {
        return FALSE;
    }

    if (!UtPALIsFuseColumnPresent(nMergeOutputPin, pPALData))
    {
        return FALSE;
    }

    ulFuseColumn = UtPALFuseColumnFromPin(nMergeOutputPin, FALSE, pPALData);

    for (ulTermIndex = 0; ulTermIndex < ulTotalTerms; ++ulTermIndex)
    {
        ulFuseRow = UtPALTermRowToFuseRow(nOutputPin, ulTermIndex, pPALData);

        UtPALRestoreFuse(ulFuseRow, ulFuseColumn, pPALData, pbyFuseData,
                         ulFuseDataLen);
    }

    return TRUE;
}

INT UtPALDeviceFindPinTermIndexUtilizedByEquation(
  LPCWSTR* ppszTerms,
  const TPALDevicePinTerms* pPinTerms,
  ULONG ulPinTermsLen)
{
    INT nIndex;

    for (nIndex = 0; (ULONG)nIndex < ulPinTermsLen; ++nIndex)
    {
        if (pPinTerms[nIndex].bOutputDone)
        {
            if (UtPALAreSimiliarTermsPresent(ppszTerms,
                                             pPinTerms[nIndex].ppszOutputTerms))
            {
                return nIndex;
            }
        }
    }

    return -1;
}

/***************************************************************************/
/*  Copyright (C) 2007-2012 Kevin Eshbach                                  */
/***************************************************************************/
