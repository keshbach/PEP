/***************************************************************************/
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsPep/UiPepCtrls.h>

#include "UiPepCtrlsUtil.h"

#include <Utils/UtHeap.h>

#include <Includes/UtMacros.h>

#include <Includes/UiMacros.h>

#include <strsafe.h>
#include <windowsx.h>

#include <assert.h>

//#define DEBUG_SHOW_COLORIZED_COLUMNS

#pragma region "Constants"

#define CDeviceNamePt 0
#define CDeviceNameValuePt 1
#define CDeviceAdapterPt 2
#define CDeviceAdapterValuePt 3
#define CDipSwitchesPt 4
#define CDipSwitch1ValueTogglePt 5
#define CDipSwitch2ValueTogglePt 6
#define CDipSwitch3ValueTogglePt 7
#define CDipSwitch4ValueTogglePt 8
#define CDipSwitch5ValueTogglePt 9
#define CDipSwitch6ValueTogglePt 10
#define CDipSwitch7ValueTogglePt 11
#define CDipSwitch8ValueTogglePt 12
#define CDipSwitch1ValueDigitPt 13
#define CDipSwitch2ValueDigitPt 14
#define CDipSwitch3ValueDigitPt 15
#define CDipSwitch4ValueDigitPt 16
#define CDipSwitch5ValueDigitPt 17
#define CDipSwitch6ValueDigitPt 18
#define CDipSwitch7ValueDigitPt 19
#define CDipSwitch8ValueDigitPt 20
#define CDeviceSizePt 21
#define CDeviceSizeValuePt 22
#define CDeviceVppPt 23
#define CDeviceVppValuePt 24
#define CDeviceBitsPt 25
#define CDeviceBitsValuePt 26
#define CDeviceChipEnablePt 27
#define CDeviceChipEnableValuePt 28
#define CDeviceOutputEnablePt 29
#define CDeviceOutputEnableValuePt 30

#define CDeviceNameLabel 0
#define CDeviceAdapterLabel 1
#define CDeviceSizeLabel 2
#define CDeviceVppLabel 3
#define CDeviceBitsLabel 4
#define CDeviceChipEnableLabel 5
#define CDeviceOutputEnableLabel 6

#define CNormalTextColor 0
#define CHighlightTextColor 1
#define CDipSwitchOnColor 2
#define CDipSwitchOffColor 3

#define CDeviceText L"Device:"
#define CAdapterText L"Adapter:"
#define CDipSwitchesText L"Dip Switches:"
#define CDeviceSizeText L"Device Size:"
#define CDeviceVppText L"Device Vpp:"
#define CDeviceBitsText L"Device Bits:"
#define CDeviceChipEnableText L"Device Chip Enable:"
#define CDeviceOutputEnableText L"Device Output Enable:"

#define CDipSwitchOn 0x25B2
#define CDipSwitchOff 0x25BC

#define CDipSwitchBorderSpacingPixels 5

#define CStartBorderMarginWidthPixels 0
#define CStartBorderMarginHeightPixels 0

#define CTextMarginWidthPixels 12
#define CTextMarginHeightPixels 4

#define CGroupMarginWidthPixels 12
#define CGroupMarginHeightPixels 0

#pragma endregion

#pragma region "Structures"

typedef struct tagTDeviceInfoCtrlData
{
    POINT Points[32];
    LPWSTR pszLabels[8];
    UINT8 nDipSwitches;
    RECT ColumnRects[6];
    SIZE MinSize;
    SIZE LastSize;
    HFONT hFont;
    HFONT hDipSwitchFont;
    COLORREF Colors[4];
    BOOL bRedraw;
    BOOL bIgnoreSizeChange;
} TDeviceInfoCtrlData;

#pragma endregion

static VOID lUpdateColors(
  TDeviceInfoCtrlData* pData)
{
    pData->Colors[CNormalTextColor] = ::GetSysColor(COLOR_BTNTEXT);
    pData->Colors[CHighlightTextColor] = RGB(255, 0, 0);
    pData->Colors[CDipSwitchOnColor] = RGB(255, 0, 0);
    pData->Colors[CDipSwitchOffColor] = ::GetSysColor(COLOR_BTNTEXT);
}

static BOOL lIsDipSwitchOn(
  _In_ UINT8 nDipSwitches,
  _In_ INT nPosition)
{
    return nDipSwitches & (1 << (nPosition - 1));
}

static BOOL lDoesDipSwitchExist(
  _In_ HDC hDC)
{
    WCHAR cChars[] = {CDipSwitchOn, CDipSwitchOff, 0};
    WORD wDimensions[] = {0, 0};

    if (GDI_ERROR == ::GetGlyphIndicesW(hDC, cChars, 2, wDimensions, GGI_MARK_NONEXISTING_GLYPHS))
    {
        return FALSE;
    }

    if (wDimensions[0] = 0xFFFF && wDimensions[1] == 0xFFFF)
    {
        return FALSE;
    }

    return TRUE;
}

static HFONT lCreateDipSwitchFont(
  _In_ HWND hWnd,
  _In_ HFONT hFont)
{
    HDC hDC = ::GetDC(hWnd);
    LOGFONTW LogFont;

    ::GetObjectW(hFont, sizeof(LogFont), &LogFont);

    ::SaveDC(hDC);

    UiPepCtrlSelectTwipsMode(hDC);

    ::SelectObject(hDC, hFont);

    if (FALSE == lDoesDipSwitchExist(hDC))
    {
        LogFont.lfCharSet = ANSI_CHARSET;
        LogFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;

        // TODO: Enumerate the fonts and if cannot find Courier New use other font

        ::StringCchCopyW(LogFont.lfFaceName, LF_FACESIZE, L"Courier New");
    }

    ::RestoreDC(hDC, -1);

    ::ReleaseDC(hWnd, hDC);

    return ::CreateFontIndirectW(&LogFont);
}

static VOID lGetVisibleScrollBars(
  _In_ HWND hWnd,
  LPBOOL pbShowHorzScrollBar,
  LPBOOL pbShowVertScrollBar)
{
    DWORD dwStyle = GetWindowStyle(hWnd);

    *pbShowHorzScrollBar = (dwStyle & WS_HSCROLL) ? TRUE : FALSE;
    *pbShowVertScrollBar = (dwStyle & WS_VSCROLL) ? TRUE : FALSE;
}

static VOID lGetScrollBarPos(
  _In_ HWND hWnd,
  LPLONG nXPos,
  LPLONG nYPos)
{
    BOOL bShowHorzScrollBar, bShowVertScrollBar;
    SCROLLINFO ScrollInfo;

    lGetVisibleScrollBars(hWnd, &bShowHorzScrollBar, &bShowVertScrollBar);

    if (bShowHorzScrollBar == FALSE)
    {
        *nXPos = 0;
    }
    else
    {
        ScrollInfo.cbSize = sizeof(SCROLLINFO);
        ScrollInfo.fMask = SIF_POS;

        ::GetScrollInfo(hWnd, SB_HORZ, &ScrollInfo);

        *nXPos = ScrollInfo.nPos;
    }

    if (bShowVertScrollBar == FALSE)
    {
        *nYPos = 0;
    }
    else
    {
        ScrollInfo.cbSize = sizeof(SCROLLINFO);
        ScrollInfo.fMask = SIF_POS;

        ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

        *nYPos = ScrollInfo.nPos;
    }
}

static VOID lUpdateScrollBars(
  _In_ HWND hWnd,
  _In_ INT nNewWidth,
  _In_ INT nNewHeight)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    BOOL bShowHorzScrollBar, bShowVertScrollBar;
    BOOL bOriginalShowHorzScrollBar, bOriginalShowVertScrollBar;
    INT nHorzScrollBarHeight, nVertScrollBarWidth;
    SCROLLINFO ScrollInfo;

    pData->bIgnoreSizeChange = TRUE;

    lGetVisibleScrollBars(hWnd, &bShowHorzScrollBar, &bShowVertScrollBar);

    bOriginalShowHorzScrollBar = bShowHorzScrollBar;
    bOriginalShowVertScrollBar = bShowVertScrollBar;

    nHorzScrollBarHeight = ::GetSystemMetrics(SM_CYHSCROLL);
    nVertScrollBarWidth = ::GetSystemMetrics(SM_CXVSCROLL);

    if (bShowHorzScrollBar == FALSE && bShowVertScrollBar == FALSE)
    {
        if (nNewWidth < pData->MinSize.cx)
        {
            bShowHorzScrollBar = TRUE;

            nNewHeight -= nHorzScrollBarHeight;
        }

        if (nNewHeight < pData->MinSize.cy)
        {
            bShowVertScrollBar = TRUE;

            nNewWidth -= nVertScrollBarWidth;

            if (bShowHorzScrollBar == FALSE && nNewWidth < pData->MinSize.cx)
            {
                bShowHorzScrollBar = TRUE;

                nNewHeight -= nHorzScrollBarHeight;
            }
        }
    }
    else if (bShowHorzScrollBar == TRUE && bShowVertScrollBar == FALSE)
    {
        if (nNewWidth >= pData->MinSize.cx)
        {
            bShowHorzScrollBar = FALSE;

            nNewHeight += nHorzScrollBarHeight;
        }

        if (nNewHeight < pData->MinSize.cy)
        {
            bShowVertScrollBar = TRUE;
        }
    }
    else if (bShowHorzScrollBar == FALSE && bShowVertScrollBar == TRUE)
    {
        if (nNewHeight >= pData->MinSize.cy)
        {
            bShowVertScrollBar = FALSE;

            nNewWidth += nVertScrollBarWidth;
        }

        if (nNewWidth < pData->MinSize.cx)
        {
            bShowHorzScrollBar = TRUE;
        }
    }
    else
    {
        if (nNewWidth + nVertScrollBarWidth >= pData->MinSize.cx &&
            nNewHeight + nHorzScrollBarHeight >= pData->MinSize.cy)
        {
            bShowHorzScrollBar = FALSE;
            bShowVertScrollBar = FALSE;

            nNewWidth += nVertScrollBarWidth;
            nNewHeight += nHorzScrollBarHeight;
        }
        else if (nNewWidth >= pData->MinSize.cx &&
                 nNewHeight < pData->MinSize.cy)
        {
            bShowHorzScrollBar = FALSE;

            nNewHeight += nHorzScrollBarHeight;
        }

        else if (nNewWidth < pData->MinSize.cx &&
                 nNewHeight >= pData->MinSize.cy)
        {
            bShowVertScrollBar = FALSE;

            nNewWidth += nVertScrollBarWidth;
        }
    }

    if (bShowHorzScrollBar)
    {
        if (bOriginalShowHorzScrollBar)
        {
            ScrollInfo.cbSize = sizeof(SCROLLINFO);
            ScrollInfo.fMask = SIF_ALL;

            ::GetScrollInfo(hWnd, SB_HORZ, &ScrollInfo);

            ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
            ScrollInfo.nPage = nNewWidth;

            if (ScrollInfo.nPos + (INT)ScrollInfo.nPage >= pData->MinSize.cx)
            {
                ScrollInfo.nPos = (pData->MinSize.cx - ScrollInfo.nPage);
            }
        }
        else
        {
            ScrollInfo.cbSize = sizeof(SCROLLINFO);
            ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
            ScrollInfo.nMin = 0;
            ScrollInfo.nMax = pData->MinSize.cx - 1;
            ScrollInfo.nPage = nNewWidth;
        }

        ::SetScrollInfo(hWnd, SB_HORZ, &ScrollInfo, TRUE);
    }

    if (bShowVertScrollBar)
    {
        if (bOriginalShowVertScrollBar)
        {
            ScrollInfo.cbSize = sizeof(SCROLLINFO);
            ScrollInfo.fMask = SIF_ALL;

            ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

            ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
            ScrollInfo.nPage = nNewHeight;

            if (ScrollInfo.nPos + (INT)ScrollInfo.nPage >= pData->MinSize.cy)
            {
                ScrollInfo.nPos = (pData->MinSize.cy - ScrollInfo.nPage);
            }
        }
        else
        {
            ScrollInfo.cbSize = sizeof(SCROLLINFO);
            ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
            ScrollInfo.nMin = 0;
            ScrollInfo.nMax = pData->MinSize.cy - 1;
            ScrollInfo.nPage = nNewHeight;
        }

        ::SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);
    }

    pData->LastSize.cx = nNewWidth;
    pData->LastSize.cy = nNewHeight;

    ::ShowScrollBar(hWnd, SB_HORZ, bShowHorzScrollBar);
    ::ShowScrollBar(hWnd, SB_VERT, bShowVertScrollBar);

    pData->bIgnoreSizeChange = FALSE;
}

static VOID lCalcMinSize(
  _In_ HDC hDC,
  TDeviceInfoCtrlData* pData)
{
    POINT Point;

    pData->MinSize.cx = 0;
    pData->MinSize.cy = 0;

    for (INT nIndex = 0; nIndex < MArrayLen(pData->ColumnRects); ++nIndex)
    {
        Point.x = pData->ColumnRects[nIndex].right;
        Point.y = pData->ColumnRects[nIndex].bottom;

        ::LPtoDP(hDC, &Point, 1);

        if (Point.x > pData->MinSize.cx)
        {
            pData->MinSize.cx = Point.x;
        }

        if (Point.y > pData->MinSize.cy)
        {
            pData->MinSize.cy = Point.y;
        }
    }
}

static VOID lLayoutColumn1TextPoints(
  HDC hDC,
  TDeviceInfoCtrlData* pData,
  LPPOINT pBorderMarginPoint,
  LPPOINT pTextMarginPoint,
  LPRECT pColumnRect)
{
    SIZE TwipsSize;

    pColumnRect->left = pBorderMarginPoint->x;
    pColumnRect->top = pBorderMarginPoint->y;
    pColumnRect->right = pColumnRect->left;
    pColumnRect->bottom = pColumnRect->top;

    pData->Points[CDeviceNamePt].x = pColumnRect->left;
    pData->Points[CDeviceNamePt].y = pColumnRect->top;

    ::GetTextExtentPoint32W(hDC, CDeviceText, ::lstrlenW(CDeviceText),
                            &TwipsSize);

    pColumnRect->right += TwipsSize.cx;
    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    pData->Points[CDeviceAdapterPt].x = pColumnRect->left;
    pData->Points[CDeviceAdapterPt].y = pColumnRect->bottom;

    ::GetTextExtentPoint32W(hDC, CAdapterText, ::lstrlenW(CAdapterText),
                            &TwipsSize);

    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }

    pData->Points[CDipSwitchesPt].x = pColumnRect->left;
    pData->Points[CDipSwitchesPt].y = pColumnRect->bottom;

    ::GetTextExtentPoint32W(hDC, CDipSwitchesText, ::lstrlenW(CDipSwitchesText),
                            &TwipsSize);

    pColumnRect->bottom += TwipsSize.cy;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }
}

static VOID lLayoutColumn2TextPoints(
  HDC hDC,
  TDeviceInfoCtrlData* pData,
  LPPOINT pBorderMarginPoint,
  LPPOINT pTextMarginPoint,
  LPRECT pColumnRect)
{
    WCHAR cDipSwitchOn = CDipSwitchOn;
    WCHAR cDipSwitchOff = CDipSwitchOff;
    SIZE TwipsSize, DipSwitchOnSize, DipSwitchOffSize;
    WCHAR cDigit;
    INT nToggleXPos, nDigitXPos;
    POINT DipSwitchBorderPoint;

    DipSwitchBorderPoint.x = CDipSwitchBorderSpacingPixels;
    DipSwitchBorderPoint.y = 0;

    ::DPtoLP(hDC, &DipSwitchBorderPoint, 1);

    pColumnRect->left = pBorderMarginPoint->x;
    pColumnRect->top = pBorderMarginPoint->y;
    pColumnRect->right = pColumnRect->left;
    pColumnRect->bottom = pColumnRect->top;

    pData->Points[CDeviceNameValuePt].x = pColumnRect->left;
    pData->Points[CDeviceNameValuePt].y = pColumnRect->top;

    if (pData->pszLabels[CDeviceNameLabel])
    {
        ::GetTextExtentPoint32W(hDC, pData->pszLabels[CDeviceNameLabel],
                                ::lstrlenW(pData->pszLabels[CDeviceNameLabel]),
                                &TwipsSize);
    }
    else
    {
        ::GetTextExtentPoint32W(hDC, CDeviceText, ::lstrlenW(CDeviceText),
                                &TwipsSize);

        TwipsSize.cx = 0;
    }

    pColumnRect->right += TwipsSize.cx;
    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    pData->Points[CDeviceAdapterValuePt].x = pColumnRect->left;
    pData->Points[CDeviceAdapterValuePt].y = pColumnRect->bottom;

    if (pData->pszLabels[CDeviceAdapterLabel])
    {
        ::GetTextExtentPoint32W(hDC, pData->pszLabels[CDeviceAdapterLabel],
                                ::lstrlenW(pData->pszLabels[CDeviceAdapterLabel]),
                                &TwipsSize);
    }
    else
    {
        ::GetTextExtentPoint32W(hDC, CAdapterText, ::lstrlenW(CAdapterText),
                                &TwipsSize);

        TwipsSize.cx = 0;
    }

    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }

    // Special case for dip switches

    ::SelectObject(hDC, pData->hDipSwitchFont);

    ::GetTextExtentPoint32W(hDC, &cDipSwitchOn, 1, &DipSwitchOnSize);
    ::GetTextExtentPoint32W(hDC, &cDipSwitchOff, 1, &DipSwitchOffSize);

    ::SelectObject(hDC, pData->hFont);

    nToggleXPos = pColumnRect->left;
    nDigitXPos = pColumnRect->left;

    for (INT nPosition = 1; nPosition <= 8; ++nPosition)
    {
        cDigit = (WCHAR)(L'0' + nPosition);

        ::GetTextExtentPoint32W(hDC, &cDigit, 1, &TwipsSize);

        pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].x = nToggleXPos;
        pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].y = pColumnRect->bottom;

        pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].x = nDigitXPos;
        pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].y = pColumnRect->bottom;

        if (nPosition > 1)
        {
            nToggleXPos += DipSwitchBorderPoint.x;
            nDigitXPos += DipSwitchBorderPoint.x;
        }

        if (lIsDipSwitchOn(pData->nDipSwitches, nPosition))
        {
            pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].y += DipSwitchOnSize.cy;

            if (DipSwitchOnSize.cx > TwipsSize.cx)
            {
                pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].x = nToggleXPos;
                pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].x = nDigitXPos + ((DipSwitchOnSize.cx - TwipsSize.cx) / 2);

                nToggleXPos += DipSwitchOnSize.cx;
                nDigitXPos += DipSwitchOnSize.cx;
            }
            else
            {
                pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].x = nToggleXPos + ((TwipsSize.cx - DipSwitchOnSize.cx) / 2);
                pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].x = nDigitXPos;

                nToggleXPos += TwipsSize.cx;
                nDigitXPos += TwipsSize.cx;
            }
        }
        else
        {
            pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].y += DipSwitchOffSize.cy;

            if (DipSwitchOffSize.cx > TwipsSize.cx)
            {
                pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].x = nToggleXPos;
                pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].x = nDigitXPos + ((DipSwitchOffSize.cx - TwipsSize.cx) / 2);

                nToggleXPos += DipSwitchOffSize.cx;
                nDigitXPos += DipSwitchOffSize.cx;
            }
            else
            {
                pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].x = nToggleXPos + ((TwipsSize.cx - DipSwitchOffSize.cx) / 2);
                pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].x = nDigitXPos;

                nToggleXPos += TwipsSize.cx;
                nDigitXPos += TwipsSize.cx;
            }
        }
    }

    if (nToggleXPos > pColumnRect->right)
    {
        pColumnRect->right = nToggleXPos;
    }

    if (nDigitXPos > pColumnRect->right)
    {
        pColumnRect->right = nDigitXPos;
    }

    pColumnRect->bottom += DipSwitchOnSize.cy;
    pColumnRect->bottom += TwipsSize.cy;
}

static VOID lLayoutColumn3TextPoints(
  HDC hDC,
  TDeviceInfoCtrlData* pData,
  LPPOINT pBorderMarginPoint,
  LPPOINT pTextMarginPoint,
  LPRECT pColumnRect)
{
    SIZE TwipsSize;

    pColumnRect->left = pBorderMarginPoint->x;
    pColumnRect->top = pBorderMarginPoint->y;
    pColumnRect->right = pColumnRect->left;
    pColumnRect->bottom = pColumnRect->top;

    pData->Points[CDeviceSizePt].x = pColumnRect->left;
    pData->Points[CDeviceSizePt].y = pColumnRect->top;

    ::GetTextExtentPoint32W(hDC, CDeviceSizeText, ::lstrlenW(CDeviceSizeText),
                            &TwipsSize);

    pColumnRect->right += TwipsSize.cx;
    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    pData->Points[CDeviceVppPt].x = pColumnRect->left;
    pData->Points[CDeviceVppPt].y = pColumnRect->bottom;

    ::GetTextExtentPoint32W(hDC, CDeviceVppText, ::lstrlenW(CDeviceVppText),
                            &TwipsSize);

    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }

    pData->Points[CDeviceBitsPt].x = pColumnRect->left;
    pData->Points[CDeviceBitsPt].y = pColumnRect->bottom;

    ::GetTextExtentPoint32W(hDC, CDeviceBitsText, ::lstrlenW(CDeviceBitsText),
                            &TwipsSize);

    pColumnRect->bottom += TwipsSize.cy;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }
}

static VOID lLayoutColumn4TextPoints(
  HDC hDC,
  TDeviceInfoCtrlData* pData,
  LPPOINT pBorderMarginPoint,
  LPPOINT pTextMarginPoint,
  LPRECT pColumnRect)
{
    SIZE TwipsSize;

    pColumnRect->left = pBorderMarginPoint->x;
    pColumnRect->top = pBorderMarginPoint->y;
    pColumnRect->right = pColumnRect->left;
    pColumnRect->bottom = pColumnRect->top;

    pData->Points[CDeviceSizeValuePt].x = pColumnRect->left;
    pData->Points[CDeviceSizeValuePt].y = pColumnRect->top;

    if (pData->pszLabels[CDeviceSizeLabel])
    {
        ::GetTextExtentPoint32W(hDC, pData->pszLabels[CDeviceSizeLabel],
                                ::lstrlenW(pData->pszLabels[CDeviceSizeLabel]),
                                &TwipsSize);
    }
    else
    {
        ::GetTextExtentPoint32W(hDC, CDeviceSizeText, ::lstrlenW(CDeviceSizeText),
                                &TwipsSize);

        TwipsSize.cx = 0;
    }

    pColumnRect->right += TwipsSize.cx;
    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    pData->Points[CDeviceVppValuePt].x = pColumnRect->left;
    pData->Points[CDeviceVppValuePt].y = pColumnRect->bottom;

    if (pData->pszLabels[CDeviceVppLabel])
    {
        ::GetTextExtentPoint32W(hDC, pData->pszLabels[CDeviceVppLabel],
                                ::lstrlenW(pData->pszLabels[CDeviceVppLabel]),
                                &TwipsSize);
    }
    else
    {
        ::GetTextExtentPoint32W(hDC, CDeviceVppText, ::lstrlenW(CDeviceVppText),
                                &TwipsSize);

        TwipsSize.cx = 0;
    }

    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }

    pData->Points[CDeviceBitsValuePt].x = pColumnRect->left;
    pData->Points[CDeviceBitsValuePt].y = pColumnRect->bottom;

    if (pData->pszLabels[CDeviceBitsLabel])
    {
        ::GetTextExtentPoint32W(hDC, pData->pszLabels[CDeviceBitsLabel],
                                ::lstrlenW(pData->pszLabels[CDeviceBitsLabel]),
                                &TwipsSize);
    }
    else
    {
        ::GetTextExtentPoint32W(hDC, CDeviceBitsText, ::lstrlenW(CDeviceBitsText),
                                &TwipsSize);

        TwipsSize.cx = 0;
    }

    pColumnRect->bottom += TwipsSize.cy;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }
}

static VOID lLayoutColumn5TextPoints(
  HDC hDC,
  TDeviceInfoCtrlData* pData,
  LPPOINT pBorderMarginPoint,
  LPPOINT pTextMarginPoint,
  LPRECT pColumnRect)
{
    SIZE TwipsSize;

    pColumnRect->left = pBorderMarginPoint->x;
    pColumnRect->top = pBorderMarginPoint->y;
    pColumnRect->right = pColumnRect->left;
    pColumnRect->bottom = pColumnRect->top;

    pData->Points[CDeviceChipEnablePt].x = pColumnRect->left;
    pData->Points[CDeviceChipEnablePt].y = pColumnRect->top;

    if (pData->pszLabels[CDeviceChipEnableLabel])
    {
        ::GetTextExtentPoint32W(hDC, CDeviceChipEnableText,
                                ::lstrlenW(CDeviceChipEnableText),
                                &TwipsSize);
    }
    else
    {
        ::GetTextExtentPoint32W(hDC, CDeviceChipEnableText,
                                ::lstrlenW(CDeviceChipEnableText),
                                &TwipsSize);

        TwipsSize.cx = 0;
    }

    pColumnRect->right += TwipsSize.cx;
    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    pData->Points[CDeviceOutputEnablePt].x = pColumnRect->left;
    pData->Points[CDeviceOutputEnablePt].y = pColumnRect->bottom;

    if (pData->pszLabels[CDeviceOutputEnableLabel])
    {
        ::GetTextExtentPoint32W(hDC, CDeviceOutputEnableText,
                                ::lstrlenW(CDeviceOutputEnableText),
                                &TwipsSize);
    }
    else
    {
        ::GetTextExtentPoint32W(hDC, CDeviceOutputEnableText,
                                ::lstrlenW(CDeviceOutputEnableText),
                                &TwipsSize);

        TwipsSize.cx = 0;
    }

    pColumnRect->bottom += TwipsSize.cy;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }
}

static VOID lLayoutColumn6TextPoints(
  HDC hDC,
  TDeviceInfoCtrlData* pData,
  LPPOINT pBorderMarginPoint,
  LPPOINT pTextMarginPoint,
  LPRECT pColumnRect)
{
    SIZE TwipsSize;

    pColumnRect->left = pBorderMarginPoint->x;
    pColumnRect->top = pBorderMarginPoint->y;
    pColumnRect->right = pColumnRect->left;
    pColumnRect->bottom = pColumnRect->top;

    pData->Points[CDeviceChipEnableValuePt].x = pColumnRect->left;
    pData->Points[CDeviceChipEnableValuePt].y = pColumnRect->top;

    ::GetTextExtentPoint32W(hDC, pData->pszLabels[CDeviceChipEnableLabel],
                            ::lstrlenW(pData->pszLabels[CDeviceChipEnableLabel]),
                            &TwipsSize);

    pColumnRect->right += TwipsSize.cx;
    pColumnRect->bottom += TwipsSize.cy + pTextMarginPoint->y;

    pData->Points[CDeviceOutputEnableValuePt].x = pColumnRect->left;
    pData->Points[CDeviceOutputEnableValuePt].y = pColumnRect->bottom;

    ::GetTextExtentPoint32W(hDC, pData->pszLabels[CDeviceOutputEnableLabel],
                            ::lstrlenW(pData->pszLabels[CDeviceOutputEnableLabel]),
                            &TwipsSize);

    pColumnRect->bottom += TwipsSize.cy;

    if (TwipsSize.cx > (pColumnRect->right - pColumnRect->left))
    {
        pColumnRect->right = pColumnRect->left + TwipsSize.cx;
    }
}

static VOID lLayoutTextPoints(
  _In_ HWND hWnd)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HDC hDC = ::GetDC(hWnd);
    POINT BorderMarginPoint, TextMarginPoint, GroupMarginPoint;
    RECT Rect;

    BorderMarginPoint.x = CStartBorderMarginWidthPixels;
    BorderMarginPoint.y = CStartBorderMarginHeightPixels;

    TextMarginPoint.x = CTextMarginWidthPixels;
    TextMarginPoint.y = CTextMarginHeightPixels;

    GroupMarginPoint.x = CGroupMarginWidthPixels;
    GroupMarginPoint.y = CGroupMarginHeightPixels;

    ::SaveDC(hDC);

    UiPepCtrlSelectTwipsMode(hDC);

    ::SelectObject(hDC, pData->hFont);

    ::DPtoLP(hDC, &BorderMarginPoint, 1);
    ::DPtoLP(hDC, &TextMarginPoint, 1);
    ::DPtoLP(hDC, &GroupMarginPoint, 1);

    lLayoutColumn1TextPoints(hDC, pData, &BorderMarginPoint, &TextMarginPoint,
                             &pData->ColumnRects[0]);

    BorderMarginPoint.x = pData->ColumnRects[0].right + TextMarginPoint.x;
    BorderMarginPoint.y = pData->ColumnRects[0].top;

    lLayoutColumn2TextPoints(hDC, pData, &BorderMarginPoint, &TextMarginPoint,
                             &pData->ColumnRects[1]);

    BorderMarginPoint.x = pData->ColumnRects[1].right + TextMarginPoint.x + GroupMarginPoint.x;
    BorderMarginPoint.y = pData->ColumnRects[1].top;

    lLayoutColumn3TextPoints(hDC, pData, &BorderMarginPoint, &TextMarginPoint,
                             &pData->ColumnRects[2]);

    BorderMarginPoint.x = pData->ColumnRects[2].right + TextMarginPoint.x;
    BorderMarginPoint.y = pData->ColumnRects[2].top;

    lLayoutColumn4TextPoints(hDC, pData, &BorderMarginPoint, &TextMarginPoint,
                             &pData->ColumnRects[3]);

    BorderMarginPoint.x = pData->ColumnRects[3].right + TextMarginPoint.x + GroupMarginPoint.x;
    BorderMarginPoint.y = pData->ColumnRects[3].top;

    lLayoutColumn5TextPoints(hDC, pData, &BorderMarginPoint, &TextMarginPoint,
                             &pData->ColumnRects[4]);

    BorderMarginPoint.x = pData->ColumnRects[4].right + TextMarginPoint.x;
    BorderMarginPoint.y = pData->ColumnRects[4].top;

    lLayoutColumn6TextPoints(hDC, pData, &BorderMarginPoint, &TextMarginPoint,
                             &pData->ColumnRects[5]);

    lCalcMinSize(hDC, pData);

    ::RestoreDC(hDC, -1);

    ::ReleaseDC(hWnd, hDC);

    if (pData->bRedraw)
    {
        ::GetClientRect(hWnd, &Rect);

        lUpdateScrollBars(hWnd, MRectWidth(Rect), MRectHeight(Rect));
    }
}

static VOID lDrawColumn1(
  _In_ HDC hDC,
  TDeviceInfoCtrlData* pData)
{
#if defined(DEBUG_SHOW_COLORIZED_COLUMNS)
    UiPepCtrlFillSolidRect(hDC, RGB(255, 0, 0),
                           pData->ColumnRects[0].left, pData->ColumnRects[0].top,
                           pData->ColumnRects[0].right - pData->ColumnRects[0].left,
                           pData->ColumnRects[0].bottom - pData->ColumnRects[0].top);
#endif

    ::SetTextColor(hDC, pData->Colors[CNormalTextColor]);

    ::TextOutW(hDC, pData->Points[CDeviceNamePt].x, pData->Points[CDeviceNamePt].y,
               CDeviceText, ::lstrlenW(CDeviceText));

    ::TextOutW(hDC, pData->Points[CDeviceAdapterPt].x, pData->Points[CDeviceAdapterPt].y,
               CAdapterText, ::lstrlenW(CAdapterText));

    ::TextOutW(hDC, pData->Points[CDipSwitchesPt].x, pData->Points[CDipSwitchesPt].y,
               CDipSwitchesText, ::lstrlenW(CDipSwitchesText));
}

static VOID lDrawColumn2(
  _In_ HDC hDC,
  TDeviceInfoCtrlData* pData)
{
    WCHAR cOn = CDipSwitchOn;
    WCHAR cOff = CDipSwitchOff;
    LPWSTR pszValue;
    WCHAR cDigit;

#if defined(DEBUG_SHOW_COLORIZED_COLUMNS)
    UiPepCtrlFillSolidRect(hDC, RGB(0, 255, 0),
                           pData->ColumnRects[1].left, pData->ColumnRects[1].top,
                           pData->ColumnRects[1].right - pData->ColumnRects[1].left,
                           pData->ColumnRects[1].bottom - pData->ColumnRects[1].top);
#endif

    ::SetTextColor(hDC, pData->Colors[CNormalTextColor]);

    if (pData->pszLabels[CDeviceNameLabel])
    {
        ::TextOutW(hDC, pData->Points[CDeviceNameValuePt].x, pData->Points[CDeviceNameValuePt].y,
                   pData->pszLabels[CDeviceNameLabel],
                   ::lstrlenW(pData->pszLabels[CDeviceNameLabel]));
    }

    ::SetTextColor(hDC, pData->Colors[CHighlightTextColor]);

    if (pData->pszLabels[CDeviceAdapterLabel])
    {
        ::TextOutW(hDC, pData->Points[CDeviceAdapterValuePt].x, pData->Points[CDeviceAdapterValuePt].y,
                   pData->pszLabels[CDeviceAdapterLabel],
                   ::lstrlenW(pData->pszLabels[CDeviceAdapterLabel]));
    }

    ::SetTextColor(hDC, pData->Colors[CNormalTextColor]);

    ::SelectObject(hDC, pData->hDipSwitchFont);

    for (INT nPosition = 1; nPosition <= 8; ++nPosition)
    {
        if (lIsDipSwitchOn(pData->nDipSwitches, nPosition))
        {
            pszValue = &cOn;

            ::SetTextColor(hDC, pData->Colors[CDipSwitchOnColor]);
        }
        else
        {
            pszValue = &cOff;

            ::SetTextColor(hDC, pData->Colors[CDipSwitchOffColor]);
        }

        ::TextOutW(hDC, pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].x,
                   pData->Points[CDipSwitch1ValueTogglePt + (nPosition - 1)].y,
                   pszValue, 1);
    }

    ::SelectObject(hDC, pData->hFont);

    for (INT nPosition = 1; nPosition <= 8; ++nPosition)
    {
        cDigit = (WCHAR)(L'0' + nPosition);

        if (lIsDipSwitchOn(pData->nDipSwitches, nPosition))
        {
            ::SetTextColor(hDC, pData->Colors[CDipSwitchOnColor]);
        }
        else
        {
            ::SetTextColor(hDC, pData->Colors[CDipSwitchOffColor]);
        }

        ::TextOutW(hDC, pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].x,
                   pData->Points[CDipSwitch1ValueDigitPt + (nPosition - 1)].y,
                   &cDigit, 1);
    }
}

static VOID lDrawColumn3(
  _In_ HDC hDC,
  TDeviceInfoCtrlData* pData)
{
#if defined(DEBUG_SHOW_COLORIZED_COLUMNS)
    UiPepCtrlFillSolidRect(hDC, RGB(0, 0, 255),
                           pData->ColumnRects[2].left, pData->ColumnRects[2].top,
                           pData->ColumnRects[2].right - pData->ColumnRects[2].left,
                           pData->ColumnRects[2].bottom - pData->ColumnRects[2].top);
#endif

    ::SetTextColor(hDC, pData->Colors[CNormalTextColor]);

    ::TextOutW(hDC, pData->Points[CDeviceSizePt].x, pData->Points[CDeviceSizePt].y,
               CDeviceSizeText, ::lstrlenW(CDeviceSizeText));

    ::TextOutW(hDC, pData->Points[CDeviceVppPt].x, pData->Points[CDeviceVppPt].y,
               CDeviceVppText, ::lstrlenW(CDeviceVppText));

    ::TextOutW(hDC, pData->Points[CDeviceBitsPt].x, pData->Points[CDeviceBitsPt].y,
               CDeviceBitsText, ::lstrlenW(CDeviceBitsText));
}

static VOID lDrawColumn4(
  _In_ HDC hDC,
  TDeviceInfoCtrlData* pData)
{
#if defined(DEBUG_SHOW_COLORIZED_COLUMNS)
    UiPepCtrlFillSolidRect(hDC, RGB(255, 255, 0),
                           pData->ColumnRects[3].left, pData->ColumnRects[3].top,
                           pData->ColumnRects[3].right - pData->ColumnRects[3].left,
                           pData->ColumnRects[3].bottom - pData->ColumnRects[3].top);
#endif

    ::SetTextColor(hDC, pData->Colors[CNormalTextColor]);

    if (pData->pszLabels[CDeviceSizeLabel])
    {
        ::TextOutW(hDC, pData->Points[CDeviceSizeValuePt].x,
                   pData->Points[CDeviceSizeValuePt].y,
                   pData->pszLabels[CDeviceSizeLabel],
                   ::lstrlenW(pData->pszLabels[CDeviceSizeLabel]));
    }

    if (pData->pszLabels[CDeviceVppLabel])
    {
        ::TextOutW(hDC, pData->Points[CDeviceVppValuePt].x,
                   pData->Points[CDeviceVppValuePt].y,
                   pData->pszLabels[CDeviceVppLabel],
                   ::lstrlenW(pData->pszLabels[CDeviceVppLabel]));
    }

    if (pData->pszLabels[CDeviceBitsLabel])
    {
        ::TextOutW(hDC, pData->Points[CDeviceBitsValuePt].x,
                   pData->Points[CDeviceBitsValuePt].y,
                   pData->pszLabels[CDeviceBitsLabel],
                   ::lstrlenW(pData->pszLabels[CDeviceBitsLabel]));
    }
}

static VOID lDrawColumn5(
  _In_ HDC hDC,
  TDeviceInfoCtrlData* pData)
{
#if defined(DEBUG_SHOW_COLORIZED_COLUMNS)
    UiPepCtrlFillSolidRect(hDC, RGB(255, 0, 255),
                           pData->ColumnRects[4].left, pData->ColumnRects[4].top,
                           pData->ColumnRects[4].right - pData->ColumnRects[4].left,
                           pData->ColumnRects[4].bottom - pData->ColumnRects[4].top);
#endif

    ::SetTextColor(hDC, pData->Colors[CNormalTextColor]);

    ::TextOutW(hDC, pData->Points[CDeviceChipEnablePt].x,
               pData->Points[CDeviceChipEnablePt].y,
               CDeviceChipEnableText, ::lstrlenW(CDeviceChipEnableText));

    ::TextOutW(hDC, pData->Points[CDeviceOutputEnablePt].x,
               pData->Points[CDeviceOutputEnablePt].y,
               CDeviceOutputEnableText, ::lstrlenW(CDeviceOutputEnableText));
}

static VOID lDrawColumn6(
  _In_ HDC hDC,
  TDeviceInfoCtrlData* pData)
{
#if defined(DEBUG_SHOW_COLORIZED_COLUMNS)
    UiPepCtrlFillSolidRect(hDC, RGB(128, 128, 128),
                           pData->ColumnRects[5].left, pData->ColumnRects[5].top,
                           pData->ColumnRects[5].right - pData->ColumnRects[5].left,
                           pData->ColumnRects[5].bottom - pData->ColumnRects[5].top);
#endif

    ::SetTextColor(hDC, pData->Colors[CNormalTextColor]);

    if (pData->pszLabels[CDeviceChipEnableLabel])
    {
        ::TextOutW(hDC, pData->Points[CDeviceChipEnableValuePt].x,
                   pData->Points[CDeviceChipEnableValuePt].y,
                   pData->pszLabels[CDeviceChipEnableLabel],
                   ::lstrlenW(pData->pszLabels[CDeviceChipEnableLabel]));
    }

    if (pData->pszLabels[CDeviceOutputEnableLabel])
    {
        ::TextOutW(hDC, pData->Points[CDeviceOutputEnableValuePt].x,
                   pData->Points[CDeviceOutputEnableValuePt].y,
                   pData->pszLabels[CDeviceOutputEnableLabel],
                   ::lstrlenW(pData->pszLabels[CDeviceOutputEnableLabel]));
    }
}

static VOID lDrawWindow(
  _In_ HWND hWnd,
  _In_ HDC hDC)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    COLORREF Color = ::GetSysColor(COLOR_BTNFACE);
    RECT ClientRect;
    POINT Point;

    lGetScrollBarPos(hWnd, &Point.x, &Point.y);

    ::GetClientRect(hWnd, &ClientRect);

    ::SaveDC(hDC);

    // Draw the background

    UiPepCtrlSelectDevUnitsMode(hDC);

#if defined(DEBUG_SHOW_COLORIZED_COLUMNS)
    Color = RGB(128, 0, 128);
#endif

    UiPepCtrlFillSolidRect(hDC, Color, 0, 0, ClientRect.right, ClientRect.bottom);

    // Draw the text

    UiPepCtrlSelectTwipsMode(hDC);

    ::DPtoLP(hDC, &Point, 1);

    ::SetWindowOrgEx(hDC, Point.x, Point.y, NULL);

    ::SelectObject(hDC, pData->hFont);

    ::SetBkMode(hDC, TRANSPARENT);

    lDrawColumn1(hDC, pData);
    lDrawColumn2(hDC, pData);
    lDrawColumn3(hDC, pData);
    lDrawColumn4(hDC, pData);
    lDrawColumn5(hDC, pData);
    lDrawColumn6(hDC, pData);

    ::RestoreDC(hDC, -1);
}

static VOID lRedrawWindow(
  _In_ HWND hWnd)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pData->bRedraw)
    {
        ::InvalidateRect(hWnd, NULL, TRUE);
        ::UpdateWindow(hWnd);
    }
}

static BOOL lSetLabelValue(
    HWND hWnd,
    LPCWSTR pszNewValue,
    INT nLabelIndex)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    INT nNewValueLen;

    if (pszNewValue)
    {
        nNewValueLen = ::lstrlenW(pszNewValue) + 1;

        if (pData->pszLabels[nLabelIndex])
        {
            UtFreeMem(pData->pszLabels[nLabelIndex]);
        }

        pData->pszLabels[nLabelIndex] = (LPWSTR)UtAllocMem(nNewValueLen * sizeof(WCHAR));

        if (pData->pszLabels[nLabelIndex] == NULL)
        {
            return FALSE;
        }

        ::StringCchCopyW(pData->pszLabels[nLabelIndex], nNewValueLen, pszNewValue);
    }
    else
    {
        if (pData->pszLabels[nLabelIndex])
        {
            UtFreeMem(pData->pszLabels[nLabelIndex]);

            pData->pszLabels[nLabelIndex] = NULL;
        }
    }

    lLayoutTextPoints(hWnd);

    lRedrawWindow(hWnd);

    return TRUE;
}

static BOOL lGetLabelValue(
    HWND hWnd,
    LPWSTR pszValue,
    LPINT pnValueLen,
    INT nLabelIndex)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pszValue)
    {
        if (pData->pszLabels[nLabelIndex])
        {
            if (*pnValueLen <= ::lstrlen(pData->pszLabels[nLabelIndex]))
            {
                return FALSE;
            }

            ::StringCchCopyW(pszValue, *pnValueLen, pData->pszLabels[nLabelIndex]);
        }
        else
        {
            if (*pnValueLen < 1)
            {
                return FALSE;
            }

            *pszValue = 0;
        }
    }
    else
    {
        if (pData->pszLabels[nLabelIndex])
        {
            *pnValueLen = ::lstrlen(pData->pszLabels[nLabelIndex]) + 1;
        }
        else
        {
            *pnValueLen = 0;
        }
    }

    return TRUE;
}

static LRESULT lOnSetNameLabelValue(
  HWND hWnd,
  LPCWSTR pszNewValue)
{
    return lSetLabelValue(hWnd, pszNewValue, CDeviceNameLabel);
}

static LRESULT lOnGetNameLabelValue(
  HWND hWnd,
  LPWSTR pszValue,
  LPINT pnValueLen)
{
    return lGetLabelValue(hWnd, pszValue, pnValueLen, CDeviceNameLabel);
}

static LRESULT lOnSetAdapterLabelValue(
  HWND hWnd,
  LPCWSTR pszNewValue)
{
    return lSetLabelValue(hWnd, pszNewValue, CDeviceAdapterLabel);
}

static LRESULT lOnGetAdapterLabelValue(
  HWND hWnd,
  LPWSTR pszValue,
  LPINT pnValueLen)
{
    return lGetLabelValue(hWnd, pszValue, pnValueLen, CDeviceAdapterLabel);
}

static LRESULT lOnSetDipSwitches(
  _In_ HWND hWnd,
  _In_ UINT8 nDipSwitches)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    pData->nDipSwitches = nDipSwitches;

    lLayoutTextPoints(hWnd);

    lRedrawWindow(hWnd);

    return TRUE;
}

static LRESULT lOnGetDipSwitches(
  _In_ HWND hWnd,
  UINT8* pnDipSwitches)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    *pnDipSwitches = pData->nDipSwitches;

    return TRUE;
}

static LRESULT lOnSetSizeLabelValue(
  HWND hWnd,
  LPCWSTR pszNewValue)
{
    return lSetLabelValue(hWnd, pszNewValue, CDeviceSizeLabel);
}

static LRESULT lOnGetSizeLabelValue(
  HWND hWnd,
  LPWSTR pszValue,
  LPINT pnValueLen)
{
    return lGetLabelValue(hWnd, pszValue, pnValueLen, CDeviceSizeLabel);
}

static LRESULT lOnSetVppLabelValue(
    HWND hWnd,
    LPCWSTR pszNewValue)
{
    return lSetLabelValue(hWnd, pszNewValue, CDeviceVppLabel);
}

static LRESULT lOnGetVppLabelValue(
  HWND hWnd,
  LPWSTR pszValue,
  LPINT pnValueLen)
{
    return lGetLabelValue(hWnd, pszValue, pnValueLen, CDeviceVppLabel);
}

static LRESULT lOnSetBitsLabelValue(
  HWND hWnd,
  LPCWSTR pszNewValue)
{
    return lSetLabelValue(hWnd, pszNewValue, CDeviceBitsLabel);
}

static LRESULT lOnGetBitsLabelValue(
  HWND hWnd,
  LPWSTR pszValue,
  LPINT pnValueLen)
{
    return lGetLabelValue(hWnd, pszValue, pnValueLen, CDeviceBitsLabel);
}

static LRESULT lOnSetChipEnableLabelValue(
  HWND hWnd,
  LPCWSTR pszNewValue)
{
    return lSetLabelValue(hWnd, pszNewValue, CDeviceChipEnableLabel);
}

static LRESULT lOnGetChipEnableLabelValue(
  HWND hWnd,
  LPWSTR pszValue,
  LPINT pnValueLen)
{
    return lGetLabelValue(hWnd, pszValue, pnValueLen, CDeviceChipEnableLabel);
}

static LRESULT lOnSetOutputEnableLabelValue(
  HWND hWnd,
  LPCWSTR pszNewValue)
{
    return lSetLabelValue(hWnd, pszNewValue, CDeviceOutputEnableLabel);
}

static LRESULT lOnGetOutputEnableLabelValue(
  HWND hWnd,
  LPWSTR pszValue,
  LPINT pnValueLen)
{
    return lGetLabelValue(hWnd, pszValue, pnValueLen, CDeviceOutputEnableLabel);
}

static LRESULT lOnGetMinRect(
  HWND hWnd,
  LPRECT pRect)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    pRect->left = 0;
    pRect->top = 0;
    pRect->right = pData->MinSize.cx;
    pRect->bottom = pData->MinSize.cy;

    return TRUE;
}

static LRESULT lOnPaintMsg(
  _In_ HWND hWnd)
{
    HDC hDC, hMemDC;
    PAINTSTRUCT PaintStruct;
    HBITMAP hBitmap;
    RECT Rect;
    INT nBitmapWidth, nBitmapHeight;

    ::GetClientRect(hWnd, &Rect);

    nBitmapWidth = MRectWidth(Rect);
    nBitmapHeight = MRectHeight(Rect);

    hDC = ::BeginPaint(hWnd, &PaintStruct);

    hMemDC = ::CreateCompatibleDC(hDC);

    hBitmap = ::CreateCompatibleBitmap(hDC, nBitmapWidth, nBitmapHeight);

    ::SaveDC(hMemDC);

    ::SelectObject(hMemDC, hBitmap);

    lDrawWindow(hWnd, hMemDC);

    ::BitBlt(hDC, 0, 0, nBitmapWidth, nBitmapHeight, hMemDC, 0, 0, SRCCOPY);

    ::RestoreDC(hMemDC, -1);

    ::DeleteObject(hBitmap);

    ::DeleteDC(hMemDC);

    ::EndPaint(hWnd, &PaintStruct);

    return 0;
}

static LRESULT lOnPrintMsg(
  _In_ HWND hWnd,
  _In_ HDC hDC,
  _In_ DWORD dwDrawingOptions)
{
    dwDrawingOptions;

    lDrawWindow(hWnd, hDC);

    return 0;
}

static LRESULT lOnPrintClientMsg(
  _In_ HWND hWnd,
  _In_ HDC hDC,
  _In_ DWORD dwDrawingOptions)
{
    dwDrawingOptions;

    lDrawWindow(hWnd, hDC);

    return 0;
}

static LRESULT lOnSizeMsg(
  _In_ HWND hWnd,
  _In_ INT nNewWidth,
  _In_ INT nNewHeight)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (FALSE == pData->bIgnoreSizeChange && TRUE == pData->bRedraw)
    {
        lUpdateScrollBars(hWnd, nNewWidth, nNewHeight);
    }

    return 0;
}

static LRESULT lOnHScrollMsg(
  _In_ HWND hWnd,
  _In_ INT nType,
  _In_ INT nPos)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    BOOL bUpdate = FALSE;
    SCROLLINFO ScrollInfo;

    nPos;

    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = SIF_ALL;

    ::GetScrollInfo(hWnd, SB_HORZ, &ScrollInfo);

    switch (nType)
    {
        case SB_LINELEFT:
            if (ScrollInfo.nPos > 0)
            {
                --ScrollInfo.nPos;

                bUpdate = TRUE;
            }
            break;
        case SB_LINERIGHT:
            if (ScrollInfo.nPos + (INT)ScrollInfo.nPage < pData->MinSize.cx)
            {
                ++ScrollInfo.nPos;

                bUpdate = TRUE;
            }
            break;
        case SB_PAGELEFT:
            ScrollInfo.nPos -= (INT)ScrollInfo.nPage;

            if (ScrollInfo.nPos < 0)
            {
                ScrollInfo.nPos = 0;
            }

            bUpdate = TRUE;
            break;
        case SB_PAGERIGHT:
            ScrollInfo.nPos += ScrollInfo.nPage;

            if (ScrollInfo.nPos + (INT)ScrollInfo.nPage >= pData->MinSize.cx)
            {
                ScrollInfo.nPos = (pData->MinSize.cx - ScrollInfo.nPage);
            }

            bUpdate = TRUE;
            break;
        case SB_THUMBTRACK:
            ScrollInfo.nPos = ScrollInfo.nTrackPos;

            bUpdate = TRUE;
            break;
    }

    if (bUpdate)
    {
        ScrollInfo.fMask = SIF_POS;

        ::SetScrollInfo(hWnd, SB_HORZ, &ScrollInfo, TRUE);

        lRedrawWindow(hWnd);
    }

    return 0;
}

static LRESULT lOnVScrollMsg(
  _In_ HWND hWnd,
  _In_ INT nType,
  _In_ INT nPos)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    BOOL bUpdate = FALSE;
    SCROLLINFO ScrollInfo;

    nPos;

    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = SIF_ALL;

    ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

    switch (nType)
    {
        case SB_LINEUP:
            if (ScrollInfo.nPos > 0)
            {
                --ScrollInfo.nPos;

                bUpdate = TRUE;
            }
            break;
        case SB_LINEDOWN:
            if (ScrollInfo.nPos + (INT)ScrollInfo.nPage < pData->MinSize.cy)
            {
                ++ScrollInfo.nPos;

                bUpdate = TRUE;
            }
            break;
        case SB_PAGEUP:
            ScrollInfo.nPos -= (INT)ScrollInfo.nPage;

            if (ScrollInfo.nPos < 0)
            {
                ScrollInfo.nPos = 0;
            }

            bUpdate = TRUE;
            break;
        case SB_PAGEDOWN:
            ScrollInfo.nPos += ScrollInfo.nPage;

            if (ScrollInfo.nPos + (INT)ScrollInfo.nPage >= pData->MinSize.cy)
            {
                ScrollInfo.nPos = (pData->MinSize.cy - ScrollInfo.nPage);
            }

            bUpdate = TRUE;
            break;
        case SB_THUMBTRACK:
            ScrollInfo.nPos = ScrollInfo.nTrackPos;

            bUpdate = TRUE;
            break;
    }

    if (bUpdate)
    {
        ScrollInfo.fMask = SIF_POS;

        ::SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);

        lRedrawWindow(hWnd);
    }

    return 0;
}

static LRESULT lOnSetRedrawMsg(
  _In_ HWND hWnd,
  _In_ BOOL bRedraw)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    RECT Rect;

    pData->bRedraw = bRedraw;

    if (bRedraw)
    {
        ::GetClientRect(hWnd, &Rect);

        lUpdateScrollBars(hWnd, MRectWidth(Rect), MRectHeight(Rect));
    }

    lRedrawWindow(hWnd);

    return 0;
}

static LRESULT lOnGetFont(
  _In_ HWND hWnd)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    return (LRESULT)pData->hFont;
}

static LRESULT lOnSetFont(
  _In_ HWND hWnd,
  _In_ HFONT hFont,
  _In_ BOOL bRedraw)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pData->hFont)
    {
        ::DeleteObject(pData->hFont);
    }

    pData->hFont = hFont;

    if (pData->hDipSwitchFont)
    {
        ::DeleteObject(pData->hDipSwitchFont);
    }

    pData->hDipSwitchFont = lCreateDipSwitchFont(hWnd, hFont);

    lLayoutTextPoints(hWnd);

    if (bRedraw)
    {
        ::InvalidateRect(hWnd, NULL, TRUE);
        ::UpdateWindow(hWnd);
    }

    return 0;
}

static LRESULT lOnSysColorChange(
  _In_ HWND hWnd)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    lUpdateColors(pData);

    lRedrawWindow(hWnd);

    return 0;
}

static LRESULT lOnCreateMsg(
  _In_ HWND hWnd,
  _In_ LPCREATESTRUCT pCreateStruct)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)UtAllocMem(sizeof(TDeviceInfoCtrlData));

    pCreateStruct;

    if (pData == NULL)
    {
        return -1;
    }

    ::ZeroMemory(pData, sizeof(TDeviceInfoCtrlData));

    ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

    lUpdateColors(pData);

    pData->bRedraw = TRUE;
    pData->bIgnoreSizeChange = FALSE;

    pData->LastSize.cx = pCreateStruct->cx;
    pData->LastSize.cy = pCreateStruct->cy;

    lLayoutTextPoints(hWnd);

    return 0;
}

static LRESULT lOnDestroyMsg(
  _In_ HWND hWnd)
{
    TDeviceInfoCtrlData* pData = (TDeviceInfoCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pData->hDipSwitchFont)
    {
        ::DeleteObject(pData->hDipSwitchFont);
    }

    for (INT nIndex = 0; nIndex < MArrayLen(pData->pszLabels); ++nIndex)
    {
        UtFreeMem(pData->pszLabels[nIndex]);
    }

    UtFreeMem(pData);

    return 0;
}

static LRESULT CALLBACK lDeviceInfoCtrlWndProc(
  _In_ HWND hWnd,
  _In_ UINT uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam)
{
    switch (uMsg)
    {
        case DIM_SETNAME:
            return lOnSetNameLabelValue(hWnd, (LPCWSTR)lParam);
        case DIM_GETNAME:
            return lOnGetNameLabelValue(hWnd, (LPWSTR)lParam, (LPINT)wParam);
        case DIM_SETADAPTER:
            return lOnSetAdapterLabelValue(hWnd, (LPCWSTR)lParam);
        case DIM_GETADAPTER:
            return lOnGetAdapterLabelValue(hWnd, (LPWSTR)lParam, (LPINT)wParam);
        case DIM_SETDIPSWITCHES:
            return lOnSetDipSwitches(hWnd, (UINT8)lParam);
        case DIM_GETDIPSWITCHES:
            return lOnGetDipSwitches(hWnd, (UINT8*)lParam);
        case DIM_SETSIZE:
            return lOnSetSizeLabelValue(hWnd, (LPCWSTR)lParam);
        case DIM_GETSIZE:
            return lOnGetSizeLabelValue(hWnd, (LPWSTR)lParam, (LPINT)wParam);
        case DIM_SETVPP:
            return lOnSetVppLabelValue(hWnd, (LPCWSTR)lParam);
        case DIM_GETVPP:
            return lOnGetVppLabelValue(hWnd, (LPWSTR)lParam, (LPINT)wParam);
        case DIM_SETBITS:
            return lOnSetBitsLabelValue(hWnd, (LPCWSTR)lParam);
        case DIM_GETBITS:
            return lOnGetBitsLabelValue(hWnd, (LPWSTR)lParam, (LPINT)wParam);
        case DIM_SETCHIPENABLE:
            return lOnSetChipEnableLabelValue(hWnd, (LPCWSTR)lParam);
        case DIM_GETCHIPENABLE:
            return lOnGetChipEnableLabelValue(hWnd, (LPWSTR)lParam, (LPINT)wParam);
        case DIM_SETOUTPUTENABLE:
            return lOnSetOutputEnableLabelValue(hWnd, (LPCWSTR)lParam);
        case DIM_GETOUTPUTENABLE:
            return lOnGetOutputEnableLabelValue(hWnd, (LPWSTR)lParam, (LPINT)wParam);
        case DIM_GETMINRECT:
            return lOnGetMinRect(hWnd, (LPRECT)lParam);
        case WM_PAINT:
            return lOnPaintMsg(hWnd);
        case WM_PRINT:
            return lOnPrintMsg(hWnd, (HDC)wParam, (DWORD)lParam);
        case WM_PRINTCLIENT:
            return lOnPrintClientMsg(hWnd, (HDC)wParam, (DWORD)lParam);
        case WM_SIZE:
            return lOnSizeMsg(hWnd, LOWORD(lParam), HIWORD(lParam));
        case WM_HSCROLL:
            return lOnHScrollMsg(hWnd, LOWORD(wParam), HIWORD(wParam));
        case WM_VSCROLL:
            return lOnVScrollMsg(hWnd, LOWORD(wParam), HIWORD(wParam));
        case WM_SETREDRAW:
            return lOnSetRedrawMsg(hWnd, (BOOL)wParam);
        case WM_GETFONT:
            return lOnGetFont(hWnd);
        case WM_SETFONT:
            return lOnSetFont(hWnd, (HFONT)wParam, LOWORD(lParam));
        case WM_SYSCOLORCHANGE:
            return lOnSysColorChange(hWnd);
        case WM_CREATE:
            return lOnCreateMsg(hWnd, (LPCREATESTRUCT)lParam);
        case WM_DESTROY:
            return lOnDestroyMsg(hWnd);
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

VOID UiDeviceInfoCtrlRegister(VOID)
{
    WNDCLASSEXW WndClassEx;

    WndClassEx.cbSize = sizeof(WndClassEx);
    WndClassEx.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
    WndClassEx.lpfnWndProc = lDeviceInfoCtrlWndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = sizeof(TDeviceInfoCtrlData*);
    WndClassEx.hInstance = UiPepCtrlGetInstance();
    WndClassEx.hIcon = NULL;
    WndClassEx.hCursor = ::LoadCursorW(NULL, IDC_ARROW);
    WndClassEx.hbrBackground = NULL;
    WndClassEx.lpszMenuName = NULL;
    WndClassEx.lpszClassName = CUiDeviceInfoCtrlClass;
    WndClassEx.hIconSm = NULL;

    ::RegisterClassExW(&WndClassEx);
}

VOID UiDeviceInfoCtrlUnregister(VOID)
{
    ::UnregisterClassW(CUiDeviceInfoCtrlClass, UiPepCtrlGetInstance());
}

/***************************************************************************/
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/
