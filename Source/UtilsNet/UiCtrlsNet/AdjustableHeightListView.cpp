/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ListViewItemSequentialSorter.h"
#include "ListViewItemGroupSorter.h"
#include "ListViewItemGroupSequentialSorter.h"

#include "ListViewComboBoxEditEventArgs.h"
#include "ListViewLabelEditPasteEventArgs.h"

#include "ComboBox.h"

#include "ListViewComboBox.h"

#include "ContextMenuStrip.h"
#include "ITextBoxClipboard.h"
#include "EditContextMenuStrip.h"
#include "ITextBoxKeyPress.h"

#include "NativeEdit.h"

#include "ListView.h"

#include "AdjustableHeightListView.h"

#include <Includes/UtMacros.h>
#include <Includes/UiMacros.h>

#include <Utils/UiDrawing.h>

#include <uxtheme.h>
#include <vssym32.h>

#pragma region "Constants"

#define WM_REFLECT 0x2000

#define CMaxListViewItemLabelLen 260 // In the API documentation but no constant defined in the header files.

#define CItemTextPaddingPixels 2
#define CSubItemTextPaddingPixels 6

#define CMouseMoveTimerMs 200

#pragma endregion

#pragma region "Local Functions"

#pragma unmanaged

static BOOL lIsCursorInWindow(
  _In_ HWND hWnd)
{
    RECT Rect;
    POINT Point;

    ::GetWindowRect(hWnd, &Rect);

    Point.x = GET_X_LPARAM(::GetMessagePos());
    Point.y = GET_Y_LPARAM(::GetMessagePos());

    return ::PtInRect(&Rect, Point);
}

static INT lGetColumnCount(
  _In_ HWND hWnd)
{
    return Header_GetItemCount(ListView_GetHeader(hWnd));
}

static void lDrawFocusRect(
  _In_ HDC hDC,
  _In_ LPCRECT pRect)
{
    UINT nBorderWidth, nBorderHeight;
    LOGBRUSH LogBrush;
    HPEN hPen, hOldPen;
    int nOldROP;

    if (!::SystemParametersInfo(SPI_GETFOCUSBORDERWIDTH, 0, &nBorderWidth, 0))
    {
        nBorderWidth = 1;
    }

    if (!::SystemParametersInfo(SPI_GETFOCUSBORDERHEIGHT, 0, &nBorderHeight, 0))
    {
        nBorderHeight = 1;
    }

    LogBrush.lbStyle = BS_SOLID;
    LogBrush.lbColor = RGB(255, 255, 255);
    LogBrush.lbHatch = NULL;

    // Draw left and right portions of the rectangle

    hPen = ::ExtCreatePen(PS_GEOMETRIC | PS_DOT, nBorderWidth, &LogBrush, 0, 0);

    hOldPen = (HPEN)::SelectObject(hDC, hPen);

    nOldROP = ::SetROP2(hDC, R2_XORPEN);

    ::MoveToEx(hDC, pRect->left, pRect->top + 1, NULL);
    ::LineTo(hDC, pRect->left, pRect->bottom);

    ::MoveToEx(hDC, pRect->right - 1, pRect->top + 1, NULL);
    ::LineTo(hDC, pRect->right - 1, pRect->bottom - 1);

    ::SetROP2(hDC, nOldROP);

    ::SelectObject(hDC, hOldPen);

    ::DeleteObject(hPen);

    // Draw top and bottom portions of the rectangle

    hPen = ::ExtCreatePen(PS_GEOMETRIC | PS_DOT, nBorderHeight, &LogBrush, 0, 0);

    hOldPen = (HPEN)::SelectObject(hDC, hPen);

    nOldROP = ::SetROP2(hDC, R2_XORPEN);

    ::MoveToEx(hDC, pRect->left + 1, pRect->top, NULL);
    ::LineTo(hDC, pRect->right, pRect->top);

    ::MoveToEx(hDC, pRect->left + 2, pRect->bottom - 1, NULL);
    ::LineTo(hDC, pRect->right, pRect->bottom - 1);

    ::SetROP2(hDC, nOldROP);

    ::SelectObject(hDC, hOldPen);

    ::DeleteObject(hPen);
}

static void lDrawItemFocusRect(
  _In_ HWND hWnd,
  _In_ HDC hDC,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    INT nColumnCount = lGetColumnCount(hWnd);
    RECT FocusRect, LabelRect, ItemRect;

    if ((ListView_GetExtendedListViewStyle(hWnd) & LVS_EX_FULLROWSELECT) == LVS_EX_FULLROWSELECT)
    {
        ListView_GetItemRect(hWnd, pDrawItemStruct->itemID, &LabelRect, LVIR_LABEL);
        ListView_GetSubItemRect(hWnd, pDrawItemStruct->itemID, nColumnCount - 1, LVIR_BOUNDS, &ItemRect);

        FocusRect.left = LabelRect.left;
        FocusRect.top = 0;
        FocusRect.right = ItemRect.right;
        FocusRect.bottom = MRectHeight(LabelRect);
    }
    else
    {
        ListView_GetItemRect(hWnd, pDrawItemStruct->itemID, &LabelRect, LVIR_LABEL);

        FocusRect.left = LabelRect.left;
        FocusRect.top = 0;
        FocusRect.right = LabelRect.right;
        FocusRect.bottom = MRectHeight(LabelRect);
    }

    // Using DrawFocusRect does not create a dotted outline for some strange reason...

    lDrawFocusRect(hDC, &FocusRect);
}

static void lNoThemeDrawTextBackground(
  _In_ HWND hWnd,
  _In_ HDC hDC,
  _In_ LPCRECT pRect,
  _In_ INT nItemState)
{
    COLORREF crBackColor;

    if ((ODS_SELECTED & nItemState) != ODS_SELECTED)
    {
        if ((ODS_DISABLED & nItemState) != ODS_DISABLED)
        {
            // Item to paint is not disabled

            crBackColor = ::GetSysColor(COLOR_WINDOW);
        }
        else
        {
            // Item to paint is disabled

            crBackColor = ::GetSysColor(COLOR_WINDOW);
        }
    }
    else
    {
        // Item to paint is the currently selected one

        if (hWnd == ::GetFocus())
        {
            crBackColor = ::GetSysColor(COLOR_HIGHLIGHT);
        }
        else
        {
            crBackColor = ::GetSysColor(COLOR_BTNFACE);
        }
    }

    UiDrawingFillSolidRectWithRGB(hDC, crBackColor, pRect->left, pRect->top,
                                  MRectWidth(*pRect), MRectHeight(*pRect));
}

static void lNoThemeDrawIconBackground(
  _In_ HDC hDC,
  _In_ LPCRECT pRect,
  _In_ INT nItemState)
{
    COLORREF crBackColor = ::GetSysColor(COLOR_WINDOW);

    nItemState;

    UiDrawingFillSolidRectWithRGB(hDC, crBackColor, pRect->left, pRect->top,
                                  MRectWidth(*pRect), MRectHeight(*pRect));
}

static void lNoThemeDrawItem(
  _In_ HWND hWnd,
  _In_ INT nRow,
  _In_ HDC hDC,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    HIMAGELIST hImageList = ListView_GetImageList(hWnd, LVSIL_SMALL);
    LVITEM ListViewItem;
    TCHAR cText[CMaxListViewItemLabelLen];
    RECT FullRect, IconRect, LabelRect, Rect;
    INT nImageWidth, nImageHeight;
    UINT nDrawImageListStyle;

    ListViewItem.mask = LVIF_TEXT | LVIF_IMAGE;
    ListViewItem.iItem = nRow;
    ListViewItem.iSubItem = 0;
    ListViewItem.pszText = cText;
    ListViewItem.cchTextMax = MArrayLen(cText);

    ListView_GetItem(hWnd, &ListViewItem);

    ListView_GetItemRect(hWnd, nRow, &FullRect, LVIR_BOUNDS);
    ListView_GetItemRect(hWnd, nRow, &IconRect, LVIR_ICON);
    ListView_GetItemRect(hWnd, nRow, &LabelRect, LVIR_LABEL);

    // Draw item background

    Rect.left = FullRect.left;
    Rect.top = 0;
    Rect.right = FullRect.right;
    Rect.bottom = MRectHeight(FullRect);

    lNoThemeDrawTextBackground(hWnd, hDC, &Rect, pDrawItemStruct->itemState);

    if (hImageList && ListViewItem.iImage > -1)
    {
        // Draw icon background

        Rect.right = LabelRect.left;

        lNoThemeDrawIconBackground(hDC, &Rect, pDrawItemStruct->itemState);

        // Draw icon

        nDrawImageListStyle = ILD_NORMAL | ILD_IMAGE;

        if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED &&
            hWnd == ::GetFocus())
        {
            if ((ODS_SELECTED & pDrawItemStruct->itemState) == ODS_SELECTED)
            {
                nDrawImageListStyle = ILD_SELECTED | ILD_IMAGE;
            }

            if ((ODS_FOCUS & pDrawItemStruct->itemState) == ODS_FOCUS)
            {
                nDrawImageListStyle = ILD_FOCUS | ILD_IMAGE;
            }
        }

        ::ImageList_GetIconSize(hImageList, &nImageWidth, &nImageHeight);

        ::ImageList_Draw(hImageList, ListViewItem.iImage, hDC, IconRect.left,
                         (MRectHeight(IconRect) - nImageHeight) / 2,
                         nDrawImageListStyle);
    }

    Rect.left = LabelRect.left + CItemTextPaddingPixels;
    Rect.top = 0;
    Rect.right = LabelRect.right - CItemTextPaddingPixels;
    Rect.bottom = MRectHeight(LabelRect);

    ::DrawText(hDC, cText, ::lstrlen(cText), &Rect,
               DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
}

static void lNoThemeDrawSubItem(
  _In_ HWND hWnd,
  _In_ INT nRow,
  _In_ INT nColumn,
  _In_ HDC hDC,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    INT nItemState = pDrawItemStruct->itemState;
    LVITEM ListViewItem;
    TCHAR cText[CMaxListViewItemLabelLen];
    RECT ItemRect, Rect;

    ListViewItem.mask = LVIF_TEXT;
    ListViewItem.iItem = nRow;
    ListViewItem.iSubItem = nColumn;
    ListViewItem.pszText = cText;
    ListViewItem.cchTextMax = MArrayLen(cText);

    ListView_GetItem(hWnd, &ListViewItem);

    ListView_GetSubItemRect(hWnd, nRow, nColumn, LVIR_BOUNDS, &ItemRect);

    // Draw background

    if ((ListView_GetExtendedListViewStyle(hWnd) & LVS_EX_FULLROWSELECT) == 0)
    {
        nItemState &= ~ODS_SELECTED;
    }

    Rect.left = ItemRect.left;
    Rect.top = 0;
    Rect.right = ItemRect.right;
    Rect.bottom = MRectHeight(ItemRect);

    lNoThemeDrawTextBackground(hWnd, hDC, &Rect, nItemState);

    // Draw text

    Rect.left = ItemRect.left + CSubItemTextPaddingPixels;
    Rect.top = 0;
    Rect.right = ItemRect.right - CSubItemTextPaddingPixels;
    Rect.bottom = MRectHeight(ItemRect);

    ::DrawText(hDC, cText, ::lstrlen(cText), &Rect,
               DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
}

static void lNoThemeDrawItem(
  _In_ HWND hWnd,
  _In_ HFONT hFont,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    INT nColumnCount = lGetColumnCount(hWnd);
    INT nWidth = MRectWidth(pDrawItemStruct->rcItem);
    INT nHeight = MRectHeight(pDrawItemStruct->rcItem);
    HDC hMemDC = ::CreateCompatibleDC(pDrawItemStruct->hDC);
    HBITMAP hMemBitmap = ::CreateCompatibleBitmap(pDrawItemStruct->hDC, nWidth, nHeight);
    COLORREF crTextColor;

    ::SaveDC(hMemDC);

    ::SelectObject(hMemDC, hMemBitmap);

    if ((ODS_SELECTED & pDrawItemStruct->itemState) != ODS_SELECTED)
    {
        if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
        {
            // Item to paint is not disabled

            crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
        }
        else
        {
            // Item to paint is disabled

            crTextColor = ::GetSysColor(COLOR_3DFACE);
        }
    }
    else
    {
        // Item to paint is the currently selected one

        if (hWnd == ::GetFocus())
        {
            crTextColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
        }
        else
        {
            crTextColor = ::GetSysColor(COLOR_BTNTEXT);
        }
    }

    ::SelectObject(hMemDC, hFont);

    ::SetBkMode(hMemDC, TRANSPARENT);

    ::SetTextColor(hMemDC, crTextColor);

    for (INT nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        if (nColumn == 0)
        {
            lNoThemeDrawItem(hWnd, pDrawItemStruct->itemID, hMemDC,
                             pDrawItemStruct);
        }
        else
        {
            lNoThemeDrawSubItem(hWnd, pDrawItemStruct->itemID, nColumn,
                                hMemDC, pDrawItemStruct);
        }
    }

    // Draw the input focus around the item if applicable

    if ((pDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS &&
        hWnd == ::GetFocus())
    {
        lDrawItemFocusRect(hWnd, hMemDC, pDrawItemStruct);
    }

    ::BitBlt(pDrawItemStruct->hDC,
             pDrawItemStruct->rcItem.left,
             pDrawItemStruct->rcItem.top,
             nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

    ::RestoreDC(hMemDC, -1);

    ::DeleteObject(hMemBitmap);

    ::DeleteDC(hMemDC);
}

static void lWindowsXPDrawTextBackground(
  _In_ HWND hWnd,
  _In_ HTHEME hTheme,
  _In_ HDC hDC,
  _In_ LPCRECT pRect,
  _In_ INT nItemState)
{
    COLORREF crBackColor;

    if ((ODS_SELECTED & nItemState) != ODS_SELECTED)
    {
        if ((ODS_DISABLED & nItemState) != ODS_DISABLED)
        {
            // Item to paint is not disabled

            crBackColor = ::GetThemeSysColor(hTheme, COLOR_WINDOW);
        }
        else
        {
            // Item to paint is disabled

            crBackColor = ::GetThemeSysColor(hTheme, COLOR_WINDOW);
        }
    }
    else
    {
        // Item to paint is the currently selected one

        if (hWnd == ::GetFocus())
        {
            crBackColor = ::GetThemeSysColor(hTheme, COLOR_HIGHLIGHT);
        }
        else
        {
            crBackColor = ::GetThemeSysColor(hTheme, COLOR_BTNFACE);
        }
    }

    UiDrawingFillSolidRectWithRGB(hDC, crBackColor, pRect->left, pRect->top,
                                  MRectWidth(*pRect), MRectHeight(*pRect));
}

static void lWindowsXPDrawIconBackground(
  _In_ HTHEME hTheme,
  _In_ HDC hDC,
  _In_ LPCRECT pRect,
  _In_ INT nItemState)
{
    COLORREF crBackColor = ::GetThemeSysColor(hTheme, COLOR_WINDOW);

    nItemState;

    UiDrawingFillSolidRectWithRGB(hDC, crBackColor, pRect->left, pRect->top,
                                  MRectWidth(*pRect), MRectHeight(*pRect));
}

static void lWindowsXPDrawItem(
  _In_ HWND hWnd,
  _In_ INT nRow,
  _In_ HTHEME hTheme,
  _In_ HDC hDC,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    HIMAGELIST hImageList = ListView_GetImageList(hWnd, LVSIL_SMALL);
    LVITEM ListViewItem;
    TCHAR cText[CMaxListViewItemLabelLen];
    RECT FullRect, IconRect, LabelRect, Rect;
    INT nImageWidth, nImageHeight;
    UINT nDrawImageListStyle;

    ListViewItem.mask = LVIF_TEXT | LVIF_IMAGE;
    ListViewItem.iItem = nRow;
    ListViewItem.iSubItem = 0;
    ListViewItem.pszText = cText;
    ListViewItem.cchTextMax = MArrayLen(cText);

    ListView_GetItem(hWnd, &ListViewItem);

    ListView_GetItemRect(hWnd, nRow, &FullRect, LVIR_BOUNDS);
    ListView_GetItemRect(hWnd, nRow, &IconRect, LVIR_ICON);
    ListView_GetItemRect(hWnd, nRow, &LabelRect, LVIR_LABEL);

    // Draw item background

    Rect.left = FullRect.left;
    Rect.top = 0;
    Rect.right = FullRect.right;
    Rect.bottom = MRectHeight(FullRect);

    lWindowsXPDrawTextBackground(hWnd, hTheme, hDC, &Rect, pDrawItemStruct->itemState);

    if (hImageList && ListViewItem.iImage > -1)
    {
        // Draw icon background

        Rect.right = LabelRect.left;

        lWindowsXPDrawIconBackground(hTheme, hDC, &Rect, pDrawItemStruct->itemState);

        // Draw icon

        nDrawImageListStyle = ILD_NORMAL | ILD_IMAGE;

        if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED &&
            hWnd == ::GetFocus())
        {
            if ((ODS_SELECTED & pDrawItemStruct->itemState) == ODS_SELECTED)
            {
                nDrawImageListStyle = ILD_SELECTED | ILD_IMAGE;
            }

            if ((ODS_FOCUS & pDrawItemStruct->itemState) == ODS_FOCUS)
            {
                nDrawImageListStyle = ILD_FOCUS | ILD_IMAGE;
            }
        }

        ::ImageList_GetIconSize(hImageList, &nImageWidth, &nImageHeight);

        ::ImageList_Draw(hImageList, ListViewItem.iImage, hDC, IconRect.left,
                         (MRectHeight(IconRect) - nImageHeight) / 2,
                         nDrawImageListStyle);
    }

    Rect.left = LabelRect.left + CItemTextPaddingPixels;
    Rect.top = 0;
    Rect.right = LabelRect.right - CItemTextPaddingPixels;
    Rect.bottom = MRectHeight(LabelRect);

    ::DrawText(hDC, cText, ::lstrlen(cText), &Rect,
               DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
}

static void lWindowsXPDrawSubItem(
  _In_ HWND hWnd,
  _In_ INT nRow,
  _In_ INT nColumn,
  _In_ HTHEME hTheme,
  _In_ HDC hDC,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    INT nItemState = pDrawItemStruct->itemState;
    LVITEM ListViewItem;
    TCHAR cText[CMaxListViewItemLabelLen];
    RECT ItemRect, Rect;

    ListViewItem.mask = LVIF_TEXT;
    ListViewItem.iItem = nRow;
    ListViewItem.iSubItem = nColumn;
    ListViewItem.pszText = cText;
    ListViewItem.cchTextMax = MArrayLen(cText);

    ListView_GetItem(hWnd, &ListViewItem);

    ListView_GetSubItemRect(hWnd, nRow, nColumn, LVIR_BOUNDS, &ItemRect);

    // Draw background

    if ((ListView_GetExtendedListViewStyle(hWnd) & LVS_EX_FULLROWSELECT) == 0)
    {
        nItemState &= ~ODS_SELECTED;
    }

    Rect.left = ItemRect.left;
    Rect.top = 0;
    Rect.right = ItemRect.right;
    Rect.bottom = MRectHeight(ItemRect);

    lWindowsXPDrawTextBackground(hWnd, hTheme, hDC, &Rect, nItemState);

    // Draw text

    Rect.left = ItemRect.left + CSubItemTextPaddingPixels;
    Rect.top = 0;
    Rect.right = ItemRect.right - CSubItemTextPaddingPixels;
    Rect.bottom = MRectHeight(ItemRect);

    ::DrawText(hDC, cText, ::lstrlen(cText), &Rect,
               DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
}

static void lWindowsXPThemeDrawItem(
  _In_ HWND hWnd,
  _In_ HFONT hFont,
  _In_ HTHEME hTheme,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    INT nColumnCount = lGetColumnCount(hWnd);
    INT nWidth = MRectWidth(pDrawItemStruct->rcItem);
    INT nHeight = MRectHeight(pDrawItemStruct->rcItem);
    HDC hMemDC = ::CreateCompatibleDC(pDrawItemStruct->hDC);
    HBITMAP hMemBitmap = ::CreateCompatibleBitmap(pDrawItemStruct->hDC, nWidth, nHeight);
    COLORREF crTextColor;

    ::SaveDC(hMemDC);

    ::SelectObject(hMemDC, hMemBitmap);

    if ((ODS_SELECTED & pDrawItemStruct->itemState) != ODS_SELECTED)
    {
        if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
        {
            // Item to paint is not disabled

            crTextColor = ::GetThemeSysColor(hTheme, COLOR_WINDOWTEXT);
        }
        else
        {
            // Item to paint is disabled

            crTextColor = ::GetThemeSysColor(hTheme, COLOR_3DFACE);
        }
    }
    else
    {
        // Item to paint is the currently selected one

        if (hWnd == ::GetFocus())
        {
            crTextColor = ::GetThemeSysColor(hTheme, COLOR_HIGHLIGHTTEXT);
        }
        else
        {
            crTextColor = ::GetThemeSysColor(hTheme, COLOR_BTNTEXT);
        }
    }

    ::SelectObject(hMemDC, hFont);

    ::SetBkMode(hMemDC, TRANSPARENT);

    ::SetTextColor(hMemDC, crTextColor);

    for (INT nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        if (nColumn == 0)
        {
            lWindowsXPDrawItem(hWnd, pDrawItemStruct->itemID, hTheme, hMemDC,
                               pDrawItemStruct);
        }
        else
        {
            lWindowsXPDrawSubItem(hWnd, pDrawItemStruct->itemID, nColumn,
                                  hTheme, hMemDC, pDrawItemStruct);
        }
    }

    // Draw the input focus around the item if applicable

    if ((pDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS &&
        hWnd == ::GetFocus())
    {
        lDrawItemFocusRect(hWnd, hMemDC, pDrawItemStruct);
    }

    ::BitBlt(pDrawItemStruct->hDC,
             pDrawItemStruct->rcItem.left,
             pDrawItemStruct->rcItem.top,
             nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

    ::RestoreDC(hMemDC, -1);

    ::DeleteObject(hMemBitmap);

    ::DeleteDC(hMemDC);
}

static void lAeroDrawItem(
  _In_ HWND hWnd,
  _In_ INT nRow,
  _In_ HDC hDC,
  _In_ HTHEME hTheme,
  _In_ INT nPartId,
  _In_ INT nStateId)
{
    HIMAGELIST hImageList = ListView_GetImageList(hWnd, LVSIL_SMALL);
    LVITEM ListViewItem;
    TCHAR cText[CMaxListViewItemLabelLen];
    RECT FullRect, IconRect, LabelRect, Rect;
    INT nImageWidth, nImageHeight;

    ListViewItem.mask = LVIF_TEXT | LVIF_IMAGE;
    ListViewItem.iItem = nRow;
    ListViewItem.iSubItem = 0;
    ListViewItem.pszText = cText;
    ListViewItem.cchTextMax = MArrayLen(cText);

    ListView_GetItem(hWnd, &ListViewItem);

    ListView_GetItemRect(hWnd, nRow, &FullRect, LVIR_BOUNDS);
    ListView_GetItemRect(hWnd, nRow, &IconRect, LVIR_ICON);
    ListView_GetItemRect(hWnd, nRow, &LabelRect, LVIR_LABEL);

    if (hImageList && ListViewItem.iImage > -1)
    {
        ::ImageList_GetIconSize(hImageList, &nImageWidth, &nImageHeight);

        // Drawing the image with the DrawThemeIcon causes the image to have a blend
        // effect applied to the edge.

#if defined(ENABLE_BUFFERED_DRAWING)
        ::ImageList_Draw(hImageList, ListViewItem.iImage, hDC, IconRect.left,
                         IconRect.top + ((MRectHeight(IconRect) - nImageHeight) / 2),
                         ILD_NORMAL | ILD_IMAGE);
#else
        ::ImageList_Draw(hImageList, ListViewItem.iImage, hDC, IconRect.left,
                         ((MRectHeight(IconRect) - nImageHeight) / 2),
                         ILD_NORMAL | ILD_IMAGE);
#endif
    }

#if defined(ENABLE_BUFFERED_DRAWING)
    Rect.left = LabelRect.left + CItemTextPaddingPixels;
    Rect.top = LabelRect.top;
    Rect.right = LabelRect.right - CItemTextPaddingPixels;
    Rect.bottom = LabelRect.top + MRectHeight(LabelRect);
#else
    Rect.left = LabelRect.left + CItemTextPaddingPixels;
    Rect.top = 0;
    Rect.right = LabelRect.right - CItemTextPaddingPixels;
    Rect.bottom = MRectHeight(LabelRect);
#endif

    ::DrawThemeText(hTheme, hDC, nPartId, nStateId, cText, -1,
                    DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER,
                    0, &Rect);
}

static void lAeroDrawSubItem(
  _In_ HWND hWnd,
  _In_ INT nRow,
  _In_ INT nColumn,
  _In_ HDC hDC,
  _In_ HTHEME hTheme,
  _In_ INT nPartId,
  _In_ INT nStateId)
{
    LVITEM ListViewItem;
    TCHAR cText[CMaxListViewItemLabelLen];
    RECT ItemRect, Rect;

    ListViewItem.mask = LVIF_TEXT;
    ListViewItem.iItem = nRow;
    ListViewItem.iSubItem = nColumn;
    ListViewItem.pszText = cText;
    ListViewItem.cchTextMax = MArrayLen(cText);

    ListView_GetItem(hWnd, &ListViewItem);

    ListView_GetSubItemRect(hWnd, nRow, nColumn, LVIR_BOUNDS, &ItemRect);

#if defined(ENABLE_BUFFERED_DRAWING)
    Rect.left = ItemRect.left + CSubItemTextPaddingPixels;
    Rect.top = ItemRect.top;
    Rect.right = ItemRect.right - CSubItemTextPaddingPixels;
    Rect.bottom = ItemRect.top + MRectHeight(ItemRect);
#else
    Rect.left = ItemRect.left + CSubItemTextPaddingPixels;
    Rect.top = 0;
    Rect.right = ItemRect.right - CSubItemTextPaddingPixels;
    Rect.bottom = MRectHeight(ItemRect);
#endif

    ::DrawThemeText(hTheme, hDC, nPartId, nStateId, cText, -1,
                    DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER,
                    0, &Rect);
}

static void lAeroThemedDrawItem(
  _In_ HWND hWnd,
  _In_ HFONT hFont,
  _In_ HTHEME hTheme,
  _In_ LPDRAWITEMSTRUCT pDrawItemStruct)
{
    INT nColumnCount = lGetColumnCount(hWnd);
    HWND hHeaderWnd = ListView_GetHeader(hWnd);
#if defined(ENABLE_BUFFERED_DRAWING)
    HPAINTBUFFER hBufferedPaint = NULL;
    HDC hDC = NULL;
#else
    INT nWidth = MRectWidth(pDrawItemStruct->rcItem);
    INT nHeight = MRectHeight(pDrawItemStruct->rcItem);
    HDC hMemDC = ::CreateCompatibleDC(pDrawItemStruct->hDC);
    HBITMAP hMemBitmap = ::CreateCompatibleBitmap(pDrawItemStruct->hDC, nWidth, nHeight);
    HDC hDC = hMemDC;
    RECT Rect;
#endif
    RECT HeaderRect;
    LV_HITTESTINFO HitTestInfo;
    int nPartId, nStateId;

#if defined(ENABLE_BUFFERED_DRAWING)
#else
    Rect.left = 0;
    Rect.top = 0;
    Rect.right = MRectWidth(pDrawItemStruct->rcItem);
    Rect.bottom = MRectHeight(pDrawItemStruct->rcItem);
#endif

    nPartId = LVP_LISTITEM;

    HitTestInfo.pt.x = GET_X_LPARAM(::GetMessagePos());
    HitTestInfo.pt.y = GET_Y_LPARAM(::GetMessagePos());

    ::GetWindowRect(hHeaderWnd, &HeaderRect);

    if (!::PtInRect(&HeaderRect, HitTestInfo.pt))
    {
        ::ScreenToClient(hWnd, &HitTestInfo.pt);

        ListView_SubItemHitTest(hWnd, &HitTestInfo);
    }
    else
    {
        HitTestInfo.iItem = -1;
        HitTestInfo.iSubItem = -1;
    }

    if (HitTestInfo.iSubItem != -1)
    {
        if ((ODS_SELECTED & pDrawItemStruct->itemState) != ODS_SELECTED)
        {
            // Item not selected

            if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
            {
                // Item is not disabled

                if (HitTestInfo.iItem == (INT)pDrawItemStruct->itemID)
                {
                    nStateId = LISS_HOT;
                }
                else
                {
                    nStateId = LISS_NORMAL;
                }
            }
            else
            {
                // Item is disabled

                nStateId = LISS_DISABLED;
            }
        }
        else
        {
            // Item selected

            if ((ODS_FOCUS & pDrawItemStruct->itemState) != ODS_FOCUS)
            {
                // Item does not have the focus

                if (hWnd == ::GetFocus()) 
                {
                    if (HitTestInfo.iItem == (INT)pDrawItemStruct->itemID)
                    {
                        nStateId = LISS_HOTSELECTED;
                    }
                    else
                    {
                        nStateId = LISS_SELECTED;
                    }
                }
                else
                {
                    // So far this is never called

                    if (HitTestInfo.iItem == (INT)pDrawItemStruct->itemID)
                    {
                        nStateId = LISS_SELECTEDNOTFOCUS;
                    }
                    else
                    {
                        nStateId = LISS_SELECTEDNOTFOCUS;
                    }
                }
            }
            else
            {
                // Item has focus

                if (hWnd == ::GetFocus())  
                {
                    if (HitTestInfo.iItem == (INT)pDrawItemStruct->itemID)
                    {
                        nStateId = LISS_HOTSELECTED;
                    }
                    else
                    {
                        nStateId = LISS_SELECTED;
                    }
                }
                else 
                {
                    if (HitTestInfo.iItem == (INT)pDrawItemStruct->itemID)
                    {
                        nStateId = LISS_HOTSELECTED;
                    }
                    else
                    {
                        nStateId = LISS_SELECTEDNOTFOCUS;
                    }
                }
            }
        }
    }
    else
    {
        // Mouse cursor not over an item or sub-item

        if ((ODS_SELECTED & pDrawItemStruct->itemState) != ODS_SELECTED)
        {
            // Item not selected

            if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
            {
                // Item is not disabled

                nStateId = LISS_NORMAL;
            }
            else
            {
                // Item is disabled

                nStateId = LISS_DISABLED;
            }
        }
        else
        {
            // Item selected

            if (hWnd == ::GetFocus())
            {
                if (::GetCapture() == NULL)
                {
                    nStateId = LISS_SELECTED;
                }
                else
                {
                    nStateId = LISS_NORMAL;
                }
            }
            else
            {
                nStateId = LISS_SELECTEDNOTFOCUS;
            }
        }
    }

#if defined(ENABLE_BUFFERED_DRAWING)
    hBufferedPaint = ::BeginBufferedPaint(pDrawItemStruct->hDC,
                                          &pDrawItemStruct->rcItem,
                                          BPBF_TOPDOWNDIB,
                                          NULL,
                                          &hDC);

    ::SaveDC(hDC);

    ::SelectObject(hDC, hFont);
#else
    ::SaveDC(hMemDC);

    ::SelectObject(hMemDC, hMemBitmap);
    ::SelectObject(hMemDC, hFont);
#endif

    if (nStateId == LISS_NORMAL)
    {
        // Drawing with the theme causes each row to have a one pixel gray border outline.

#if defined(ENABLE_BUFFERED_DRAWING)
        UiDrawingFillSolidRectWithRGB(hDC, ::GetThemeSysColor(hTheme, COLOR_WINDOW),
                                      pDrawItemStruct->rcItem.left,
                                      pDrawItemStruct->rcItem.top,
                                      MRectWidth(pDrawItemStruct->rcItem),
                                      MRectHeight(pDrawItemStruct->rcItem));
#else
        UiDrawingFillSolidRectWithRGB(hMemDC, ::GetThemeSysColor(hTheme, COLOR_WINDOW), 0, 0,
                                      MRectWidth(pDrawItemStruct->rcItem),
                                      MRectHeight(pDrawItemStruct->rcItem));
#endif
    }
    else
    {
        if (::IsThemeBackgroundPartiallyTransparent(hTheme, nPartId, nStateId))
        {
#if defined(ENABLE_BUFFERED_DRAWING)
            ::DrawThemeParentBackground(hWnd, hDC, &pDrawItemStruct->rcItem);
#else
            ::DrawThemeParentBackground(hWnd, hMemDC, &Rect);
#endif
        }

#if defined(ENABLE_BUFFERED_DRAWING)
        ::DrawThemeBackground(hTheme, hDC, nPartId, nStateId,
                              &pDrawItemStruct->rcItem, NULL);
#else
        ::DrawThemeBackground(hTheme, hMemDC, nPartId, nStateId,
                              &Rect, NULL);
#endif
    }

    for (INT nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        if (nColumn == 0)
        {
            lAeroDrawItem(hWnd, pDrawItemStruct->itemID, hDC, hTheme,
                          nPartId, nStateId);
        }
        else
        {
            lAeroDrawSubItem(hWnd, pDrawItemStruct->itemID, nColumn, hDC,
                             hTheme, nPartId, nStateId);
        }
    }

#if defined(ENABLE_BUFFERED_DRAWING)
    ::RestoreDC(hDC, -1);

    ::EndBufferedPaint(hBufferedPaint, TRUE);
#else
    ::BitBlt(pDrawItemStruct->hDC,
             pDrawItemStruct->rcItem.left,
             pDrawItemStruct->rcItem.top,
             nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

    ::RestoreDC(hMemDC, -1);

    ::DeleteObject(hMemBitmap);

    ::DeleteDC(hMemDC);
#endif
}

#pragma managed

#pragma endregion

#pragma region "Constructor"

Common::Forms::AdjustableHeightListView::AdjustableHeightListView() :
  m_nItemHeight(15),
  m_Timer(nullptr),
  m_hListViewTheme(NULL),
  m_nMouseMoveLastItem(-1)
{
	InitializeComponent();

    SetStyle(System::Windows::Forms::ControlStyles::OptimizedDoubleBuffer |
             System::Windows::Forms::ControlStyles::AllPaintingInWmPaint, true);
}

#pragma endregion

#pragma region "Destructor"

Common::Forms::AdjustableHeightListView::~AdjustableHeightListView()
{
    if (components)
    {
        delete components;
    }
}

#pragma endregion

void Common::Forms::AdjustableHeightListView::WndProc(
  System::Windows::Forms::Message% Message)
{
    switch (Message.Msg)
    {
        case WM_NOTIFY:
            HandleNotify(Message);
            break;
        case WM_REFLECT + WM_MEASUREITEM:
            HandleMeasureItem(Message);
            break;
        case WM_REFLECT + WM_DRAWITEM:
            HandleDrawItem(Message);
            break;
        case WM_MOUSEMOVE:
            HandleMouseMove(Message);
            break;
        case WM_MOVE:
            HandleMove(Message);
            break;
        case WM_SIZE:
            HandleSize(Message);
            break;
        case WM_THEMECHANGED:
            HandleThemeChanged(Message);
            break;
        case WM_CREATE:
            HandleCreate(Message);
            break;
        case WM_DESTROY:
            HandleDestroy(Message);
            break;
        default:
            Common::Forms::ListView::WndProc(Message);
            break;
    }
}

void Common::Forms::AdjustableHeightListView::HandleNotify(
  System::Windows::Forms::Message% Message)
{
    LPNMHDR pNotifyHeader = (LPNMHDR)Message.LParam.ToPointer();

    if (pNotifyHeader->code == NM_CUSTOMDRAW)
    {
        Message.Result = System::IntPtr(CDRF_SKIPDEFAULT);
    }
    else
    {
        Common::Forms::ListView::WndProc(Message);
    }
}

void Common::Forms::AdjustableHeightListView::HandleMeasureItem(
  System::Windows::Forms::Message% Message)
{
    LPMEASUREITEMSTRUCT pMeasureItem = (LPMEASUREITEMSTRUCT)Message.LParam.ToPointer();

    pMeasureItem->itemHeight = m_nItemHeight;

    Message.Result = System::IntPtr(1);
}

void Common::Forms::AdjustableHeightListView::HandleDrawItem(
  System::Windows::Forms::Message% Message)
{
    LPDRAWITEMSTRUCT pDrawItemStruct = (LPDRAWITEMSTRUCT)Message.LParam.ToPointer();
    HWND hWnd = (HWND)Handle.ToPointer();
    HFONT hFont = (HFONT)::SendMessage(hWnd, WM_GETFONT, 0, 0);

    if (m_hListViewTheme)
    {
        if (IsWindowsVistaOrGreater())
        {
            lAeroThemedDrawItem(hWnd, hFont, m_hListViewTheme, pDrawItemStruct);
        }
        else
        {
            lWindowsXPThemeDrawItem(hWnd, hFont, m_hListViewTheme, pDrawItemStruct);
        }
    }
    else
    {
        lNoThemeDrawItem(hWnd, hFont, pDrawItemStruct);
    }
    
    Message.Result = System::IntPtr(1);
}

void Common::Forms::AdjustableHeightListView::HandleMouseMove(
  System::Windows::Forms::Message% Message)
{
    POINT Point;

    Common::Forms::ListView::WndProc(Message);

    if (!HotTracking && ::GetCapture() == NULL)
    {
        m_Timer->Start();

        Point.x = GET_X_LPARAM(Message.LParam.ToInt32());
        Point.y = GET_Y_LPARAM(Message.LParam.ToInt32());

        ProcessMouseMove(&Point);
    }
}

void Common::Forms::AdjustableHeightListView::HandleMove(
  System::Windows::Forms::Message% Message)
{
    Common::Forms::ListView::WndProc(Message);

    if (m_Timer->Enabled)
    {
        ProcessTimerEvent();
    }
}

void Common::Forms::AdjustableHeightListView::HandleSize(
  System::Windows::Forms::Message% Message)
{
    Common::Forms::ListView::WndProc(Message);

    if (m_Timer->Enabled)
    {
        ProcessTimerEvent();
    }
}

void Common::Forms::AdjustableHeightListView::HandleThemeChanged(
  System::Windows::Forms::Message% Message)
{
    Common::Forms::ListView::WndProc(Message);

    if (m_hListViewTheme)
    {
        ::CloseThemeData(m_hListViewTheme);

        m_hListViewTheme = NULL;
    }

    if (::IsThemeActive())
    {
        m_hListViewTheme = ::OpenThemeData((HWND)Handle.ToPointer(), VSCLASS_LISTVIEW);
    }
}

void Common::Forms::AdjustableHeightListView::HandleCreate(
  System::Windows::Forms::Message% Message)
{
    HWND hWnd = (HWND)Message.HWnd.ToPointer();

    Common::Forms::ListView::WndProc(Message);

    if (::IsThemeActive())
    {
        m_hListViewTheme = ::OpenThemeData(hWnd, VSCLASS_LISTVIEW);
    }

    m_Timer = gcnew System::Windows::Forms::Timer();

    m_Timer->Interval = CMouseMoveTimerMs;
    m_Timer->Tag = this;

    m_Timer->Tick += gcnew System::EventHandler(TimerEventProcessor);

    m_Timer->Start();
}

void Common::Forms::AdjustableHeightListView::HandleDestroy(
  System::Windows::Forms::Message% Message)
{
    Common::Forms::ListView::WndProc(Message);

    if (m_hListViewTheme)
    {
        ::CloseThemeData(m_hListViewTheme);
    }

    m_Timer->Stop();

    m_Timer->Tick -= gcnew System::EventHandler(TimerEventProcessor);

    m_Timer->Tag = nullptr;

    delete m_Timer;

    m_Timer = nullptr;
}

void Common::Forms::AdjustableHeightListView::ProcessMouseMove(
  LPPOINT pPoint)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    HWND hHeaderWnd = ListView_GetHeader(hWnd);
    POINT HeaderPoint = *pPoint;
    RECT Rect;
    LV_HITTESTINFO HitTestInfo;
    INT nIndex;

    ::GetWindowRect(hHeaderWnd, &Rect);

    ::ClientToScreen(hWnd, &HeaderPoint);

    if (!::PtInRect(&Rect, HeaderPoint))
    {
        HitTestInfo.pt = *pPoint;

        ListView_SubItemHitTest(hWnd, &HitTestInfo);
    }
    else
    {
        HitTestInfo.iItem = -1;
        HitTestInfo.iSubItem = -1;
    }

    if (HitTestInfo.iItem != -1 && HitTestInfo.iSubItem == -1)
    {
        HitTestInfo.iItem = -1;
    }

    if (HitTestInfo.iItem != -1)
    {
        if (m_nMouseMoveLastItem != -1)
        {
            if (HitTestInfo.iItem != m_nMouseMoveLastItem)
            {
                if (m_nMouseMoveLastItem < HitTestInfo.iItem)
                {
                    ListView_RedrawItems(hWnd, m_nMouseMoveLastItem, HitTestInfo.iItem);
                }
                else
                {
                    ListView_RedrawItems(hWnd, HitTestInfo.iItem, m_nMouseMoveLastItem);
                }
            }
        }
        else
        {
            ListView_RedrawItems(hWnd, HitTestInfo.iItem, HitTestInfo.iItem);
        }
    }
    else
    {
        if (m_nMouseMoveLastItem != -1)
        {
            ListView_RedrawItems(hWnd, m_nMouseMoveLastItem, m_nMouseMoveLastItem);
        }

        if (!lIsCursorInWindow(hWnd))
        {
            nIndex = ListView_GetSelectionMark(hWnd);

            if (nIndex != -1 && m_nMouseMoveLastItem != nIndex)
            {
                ListView_RedrawItems(hWnd, nIndex, nIndex);
            }
        }
    }

    m_nMouseMoveLastItem = HitTestInfo.iItem;
}

void Common::Forms::AdjustableHeightListView::ProcessTimerEvent(void)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    POINT Point;

    if (::GetCapture() == NULL && !lIsCursorInWindow(hWnd))
    {
        m_Timer->Stop();

        Point.x = GET_X_LPARAM(::GetMessagePos());
        Point.y = GET_Y_LPARAM(::GetMessagePos());

        ::ScreenToClient(hWnd, &Point);

        ProcessMouseMove(&Point);
    }
}

void Common::Forms::AdjustableHeightListView::TimerEventProcessor(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    System::Windows::Forms::Timer^ Timer = (System::Windows::Forms::Timer^)Object;
    AdjustableHeightListView^ ListView = (AdjustableHeightListView^)Timer->Tag;

    EventArgs;

    ListView->ProcessTimerEvent();
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
