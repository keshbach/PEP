/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <assert.h>

#include <UtilsDevice/UtIntelHex.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#define COLON ':'
#define CR 0x0D
#define LF 0x0A

#define PAGE_LEN 0x00010000

typedef enum ERecordType
{
    ertData,
    ertEndOfFile,
    ertExtendedSegmentAddress,
    ertStartSegmentAddress,
    ertExtendedLinearAddress,
    ertStartLinearAddress,
    ertUndefined
} ERecordType;

typedef struct tagTPageData
{
    UINT32 nAddress;
    UINT32 nLength;
    UINT8* pData;
} TPageData;

typedef struct tagTHandleData
{
    UINT16 nErrorCode;
    UINT32 nErrorLineNumber;
    UINT32 nTotalPages;
    TPageData PageData[1];
} THandleData;

typedef struct tagTPageStart
{
    TPageData PageData;
    UINT8* pbDataSet;
} TPageStart;

static BOOL l_bInitialize = TRUE;

static BOOL lIsHexDigit(
  CHAR cDigit)
{
    if ((cDigit >= 'a' && cDigit <= 'f') ||
        (cDigit >= 'A' && cDigit <= 'F') ||
        (cDigit >= '0' && cDigit <= '9'))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL lValidateHexNumber(
  LPCSTR pszDigits,
  INT nTotalDigits)
{
    for (INT nIndex = 0; nIndex < nTotalDigits; ++nIndex)
    {
        if (FALSE == lIsHexDigit(pszDigits[nIndex]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static WORD lNumberFromHex(
  LPCSTR pszDigits,
  INT nTotalDigits)
{
    WORD Value = 0;
    BYTE CurrentDigit;

    for (INT nIndex = 0; nIndex < nTotalDigits; ++nIndex)
    {
        CurrentDigit = pszDigits[nIndex];

        if (CurrentDigit >= '0' && CurrentDigit <= '9')
        {
            CurrentDigit = CurrentDigit - '0';
        }
        else if (CurrentDigit >= 'a' && CurrentDigit <= 'f')
        {
            CurrentDigit = CurrentDigit - 'a' + 10;
        }
        else
        {
            CurrentDigit = CurrentDigit - 'A' + 10;
        }

        Value = (Value << 4) | (CurrentDigit & 0x0F);
    }

    return Value;
}

static inline BOOL lValidateHexByte(
  LPCSTR pszDigits)
{
    return lValidateHexNumber(pszDigits, 2);
}

static inline BOOL lValidateHexWord(
  LPCSTR pszDigits)
{
    return lValidateHexNumber(pszDigits, 4);
}

static inline BYTE lHexToByte(
  LPCSTR pszDigits)
{
    return (BYTE)lNumberFromHex(pszDigits, 2);
}

static inline WORD lHexToWord(
  LPCSTR pszDigits)
{
    return lNumberFromHex(pszDigits, 4);
}

static ERecordType lTranslateRecordType(
  LPCSTR pszRecordType)
{
    if (pszRecordType[0] == '0' && pszRecordType[1] == '0')
    {
        return ertData;
    }
    else if (pszRecordType[0] == '0' && pszRecordType[1] == '1')
    {
        return ertEndOfFile;
    }
    else if (pszRecordType[0] == '0' && pszRecordType[1] == '2')
    {
        return ertExtendedSegmentAddress;
    }
    else if (pszRecordType[0] == '0' && pszRecordType[1] == '3')
    {
        return ertStartSegmentAddress;
    }
    else if (pszRecordType[0] == '0' && pszRecordType[1] == '4')
    {
        return ertExtendedLinearAddress;
    }
    else if (pszRecordType[0] == '0' && pszRecordType[1] == '5')
    {
        return ertStartLinearAddress;
    }

    return ertUndefined;
}

static BOOL lLoadFile(
  LPCTSTR pszFile,
  LPBYTE* ppbyData,
  LPDWORD pdwDataLen)
{
    HANDLE hFile;
    LARGE_INTEGER LargeInteger;
    LPBYTE pbyData;
    DWORD dwNumberOfBytesRead;

    *ppbyData = NULL;
    *pdwDataLen = 0;

    hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (FALSE == GetFileSizeEx(hFile, &LargeInteger))
    {
        CloseHandle(hFile);

        return FALSE;
    }

    *pdwDataLen = LargeInteger.LowPart;

    pbyData = (LPBYTE)UtAllocMem(LargeInteger.LowPart);

    if (pbyData == NULL)
    {
        CloseHandle(hFile);

        *pdwDataLen = 0;

        return FALSE;
    }

    if (FALSE == ReadFile(hFile, pbyData, LargeInteger.LowPart, &dwNumberOfBytesRead, NULL))
    {
        UtFreeMem(pbyData);

        CloseHandle(hFile);

        *pdwDataLen = 0;

        return FALSE;
    }

    CloseHandle(hFile);

    *ppbyData = pbyData;

    return TRUE;
}

static BOOL lFindNextRecord(
  LPCSTR pszData,
  DWORD dwDataLen,
  LPCSTR* ppszRecordStart,
  LPCSTR* ppszRecordEnd)
{
    *ppszRecordStart = NULL;
    *ppszRecordEnd = NULL;

    while (dwDataLen > 0 && *pszData != COLON)
    {
        --dwDataLen;
        ++pszData;
    }

    if (dwDataLen == 0)
    {
        return FALSE;
    }

    *ppszRecordStart = pszData;

    while (dwDataLen > 0 && *pszData != CR && *pszData != LF)
    {
        --dwDataLen;
        ++pszData;
    }

    if (dwDataLen == 0)
    {
        *ppszRecordStart = NULL;

        return FALSE;
    }

    *ppszRecordEnd = pszData;

    return TRUE;
}

static BOOL lFindRecordFields(
  LPCSTR pszRecord,
  DWORD dwRecordLen,
  UINT8* pnFieldByteCount,
  UINT16* pnFieldAddress,
  ERecordType* pRecordType,
  LPBYTE* ppbyData,
  UINT16* pnErrorCode)
{
    UINT8 nChecksum = 0;

    *pnFieldByteCount = 0;
    *pnFieldAddress = 0;
    *pRecordType = ertUndefined;
    *ppbyData = NULL;
    *pnErrorCode = UtIntelHexErrorCodeSuccess;

    if (dwRecordLen < 11)
    {
        *pnErrorCode = UtIntelHexErrorCodeMinRecordLenNotSatisfied;

        return FALSE;
    }

    // Skip the colon

    ++pszRecord;
    --dwRecordLen;

    // Validate Byte Count

    if (!lValidateHexByte(pszRecord))
    {
        *pnErrorCode = UtIntelHexErrorCodeInvalidByteCount;

        return FALSE;
    }

    *pnFieldByteCount = lHexToByte(pszRecord);

    pszRecord += 2;
    dwRecordLen -= 2;

    nChecksum += *pnFieldByteCount;

    // Validate Address

    if (!lValidateHexWord(pszRecord))
    {
        *pnErrorCode = UtIntelHexErrorCodeInvalidAddress;

        return FALSE;
    }

    *pnFieldAddress = lHexToWord(pszRecord);

    pszRecord += 4;
    dwRecordLen -= 4;

    nChecksum += (UINT8)(*pnFieldAddress >> 8);
    nChecksum += (UINT8)(*pnFieldAddress);

    // Validate Record Type

    if (!lValidateHexByte(pszRecord))
    {
        *pnErrorCode = UtIntelHexErrorCodeInvalidRecordType;

        return FALSE;
    }

    if (lTranslateRecordType(pszRecord) == ertUndefined)
    {
        *pnErrorCode = UtIntelHexErrorCodeUnrecognizedRecordType;

        return FALSE;
    }

    nChecksum += lHexToByte(pszRecord);

    *pRecordType = lTranslateRecordType(pszRecord);

    pszRecord += 2;
    dwRecordLen -= 2;

    // Validate Data

    if ((dwRecordLen - (*pnFieldByteCount * 2)) - 2 != 0)
    {
        *pnErrorCode = UtIntelHexErrorCodeInvalidDataLen;

        return FALSE;
    }

    for (INT nIndex = 0; nIndex < *pnFieldByteCount; ++nIndex)
    {
        if (!lValidateHexByte(pszRecord + (nIndex * 2)))
        {
            *pnErrorCode = UtIntelHexErrorCodeInvalidData;

            return FALSE;
        }
    }

    *ppbyData = UtAllocMem(*pnFieldByteCount);

    if (*ppbyData == NULL)
    {
        *pnErrorCode = UtIntelHexErrorCodeOutOfMemory;

        return FALSE;
    }

    for (INT nIndex = 0; nIndex < *pnFieldByteCount; ++nIndex)
    {
        nChecksum += lHexToByte(pszRecord);

        (*ppbyData)[nIndex] = lHexToByte(pszRecord);

        pszRecord += 2;
        dwRecordLen -= 2;
    }

    // Validate Checksum

    if (!lValidateHexByte(pszRecord))
    {
        *pnErrorCode = UtIntelHexErrorCodeInvalidChecksum;

        return FALSE;
    }

    nChecksum += lHexToByte(pszRecord);

    if (nChecksum != 0x00)
    {
        *pnErrorCode = UtIntelHexErrorCodeChecksumMismatch;

        return FALSE;
    }

    return TRUE;
}

static TPageStart* lAllocPageStarts(
  TPageStart* pPageStarts,
  UINT32 nTotalPageStarts)
{
    TPageStart* pNewPageStarts;

    if (pPageStarts)
    {
        pNewPageStarts = (TPageStart*)UtReAllocMem(pPageStarts, sizeof(TPageStart) * (nTotalPageStarts + 1));
    }
    else
    {
        pNewPageStarts = (TPageStart*)UtAllocMem(sizeof(TPageStart) * nTotalPageStarts);
    }

    if (pNewPageStarts == NULL)
    {
        return NULL;
    }

    pNewPageStarts[nTotalPageStarts - 1].PageData.nAddress = 0x0000;
    pNewPageStarts[nTotalPageStarts - 1].PageData.nLength = 0;
    pNewPageStarts[nTotalPageStarts - 1].PageData.pData = NULL;
    pNewPageStarts[nTotalPageStarts - 1].pbDataSet = NULL;

    return pNewPageStarts;
}

static VOID lFreePageStarts(
  TPageStart* pPageStarts,
  UINT32 nTotalPageStarts)
{
    if (pPageStarts)
    {
        for (UINT32 nIndex = 0; nIndex < nTotalPageStarts; ++nIndex)
        {
            if (pPageStarts[nIndex].PageData.pData)
            {
                UtFreeMem(pPageStarts[nIndex].PageData.pData);
            }

            if (pPageStarts[nIndex].pbDataSet)
            {
                UtFreeMem(pPageStarts[nIndex].pbDataSet);
            }
        }

        UtFreeMem(pPageStarts);
    }
}

static BOOL lDoesPageAddressExist(
  UINT nPageAddress,
  TPageStart* pPageStart,
  UINT32 nTotalPageStarts)
{
    for (UINT32 nIndex = 0; nIndex < nTotalPageStarts; ++nIndex)
    {
        if (pPageStart[nIndex].PageData.nAddress == nPageAddress &&
            pPageStart[nIndex].PageData.nLength > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static VOID lSortPageStart(
  TPageStart* pPageStart,
  UINT32 nTotalPageStarts)
{
    UINT32 nLength = nTotalPageStarts;
    BOOL bQuit = FALSE;
    BOOL bSwapped;
    TPageStart TempPageStart;

    while (!bQuit)
    {
        bSwapped = FALSE;

        for (UINT32 nIndex = 1; nIndex < nLength - 1; ++nIndex)
        {
            if (pPageStart[nIndex - 1].PageData.nAddress > pPageStart[nIndex].PageData.nAddress)
            {
                CopyMemory(&TempPageStart, &pPageStart[nIndex - 1], sizeof(TPageStart));
                CopyMemory(&pPageStart[nIndex - 1], &pPageStart[nIndex], sizeof(TPageStart));
                CopyMemory(&pPageStart[nIndex], &TempPageStart, sizeof(TPageStart));

                bSwapped = TRUE;
            }
        }

        if (!bSwapped)
        {
            bQuit = TRUE;
        }
    }
}

static INT32 lFindNextPageStartIndex(
  TPageStart* pPageStart)
{
    for (UINT32 nIndex = 0; nIndex < pPageStart->PageData.nLength; ++nIndex)
    {
        if (pPageStart->pbDataSet[nIndex])
        {
            return nIndex;
        }
    }

    return -1;
}

static UINT32 lFindPageStartValidByteCount(
  TPageStart* pPageStart,
  UINT32 nStartIndex)
{
    UINT32 nByteCount = 0;

    for (UINT32 nIndex = nStartIndex;
         nIndex < pPageStart->PageData.nLength && pPageStart->pbDataSet[nIndex];
         ++nIndex)
    {
        ++nByteCount;
    }

    return nByteCount;
}

static BOOL lFindPageStartNextValidStartIndex(
  TPageStart* pPageStart,
  UINT32 nStartIndex,
  UINT32* pnNextStartIndex)
{
    *pnNextStartIndex = 0;

    while (nStartIndex < pPageStart->PageData.nLength)
    {
        if (pPageStart->pbDataSet[nStartIndex])
        {
            *pnNextStartIndex = nStartIndex;

            return TRUE;
        }

        ++nStartIndex;
    }

    return FALSE;
}

static THandleData* lTranslatePageStartToHandleData(
  TPageStart* pPageStart,
  UINT32 nTotalPageStarts,
  THandleData* pHandleData)
{
    THandleData* pTempHandleData;
    UINT8* pTempData;
    UINT32 nByteCount, nNextStartIndex;
    INT32 nStartIndex;

    lSortPageStart(pPageStart, nTotalPageStarts);

    for (UINT32 nIndex = 0; nIndex < nTotalPageStarts; ++nIndex)
    {
        nStartIndex = lFindNextPageStartIndex(&pPageStart[nIndex]);

        if (nStartIndex == -1)
        {
            continue;
        }

        while ((UINT32)nStartIndex < pPageStart[nIndex].PageData.nLength)
        {
            nByteCount = lFindPageStartValidByteCount(&pPageStart[nIndex], nStartIndex);

            if (pHandleData->nTotalPages > 0)
            {
                if (nByteCount == 0)
                {
                    if (lFindPageStartNextValidStartIndex(&pPageStart[nIndex], nStartIndex, &nNextStartIndex))
                    {
                        nStartIndex = nNextStartIndex;
                    }
                    else
                    {
                        nStartIndex = pPageStart[nIndex].PageData.nLength;
                    }

                    continue;
                }

                if (pHandleData->PageData[pHandleData->nTotalPages - 1].nAddress + pHandleData->PageData[pHandleData->nTotalPages - 1].nLength == pPageStart[nIndex].PageData.nAddress + nStartIndex)
                {
                    // Continuation of previous page

                    pTempData = UtReAllocMem(pHandleData->PageData[pHandleData->nTotalPages - 1].pData,
                                             pHandleData->PageData[pHandleData->nTotalPages - 1].nLength + nByteCount);

                    if (pTempData == NULL)
                    {
                        pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

                        return pHandleData;
                    }

                    pHandleData->PageData[pHandleData->nTotalPages - 1].pData = pTempData;

                    CopyMemory(pTempData + pHandleData->PageData[pHandleData->nTotalPages - 1].nLength,
                               pPageStart[nIndex].PageData.pData + nStartIndex,
                               nByteCount);

                    pHandleData->PageData[pHandleData->nTotalPages - 1].nLength += nByteCount;
                }
                else
                {
                    // Start new page

                    pTempHandleData = UtReAllocMem(pHandleData,
                                                   sizeof(THandleData) +
                                                       (pHandleData->nTotalPages * sizeof(TPageData)));

                    if (pTempHandleData == NULL)
                    {
                        pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

                        return pHandleData;
                    }

                    pHandleData = pTempHandleData;

                    pHandleData->PageData[pHandleData->nTotalPages].pData = UtAllocMem(nByteCount);

                    if (pHandleData->PageData[pHandleData->nTotalPages].pData == NULL)
                    {
                        pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

                        return pHandleData;
                    }

                    pHandleData->PageData[pHandleData->nTotalPages].nAddress = pPageStart[nIndex].PageData.nAddress + nStartIndex;
                    pHandleData->PageData[pHandleData->nTotalPages].nLength = nByteCount;

                    CopyMemory(pHandleData->PageData[pHandleData->nTotalPages].pData,
                               pPageStart[nIndex].PageData.pData + nStartIndex,
                               nByteCount);

                    ++pHandleData->nTotalPages;
                }
            }
            else
            {
                pHandleData->PageData->pData = UtAllocMem(nByteCount);

                if (pHandleData->PageData->pData == NULL)
                {
                    pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

                    return pHandleData;
                }

                pHandleData->PageData->nAddress = pPageStart[nIndex].PageData.nAddress + nStartIndex;
                pHandleData->PageData->nLength = nByteCount;

                CopyMemory(pHandleData->PageData->pData,
                           pPageStart[nIndex].PageData.pData + nStartIndex,
                           nByteCount);

                pHandleData->nTotalPages = 1;
            }

            nStartIndex += nByteCount;
        }
    }

    return pHandleData;
}

BOOL UTINTELHEXAPI UtIntelHexInitialize(VOID)
{
    if (l_bInitialize == FALSE)
    {
        return TRUE;
    }

    if (UtInitHeap() == FALSE)
    {
        return FALSE;
    }

    l_bInitialize = TRUE;

    return TRUE;
}

BOOL UTINTELHEXAPI UtIntelHexUninitialize(VOID)
{
    if (l_bInitialize == FALSE)
    {
        return FALSE;
    }

    l_bInitialize = TRUE;

    if (UtUninitHeap() == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

TUtIntelHexHandle UTINTELHEXAPI UtIntelHexLoadFile(
  LPCWSTR pszFile)
{
    THandleData* pHandleData;
    LPBYTE pbyData;
    DWORD dwDataLen, dwTempDataLen;
    LPCSTR pszDataPos, pszRecordStart, pszRecordEnd;
    UINT8 nFieldByteCount;
    UINT16 nFieldAddress;
    ERecordType RecordType, LastRecordType;
    LPBYTE pbyRecordData;
    UINT32 nLineNumber;
    TPageStart* pPageStart;
    TPageStart* pTempPageStart;
    UINT32 nTotalPageStarts, nPageAddress;
    BOOL bQuit, bAddPageStart;

    pHandleData = (THandleData*)UtAllocMem(sizeof(THandleData));

    if (pHandleData == NULL)
    {
        return NULL;
    }

    ZeroMemory(pHandleData, sizeof(*pHandleData));

    if (FALSE == lLoadFile(pszFile, &pbyData, &dwDataLen))
    {
        pHandleData->nErrorCode = UtIntelHexErrorCodeFileError;

        return pHandleData;
    }

    LastRecordType = ertUndefined;

    nLineNumber = 1;

    pszDataPos = (LPCSTR)pbyData;
    dwTempDataLen = dwDataLen;

    nTotalPageStarts = 1;

    pPageStart = lAllocPageStarts(NULL, nTotalPageStarts);

    if (pPageStart == NULL)
    {
        UtFreeMem(pbyData);

        pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

        return pHandleData;
    }

    bQuit = FALSE;

    while (!bQuit && lFindNextRecord(pszDataPos, dwTempDataLen, &pszRecordStart, &pszRecordEnd))
    {
        if (FALSE == lFindRecordFields(pszRecordStart, (DWORD)(pszRecordEnd - pszRecordStart),
                                       &nFieldByteCount, &nFieldAddress, &RecordType,
                                       &pbyRecordData, &pHandleData->nErrorCode))
        {
            lFreePageStarts(pPageStart, nTotalPageStarts);

            UtFreeMem(pbyData);

            return pHandleData;
        }

        if (RecordType == ertData)
        {
            if (pPageStart[nTotalPageStarts - 1].PageData.nLength == 0)
            {
                pPageStart[nTotalPageStarts - 1].PageData.pData = (UINT8*)UtAllocMem(PAGE_LEN);

                if (pPageStart[nTotalPageStarts - 1].PageData.pData == NULL)
                {
                    pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

                    lFreePageStarts(pPageStart, nTotalPageStarts);

                    UtFreeMem(pbyRecordData);
                    UtFreeMem(pbyData);

                    return pHandleData;
                }

                FillMemory(pPageStart[nTotalPageStarts - 1].PageData.pData, PAGE_LEN, 0xFF);

                pPageStart[nTotalPageStarts - 1].pbDataSet = (UINT8*)UtAllocMem(PAGE_LEN);

                if (pPageStart[nTotalPageStarts - 1].pbDataSet == NULL)
                {
                    pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

                    lFreePageStarts(pPageStart, nTotalPageStarts);

                    UtFreeMem(pbyRecordData);
                    UtFreeMem(pbyData);

                    return pHandleData;
                }

                FillMemory(pPageStart[nTotalPageStarts - 1].pbDataSet, PAGE_LEN, 0x00);
            }

            CopyMemory(pPageStart[nTotalPageStarts - 1].PageData.pData + nFieldAddress, pbyRecordData, nFieldByteCount);

            FillMemory(pPageStart[nTotalPageStarts - 1].pbDataSet + nFieldAddress, nFieldByteCount, 0x01);

            if (pPageStart[nTotalPageStarts - 1].PageData.nLength < nFieldAddress ||
                (UINT32)nFieldAddress + (UINT32)nFieldByteCount > pPageStart[nTotalPageStarts - 1].PageData.nLength)
            {
                pPageStart[nTotalPageStarts - 1].PageData.nLength = nFieldAddress + nFieldByteCount;
            }
        }
        else if (RecordType == ertEndOfFile)
        {
            // No more data to parse

            bQuit = TRUE;
        }
        else if (RecordType == ertExtendedSegmentAddress)
        {
            pHandleData->nErrorCode = UtIntelHexErrorCodeUnsupportedRecordType;
            pHandleData->nErrorLineNumber = nLineNumber;

            lFreePageStarts(pPageStart, nTotalPageStarts);

            UtFreeMem(pbyRecordData);
            UtFreeMem(pbyData);

            return pHandleData;
        }
        else if (RecordType == ertStartSegmentAddress)
        {
            pHandleData->nErrorCode = UtIntelHexErrorCodeUnsupportedRecordType;
            pHandleData->nErrorLineNumber = nLineNumber;

            lFreePageStarts(pPageStart, nTotalPageStarts);

            UtFreeMem(pbyRecordData);
            UtFreeMem(pbyData);

            return pHandleData;
        }
        else if (RecordType == ertExtendedLinearAddress)
        {
            if (nFieldByteCount != 2)
            {
                pHandleData->nErrorCode = UtIntelHexErrorCodeInvalidFieldByteCount;
                pHandleData->nErrorLineNumber = nLineNumber;

                lFreePageStarts(pPageStart, nTotalPageStarts);

                UtFreeMem(pbyRecordData);
                UtFreeMem(pbyData);

                return pHandleData;
            }

            bAddPageStart = FALSE;

            if (LastRecordType == ertExtendedLinearAddress ||
                LastRecordType == ertData)
            {
                nPageAddress = ((UINT32)pbyRecordData[0] << 24) |
                               ((UINT32)pbyRecordData[1] << 16);

                if (!lDoesPageAddressExist(nPageAddress, pPageStart, nTotalPageStarts))
                {
                    bAddPageStart = TRUE;
                }
            }
            else if (LastRecordType == ertUndefined)
            {
                nPageAddress = ((UINT32)pbyRecordData[0] << 24) |
                               ((UINT32)pbyRecordData[1] << 16);

                bAddPageStart = TRUE;
            }
            else
            {
                pHandleData->nErrorCode = UtIntelHexErrorCodeInvalidRecordType;
                pHandleData->nErrorLineNumber = nLineNumber;

                lFreePageStarts(pPageStart, nTotalPageStarts);

                UtFreeMem(pbyRecordData);
                UtFreeMem(pbyData);

                return pHandleData;
            }

            if (bAddPageStart)
            {
                if (pPageStart[nTotalPageStarts - 1].PageData.nLength > 0)
                {
                    pTempPageStart = lAllocPageStarts(pPageStart, nTotalPageStarts + 1);

                    if (pTempPageStart == NULL)
                    {
                        pHandleData->nErrorCode = UtIntelHexErrorCodeOutOfMemory;

                        lFreePageStarts(pPageStart, nTotalPageStarts);

                        UtFreeMem(pbyRecordData);
                        UtFreeMem(pbyData);

                        return pHandleData;
                    }

                    pPageStart = pTempPageStart;

                    ++nTotalPageStarts;
                }

                pPageStart[nTotalPageStarts - 1].PageData.nAddress = nPageAddress;
            }
        }
        else if (RecordType == ertStartLinearAddress)
        {
            pHandleData->nErrorCode = UtIntelHexErrorCodeUnsupportedRecordType;
            pHandleData->nErrorLineNumber = nLineNumber;

            lFreePageStarts(pPageStart, nTotalPageStarts);

            UtFreeMem(pbyRecordData);
            UtFreeMem(pbyData);

            return pHandleData;
        }
        else
        {
            pHandleData->nErrorCode = UtIntelHexErrorCodeUnsupportedRecordType;
            pHandleData->nErrorLineNumber = nLineNumber;

            lFreePageStarts(pPageStart, nTotalPageStarts);

            UtFreeMem(pbyRecordData);
            UtFreeMem(pbyData);

            return pHandleData;
        }

        UtFreeMem(pbyRecordData);

        pszDataPos = pszRecordEnd + 1;

        dwTempDataLen = dwDataLen - (DWORD)((LPBYTE)pszDataPos - pbyData);

        ++nLineNumber;

        LastRecordType = RecordType;
    }

    pHandleData = lTranslatePageStartToHandleData(pPageStart, nTotalPageStarts, pHandleData);

    lFreePageStarts(pPageStart, nTotalPageStarts);

    UtFreeMem(pbyData);

    return pHandleData;
}

BOOL UTINTELHEXAPI UtIntelHexFreeData(
  TUtIntelHexHandle handle)
{
    THandleData* pHandleData = (THandleData*)handle;

    for (UINT32 nIndex = 0; nIndex < pHandleData->nTotalPages; ++nIndex)
    {
        UtFreeMem(pHandleData->PageData[nIndex].pData);
    }

    UtFreeMem(pHandleData);

    return TRUE;
}

BOOL UTINTELHEXAPI UtIntelHexTotalPages(
  TUtIntelHexHandle handle,
  UINT32* pnTotalPages)
{
    THandleData* pHandleData = (THandleData*)handle;

    *pnTotalPages = pHandleData->nTotalPages;

    return TRUE;
}

BOOL UTINTELHEXAPI UtIntelHexGetPageAddress(
  TUtIntelHexHandle handle,
  UINT32 nPageNumber,
  UINT32* pnAddress)
{
    THandleData* pHandleData = (THandleData*)handle;

    *pnAddress = 0;

    if (nPageNumber >= pHandleData->nTotalPages)
    {
        return FALSE;
    }

    *pnAddress = pHandleData->PageData[nPageNumber].nAddress;

    return TRUE;
}

BOOL UTINTELHEXAPI UtIntelHexGetPageAddressData(
  TUtIntelHexHandle handle,
  UINT32 nPageNumber,
  const UINT8** ppData,
  UINT32* pnDataLen)
{
    THandleData* pHandleData = (THandleData*)handle;

    *ppData = NULL;
    *pnDataLen = 0;

    if (nPageNumber >= pHandleData->nTotalPages)
    {
        return FALSE;
    }

    *ppData = pHandleData->PageData[nPageNumber].pData;
    *pnDataLen = pHandleData->PageData[nPageNumber].nLength;

    return TRUE;
}

BOOL UTINTELHEXAPI UtIntelHexErrorLineNumber(
  TUtIntelHexHandle handle,
  UINT32* pnLineNumber)
{
    THandleData* pHandleData = (THandleData*)handle;

    *pnLineNumber = pHandleData->nErrorLineNumber;

    return TRUE;
}

BOOL UTINTELHEXAPI UtIntelHexErrorCode(
  TUtIntelHexHandle handle,
  UINT16* pnErrorCode)
{
    THandleData* pHandleData = (THandleData*)handle;

    *pnErrorCode = pHandleData->nErrorCode;

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
