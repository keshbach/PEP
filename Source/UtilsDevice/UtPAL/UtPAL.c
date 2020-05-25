/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <UtilsDevice/UtPepDevices.h>

#include <UtilsDevice/UtPAL.h>
#include <UtilsDevice/UtPALDefs.h>

#include <UtilsDevice/UtEspresso.h>

#include <Utils/UtHeap.h>

#include <Includes/UtMacros.h>

#include "Quine-McClusky.h"
#include "MatchTerms.h"

#pragma region "Local Functions"

static ULONG lCalcFuseColumnCount(
  const TPALData* pPALData)
{
    return pPALData->nDevicePinFuseColumnsCount * 2;
}

static ULONG lCalcFuseCount(
  const TPALData* pPALData)
{
    ULONG ulFuseCount, ulFuseColumnCount, ulIndex;

    ulFuseCount = 0;
    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);

    for (ulIndex = 0; ulIndex < pPALData->nDevicePinFuseRowsCount; ++ulIndex)
    {
        if (pPALData->pDevicePinFuseRows[ulIndex].nFuseRowOutputEnable != CNoOutputEnableFuseRow)
        {
            if (pPALData->pDevicePinFuseRows[ulIndex].nFuseRowOutputEnable > ulFuseCount)
            {
                ulFuseCount = pPALData->pDevicePinFuseRows[ulIndex].nFuseRowOutputEnable;
            }
        }

        if (pPALData->pDevicePinFuseRows[ulIndex].nFuseRowTermEnd > ulFuseCount)
        {
            ulFuseCount = pPALData->pDevicePinFuseRows[ulIndex].nFuseRowTermEnd;
        }
    }

    ulFuseCount += ulFuseColumnCount;

    return ulFuseCount;
}

static ULONG lCalcFuseRowCount(
  const TPALData* pPALData)
{
    return lCalcFuseCount(pPALData) / lCalcFuseColumnCount(pPALData);
}

static BOOL lSetFuseValue(
  ULONG nFuseRow,
  ULONG nFuseColumn,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BYTE byFuseData)
{
    ULONG ulFuseSize, nFuseRowIndex, ulFuseColumnCount;

    if (FALSE == UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        ulDataLen != ulFuseSize)
    {
        return FALSE;
    }

    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);

    for (nFuseRowIndex = 0; nFuseRowIndex < nFuseRow; ++nFuseRowIndex)
    {
        pbyData += ulFuseColumnCount;
    }

    pbyData += nFuseColumn;

    *pbyData = byFuseData;

    return TRUE;
}

static BOOL lSetFuseRowValue(
  ULONG nFuseRow,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BYTE byFuseData)
{
    ULONG ulFuseSize, nFuseRowIndex, nFuseColumnIndex, ulFuseColumnCount;

    if (nFuseRow >= lCalcFuseRowCount(pPALData) ||
        FALSE == UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        ulDataLen != ulFuseSize)
    {
        return FALSE;
    }

    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);

    for (nFuseRowIndex = 0; nFuseRowIndex < nFuseRow; ++nFuseRowIndex)
    {
        pbyData += ulFuseColumnCount;
    }

    for (nFuseColumnIndex = 0; nFuseColumnIndex < ulFuseColumnCount;
         ++nFuseColumnIndex)
    {
        *(pbyData + nFuseColumnIndex) = byFuseData;
    }

    return TRUE;
}

static BOOL lWriteFileEndOfLine(
  HANDLE hFile)
{
    static CHAR cEndOfLine[] = {0x0d, 0x0a};
    DWORD dwBytesWritten;

    return WriteFile(hFile, cEndOfLine, MArrayLen(cEndOfLine), &dwBytesWritten, NULL);
}

static BOOL lWriteFileStartOfText(
  HANDLE hFile)
{
    static CHAR cStartOfText[] = {0x02};
    BOOL bResult = FALSE;
    DWORD dwBytesWritten;

    if (TRUE == WriteFile(hFile, cStartOfText, MArrayLen(cStartOfText),
                          &dwBytesWritten, NULL) &&
        TRUE == lWriteFileEndOfLine(hFile))
    {
        bResult = TRUE;
    }

    return bResult;
}

static BOOL lWriteFileEndOfText(
  HANDLE hFile)
{
    static CHAR cEndOfText[] = {0x03};
    BOOL bResult = FALSE;
    DWORD dwBytesWritten;

    if (TRUE == WriteFile(hFile, cEndOfText, MArrayLen(cEndOfText),
                          &dwBytesWritten, NULL) &&
        TRUE == lWriteFileEndOfLine(hFile))
    {
        bResult = TRUE;
    }

    return bResult;
}

static BOOL lWriteFilePALType(
  HANDLE hFile,
  LPCWSTR pszPALType)
{
    BOOL bResult = FALSE;
    INT nBufferLen;
    LPSTR pszBuffer;
    DWORD dwBytesWritten;

    nBufferLen = WideCharToMultiByte(CP_ACP, 0, pszPALType, -1, NULL, 0, NULL, NULL);

    if (nBufferLen <= 0)
    {
        return FALSE;
    }

    pszBuffer = (LPSTR)UtAllocMem(nBufferLen);

    if (pszBuffer == NULL)
    {
        return FALSE;
    }

    WideCharToMultiByte(CP_ACP, 0, pszPALType, -1, pszBuffer, nBufferLen,
                        NULL, NULL);

    if (TRUE == WriteFile(hFile, pszBuffer, nBufferLen - 1,
                          &dwBytesWritten, NULL) &&
        TRUE == lWriteFileEndOfLine(hFile))
    {
        bResult = TRUE;
    }

    UtFreeMem(pszBuffer);

    return bResult;
}

static BOOL lWriteFilePALDeviceData(
  HANDLE hFile,
  const TPALData* pPALData,
  UINT nPinCount)
{
    BOOL bResult = FALSE;
    CHAR cBuffer[100];
    ULONG ulFuseSize;
    DWORD dwBytesWritten;

    if (FALSE == UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        FAILED(StringCchPrintfA(cBuffer, MArrayLen(cBuffer),
                                "QP%d* QF%d*", nPinCount, ulFuseSize)))
    {
        return FALSE;
    }

    if (TRUE == WriteFile(hFile, cBuffer, lstrlenA(cBuffer),
                          &dwBytesWritten, NULL) &&
        TRUE == lWriteFileEndOfLine(hFile))
    {
        bResult = TRUE;
    }

    return bResult;
}

static BOOL lWriteFilePALFuseMap(
  HANDLE hFile,
  const TPALData* pPALData,
  LPBYTE pbyData)
{
    CHAR cLineNum[100], cLineNumFormat[20];
    ULONG ulFuseRowIndex;
    ULONG ulFuseCount, ulFuseColumnCount, ulFuseRowCount, ulFuseNumber;
    DWORD dwBytesWritten;
    BOOL bResult;
    LPCWSTR pszFuseMapText;
    LPSTR pszTmpFuseMapText;
    INT nTmpFuseMapTextLen;

    bResult = FALSE;

    if (pPALData->pAllocFuseMapTextFunc == NULL &&
        pPALData->pFreeFuseMapTextFunc == NULL)
    {
        ulFuseCount = lCalcFuseCount(pPALData);
        ulFuseColumnCount = lCalcFuseColumnCount(pPALData);
        ulFuseRowCount = lCalcFuseRowCount(pPALData);

        if (FAILED(StringCchPrintfA(cLineNumFormat, MArrayLen(cLineNumFormat),
                                    "%d", ulFuseCount)) ||
            FAILED(StringCchPrintfA(cLineNumFormat, MArrayLen(cLineNumFormat),
                                    "L%%0%dd ", lstrlenA(cLineNumFormat))))

        {
            return FALSE;
        }

        ulFuseNumber = 0;

        for (ulFuseRowIndex = 0; ulFuseRowIndex < ulFuseRowCount; ++ulFuseRowIndex)
        {
            if (FAILED(StringCchPrintfA(cLineNum, MArrayLen(cLineNum),
                                        cLineNumFormat, ulFuseNumber)))
            {
                return FALSE;
            }

            if (FALSE == WriteFile(hFile, cLineNum, lstrlenA(cLineNum),
                                    &dwBytesWritten, NULL) ||
                FALSE == WriteFile(hFile, pbyData, ulFuseColumnCount,
                                    &dwBytesWritten, NULL) ||
                FALSE == WriteFile(hFile, "*", 1, &dwBytesWritten, NULL) ||
                FALSE == lWriteFileEndOfLine(hFile))
            {
                return FALSE;
            }

            ulFuseNumber += ulFuseColumnCount;
            pbyData += ulFuseColumnCount;
        }

        bResult = TRUE;
    }
    else
    {
        pszFuseMapText = pPALData->pAllocFuseMapTextFunc(pbyData);

        nTmpFuseMapTextLen = WideCharToMultiByte(CP_ACP, 0, pszFuseMapText, -1,
                                                 NULL, 0, NULL, NULL);

        if (nTmpFuseMapTextLen <= 0)
        {
            pPALData->pFreeFuseMapTextFunc(pszFuseMapText);

            return FALSE;
        }

        pszTmpFuseMapText = (LPSTR)UtAllocMem(nTmpFuseMapTextLen * sizeof(CHAR));

        if (pszTmpFuseMapText == NULL)
        {
            pPALData->pFreeFuseMapTextFunc(pszFuseMapText);

            return FALSE;
        }

        WideCharToMultiByte(CP_ACP, 0, pszFuseMapText, -1,
                            pszTmpFuseMapText, nTmpFuseMapTextLen, NULL, NULL);

        if (TRUE == WriteFile(hFile, pszTmpFuseMapText, lstrlenA(pszTmpFuseMapText),
                              &dwBytesWritten, NULL))
        {
            bResult = TRUE;
        }

        UtFreeMem(pszTmpFuseMapText);

        pPALData->pFreeFuseMapTextFunc(pszFuseMapText);
    }

    return bResult;
}

static BOOL lWriteFilePALFuseMapChecksum(
  HANDLE hFile,
  const TPALData* pPALData,
  LPBYTE pbyData)
{
    BOOL bResult = FALSE;
    CHAR cBuffer[10];
    ULONG ulFuseSize, ulFuseIndex, ulByteIndex, ulChecksum;
    DWORD dwBytesWritten;
    BYTE byChecksumData;

    if (FALSE == UtPALGetFuseMapSize(pPALData, &ulFuseSize))
    {
        return FALSE;
    }

    ulChecksum = 0;
    ulFuseIndex = 0;

    while (ulFuseIndex < ulFuseSize)
    {
        byChecksumData = 0;

        for (ulByteIndex = 0; ulByteIndex < 8 && ulFuseIndex < ulFuseSize;
             ++ulByteIndex)
        {
            if (pbyData[ulFuseIndex] == CPALFuseBlown)
            {
                byChecksumData |= (1 << ulByteIndex);
            }

            ++ulFuseIndex;
        }

        ulChecksum += byChecksumData;
    }

    if (ulChecksum > 65535)
    {
        return FALSE;
    }

    if (FAILED(StringCchPrintfA(cBuffer, MArrayLen(cBuffer), "C%04X*",
                                ulChecksum)))
    {
        return FALSE;
    }

    if (TRUE == WriteFile(hFile, cBuffer, lstrlenA(cBuffer),
                          &dwBytesWritten, NULL) &&
        TRUE == lWriteFileEndOfLine(hFile))
    {
        bResult = TRUE;
    }

    return bResult;
}

static BOOL lGenerateFuseMapText(
  const TPALData* pPALData,
  LPBYTE pbyData,
  LPSTR* ppszText,
  PULONG pulTextLen,
  BOOL bFullFuseMap)
{
    BOOL bResult = FALSE;
    static CHAR cLineEndData[] = {'*', 0x0d, 0x0a};
    CHAR cLineNumFormat[20];
    ULONG ulFuseNumber, ulTmpTextLen, ulFuseColumnIndex, ulFuseRowIndex;
    ULONG ulFuseCount, ulFuseColumnCount, ulFuseRowCount;
    LPSTR pszTextPos;
    BOOL bCopyRow;

    *ppszText = NULL;
    *pulTextLen = 0;

    ulFuseCount = lCalcFuseCount(pPALData);
    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);
    ulFuseRowCount = lCalcFuseRowCount(pPALData);

    if (FAILED(StringCchPrintfA(cLineNumFormat, MArrayLen(cLineNumFormat),
                                "%d", ulFuseCount)))
    {
        return FALSE;
    }

    for (ulFuseRowIndex = 0; ulFuseRowIndex < ulFuseRowCount; ++ulFuseRowIndex)
    {
        *pulTextLen += lstrlenA(cLineNumFormat);
        *pulTextLen += ulFuseColumnCount;
        *pulTextLen += 5;
    }

    if (FAILED(StringCchPrintfA(cLineNumFormat, MArrayLen(cLineNumFormat),
                                "L%%0%dd ", lstrlenA(cLineNumFormat))))
    {
        return FALSE;
    }

    *ppszText = (LPSTR)UtAllocMem(*pulTextLen);

    pszTextPos = *ppszText;
    ulTmpTextLen = *pulTextLen;

    ulFuseNumber = 0;

    for (ulFuseRowIndex = 0; ulFuseRowIndex < ulFuseRowCount; ++ulFuseRowIndex)
    {
        bCopyRow = TRUE;

        if (bFullFuseMap == FALSE)
        {
            for (ulFuseColumnIndex = 0;
                 bCopyRow == TRUE && ulFuseColumnIndex < ulFuseColumnCount;
                 ++ulFuseColumnIndex)
            {
                if (pbyData[ulFuseColumnIndex] == CPALFuseBlown)
                {
                    bCopyRow = FALSE;
                }
            }
        }

        if (bCopyRow)
        {
            // Copy fuse line number

            if (FAILED(StringCchPrintfA(pszTextPos, ulTmpTextLen,
                                        cLineNumFormat, ulFuseNumber)))
            {
                goto End;
            }

            ulTmpTextLen -= lstrlenA(pszTextPos);
            pszTextPos += lstrlenA(pszTextPos);

            // Copy fuse data

            CopyMemory(pszTextPos, pbyData, ulFuseColumnCount);

            ulTmpTextLen -= ulFuseColumnCount;
            pszTextPos += ulFuseColumnCount;

            // Copy fuse data end

            CopyMemory(pszTextPos, cLineEndData, MArrayLen(cLineEndData));

            // Copy fuse line end data

            ulTmpTextLen -= MArrayLen(cLineEndData);
            pszTextPos += MArrayLen(cLineEndData);
        }

        ulFuseNumber += ulFuseColumnCount;
        pbyData += ulFuseColumnCount;
    }

    bResult = TRUE;

End:
    if (bResult == FALSE)
    {
        UtFreeMem(*ppszText);

        *ppszText = NULL;
        *pulTextLen = 0;
    }

    return bResult;
}

static VOID lInitPALPinDefined(
  const TPALData* pPALData,
  TPALPinDefined* pPALPinDefined)
{
    ULONG nIndex;

    for (nIndex = 0; nIndex < pPALData->nDevicePinConfigValuesCount; ++nIndex)
    {
        pPALPinDefined[nIndex].nPin = pPALData->pDevicePinConfigValues[nIndex].nPin;
        pPALPinDefined[nIndex].bDefined = FALSE;
    }
}

static TPALPinDefined* lFindPALPinDefined(
  UINT nPin,
  TPALPinDefined* pPALPinDefined,
  ULONG nTotalPALPinDefined)
{
    ULONG nIndex;

    for (nIndex = 0; nIndex < nTotalPALPinDefined; ++nIndex)
    {
        if (pPALPinDefined[nIndex].nPin == nPin)
        {
            return &pPALPinDefined[nIndex];
        }
    }

    return NULL;
}

static const TDevicePinConfigValues* lFindDevicePinConfigValues(
  UINT nPin,
  const TDevicePinConfigValues* pDevicePinConfigValues,
  UINT nTotalDevicePinConfigValues)
{
    ULONG ulIndex;

    for (ulIndex = 0; ulIndex < nTotalDevicePinConfigValues; ++ulIndex)
    {
        if (pDevicePinConfigValues[ulIndex].nPin == nPin)
        {
            return &pDevicePinConfigValues[ulIndex];
        }
    }

    return NULL;
}

static BOOL lIsValidDevicePinConfigType(
  const TDevicePinConfigValues* pDevicePinConfigValues,
  LPCWSTR pszType)
{
    UINT nIndex;

    for (nIndex = 0; nIndex < pDevicePinConfigValues->nTypesCount; ++nIndex)
    {
        if (0 == lstrcmpiW(pszType, pDevicePinConfigValues->ppszTypes[nIndex]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL lIsValidDevicePinConfigPolarity(
  const TDevicePinConfigValues* pDevicePinConfigValues,
  const TDevicePinConfig* pDevicePinConfig)
{
    UINT nIndex;

    if (0 == lstrcmpiW(pDevicePinConfig->pszType, CInputPinType) ||
        0 == lstrcmpiW(pDevicePinConfig->pszType, CNotUsedPinType))
    {
        return pDevicePinConfig->pszPolarity ? FALSE : TRUE;
    }
    else if (0 == lstrcmpiW(pDevicePinConfig->pszType, COutputPinType) ||
             0 == lstrcmpiW(pDevicePinConfig->pszType, CRegisteredPinType))
    {
        if (pDevicePinConfig->pszPolarity == NULL)
        {
            if (pDevicePinConfigValues->nPolaritiesCount == 1)
            {
                return TRUE;
            }
        }
        else
        {
            for (nIndex = 0; nIndex < pDevicePinConfigValues->nPolaritiesCount;
                 ++nIndex)
            {
                if (0 == lstrcmpiW(pDevicePinConfigValues->ppszPolarities[nIndex],
                                   pDevicePinConfig->pszPolarity))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

static BOOL lSetPALPinDefined(
  UINT nPin,
  TPALPinDefined* pPALPinDefined,
  ULONG nTotalPALPinDefined)
{
    TPALPinDefined* pTmpPALPinDefined;

    pTmpPALPinDefined = lFindPALPinDefined(nPin, pPALPinDefined,
                                           nTotalPALPinDefined);

    if (pTmpPALPinDefined)
    {
        pTmpPALPinDefined->bDefined = TRUE;

        return TRUE;
    }

    return FALSE;
}

static const TDevicePinFuseRows* lFindDevicePinFuseRows(
  UINT nPin,
  const TDevicePinFuseRows* pDevicePinFuseRows,
  UINT nTotalDevicePinFuseRows)
{
    ULONG ulIndex;

    for (ulIndex = 0; ulIndex < nTotalDevicePinFuseRows; ++ulIndex)
    {
        if (pDevicePinFuseRows[ulIndex].nPin == nPin)
        {
            return &pDevicePinFuseRows[ulIndex];
        }
    }

    return NULL;
}

static const TDevicePinFuseColumns* lFindDevicePinFuseColumns(
  UINT nPin,
  const TDevicePinFuseColumns* pDevicePinFuseColumns,
  UINT nTotalDevicePinFuseColumns)
{
    ULONG ulIndex;

    for (ulIndex = 0; ulIndex < nTotalDevicePinFuseColumns; ++ulIndex)
    {
        if (pDevicePinFuseColumns[ulIndex].nPin == nPin)
        {
            return &pDevicePinFuseColumns[ulIndex];
        }
    }

    return NULL;
}

#pragma endregion

BOOL UTPALAPI UtPALInitialize(VOID)
{
	return UtInitHeap();
}

BOOL UTPALAPI UtPALUninitialize(VOID)
{
	return UtUninitHeap();
}

BOOL UTPALAPI UtPALClearFuseMap(
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    if (pbyData == NULL || ulDataLen == 0)
    {
        return FALSE;
    }

    FillMemory(pbyData, ulDataLen, CPALFuseIntact);

    return TRUE;
}

BOOL UTPALAPI UtPALGetFuseMapSize(
  const TPALData* pPALData,
  PULONG pulFuseSize)
{
    if (pPALData == NULL || pulFuseSize == NULL)
    {
        return FALSE;
    }

    if (pPALData->pGetFuseMapSizeFunc == NULL)
    {
        *pulFuseSize = lCalcFuseCount(pPALData);
    }
    else
    {
        *pulFuseSize = pPALData->pGetFuseMapSizeFunc();
    }

    return TRUE;
}

BOOL UTPALAPI UtPALBlowFuse(
  ULONG nFuseRow,
  ULONG nFuseColumn,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    return lSetFuseValue(nFuseRow, nFuseColumn, pPALData, pbyData, ulDataLen,
                         CPALFuseBlown);
}

BOOL UTPALAPI UtPALBlowFuseRange(
  ULONG nStartFuseRow,
  ULONG nStartFuseColumn,
  ULONG nEndFuseRow,
  ULONG nEndFuseColumn,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    ULONG ulFuseRow, ulFuseColumn;

    if (nEndFuseRow < nStartFuseRow || nEndFuseColumn < nStartFuseColumn)
    {
        return FALSE;
    }

    for (ulFuseRow = nStartFuseRow; ulFuseRow <= nEndFuseRow; ++ulFuseRow)
    {
        for (ulFuseColumn = nStartFuseColumn; ulFuseColumn <= nEndFuseColumn; ++ulFuseColumn)
        {
            if (FALSE == UtPALBlowFuse(ulFuseRow, ulFuseColumn, pPALData,
                                       pbyData, ulDataLen))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL UTPALAPI UtPALBlowFuseRow(
  ULONG nFuseRow,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    return lSetFuseRowValue(nFuseRow, pPALData, pbyData, ulDataLen, CPALFuseBlown);
}

BOOL UTPALAPI UtPALBlowFuseRows(
  ULONG nStartFuseRow,
  ULONG nEndFuseRow,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    ULONG ulFuseRow;

    if (nEndFuseRow < nStartFuseRow)
    {
        return FALSE;
    }

    for (ulFuseRow = nStartFuseRow; ulFuseRow <= nEndFuseRow; ++ulFuseRow)
    {
        if (FALSE == lSetFuseRowValue(ulFuseRow, pPALData, pbyData, ulDataLen,
                                      CPALFuseBlown))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL UTPALAPI UtPALBlowFuseColumn(
  ULONG nFuseColumn,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    ULONG ulFuseSize, nFuseRowIndex, ulFuseColumnCount, ulFuseRowCount;

    if (FALSE == UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        ulDataLen != ulFuseSize)
    {
        return FALSE;
    }

    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);
    ulFuseRowCount = lCalcFuseRowCount(pPALData);

    for (nFuseRowIndex = 0; nFuseRowIndex < ulFuseRowCount; ++nFuseRowIndex)
    {
        if (nFuseColumn >= ulFuseColumnCount)
        {
            return FALSE;
        }
    }

    for (nFuseRowIndex = 0; nFuseRowIndex < ulFuseRowCount; ++nFuseRowIndex)
    {
        *(pbyData + nFuseColumn) = CPALFuseBlown;

        pbyData += ulFuseColumnCount;
    }

    return TRUE;
}

BOOL UTPALAPI UtPALBlowFuseColumns(
  ULONG nStartFuseColumn,
  ULONG nEndFuseColumn,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    ULONG ulFuseColumn;

    if (nEndFuseColumn < nStartFuseColumn)
    {
        return FALSE;
    }

    for (ulFuseColumn = nStartFuseColumn; ulFuseColumn <= nEndFuseColumn;
         ++ulFuseColumn)
    {
        if (FALSE == UtPALBlowFuseColumn(ulFuseColumn, pPALData, pbyData, ulDataLen))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL UTPALAPI UtPALRestoreFuse(
  ULONG nFuseRow,
  ULONG nFuseColumn,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    return lSetFuseValue(nFuseRow, nFuseColumn, pPALData, pbyData, ulDataLen,
                         CPALFuseIntact);
}

BOOL UTPALAPI UtPALRestoreFuseRow(
  ULONG nFuseRow,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    return lSetFuseRowValue(nFuseRow, pPALData, pbyData, ulDataLen, CPALFuseIntact);
}

ULONG UTPALAPI UtPALFuseNumberToFuseRow(
  ULONG nFuseNumber,
  const TPALData* pPALData)
{
    ULONG nFuseRowIndex, nTmpFuseNumber, ulFuseColumnCount, ulFuseRowCount;

    nTmpFuseNumber = 0;
    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);
    ulFuseRowCount = lCalcFuseRowCount(pPALData);

    for (nFuseRowIndex = 0; nFuseRowIndex < ulFuseRowCount; ++nFuseRowIndex)
    {
        nTmpFuseNumber += ulFuseColumnCount;

        if (nTmpFuseNumber > nFuseNumber)
        {
            return nFuseRowIndex;
        }
    }

    return CNoFuseRow;
}

ULONG UTPALAPI UtPALFuseColumnFromPin(
  UINT nPin,
  BOOL bHighFuseColumn,
  const TPALData* pPALData)
{
    const TDevicePinFuseColumns* pDevicePinFuseColumns;

    pDevicePinFuseColumns = lFindDevicePinFuseColumns(nPin,
                                pPALData->pDevicePinFuseColumns,
                                pPALData->nDevicePinFuseColumnsCount);

    if (pDevicePinFuseColumns == NULL)
    {
        return CNoFuseColumn;
    }

    if (bHighFuseColumn)
    {
        return pDevicePinFuseColumns->nHighFuseColumn;
    }

    return pDevicePinFuseColumns->nLowFuseColumn;
}

BOOL UTPALAPI UtPALIsFuseColumnPresent(
  UINT nPin,
  const TPALData* pPALData)
{
    const TDevicePinFuseColumns* pDevicePinFuseColumns;

    pDevicePinFuseColumns = lFindDevicePinFuseColumns(nPin,
                                pPALData->pDevicePinFuseColumns,
                                pPALData->nDevicePinFuseColumnsCount);

    return (pDevicePinFuseColumns != NULL) ? TRUE : FALSE;
}

ULONG UTPALAPI UtPALTermCount(
  UINT nPin,
  const TPALData* pPALData)
{
    const TDevicePinFuseRows* pDevicePinFuseRows;
    ULONG ulFuseColumnCount, ulTermCount;

    pDevicePinFuseRows = lFindDevicePinFuseRows(nPin,
                                                pPALData->pDevicePinFuseRows,
                                                pPALData->nDevicePinFuseRowsCount);

    if (pDevicePinFuseRows == NULL)
    {
        return 0;
    }

    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);

    ulTermCount = pDevicePinFuseRows->nFuseRowTermEnd -
                      pDevicePinFuseRows->nFuseRowTermStart;

    return (ulTermCount / ulFuseColumnCount) + 1;
}

ULONG UTPALAPI UtPALTermRowToFuseRow(
  UINT nPin,
  ULONG nTerm,
  const TPALData* pPALData)
{
    ULONG ulFuseRowIndex, ulFuseColumnCount, ulFuseRow;

    if (nTerm >= UtPALTermCount(nPin, pPALData))
    {
        return (ULONG)-1;
    }

    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);

    for (ulFuseRowIndex = 0;
         ulFuseRowIndex < pPALData->nDevicePinFuseRowsCount;
         ++ulFuseRowIndex)
    {
        if (nPin == pPALData->pDevicePinFuseRows[ulFuseRowIndex].nPin)
        {
            ulFuseRow = pPALData->pDevicePinFuseRows[ulFuseRowIndex].nFuseRowTermStart;
            ulFuseRow /= ulFuseColumnCount;
            ulFuseRow += nTerm;

            return ulFuseRow;
        }
    }

    return (ULONG)-1;
}

BOOL UTPALAPI UtPALIsOutputEnablePresent(
  UINT nPin,
  const TPALData* pPALData)
{
    ULONG ulFuseRowIndex, ulFuseColumnCount, ulFuseRow;

    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);

    for (ulFuseRowIndex = 0;
         ulFuseRowIndex < pPALData->nDevicePinFuseRowsCount;
         ++ulFuseRowIndex)
    {
        if (nPin == pPALData->pDevicePinFuseRows[ulFuseRowIndex].nPin)
        {
            ulFuseRow = pPALData->pDevicePinFuseRows[ulFuseRowIndex].nFuseRowOutputEnable;

            if (ulFuseRow != CNoOutputEnableFuseRow)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

ULONG UTPALAPI UtPALOutputEnableFuseRow(
  UINT nPin,
  const TPALData* pPALData)
{
    ULONG ulFuseRowIndex, ulFuseColumnCount, ulFuseRow;

    ulFuseColumnCount = lCalcFuseColumnCount(pPALData);

    for (ulFuseRowIndex = 0;
         ulFuseRowIndex < pPALData->nDevicePinFuseRowsCount;
         ++ulFuseRowIndex)
    {
        if (nPin == pPALData->pDevicePinFuseRows[ulFuseRowIndex].nPin)
        {
            ulFuseRow = pPALData->pDevicePinFuseRows[ulFuseRowIndex].nFuseRowOutputEnable;

            if (ulFuseRow != CNoOutputEnableFuseRow)
            {
                ulFuseRow /= ulFuseColumnCount;

                return ulFuseRow;
            }
        }
    }

    return CNoOutputEnableFuseRow;
}

BOOL UTPALAPI UtPALWriteFuseMapToJEDFile(
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  LPCWSTR pszPALType,
  UINT nPinCount,
  LPCWSTR pszFile)
{
    BOOL bResult = FALSE;
    ULONG ulFuseSize;
    HANDLE hFile;

    if (FALSE == UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        ulDataLen != ulFuseSize)
    {
		return FALSE;
    }

    hFile = CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (TRUE == lWriteFileStartOfText(hFile) &&
        TRUE == lWriteFilePALType(hFile, pszPALType) &&
        TRUE == lWriteFilePALDeviceData(hFile, pPALData, nPinCount) &&
        TRUE == lWriteFilePALFuseMap(hFile, pPALData, pbyData) &&
        TRUE == lWriteFilePALFuseMapChecksum(hFile, pPALData, pbyData) &&
        TRUE == lWriteFileEndOfText(hFile))
    {
        bResult = TRUE;
    }

    CloseHandle(hFile);

    if (bResult == FALSE)
    {
        DeleteFileW(pszFile);
    }

    return bResult;
}

BOOL UTPALAPI UtPALFuseMapText(
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  LPCWSTR pszPALType,
  UINT nPinCount,
  LPWSTR pszText,
  PULONG pulTextLen)
{
    BOOL bResult = FALSE;
    WCHAR cTempPath[MAX_PATH], cFile[MAX_PATH];
    DWORD dwFileLen, dwBytesRead;
    HANDLE hFile;
    LPSTR pszFileData;
    INT nBufferLen;

    if (0 == GetTempPathW(MArrayLen(cTempPath), cTempPath) ||
        0 == GetTempFileNameW(cTempPath, L"PAL", 1, cFile) ||
        FALSE == UtPALWriteFuseMapToJEDFile(pPALData, pbyData, ulDataLen,
                                            pszPALType, nPinCount, cFile))
    {
        return FALSE;
    }

    hFile = CreateFile(cFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        goto EndDelete;
    }

    dwFileLen = GetFileSize(hFile, NULL) + 1;

    pszFileData = (LPSTR)UtAllocMem(dwFileLen);

    if (pszFileData == NULL)
    {
        goto EndFreeMem;
    }

    if (FALSE == ReadFile(hFile, pszFileData, dwFileLen, &dwBytesRead, NULL))
    {
        goto EndFreeMem;
    }

    *(pszFileData + dwBytesRead) = 0;

    nBufferLen = MultiByteToWideChar(CP_ACP, 0, pszFileData, dwFileLen,
                                     NULL, 0);

    if (pszText)
    {
        if (*pulTextLen >= (ULONG)nBufferLen)
        {
            MultiByteToWideChar(CP_ACP, 0, pszFileData, dwFileLen,
                                pszText, *pulTextLen);

            bResult = TRUE;
        }
    }
    else
    {
        *pulTextLen = nBufferLen;

        bResult = TRUE;
    }

EndFreeMem:
    UtFreeMem(pszFileData);

    CloseHandle(hFile);

EndDelete:
    DeleteFileW(cFile);

    return bResult;
}

BOOL UTPALAPI UtPALFuseMapSampleText(
  const TPALData* pPALData,
  LPWSTR pszText,
  PULONG pulTextLen)
{
    BOOL bResult = FALSE;
    LPBYTE pbyData;
    ULONG ulFuseSize, ulTmpTextLen;
    LPSTR pszTmpText;
    INT nBufferLen;
    LPCWSTR pszSampleFuseMapText;

    if (pPALData->pAllocSampleFuseMapTextFunc == NULL &&
        pPALData->pFreeSampleFuseMapTextFunc == NULL)
    {
        if (FALSE == UtPALGetFuseMapSize(pPALData, &ulFuseSize))
        {
            return FALSE;
        }

        pbyData = (LPBYTE)UtAllocMem(ulFuseSize);

        FillMemory(pbyData, ulFuseSize, 'x');

        if (lGenerateFuseMapText(pPALData, pbyData, &pszTmpText, &ulTmpTextLen, FALSE))
        {
            nBufferLen = MultiByteToWideChar(CP_ACP, 0, pszTmpText, ulTmpTextLen,
                                             NULL, 0);

            if (pszText)
            {
                if (*pulTextLen >= (ULONG)nBufferLen + 2)
                {
                    MultiByteToWideChar(CP_ACP, 0, pszTmpText, ulTmpTextLen,
                                        pszText, *pulTextLen);

                    pszText[nBufferLen] = 0;

                    bResult = TRUE;
                }
            }
            else
            {
                *pulTextLen = nBufferLen + 2;

                bResult = TRUE;
            }

            UtFreeMem(pszTmpText);
        }

        UtFreeMem(pbyData);
    }
    else
    {
        pszSampleFuseMapText = pPALData->pAllocSampleFuseMapTextFunc();

        nBufferLen = lstrlenW(pszSampleFuseMapText);

        if (pszText)
        {
            if (*pulTextLen >= (ULONG)nBufferLen + 2)
            {
                StringCchCopyW(pszText, *pulTextLen, pszSampleFuseMapText);

                bResult = TRUE;
            }
        }
        else
        {
            *pulTextLen = nBufferLen + 2;

            bResult = TRUE;
        }

        pPALData->pFreeSampleFuseMapTextFunc(pszSampleFuseMapText);
    }

    return bResult;
}

BOOL UTPALAPI UtPALDevicePinConfigToPinDefined(
  const TPALData* pPALData,
  const TDevicePinConfig* pDevicePinConfigs,
  ULONG nTotalDevicePinConfigs,
  TPALPinDefined* pPALPinDefined,
  ULONG nTotalPALPinDefined)
{
    const TDevicePinConfigValues* pDevicePinConfigValues;
    ULONG nIndex;
    UINT nPin;

    if (pPALData->nDevicePinConfigValuesCount != nTotalDevicePinConfigs ||
        pPALData->nDevicePinConfigValuesCount != nTotalPALPinDefined)
    {
        return FALSE;
    }

    lInitPALPinDefined(pPALData, pPALPinDefined);

    for (nIndex = 0; nIndex < nTotalDevicePinConfigs; ++nIndex)
    {
        nPin = pDevicePinConfigs[nIndex].nPin;

        pDevicePinConfigValues = lFindDevicePinConfigValues(nPin, pPALData->pDevicePinConfigValues,
                                                            pPALData->nDevicePinConfigValuesCount);

        if (pDevicePinConfigValues == NULL)
        {
            return FALSE;
        }

        if (FALSE == lIsValidDevicePinConfigType(pDevicePinConfigValues,
                                                 pDevicePinConfigs[nIndex].pszType))
        {
            return FALSE;
        }

        if (FALSE == lIsValidDevicePinConfigPolarity(pDevicePinConfigValues,
                                                     &pDevicePinConfigs[nIndex]))
        {
            return FALSE;
        }

        if (FALSE == lSetPALPinDefined(nPin, pPALPinDefined, nTotalPALPinDefined))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL UTPALAPI UtPALVerifyDevicePinConfig(
  const TPALData* pPALData,
  const TDevicePinConfig* pDevicePinConfigs,
  ULONG nTotalDevicePinConfigs,
  LPBOOL pbValid)
{
    TPALPinDefined* pPALPinDefined;
    ULONG nIndex;

    *pbValid = FALSE;

    pPALPinDefined = (TPALPinDefined*)UtAllocMem(sizeof(TPALPinDefined) *
                                                     nTotalDevicePinConfigs);

    if (pPALPinDefined == NULL)
    {
        return FALSE;
    }

    if (UtPALDevicePinConfigToPinDefined(pPALData, pDevicePinConfigs,
                                         nTotalDevicePinConfigs,
                                         pPALPinDefined,
                                         nTotalDevicePinConfigs))
    {
        *pbValid = TRUE;

        for (nIndex = 0; nIndex < nTotalDevicePinConfigs; ++nIndex)
        {
            if (pPALPinDefined[nIndex].bDefined == FALSE)
            {
                *pbValid = FALSE;
            }
        }
    }
    
    UtFreeMem(pPALPinDefined);

    return TRUE;
}

LPWSTR* UTPALAPI UtPALAllocMinimizedTerms(
  INT nMinimizerAlgo,
  LPCWSTR* ppszTerms)
{
    switch (nMinimizerAlgo)
    {
        case CQuine_McCluskyMinimizer:
            return QM_AllocMinimizedTerms(ppszTerms);
        case CEspressoMinimizer:
            return Espresso_AllocMinimizedTerms(ppszTerms);
    }

    return NULL;
}

BOOL UTPALAPI UtPALFreeMinimizedTerms(
  INT nMinimizerAlgo,
  LPWSTR* ppszMinimizedTerms)
{
    switch (nMinimizerAlgo)
    {
        case CQuine_McCluskyMinimizer:
            QM_FreeMinimizedTerms(ppszMinimizedTerms);

            return TRUE;
        case CEspressoMinimizer:
            Espresso_FreeMinimizedTerms(ppszMinimizedTerms);

            return TRUE;
    }

    return FALSE;
}

BOOL UTPALAPI UtPALAreSimiliarTermsPresent(
  LPCWSTR* ppszTerms,
  LPCWSTR* ppszTermsMatch)
{
	return AreSimiliarTermsPresent(ppszTerms, ppszTermsMatch);
}

LPWSTR* UTPALAPI UtPALAllocMergedTerms(
  LPCWSTR* ppszTerms,
  LPCWSTR* ppszTermsMatch)
{
	return AllocMergedTerms(ppszTerms, ppszTermsMatch);
}

VOID UTPALAPI UtPALFreeMergedTerms(
  LPWSTR* ppszTerms)
{
    FreeMergedTerms(ppszTerms);
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
