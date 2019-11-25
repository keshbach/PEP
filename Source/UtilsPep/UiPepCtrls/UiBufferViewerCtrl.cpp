/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsPep/UiPepCtrls.h>

#include "UiPepCtrlsUtil.h"

#include <Utils/UtHeap.h>

#include <Includes/UtMacros.h>

#include <strsafe.h>
#include <windowsx.h>
#include <uxtheme.h>
#include <vssym32.h>

#include <assert.h>

#pragma region "Constants"

#define CFontName L"Courier New"

#define CPtSizesPerInch 72

#define MPtSizeToTwips(nPtSize) (nPtSize * 20)
#define MTwipsToPtSize(nTwipsSize) (nTwipsSize / 20)

#define CBufferMarginDevWidth 1

#define CAddressTotalChars 8
#define CAddressToDataTotalChars 2

#define CByteTotalChars 2
#define CWordTotalChars 4

#define CAddressFormat L"%08X"
#define CByteFormat L"%02X"
#define CWordFormat L"%04X"

#define CThemeClassName L"EditStyle;Edit"

#pragma endregion

#pragma region "Structures"

typedef struct tagTBufferViewerCtrlData
{
    LPBYTE pbyBuffer;
    ULONG nBufferLen;
    INT nDataOrganization;
    INT nFontPtSize;
    SIZE CharTwipsSize;
    SIZE CharDevSize;
    INT nTotalVisibleRows;
    INT nTotalVisibleColumns;
    INT nTotalRows;
    BOOL bRedraw;
    ULONG nStartAddress;
    HTHEME hTheme;
} TBufferViewerCtrlData;

#pragma endregion

static HFONT lCreateFont(
  int nFontPtSize)
{
    LOGFONTW LogFont;

    LogFont.lfHeight         = MPtSizeToTwips(nFontPtSize) * -1;
    LogFont.lfWidth          = 0;
    LogFont.lfEscapement     = 0;
    LogFont.lfOrientation    = 0;
    LogFont.lfWeight         = FW_NORMAL;
    LogFont.lfItalic         = FALSE;
    LogFont.lfUnderline      = FALSE;
    LogFont.lfStrikeOut      = FALSE;
    LogFont.lfCharSet        = ANSI_CHARSET;
    LogFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    LogFont.lfQuality        = PROOF_QUALITY;
    LogFont.lfPitchAndFamily = FF_MODERN;
    LogFont.lfOutPrecision   = OUT_TT_PRECIS;

    ::StringCchCopyW(LogFont.lfFaceName, MArrayLen(LogFont.lfFaceName), CFontName);

    return ::CreateFontIndirectW(&LogFont);
}

static VOID lSelectTwipsMode(
  HDC hDC)
{
    ::SetMapMode(hDC, MM_ANISOTROPIC);

    ::SetWindowExtEx(hDC, MPtSizeToTwips(CPtSizesPerInch),
                     MPtSizeToTwips(CPtSizesPerInch), NULL);

    ::SetViewportExtEx(hDC, ::GetDeviceCaps(hDC, LOGPIXELSX),
                       ::GetDeviceCaps(hDC, LOGPIXELSY), NULL);

    ::SetWindowOrgEx(hDC, 0, 0, NULL);

    ::SetViewportOrgEx(hDC, 0, 0, NULL);
}

static void lSelectDevUnitsMode(
  HDC hDC)
{
    ::SetMapMode(hDC, MM_TEXT);

    ::SetWindowOrgEx(hDC, 0, 0, NULL);
    ::SetViewportOrgEx(hDC, 0, 0, NULL);
}

static VOID lGetCharSizes(
  HWND hWnd,
  INT nPtSize,
  LPSIZE pTwipsSize,
  LPSIZE pDevSize)
{
    HDC hDC = ::GetDC(hWnd);
    HFONT hFont = lCreateFont(nPtSize);
    POINT Point;

    ::SaveDC(hDC);

    lSelectTwipsMode(hDC);

    ::SelectObject(hDC, hFont);

    ::GetTextExtentPoint32W(hDC, L"0", 1, pTwipsSize);

    Point.x = pTwipsSize->cx;
    Point.y = pTwipsSize->cy;

    ::LPtoDP(hDC, &Point, 1);

    pDevSize->cx = Point.x;
    pDevSize->cy = Point.y;

    ::RestoreDC(hDC, -1);

    ::DeleteObject(hFont);

    ::ReleaseDC(hWnd, hDC);
}

static VOID lThemeFillSolidRect(
  HDC hDC,
  HBRUSH hBrush,
  INT nXPos,
  INT nYPos,
  INT nWidth,
  INT nHeight)
{
	RECT Rect;

	Rect.left = nXPos;
	Rect.top = nYPos;
	Rect.right = nXPos + nWidth;
	Rect.bottom = nYPos + nHeight;

	::FillRect(hDC, &Rect, hBrush);
}

static VOID lFillSolidRect(
    HDC hDC,
    COLORREF crBack,
    INT nXPos,
    INT nYPos,
    INT nWidth,
    INT nHeight)
{
    HBRUSH hBrush = ::CreateSolidBrush(crBack);
    RECT Rect;

    Rect.left = nXPos;
    Rect.top = nYPos;
    Rect.right = nXPos + nWidth;
    Rect.bottom = nYPos + nHeight;

    ::FillRect(hDC, &Rect, hBrush);

    ::DeleteObject(hBrush);
}

static VOID lDrawAddressForRow(
  HDC hDC,
  COLORREF crText,
  COLORREF crBack,
  INT nAddress,
  INT nXPos,
  INT nYPos)
{
    WCHAR cBuffer[CAddressTotalChars + 1];

    ::StringCchPrintfW(cBuffer, MArrayLen(cBuffer), CAddressFormat, nAddress);

	::SetTextColor(hDC, crText);
    ::SetBkColor(hDC, crBack);

    ::TextOutW(hDC, nXPos, nYPos, cBuffer, ::lstrlenW(cBuffer));
}

static UINT lCalcDataPerRow(
  UINT nTotalColumns,
  INT nDataOrganization)
{
    LONGLONG nDataPerRow(0);

	nTotalColumns -= (CAddressTotalChars + CAddressToDataTotalChars);

	// Exclude the address

    if (nTotalColumns < 1)
    {
        return 1;
    }

    switch (nDataOrganization)
    {
        case CBufferViewerAsciiData:
            // 00000000 ABCDDCBA

		    nDataPerRow = (LONGLONG)nTotalColumns;
            break;
        case CBufferViewerByteData:
            // 00000000 AB CD DC BA

		    nDataPerRow = (LONGLONG)nTotalColumns / 3;

            if (nTotalColumns - (nDataPerRow * 3) == 2)
            {
                ++nDataPerRow;
            }
            break;
        case CBufferViewerWordBigEndianData:
        case CBufferViewerWordLittleEndianData:
    		// 00000000 ABCD DCBA

		    nDataPerRow = (LONGLONG)nTotalColumns / 5;

            if (nTotalColumns - (nDataPerRow * 5) == 4)
            {
                ++nDataPerRow;
            }
            break;
        default:
            assert(false);
            break;
    }

	if (nDataPerRow <= 0)
	{
		nDataPerRow = 1;
	}

	return (UINT)nDataPerRow;
}

static INT lCalcTotalAddresses(
  ULONG nBufferLen,
  INT nDataOrganization)
{
    switch (nDataOrganization)
    {
        case CBufferViewerAsciiData:
        case CBufferViewerByteData:
            return nBufferLen;
        case CBufferViewerWordBigEndianData:
        case CBufferViewerWordLittleEndianData:
            return nBufferLen / 2;
        default:
            assert(false);
            break;
    }

    return nBufferLen;
}

static LPBYTE lIncrementBuffer(
  LPBYTE pbyBuffer,
  ULONG nBufferLen,
  INT nIncrement,
  INT nDataOrganization)
{
    while (nBufferLen > 0 && nIncrement > 0)
    {
        switch (nDataOrganization)
        {
            case CBufferViewerAsciiData:
            case CBufferViewerByteData:
                nBufferLen -= 1;
                pbyBuffer += 1;
                break;
            case CBufferViewerWordBigEndianData:
            case CBufferViewerWordLittleEndianData:
                nBufferLen -= 2;
                pbyBuffer += 2;
                break;
            default:
                assert(false);
                break;
        }

        --nIncrement;
    }

    return pbyBuffer;
}

static LPWSTR lAllocBufferAsciiOrganization(
  LPBYTE pbyBuffer,
  ULONG nBufferLen,
  UINT nTotalData)
{
    LPWSTR pszNewBuffer;
    UINT nNewBufferLen;
    BYTE byData;

    pszNewBuffer = (LPWSTR)UtAllocMem(sizeof(WCHAR));
    nNewBufferLen = 1;

    while (nBufferLen > 0 && nTotalData > 0)
    {
        byData = *pbyBuffer;

        if (byData < 32 || byData > 127)
        {
            byData = '.';
        }

        ++nNewBufferLen;

        pszNewBuffer = (LPWSTR)UtReAllocMem(pszNewBuffer, nNewBufferLen * sizeof(WCHAR));

        *(pszNewBuffer + (nNewBufferLen - 2)) = byData;

        ++pbyBuffer;

        --nBufferLen;
        --nTotalData;
    }

    *(pszNewBuffer + (nNewBufferLen - 1)) = 0;

    return pszNewBuffer;
}

static LPWSTR lAllocBufferByteOrganization(
  LPBYTE pbyBuffer,
  ULONG nBufferLen,
  UINT nTotalData)
{
    LPWSTR pszNewBuffer;
    UINT nNewBufferLen;
    WCHAR cBuffer[CByteTotalChars + 1];

    pszNewBuffer = (LPWSTR)UtAllocMem(sizeof(WCHAR));
    nNewBufferLen = 1;

    while (nBufferLen > 0 && nTotalData > 0)
    {
        if (nNewBufferLen > 1)
        {
            ++nNewBufferLen;

            pszNewBuffer = (LPWSTR)UtReAllocMem(pszNewBuffer, nNewBufferLen * sizeof(WCHAR));

            *(pszNewBuffer + (nNewBufferLen - CByteTotalChars)) = L' ';
        }

        ::StringCchPrintfW(cBuffer, MArrayLen(cBuffer), CByteFormat, (INT)*pbyBuffer);

        nNewBufferLen += CByteTotalChars;

        pszNewBuffer = (LPWSTR)UtReAllocMem(pszNewBuffer, nNewBufferLen * sizeof(WCHAR));

        ::StringCchCopy(pszNewBuffer + (nNewBufferLen - (CByteTotalChars + 1)),
                        MArrayLen(cBuffer), cBuffer);

        ++pbyBuffer;

        --nBufferLen;
        --nTotalData;
    }

    *(pszNewBuffer + (nNewBufferLen - 1)) = 0;

    return pszNewBuffer;
}

static LPWSTR lAllocBufferWordBigEndianOrganization(
  LPBYTE pbyBuffer,
  ULONG nBufferLen,
  UINT nTotalData)
{
    LPWSTR pszNewBuffer;
    UINT nNewBufferLen;
    WCHAR cBuffer[CWordTotalChars + 1];
    INT nTmpValue;

    pszNewBuffer = (LPWSTR)UtAllocMem(sizeof(WCHAR));
    nNewBufferLen = 1;

    while (nBufferLen > 0 && nTotalData > 0)
    {
        if (nNewBufferLen > 1)
        {
            ++nNewBufferLen;

            pszNewBuffer = (LPWSTR)UtReAllocMem(pszNewBuffer, nNewBufferLen * sizeof(WCHAR));

            *(pszNewBuffer + (nNewBufferLen - 2)) = L' ';
        }

        nTmpValue = (INT)*pbyBuffer << 8;
        nTmpValue |= (INT)*(pbyBuffer + 1);

        ::StringCchPrintfW(cBuffer, MArrayLen(cBuffer), CWordFormat, nTmpValue);

        nNewBufferLen += CWordTotalChars;

        pszNewBuffer = (LPWSTR)UtReAllocMem(pszNewBuffer, nNewBufferLen * sizeof(WCHAR));

        ::StringCchCopy(pszNewBuffer + (nNewBufferLen - (CWordTotalChars + 1)),
                        MArrayLen(cBuffer), cBuffer);

        pbyBuffer += 2;

        nBufferLen -= 2;
        --nTotalData;
    }

    *(pszNewBuffer + (nNewBufferLen - 1)) = 0;

    return pszNewBuffer;
}

static LPWSTR lAllocBufferWordLittleEndianOrganization(
  LPBYTE pbyBuffer,
  ULONG nBufferLen,
  UINT nTotalData)
{
    LPWSTR pszNewBuffer;
    UINT nNewBufferLen;
    WCHAR cBuffer[CWordTotalChars + 1];
    INT nTmpValue;

    pszNewBuffer = (LPWSTR)UtAllocMem(sizeof(WCHAR));
    nNewBufferLen = 1;

    while (nBufferLen > 0 && nTotalData > 0)
    {
        if (nNewBufferLen > 1)
        {
            ++nNewBufferLen;

            pszNewBuffer = (LPWSTR)UtReAllocMem(pszNewBuffer, nNewBufferLen * sizeof(WCHAR));

            *(pszNewBuffer + (nNewBufferLen - 2)) = L' ';
        }

        nTmpValue = (INT)*pbyBuffer;
        nTmpValue |= (INT)*(pbyBuffer + 1) << 8;

        ::StringCchPrintfW(cBuffer, MArrayLen(cBuffer), CWordFormat, nTmpValue);

        nNewBufferLen += CWordTotalChars;

        pszNewBuffer = (LPWSTR)UtReAllocMem(pszNewBuffer, nNewBufferLen * sizeof(WCHAR));

        ::StringCchCopy(pszNewBuffer + (nNewBufferLen - (CWordTotalChars + 1)),
                        MArrayLen(cBuffer), cBuffer);

        pbyBuffer += 2;

        nBufferLen -= 2;
        --nTotalData;
    }

    *(pszNewBuffer + (nNewBufferLen - 1)) = 0;

    return pszNewBuffer;
}

static VOID lCalcNewTotalRowsAndColumns(
  TBufferViewerCtrlData* pData,
  INT nNewWidth,
  INT nNewHeight)
{
    pData->nTotalVisibleRows = nNewHeight / pData->CharDevSize.cy;
    pData->nTotalVisibleColumns = (nNewWidth - CBufferMarginDevWidth) /
                                      pData->CharDevSize.cx;

    if (pData->nTotalVisibleColumns < 0)
    {
        pData->nTotalVisibleColumns = 0;
    }
}

static VOID lUpdateScrollBars(
  HWND hWnd)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    UINT nTotalAddresses, nDataPerRow, nTotalRows;
    SCROLLINFO ScrollInfo;

    nTotalAddresses = lCalcTotalAddresses(pData->nBufferLen,
                                          pData->nDataOrganization);

    nDataPerRow = lCalcDataPerRow(pData->nTotalVisibleColumns,
                                  pData->nDataOrganization);

    nTotalRows = nTotalAddresses / nDataPerRow;

    if (nTotalRows * nDataPerRow < nTotalAddresses)
    {
        ++nTotalRows;
    }

    pData->nTotalRows = nTotalRows;
    pData->nStartAddress = 0;

    if (pData->nTotalVisibleRows < pData->nTotalRows)
    {
        ::EnableScrollBar(hWnd, SB_VERT, ESB_ENABLE_BOTH);

        ScrollInfo.cbSize = sizeof(ScrollInfo);
        ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
        ScrollInfo.nMin = 0;
        ScrollInfo.nMax = nTotalRows - 1;
        ScrollInfo.nPage = pData->nTotalVisibleRows;
        ScrollInfo.nPos = 0;

        ::SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);
    }
    else
    {
        ::EnableScrollBar(hWnd, SB_VERT, ESB_DISABLE_BOTH);
    }
}

static VOID lDrawWindow(
  HWND hWnd,
  HDC hDC)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HFONT hFont = lCreateFont(pData->nFontPtSize);
    UINT nTotalRows, nDataPerRow, nCurAddress, nTotalAddresses;
    ULONG nBufferLen;
    POINT Point[2];
	INT nXPos, nYPos;
    RECT ClientRect;
    LPWSTR pszTmpBuffer;
    LPBYTE pbyBuffer;

    ::GetClientRect(hWnd, &ClientRect);

    ::SaveDC(hDC);

    // Draw the margin

    lSelectDevUnitsMode(hDC);

    if (pData->hTheme)
    {
        lThemeFillSolidRect(hDC, ::GetThemeSysColorBrush(pData->hTheme, TMT_BTNFACE), 0, 0,
                            CBufferMarginDevWidth, ClientRect.bottom);
    }
    else
    {
        lFillSolidRect(hDC, ::GetSysColor(COLOR_BTNFACE), 0, 0,
                       CBufferMarginDevWidth, ClientRect.bottom);
    }


    // Draw the buffer text

    lSelectTwipsMode(hDC);

    ::SelectObject(hDC, hFont);

    ::SetBkMode(hDC, OPAQUE);

    Point[0].x = CBufferMarginDevWidth;
    Point[0].y = 0;

    Point[1].x = ClientRect.right;
    Point[1].y = ClientRect.bottom;

    ::DPtoLP(hDC, Point, MArrayLen(Point));

    pbyBuffer = pData->pbyBuffer;
    nBufferLen = pData->nBufferLen;
    nTotalRows = pData->nTotalVisibleRows + 1;
    nCurAddress = pData->nStartAddress;
    nTotalAddresses = lCalcTotalAddresses(pData->nBufferLen,
                                          pData->nDataOrganization);
	nDataPerRow = lCalcDataPerRow(pData->nTotalVisibleColumns,
                                  pData->nDataOrganization);
	nYPos = 0;

    pbyBuffer = lIncrementBuffer(pbyBuffer, nBufferLen, nCurAddress,
                                 pData->nDataOrganization);

    nBufferLen -= (ULONG)(pbyBuffer - pData->pbyBuffer);

	for (UINT nCurRow = 0; nCurRow < nTotalRows; ++nCurRow)
    {
	    nXPos = Point[0].x;

        // Draw the address

        if (nCurAddress < nTotalAddresses)
        {
		    lDrawAddressForRow(hDC, ::GetSysColor(COLOR_BTNTEXT),
			                   ::GetSysColor(COLOR_BTNFACE), nCurAddress, nXPos, nYPos);
        }
        else
        {
            lFillSolidRect(hDC, ::GetSysColor(COLOR_BTNFACE),  nXPos, nYPos,
                           pData->CharTwipsSize.cx * CAddressTotalChars,
                           pData->CharTwipsSize.cy);
        }

        nXPos += (pData->CharTwipsSize.cx * CAddressTotalChars);

        // Draw the area between the address and data

        lFillSolidRect(hDC, ::GetSysColor(COLOR_WINDOW),  nXPos, nYPos,
                       pData->CharTwipsSize.cx * CAddressToDataTotalChars,
                       pData->CharTwipsSize.cy);

        nXPos += (pData->CharTwipsSize.cx * CAddressToDataTotalChars);

        // Draw the data

        switch (pData->nDataOrganization)
        {
            case CBufferViewerAsciiData:
                pszTmpBuffer = lAllocBufferAsciiOrganization(pbyBuffer, nBufferLen, nDataPerRow);
                break;
            case CBufferViewerByteData:
                pszTmpBuffer = lAllocBufferByteOrganization(pbyBuffer, nBufferLen, nDataPerRow);
                break;
            case CBufferViewerWordBigEndianData:
                pszTmpBuffer = lAllocBufferWordBigEndianOrganization(pbyBuffer, nBufferLen, nDataPerRow);
                break;
            case CBufferViewerWordLittleEndianData:
                pszTmpBuffer = lAllocBufferWordLittleEndianOrganization(pbyBuffer, nBufferLen, nDataPerRow);
                break;
            default:
                pszTmpBuffer = NULL;

                assert(false);
                break;
        }

        ::SetTextColor(hDC, ::GetSysColor(COLOR_BTNTEXT));
		::SetBkColor(hDC, ::GetSysColor(COLOR_WINDOW));

        ::TextOutW(hDC, nXPos, nYPos, pszTmpBuffer, ::lstrlenW(pszTmpBuffer));

	    nXPos += (pData->CharTwipsSize.cx * ::lstrlenW(pszTmpBuffer));

        pbyBuffer = lIncrementBuffer(pbyBuffer, nBufferLen, nDataPerRow, pData->nDataOrganization);

        nBufferLen = pData->nBufferLen - (ULONG)(pbyBuffer - pData->pbyBuffer);

        UtFreeMem(pszTmpBuffer);

        // Draw the area after the data

        lFillSolidRect(hDC, ::GetSysColor(COLOR_WINDOW), nXPos, nYPos,
                       Point[1].x - nXPos, pData->CharTwipsSize.cy);
	
		nYPos += pData->CharTwipsSize.cy;

        if (nCurAddress < nTotalAddresses)
        {
            nCurAddress += nDataPerRow;
        }
    }

    ::RestoreDC(hDC, -1);

    ::DeleteObject(hFont);
}

static VOID lRedrawWindow(
  HWND hWnd)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pData->bRedraw)
    {
        lUpdateScrollBars(hWnd);

        ::InvalidateRect(hWnd, NULL, TRUE);
        ::UpdateWindow(hWnd);
    }
}

static LRESULT lOnGetBuffer(
  HWND hWnd,
  TUiBufferViewerMem* pBufferViewerMem)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pBufferViewerMem->pbyBuffer == NULL)
    {
        pBufferViewerMem->nBufferLen = pData->nBufferLen;

        return TRUE;
    }

    if (pBufferViewerMem->nBufferLen < pData->nBufferLen)
    {
        return FALSE;
    }

    ::CopyMemory((LPBYTE)pBufferViewerMem->pbyBuffer, pData->pbyBuffer,
                 pData->nBufferLen);

    pBufferViewerMem->nBufferLen = pData->nBufferLen;

    return TRUE;
}

static LRESULT lOnSetBuffer(
  HWND hWnd,
  TUiBufferViewerMem* pBufferViewerMem)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    BOOL bResult = FALSE;

    UtFreeMem(pData->pbyBuffer);

    pData->pbyBuffer = (LPBYTE)UtAllocMem(pBufferViewerMem->nBufferLen);
    pData->nBufferLen = 0;

    if (pData->pbyBuffer)
    {
        pData->nBufferLen = pBufferViewerMem->nBufferLen;

        ::CopyMemory(pData->pbyBuffer, pBufferViewerMem->pbyBuffer,
                     pBufferViewerMem->nBufferLen);

        bResult = TRUE;
    }

    lRedrawWindow(hWnd);

    return bResult;
}

static LRESULT lOnGetDataOrganization(
  HWND hWnd)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    return pData->nDataOrganization;
}

static LRESULT lOnSetDataOrganization(
  HWND hWnd,
  INT nDataOrganization)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (nDataOrganization)
    {
        case CBufferViewerAsciiData:
        case CBufferViewerByteData:
        case CBufferViewerWordBigEndianData:
        case CBufferViewerWordLittleEndianData:
            break;
        default:
            return FALSE;
    }

    pData->nDataOrganization = nDataOrganization;

    lRedrawWindow(hWnd);

    return TRUE;
}

static LRESULT lOnGetFontPtSize(
  HWND hWnd)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    return pData->nFontPtSize;
}

static LRESULT lOnSetFontPtSize(
  HWND hWnd,
  INT nNewFontPtSize)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    RECT Rect;

    if (nNewFontPtSize < CBufferViewerMinPtSize ||
        nNewFontPtSize > CBufferViewwerMaxPtSize)
    {
        return FALSE;
    }

    pData->nFontPtSize = nNewFontPtSize;

    lGetCharSizes(hWnd, pData->nFontPtSize, &pData->CharTwipsSize,
                  &pData->CharDevSize);

    ::GetClientRect(hWnd, &Rect);

    lCalcNewTotalRowsAndColumns(pData, Rect.right, Rect.bottom);

    lRedrawWindow(hWnd);

    return TRUE;
}

static LRESULT lOnPaintMsg(
  HWND hWnd)
{
    HDC hDC;
    PAINTSTRUCT PaintStruct;

    hDC = ::BeginPaint(hWnd, &PaintStruct);

    lDrawWindow(hWnd, hDC);

    ::EndPaint(hWnd, &PaintStruct);

    return 0;
}

static LRESULT lOnPrintMsg(
  HWND hWnd,
  HDC hDC,
  DWORD dwDrawingOptions)
{
    dwDrawingOptions;

    lDrawWindow(hWnd, hDC);

    return 0;
}

static LRESULT lOnPrintClientMsg(
  HWND hWnd,
  HDC hDC,
  DWORD dwDrawingOptions)
{
    dwDrawingOptions;

    lDrawWindow(hWnd, hDC);

    return 0;
}

static LRESULT lOnKeyDownMsg(
  HWND hWnd,
  INT nVirtKey,
  DWORD dwData)
{
    hWnd;
    nVirtKey;
    dwData;

    return 0;
}

static LRESULT lOnLButtonDownMsg(
  HWND hWnd,
  INT nXPos,
  INT nYPos,
  DWORD dwVirtKey)
{
    hWnd;
    nXPos;
    nYPos;
    dwVirtKey;

    return 0;
}

static LRESULT lOnLButtonUpMsg(
  HWND hWnd,
  INT nXPos,
  INT nYPos,
  DWORD dwVirtKey)
{
    hWnd;
    nXPos;
    nYPos;
    dwVirtKey;

    return 0;
}

static LRESULT lOnLButtonDblClkMsg(
  HWND hWnd,
  INT nXPos,
  INT nYPos,
  DWORD dwVirtKey)
{
    hWnd;
    nXPos;
    nYPos;
    dwVirtKey;

    return 0;
}

static LRESULT lOnRButtonDownMsg(
  HWND hWnd,
  INT nXPos,
  INT nYPos,
  DWORD dwVirtKey)
{
    hWnd;
    nXPos;
    nYPos;
    dwVirtKey;

    return 0;
}

static LRESULT lOnRButtonUpMsg(
  HWND hWnd,
  INT nXPos,
  INT nYPos,
  DWORD dwVirtKey)
{
    hWnd;
    nXPos;
    nYPos;
    dwVirtKey;

    return 0;
}

static LRESULT lOnRButtonDblClkMsg(
  HWND hWnd,
  INT nXPos,
  INT nYPos,
  DWORD dwVirtKey)
{
    hWnd;
    nXPos;
    nYPos;
    dwVirtKey;

    return 0;
}

static LRESULT lOnSizeMsg(
  HWND hWnd,
  INT nNewWidth,
  INT nNewHeight)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    lCalcNewTotalRowsAndColumns(pData, nNewWidth, nNewHeight);

    lUpdateScrollBars(hWnd);

    return 0;
}

static LRESULT lOnVScrollMsg(
  HWND hWnd,
  INT nType,
  INT nPos)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    UINT nDataPerRow;
    SCROLLINFO ScrollInfo;

	nPos;

    nDataPerRow = lCalcDataPerRow(pData->nTotalVisibleColumns,
                                  pData->nDataOrganization);

    ScrollInfo.cbSize = sizeof(ScrollInfo);
    ScrollInfo.fMask = SIF_POS | SIF_TRACKPOS;

    ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

    switch (nType)
    {
        case SB_LINEDOWN:
            if (ScrollInfo.nPos + pData->nTotalVisibleRows < pData->nTotalRows)
            {
                ++ScrollInfo.nPos;
            }
            break;
        case SB_LINEUP:
            if (ScrollInfo.nPos > 0)
            {
                --ScrollInfo.nPos;
            }
            break;
        case SB_PAGEDOWN:
            ScrollInfo.nPos += pData->nTotalVisibleRows;

            if (ScrollInfo.nPos > pData->nTotalRows - pData->nTotalVisibleRows)
            {
                ScrollInfo.nPos = pData->nTotalRows - pData->nTotalVisibleRows;
            }
            break;
        case SB_PAGEUP:
            ScrollInfo.nPos -= pData->nTotalVisibleRows;

            if (ScrollInfo.nPos < 0)
            {
                ScrollInfo.nPos = 0;
            }
            break;
        case SB_THUMBTRACK:
            ScrollInfo.nPos = ScrollInfo.nTrackPos;
            break;
    }

    ::SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);

    pData->nStartAddress = (ScrollInfo.nPos * nDataPerRow);

    ::InvalidateRect(hWnd, NULL, TRUE);
    ::UpdateWindow(hWnd);

    return 0;
}

static LRESULT lOnSetRedrawMsg(
  HWND hWnd,
  BOOL bRedraw)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    pData->bRedraw = bRedraw;

    lRedrawWindow(hWnd);

    return 0;
}

static LRESULT lOnThemeChangedMsg(
  HWND hWnd)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pData->hTheme)
    {
        ::CloseThemeData(pData->hTheme);

        pData->hTheme = NULL;
    }

    if (::IsThemeActive())
    {
        pData->hTheme = ::OpenThemeData(hWnd, CThemeClassName);
    }

    return 0;
}

static LRESULT lOnCreateMsg(
  HWND hWnd,
  LPCREATESTRUCT pCreateStruct)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)UtAllocMem(sizeof(TBufferViewerCtrlData));
    RECT Rect;

    pCreateStruct;

    if (pData == NULL)
    {
        return -1;
    }

    ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

    pData->pbyBuffer = NULL;
    pData->nBufferLen = 0;
    pData->nDataOrganization = CBufferViewerByteData;
    pData->nFontPtSize = CBufferViewerDefPtSize;
    pData->nTotalVisibleRows = 0;
    pData->nTotalVisibleColumns = 0;
    pData->nTotalRows = 0;
    pData->bRedraw = TRUE;
    pData->nStartAddress = 0;
    pData->hTheme = NULL;

    if (::IsThemeActive())
    {
        pData->hTheme = ::OpenThemeData(hWnd, CThemeClassName);
    }

    ::ZeroMemory(&pData->CharTwipsSize, sizeof(pData->CharTwipsSize));
    ::ZeroMemory(&pData->CharDevSize, sizeof(pData->CharDevSize));

    lGetCharSizes(hWnd, pData->nFontPtSize, &pData->CharTwipsSize,
                  &pData->CharDevSize);

    ::ShowScrollBar(hWnd, SB_VERT, TRUE);

    ::GetClientRect(hWnd, &Rect);

    lCalcNewTotalRowsAndColumns(pData, Rect.right, Rect.bottom);

    lUpdateScrollBars(hWnd);

    return 0;
}

static LRESULT lOnDestroyMsg(
  HWND hWnd)
{
    TBufferViewerCtrlData* pData = (TBufferViewerCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pData->hTheme)
    {
        ::CloseThemeData(pData->hTheme);
    }

    UtFreeMem(pData->pbyBuffer);
    UtFreeMem(pData);

    return 0;
}

static LRESULT CALLBACK lBufferViewerCtrlWndProc(
  HWND hWnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
    switch (uMsg)
    {
        case BVM_GETBUFFER:
            return lOnGetBuffer(hWnd, (TUiBufferViewerMem*)wParam);
        case BVM_SETBUFFER:
            return lOnSetBuffer(hWnd, (TUiBufferViewerMem*)lParam);
        case BVM_GETDATAORGANIZATION:
            return lOnGetDataOrganization(hWnd);
        case BVM_SETDATAORGANIZATION:
            return  lOnSetDataOrganization(hWnd, (INT)lParam);
        case BVM_GETFONTPTSIZE:
            return lOnGetFontPtSize(hWnd);
        case BVM_SETFONTPTSIZE:
            return lOnSetFontPtSize(hWnd, (INT)lParam);
        case WM_PAINT:
            return lOnPaintMsg(hWnd);
        case WM_PRINT:
            return lOnPrintMsg(hWnd, (HDC)wParam, (DWORD)lParam);
        case WM_PRINTCLIENT:
            return lOnPrintClientMsg(hWnd, (HDC)wParam, (DWORD)lParam);
        case WM_KEYDOWN:
            return lOnKeyDownMsg(hWnd, (INT)wParam, (DWORD)lParam);
        case WM_LBUTTONDOWN:
            return  lOnLButtonDownMsg(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        case WM_LBUTTONUP:
            return  lOnLButtonUpMsg(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        case WM_LBUTTONDBLCLK:
            return  lOnLButtonDblClkMsg(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        case WM_RBUTTONDOWN:
            return  lOnRButtonDownMsg(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        case WM_RBUTTONUP:
            return  lOnRButtonUpMsg(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        case WM_RBUTTONDBLCLK:
            return  lOnRButtonDblClkMsg(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        case WM_SIZE:
            return lOnSizeMsg(hWnd, LOWORD(lParam), HIWORD(lParam));
        case WM_VSCROLL:
            return lOnVScrollMsg(hWnd, LOWORD(wParam), HIWORD(wParam));
        case WM_SETREDRAW:
            return lOnSetRedrawMsg(hWnd, (BOOL)wParam);
        case WM_THEMECHANGED:
            return lOnThemeChangedMsg(hWnd);
        case WM_CREATE:
            return lOnCreateMsg(hWnd, (LPCREATESTRUCT)lParam);
        case WM_DESTROY:
            return lOnDestroyMsg(hWnd);
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

VOID UiBufferViewerCtrlRegister(VOID)
{
    WNDCLASSEXW WndClassEx;

    WndClassEx.cbSize = sizeof(WndClassEx);
    WndClassEx.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
    WndClassEx.lpfnWndProc = lBufferViewerCtrlWndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = sizeof(TBufferViewerCtrlData*);
    WndClassEx.hInstance = UiPepCtrlGetInstance();
    WndClassEx.hIcon = NULL;
    WndClassEx.hCursor = ::LoadCursorW(NULL, IDC_ARROW);
    WndClassEx.hbrBackground = NULL;
    WndClassEx.lpszMenuName = NULL;
    WndClassEx.lpszClassName = CUiBufferViewerCtrlClass;
    WndClassEx.hIconSm = NULL;

    ::RegisterClassExW(&WndClassEx);
}

VOID UiBufferViewerCtrlUnregister(VOID)
{
    ::UnregisterClassW(CUiBufferViewerCtrlClass, UiPepCtrlGetInstance());
}

/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/
