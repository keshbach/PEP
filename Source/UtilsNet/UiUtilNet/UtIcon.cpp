/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UtIcon.h"

#include <Utils/UtHeap.h>

#pragma unmanaged

#pragma region "Local Functions"

static UINT32 lCalc1Plane1BitBitmapWidthBytes(
  _In_ UINT32 nWidth)
{
    UINT32 nBytesPerRow = 1;

    while ((nBytesPerRow * 8) < nWidth)
    {
        ++nBytesPerRow;
    }

    return nBytesPerRow;
}

static UINT8* lAlloc1Plane1BitBitmap(
  _In_ UINT32 nWidth,
  _In_ UINT32 nHeight)
{
    UINT32 nDataLen = lCalc1Plane1BitBitmapWidthBytes(nWidth) * nHeight;
    UINT8* pData = (UINT8*)UtAllocMem(nDataLen);

    if (pData)
    {
        ::ZeroMemory(pData, nDataLen);
    }

    return pData;
}

static UINT8* lAlloc1Plane32BitBitmap(
  _In_ UINT32 nWidth,
  _In_ UINT32 nHeight)
{
    UINT32 nDataLen = (nWidth * 4) * nHeight;
    UINT8* pData = (UINT8*)UtAllocMem(nDataLen);

    if (pData)
    {
        ::ZeroMemory(pData, nDataLen);
    }

    return pData;
}

static VOID lGetColor1Plane1Bit(
  _In_ const UINT8* pData,
  _In_ UINT32 nRow,
  _In_ UINT32 nColumn,
  _In_ UINT32 nWidthBytes,
  _In_reads_bytes_(sizeof(RGBQUAD)) LPRGBQUAD pQuad)
{
    UINT32 nRowIndex = nRow * nWidthBytes;
    UINT32 nColumnIndex = nColumn / 8;
    UINT8 Byte = pData[nRowIndex + nColumnIndex];
    UINT8 Bit = 0x01 << (nColumn - (nColumnIndex * 8));

    pQuad->rgbReserved = 0xFF;

    if (Byte & Bit)
    {
        pQuad->rgbRed = 0xFF;
        pQuad->rgbGreen = 0xFF;
        pQuad->rgbBlue = 0xFF;
    }
    else
    {
        pQuad->rgbRed = 0x00;
        pQuad->rgbGreen = 0x00;
        pQuad->rgbBlue = 0x00;
    }
}

static VOID lGetColor1Plane32Bit(
  _In_ const UINT8* pData,
  _In_ UINT32 nRow,
  _In_ UINT32 nColumn,
  _In_ UINT32 nWidthBytes,
  _In_reads_bytes_(sizeof(RGBQUAD)) LPRGBQUAD pQuad)
{
    UINT32 nRowIndex = nRow * nWidthBytes;
    UINT32 nColumnIndex = nColumn * 4;

    ::CopyMemory(pQuad, pData + (nRowIndex + nColumnIndex), sizeof(RGBQUAD));
}

static VOID lSetColor1Plane1Bit(
  _Out_writes_to_ptr_(pData) UINT8* pData,
  _In_ UINT32 nRow,
  _In_ UINT32 nColumn,
  _In_ UINT32 nWidthBytes,
  _In_ LPRGBQUAD pQuad)
{
    UINT32 nRowIndex = nRow * nWidthBytes;
    UINT32 nColumnIndex = nColumn / 8;
    UINT8 Bit = 0x01 << (nColumn - (nColumnIndex * 8));

    if (pQuad->rgbRed > 0 || pQuad->rgbGreen > 0 || pQuad->rgbBlue > 0)
    {
        pData[nRowIndex + nColumnIndex] |= Bit;
    }
    else
    {
        pData[nRowIndex + nColumnIndex] &= ~Bit;
    }
}

static VOID lSetColor1Plane32Bit(
  _Out_writes_to_ptr_(pData) UINT8* pData,
  _In_ UINT32 nRow,
  _In_ UINT32 nColumn,
  _In_ UINT32 nWidthBytes,
  _In_ LPRGBQUAD pQuad)
{
    UINT32 nRowIndex = nRow * nWidthBytes;
    UINT32 nColumnIndex = nColumn * 4;

    ::CopyMemory(pData + (nRowIndex + nColumnIndex), pQuad, sizeof(RGBQUAD));
}

static UINT8* lCreateIconMaskData(
  _In_ HBITMAP hMaskBitmap,
  _In_ UINT32 nLeftBorderWidth,
  _In_ UINT32 nTopBorderHeight,
  _In_ UINT32 nRightBorderWidth,
  _In_ UINT32 nBottomBorderHeight)
{
    UINT8* pNewData;
    UINT8* pMaskData;
    BITMAP Bitmap;
    RGBQUAD Quad;
    LONG nMaskDataLen;
    UINT32 nNewBitmapWidthBytes;

    if (0 == ::GetObject(hMaskBitmap, sizeof(Bitmap), &Bitmap))
    {
        return NULL;
    }

    nNewBitmapWidthBytes = lCalc1Plane1BitBitmapWidthBytes(Bitmap.bmWidth + (nLeftBorderWidth + nRightBorderWidth));

    pNewData = lAlloc1Plane1BitBitmap(Bitmap.bmWidth + (nLeftBorderWidth + nRightBorderWidth),
                                      Bitmap.bmHeight + (nTopBorderHeight + nBottomBorderHeight));

    if (pNewData == NULL)
    {
        return NULL;
    }

    nMaskDataLen = Bitmap.bmHeight * Bitmap.bmWidthBytes;

    pMaskData = (UINT8*)UtAllocMem(nMaskDataLen);

    if (pMaskData == NULL)
    {
        UtFreeMem(pNewData);

        return NULL;
    }

    if (nMaskDataLen != ::GetBitmapBits(hMaskBitmap, nMaskDataLen, pMaskData))
    {
        UtFreeMem(pMaskData);
        UtFreeMem(pNewData);

        return NULL;
    }

    for (LONG nRow = 0; nRow < Bitmap.bmHeight; ++nRow)
    {
        for (LONG nColumn = 0; nColumn < Bitmap.bmWidth; ++nColumn)
        {
            lGetColor1Plane1Bit(pMaskData, nRow, nColumn, Bitmap.bmWidthBytes, &Quad);

            lSetColor1Plane1Bit(pNewData, nRow + nTopBorderHeight,
                                nColumn + nLeftBorderWidth,
                                nNewBitmapWidthBytes, &Quad);
        }
    }

    UtFreeMem(pMaskData);

    return pNewData;
}

static UINT8* lCreate1Plane32BitIconColorData(
  _In_ HBITMAP hColorBitmap,
  _In_ LPBITMAP pBitmap,
  _In_ UINT32 nLeftBorderWidth,
  _In_ UINT32 nTopBorderHeight,
  _In_ UINT32 nRightBorderWidth,
  _In_ UINT32 nBottomBorderHeight)
{
    UINT8* pNewData;
    UINT8* pColorData;
    RGBQUAD Quad;
    LONG nColorDataLen;
    UINT32 nNewBitmapWidthBytes;

    nColorDataLen = pBitmap->bmWidthBytes * pBitmap->bmHeight;
    nNewBitmapWidthBytes = pBitmap->bmWidthBytes + ((nLeftBorderWidth + nRightBorderWidth) * 4);

    pColorData = (UINT8*)UtAllocMem(nColorDataLen);

    if (pColorData == NULL)
    {
        return NULL;
    }

    if (nColorDataLen != ::GetBitmapBits(hColorBitmap, nColorDataLen, pColorData))
    {
        UtFreeMem(pColorData);

        return NULL;
    }

    pNewData = lAlloc1Plane32BitBitmap(pBitmap->bmWidth + (nLeftBorderWidth + nRightBorderWidth),
                                       pBitmap->bmHeight + (nTopBorderHeight + nBottomBorderHeight));

    if (pNewData == NULL)
    {
        UtFreeMem(pColorData);

        return NULL;
    }

    for (LONG nRow = 0; nRow < pBitmap->bmHeight; ++nRow)
    {
        for (LONG nColumn = 0; nColumn < pBitmap->bmWidth; ++nColumn)
        {
            lGetColor1Plane32Bit(pColorData, nRow, nColumn, pBitmap->bmWidthBytes, &Quad);

            lSetColor1Plane32Bit(pNewData, nRow + nTopBorderHeight,
                                 nColumn + nLeftBorderWidth,
                                 nNewBitmapWidthBytes, &Quad);
        }
    }

    UtFreeMem(pColorData);

    return pNewData;
}

static UINT8* lCreateIconColorData(
  _In_ HBITMAP hColorBitmap,
  _In_ UINT32 nLeftBorderWidth,
  _In_ UINT32 nTopBorderHeight,
  _In_ UINT32 nRightBorderWidth,
  _In_ UINT32 nBottomBorderHeight)
{
    UINT8* pNewData = NULL;
    BITMAP Bitmap;

    if (0 == ::GetObject(hColorBitmap, sizeof(Bitmap), &Bitmap))
    {
        return NULL;
    }

    switch (Bitmap.bmBitsPixel)
    {
        case 32:
            pNewData = lCreate1Plane32BitIconColorData(hColorBitmap, &Bitmap,
                                                       nLeftBorderWidth, nTopBorderHeight,
                                                       nRightBorderWidth, nBottomBorderHeight);
            break;
    }

    return pNewData;
}

#pragma endregion

HICON UtIconAddBorder(
  _In_ HICON hIcon,
  _In_ UINT32 nLeftBorderWidth,
  _In_ UINT32 nTopBorderHeight,
  _In_ UINT32 nRightBorderWidth,
  _In_ UINT32 nBottomBorderHeight)
{
	ICONINFO IconInfo;
    BITMAP Bitmap;
    UINT8* pIconMaskData;
    UINT8* pIconColorData;
    HBITMAP hMaskBitmap, hColorBitmap;
    HICON hNewIcon;
    INT nNewWidth, nNewHeight;

    if (FALSE == ::GetIconInfo(hIcon, &IconInfo) ||
        0 == ::GetObject(IconInfo.hbmMask, sizeof(Bitmap), &Bitmap))
    {
        return NULL;
    }

    nNewWidth = Bitmap.bmWidth + (nLeftBorderWidth + nRightBorderWidth);
    nNewHeight = Bitmap.bmHeight + (nTopBorderHeight + nBottomBorderHeight);

    pIconMaskData = lCreateIconMaskData(IconInfo.hbmMask, nLeftBorderWidth,
                                        nTopBorderHeight, nRightBorderWidth,
                                        nBottomBorderHeight);

    if (pIconMaskData == NULL)
    {
        return NULL;
    }

    pIconColorData = lCreateIconColorData(IconInfo.hbmColor, nLeftBorderWidth,
                                          nTopBorderHeight, nRightBorderWidth,
                                          nBottomBorderHeight);

    if (pIconColorData == NULL)
    {
        UtFreeMem(pIconMaskData);

        return NULL;
    }

    hMaskBitmap = ::CreateBitmap(nNewWidth, nNewHeight, 1, 1, pIconMaskData);

    if (hMaskBitmap == NULL)
    {
        UtFreeMem(pIconColorData);
        UtFreeMem(pIconMaskData);

        return NULL;
    }

    hColorBitmap = ::CreateBitmap(nNewWidth, nNewHeight, 1, 32, pIconColorData);

    if (hMaskBitmap == NULL)
    {
        ::DeleteObject(hMaskBitmap);

        UtFreeMem(pIconColorData);
        UtFreeMem(pIconMaskData);

        return NULL;
    }

    UtFreeMem(pIconColorData);
    UtFreeMem(pIconMaskData);

    IconInfo.fIcon = TRUE;
    IconInfo.xHotspot = 0;
    IconInfo.yHotspot = 0;
    IconInfo.hbmMask = hMaskBitmap;
    IconInfo.hbmColor = hColorBitmap;

    hNewIcon = ::CreateIconIndirect(&IconInfo);

    if (FALSE == ::DeleteObject(hMaskBitmap))
    {
#if !defined(NDEBUG)
        ::OutputDebugStringW(L"*** Warning failed to delete icon mask bitmap ***");
#endif
    }

    if (FALSE == ::DeleteObject(hColorBitmap))
    {
#if !defined(NDEBUG)
        ::OutputDebugStringW(L"*** Warning failed to delete icon color bitmap ***");
#endif
    }

	return hNewIcon;
}

#pragma managed

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
