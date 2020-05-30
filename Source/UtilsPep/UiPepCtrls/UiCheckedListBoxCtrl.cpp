/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsPep/UiPepCtrls.h>

#include "UiPepCtrlsUtil.h"

#include "UiListBoxCtrl.h"

#include <Utils/UtHeap.h>

#include <Includes/UtMacros.h>
#include <Includes/UiMacros.h>

#include <strsafe.h>
#include <windowsx.h>
#include <uxtheme.h>
#include <vssym32.h>

#include <assert.h>

#pragma region "Constants"

#define CCheckBoxBorder 2

#define CLabelMargin 2

#define CListBoxThemeClassName L"ListBox"
#define CButtonThemeClassName L"Button"

#define CLBM_INTERNAL_SETFOCUS (WM_USER + 0x0050)

#pragma endregion

#pragma region "Structures"

typedef struct tagTCheckedListBoxCtrlData
{
	HWND hListBox;
	HFONT hFont;
	HTHEME hListBoxTheme;
	HTHEME hButtonTheme;
} TCheckedListBoxCtrlData;

typedef struct tagTListBoxItemData
{
	DWORD dwState;
	TCHAR cText[1];
} TListBoxItemData;

#pragma endregion

#pragma region "Local Functions"

static VOID lCalcRects(
  _In_ LPCRECT pItemRect,
  _Out_ LPRECT pCheckBoxRect,
  _Out_ LPRECT pInnerCheckBoxRect,
  _Out_ LPRECT pLabelRect)
{
	pCheckBoxRect->left = pItemRect->left;
	pCheckBoxRect->top = pItemRect->top;
	pCheckBoxRect->right = pItemRect->left + MRectHeight(*pItemRect);
	pCheckBoxRect->bottom = pItemRect->top + MRectHeight(*pItemRect);

	pInnerCheckBoxRect->left = pCheckBoxRect->left + CCheckBoxBorder;
	pInnerCheckBoxRect->top = pCheckBoxRect->top + CCheckBoxBorder;
	pInnerCheckBoxRect->right = pCheckBoxRect->right - CCheckBoxBorder;
	pInnerCheckBoxRect->bottom = pCheckBoxRect->bottom - CCheckBoxBorder;

	pLabelRect->left = pCheckBoxRect->right;
	pLabelRect->top = pItemRect->top;
	pLabelRect->right = pItemRect->right;
	pLabelRect->bottom = pItemRect->bottom;
}

static VOID lCalcRects(
  _In_ INT nWidth,
  _In_ INT nHeight,
  _Out_ LPRECT pCheckBoxRect,
  _Out_ LPRECT pInnerCheckBoxRect,
  _Out_ LPRECT pLabelRect)
{
	RECT ItemRect;

	ItemRect.left = 0;
	ItemRect.top = 0;
	ItemRect.right = nWidth;
	ItemRect.bottom = nHeight;

	lCalcRects(&ItemRect, pCheckBoxRect, pInnerCheckBoxRect, pLabelRect);
}

static VOID lChangeItemState(
  _In_ TListBoxItemData* pItemData)
{
	switch (pItemData->dwState)
	{
    	case CCheckedListBoxUncheckedState:
	    	pItemData->dwState = CCheckedListBoxCheckedState;
		   break;
	    case CCheckedListBoxCheckedState:
		    pItemData->dwState = CCheckedListBoxUncheckedState;
		    break;
	    case CCheckedListBoxIndeterminateState:
		    pItemData->dwState = CCheckedListBoxUncheckedState;
		    break;
	}
}

static VOID lSendChangeItemStateNotification(
  _In_ HWND hWnd,
  _In_ INT nIndex,
  _In_ DWORD dwState)
{
	TUiCheckedListBoxCtrlNMStateChange HdrStateChange;

	HdrStateChange.Hdr.hwndFrom = hWnd;
	HdrStateChange.Hdr.idFrom = ::GetDlgCtrlID(hWnd);
	HdrStateChange.Hdr.code = CLBNM_STATECHANGE;
	HdrStateChange.nIndex = nIndex;
	HdrStateChange.dwNewState = dwState;

	::SendMessage(::GetParent(hWnd), WM_NOTIFY, HdrStateChange.Hdr.idFrom, (LPARAM)&HdrStateChange);
}

static VOID lNonThemedDrawItem(
  HWND hWnd,
  TCheckedListBoxCtrlData* pCheckedListBoxCtrlData,
  LPDRAWITEMSTRUCT pDrawItemStruct)
{
	TListBoxItemData* pItemData = (TListBoxItemData*)pDrawItemStruct->itemData;
	INT nWidth = MRectWidth(pDrawItemStruct->rcItem);
	INT nHeight = MRectHeight(pDrawItemStruct->rcItem);
	HDC hMemDC = ::CreateCompatibleDC(pDrawItemStruct->hDC);
	HBITMAP hMemBitmap = ::CreateCompatibleBitmap(pDrawItemStruct->hDC, nWidth, nHeight);
	COLORREF crTextColor, crBackColor;
	RECT CheckBoxRect, InnerCheckBoxRect, LabelRect;
	POINT Point;
	SIZE Size;
	UINT nState;

	hWnd;

	lCalcRects(nWidth, nHeight, &CheckBoxRect, &InnerCheckBoxRect, &LabelRect);

	::SaveDC(hMemDC);

	::SelectObject(hMemDC, hMemBitmap);

	// Paint the item's background

	if ((ODS_SELECTED & pDrawItemStruct->itemState) != ODS_SELECTED)
	{
		if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
		{
			// Item to paint is not disabled

			crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
			crBackColor = ::GetSysColor(COLOR_WINDOW);
		}
		else
		{
			// Item to paint is disabled

			crTextColor = ::GetSysColor(COLOR_3DFACE);
			crBackColor = ::GetSysColor(COLOR_WINDOW);
		}
	}
	else
	{
		// Item to paint is the currently selected one

		crTextColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		crBackColor = ::GetSysColor(COLOR_HIGHLIGHT);
	} // end of if...else statement

	UiPepCtrlFillSolidRect(hMemDC, ::GetSysColor(COLOR_WINDOW),
                           CheckBoxRect.left, CheckBoxRect.top,
                           MRectWidth(CheckBoxRect),
                           MRectHeight(CheckBoxRect));

	UiPepCtrlFillSolidRect(hMemDC, crBackColor,
                           LabelRect.left, LabelRect.top,
                           MRectWidth(LabelRect),
                           MRectHeight(LabelRect));

	UiPepCtrlSelectTwipsMode(hMemDC);

	::SelectObject(hMemDC, pCheckedListBoxCtrlData->hFont);

	::SetBkMode(hMemDC, TRANSPARENT);

	::SetTextColor(hMemDC, crTextColor);
	::SetBkColor(hMemDC, crBackColor);

	Point.x = LabelRect.left + CLabelMargin;
	Point.y = LabelRect.bottom;

	::DPtoLP(hMemDC, &Point, 1);

	::GetTextExtentPoint32(hMemDC, pItemData->cText, ::lstrlen(pItemData->cText), &Size);

	Point.y = (Point.y - Size.cy) / 2;

	::TextOutW(hMemDC, Point.x, Point.y, pItemData->cText, ::lstrlenW(pItemData->cText));

	UiPepCtrlSelectDevUnitsMode(hMemDC);

	switch (pItemData->dwState)
	{
        case CCheckedListBoxUncheckedState:
            nState = DFCS_BUTTONCHECK;
            break;
        case CCheckedListBoxCheckedState:
            nState = DFCS_BUTTONCHECK | DFCS_CHECKED;
            break;
        case CCheckedListBoxIndeterminateState:
            nState = DFCS_BUTTON3STATE | DFCS_CHECKED;
            break;
    }

	nState |= DFCS_MONO;

	if ((ODS_DISABLED & pDrawItemStruct->itemState) == ODS_DISABLED)
	{
		nState |= DFCS_INACTIVE;
	}

	::DrawFrameControl(hMemDC, &InnerCheckBoxRect, DFC_BUTTON, nState);

	// Draw the input focus around the item if applicable

	if ((pDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS)
	{
		::DrawFocusRect(hMemDC, &LabelRect);
	}

	::BitBlt(pDrawItemStruct->hDC,
             pDrawItemStruct->rcItem.left,
             pDrawItemStruct->rcItem.top,
             nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

	::RestoreDC(hMemDC, -1);

	::DeleteObject(hMemBitmap);

	::DeleteDC(hMemDC);
}

static VOID lThemedDrawItem(
  HWND hWnd,
  TCheckedListBoxCtrlData* pCheckedListBoxCtrlData,
  LPDRAWITEMSTRUCT pDrawItemStruct)
{
	TListBoxItemData* pItemData = (TListBoxItemData*)pDrawItemStruct->itemData;
	INT nWidth = MRectWidth(pDrawItemStruct->rcItem);
	INT nHeight = MRectHeight(pDrawItemStruct->rcItem);
	HDC hMemDC = ::CreateCompatibleDC(pDrawItemStruct->hDC);
	HBITMAP hMemBitmap = ::CreateCompatibleBitmap(pDrawItemStruct->hDC, nWidth, nHeight);
	COLORREF crTextColor, crBackColor;
	RECT CheckBoxRect, InnerCheckBoxRect, LabelRect;
	POINT Point;
	SIZE Size;
	INT nStateId;

	hWnd;

	lCalcRects(nWidth, nHeight, &CheckBoxRect, &InnerCheckBoxRect, &LabelRect);

	::SaveDC(hMemDC);

	::SelectObject(hMemDC, hMemBitmap);

	// Paint the item's background

	if ((ODS_SELECTED & pDrawItemStruct->itemState) != ODS_SELECTED)
	{
		if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
		{
			// Item to paint is not disabled

			crTextColor = ::GetThemeSysColor(pCheckedListBoxCtrlData->hListBoxTheme, COLOR_WINDOWTEXT);
			crBackColor = ::GetThemeSysColor(pCheckedListBoxCtrlData->hListBoxTheme, COLOR_WINDOW);
		}
		else
		{
			// Item to paint is disabled

			crTextColor = ::GetThemeSysColor(pCheckedListBoxCtrlData->hListBoxTheme, COLOR_3DFACE);
			crBackColor = ::GetThemeSysColor(pCheckedListBoxCtrlData->hListBoxTheme, COLOR_WINDOW);
		}
	}
	else
	{
		// Item to paint is the currently selected one

		crTextColor = ::GetThemeSysColor(pCheckedListBoxCtrlData->hListBoxTheme, COLOR_HIGHLIGHTTEXT);
		crBackColor = ::GetThemeSysColor(pCheckedListBoxCtrlData->hListBoxTheme, COLOR_HIGHLIGHT);
	} // end of if...else statement

	UiPepCtrlFillSolidRect(hMemDC,
                           ::GetThemeSysColor(pCheckedListBoxCtrlData->hListBoxTheme, COLOR_WINDOW),
                           CheckBoxRect.left, CheckBoxRect.top,
                           MRectWidth(CheckBoxRect),
                           MRectHeight(CheckBoxRect));

	switch (pItemData->dwState)
	{
	    case CCheckedListBoxUncheckedState:
		    if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
		    {
			    nStateId = CBS_UNCHECKEDNORMAL;
		    }
		    else
		    {
			    nStateId = CBS_UNCHECKEDDISABLED;
		    }
		    break;
	    case CCheckedListBoxCheckedState:
		    if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
		    {
			    nStateId = CBS_CHECKEDNORMAL;
		    }
		    else
		    {
			    nStateId = CBS_CHECKEDDISABLED;
		    }
		    break;
	    case CCheckedListBoxIndeterminateState:
		    if ((ODS_DISABLED & pDrawItemStruct->itemState) != ODS_DISABLED)
		    {
			    nStateId = CBS_MIXEDNORMAL;
		    }
		    else
		    {
			    nStateId = CBS_MIXEDDISABLED;
		    }
		    break;
		default:
			nStateId = CBS_UNCHECKEDNORMAL;
			break;
	}

	::DrawThemeBackground(pCheckedListBoxCtrlData->hButtonTheme, hMemDC,
                          BP_CHECKBOX, nStateId, &CheckBoxRect, NULL);

	UiPepCtrlFillSolidRect(hMemDC, crBackColor,
                           LabelRect.left, LabelRect.top,
                           MRectWidth(LabelRect),
                           MRectHeight(LabelRect));

	UiPepCtrlSelectTwipsMode(hMemDC);

	::SelectObject(hMemDC, pCheckedListBoxCtrlData->hFont);

	::SetBkMode(hMemDC, TRANSPARENT);

	::SetTextColor(hMemDC, crTextColor);
	::SetBkColor(hMemDC, crBackColor);

	Point.x = LabelRect.left + CLabelMargin;
	Point.y = LabelRect.bottom;

	::DPtoLP(hMemDC, &Point, 1);

	::GetTextExtentPoint32(hMemDC, pItemData->cText, ::lstrlen(pItemData->cText), &Size);

	Point.y = (Point.y - Size.cy) / 2;

	::TextOutW(hMemDC, Point.x, Point.y, pItemData->cText, ::lstrlenW(pItemData->cText));

	UiPepCtrlSelectDevUnitsMode(hMemDC);

	// Draw the input focus around the item if applicable

	if ((pDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS)
	{
		::DrawFocusRect(hMemDC, &LabelRect);
	}

	::BitBlt(pDrawItemStruct->hDC,
             pDrawItemStruct->rcItem.left,
             pDrawItemStruct->rcItem.top,
             nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

	::RestoreDC(hMemDC, -1);

	::DeleteObject(hMemBitmap);

	::DeleteDC(hMemDC);
}

static VOID lOnNotifyListBoxCtrlClick(
  _In_ HWND hWnd,
  _In_ TListBoxCtrlNMClick* pClickHdr)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	TListBoxItemData* pItemData;
	RECT ItemRect, CheckBoxRect, InnerCheckBoxRect, LabelRect;

	::SendMessage(pData->hListBox, LB_GETITEMRECT, pClickHdr->nIndex, (LPARAM)&ItemRect);

	lCalcRects(&ItemRect, &CheckBoxRect, &InnerCheckBoxRect, &LabelRect);

	if (::PtInRect(&InnerCheckBoxRect, pClickHdr->Point))
	{
		pItemData = (TListBoxItemData*)::SendMessage(pData->hListBox, LB_GETITEMDATA, pClickHdr->nIndex, 0);

		lChangeItemState(pItemData);

		lSendChangeItemStateNotification(hWnd, pClickHdr->nIndex, pItemData->dwState);

		::InvalidateRect(pData->hListBox, &ItemRect, FALSE);
	}
}

static LRESULT lOnCheckedListBoxAddItem(
  HWND hWnd,
  LPCTSTR pszValue)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	INT nValueLen = ::lstrlen(pszValue);
	TListBoxItemData* pItemData;
	INT nIndex;

	nIndex = (INT)::SendMessage(pData->hListBox, LB_ADDSTRING, 0, 0);

	if (nIndex == LB_ERR)
	{
		return FALSE;
	}

	pItemData = (TListBoxItemData*)UtAllocMem(sizeof(TListBoxItemData) + (nValueLen * sizeof(TCHAR)));

	pItemData->dwState = CCheckedListBoxUncheckedState;

	::StringCchCopy(pItemData->cText, nValueLen + 1, pszValue);

	::SendMessage(pData->hListBox, LB_SETITEMDATA, nIndex, (LPARAM)pItemData);

	return TRUE;
}

static LRESULT lOnCheckedListBoxInsertItem(
  HWND hWnd,
  INT nIndex,
  LPCTSTR pszValue)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	INT nValueLen = ::lstrlen(pszValue);
	TListBoxItemData* pItemData;

	if (LB_ERR == ::SendMessage(pData->hListBox, LB_INSERTSTRING, nIndex, 0))
	{
		return FALSE;
	}

	pItemData = (TListBoxItemData*)UtAllocMem(sizeof(TListBoxItemData) + (nValueLen * sizeof(TCHAR)));

	pItemData->dwState = CCheckedListBoxUncheckedState;

	::StringCchCopy(pItemData->cText, nValueLen + 1, pszValue);

	::SendMessage(pData->hListBox, LB_SETITEMDATA, nIndex, (LPARAM)pItemData);

	return TRUE;
}

static LRESULT lOnCheckedListBoxDeleteItem(
  HWND hWnd,
  INT nIndex)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	return (::SendMessage(pData->hListBox, LB_DELETESTRING, nIndex, 0) != LB_ERR) ? TRUE : FALSE;
}

static LRESULT lOnCheckedListBoxDeleteAllItems(
  HWND hWnd)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	::SendMessage(pData->hListBox, LB_RESETCONTENT, 0, 0);

	return TRUE;
}

static LRESULT lOnCheckedListBoxSetName(
  HWND hWnd,
  INT nIndex,
  LPCTSTR pszName)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	TListBoxItemData* pItemData = (TListBoxItemData*)::SendMessage(pData->hListBox, LB_GETITEMDATA, nIndex, 0);
	INT nValueLen = ::lstrlen(pszName);
	TListBoxItemData* pNewItemData;
	RECT Rect;

	if (PtrToInt(pItemData) == LB_ERR)
	{
		return FALSE;
	}

	::SendMessage(pData->hListBox, LB_GETITEMRECT, nIndex, (LPARAM)&Rect);

	if (::lstrlen(pItemData->cText) >= nValueLen)
	{
		::StringCchCopy(pItemData->cText, nValueLen + 1, pszName);
	}
	else
	{
		pNewItemData = (TListBoxItemData*)UtAllocMem(sizeof(TListBoxItemData) + (nValueLen * sizeof(TCHAR)));

		pNewItemData->dwState = pItemData->dwState;

		::StringCchCopy(pItemData->cText, nValueLen + 1, pszName);

		UtFreeMem(pItemData);

		::SendMessage(pData->hListBox, LB_SETITEMDATA, nIndex, (LPARAM)pNewItemData);
	}

	::InvalidateRect(pData->hListBox, &Rect, FALSE);

	return TRUE;
}

static LRESULT lOnCheckedListBoxGetName(
  HWND hWnd,
  INT nIndex,
  TUiCheckedListBoxGetName* pCheckedListBoxGetName)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	TListBoxItemData* pItemData = (TListBoxItemData*)::SendMessage(pData->hListBox, LB_GETITEMDATA, nIndex, 0);
	INT nValueLen;

	if (PtrToInt(pItemData) == LB_ERR)
	{
		return -1;
	}

	nValueLen = ::lstrlen(pItemData->cText);

	if (pCheckedListBoxGetName->pszName == NULL)
	{
		pCheckedListBoxGetName->nNameLen = nValueLen;

		return TRUE;
	}

	if (pCheckedListBoxGetName->nNameLen < nValueLen)
	{
		return FALSE;
	}

	::StringCchCopy(pCheckedListBoxGetName->pszName, nValueLen + 1, pItemData->cText);

	return TRUE;
}

static LRESULT lOnCheckedListBoxSetCheckState(
  HWND hWnd,
  INT nIndex,
  DWORD dwState)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	TListBoxItemData* pItemData = (TListBoxItemData*)::SendMessage(pData->hListBox, LB_GETITEMDATA, nIndex, 0);
	RECT Rect;

	if (PtrToInt(pItemData) == LB_ERR)
	{
		return FALSE;
	}

	if (dwState != CCheckedListBoxUncheckedState &&
		dwState != CCheckedListBoxCheckedState &&
		dwState != CCheckedListBoxIndeterminateState)
	{
		return FALSE;
	}

	pItemData->dwState = dwState;

	::SendMessage(pData->hListBox, LB_GETITEMRECT, nIndex, (LPARAM)&Rect);
	
	::InvalidateRect(pData->hListBox, &Rect, FALSE);
		
    return TRUE;
}

static LRESULT lOnCheckedListBoxGetCheckState(
  HWND hWnd,
  INT nIndex)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	TListBoxItemData* pItemData = (TListBoxItemData*)::SendMessage(pData->hListBox, LB_GETITEMDATA, nIndex, 0);

	if (PtrToInt(pItemData) == LB_ERR)
	{
		return 0;
	}

	return pItemData->dwState;
}

static LRESULT lOnCheckedListBoxGetMinWidth(
  _In_ HWND hWnd,
  _Out_ LPINT pnWidth)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	HDC hDC = ::GetDC(pData->hListBox);
	INT nTotalItems = (INT)::SendMessage(pData->hListBox, LB_GETCOUNT, 0, 0);
	TListBoxItemData* pItemData;
	SIZE Size;
	POINT Point;
	RECT Rect, CheckBoxRect, InnerCheckBoxRect, LabelRect;

	*pnWidth = 0;

	if (nTotalItems == 0)
	{
		if (FALSE == lOnCheckedListBoxAddItem(hWnd, TEXT("")))
		{
			return FALSE;
		}
	}

	if (LB_ERR == ::SendMessage(pData->hListBox, LB_GETITEMRECT, 0, (LPARAM)&Rect))
	{
		if (nTotalItems == 0)
		{
			lOnCheckedListBoxDeleteAllItems(hWnd);
		}

		return FALSE;
	}

	if (nTotalItems == 0)
	{
		if (FALSE == lOnCheckedListBoxDeleteAllItems(hWnd))
		{
			return FALSE;
		}
	}

	lCalcRects(&Rect, &CheckBoxRect, &InnerCheckBoxRect, &LabelRect);

	::SaveDC(hDC);

	UiPepCtrlSelectTwipsMode(hDC);

	::SelectObject(hDC, pData->hFont);

	::ZeroMemory(&Point, sizeof(Point));

	for (INT nIndex = 0; nIndex < nTotalItems; ++nIndex)
	{
		pItemData = (TListBoxItemData*)::SendMessage(pData->hListBox, LB_GETITEMDATA, nIndex, 0);

		::GetTextExtentPoint32(hDC, pItemData->cText, ::lstrlen(pItemData->cText), &Size);

		if (Size.cx > Point.x)
		{
			Point.x = Size.cx;
		}

		if (Size.cy > Point.y)
		{
			Point.y = Size.cy;
		}
	}

	::LPtoDP(hDC, &Point, 1);

	::RestoreDC(hDC, -1);

	::ReleaseDC(pData->hListBox, hDC);

	*pnWidth = LabelRect.left - CheckBoxRect.left;

	if (nTotalItems > 0)
	{
		*pnWidth += ((CLabelMargin * 2) + Point.x);
	}

	return TRUE;
}

static LRESULT lOnCheckedListBoxInternalSetFocus(
  HWND hWnd)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	::SetFocus(pData->hListBox);

	return 0;
}

static LRESULT lOnDrawItem(
  HWND hWnd,
  INT nControlId,
  LPDRAWITEMSTRUCT pDrawItemStruct)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	nControlId;

	if (pData->hListBoxTheme && pData->hButtonTheme)
	{
		lThemedDrawItem(hWnd, pData, pDrawItemStruct);
	}
	else
	{
		lNonThemedDrawItem(hWnd, pData, pDrawItemStruct);
	}

	return TRUE;
}

static LRESULT lOnMeasureItem(
  HWND hWnd,
  INT nControlId,
  LPMEASUREITEMSTRUCT pMeasureItemStruct)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	HDC hDC = ::GetDC(hWnd);
    SIZE Size;
	POINT Point;

	nControlId;

	::SaveDC(hDC);

	UiPepCtrlSelectTwipsMode(hDC);

	::SelectObject(hDC, pData->hFont);

	::GetTextExtentPoint32(hDC, L"Ay", 2, &Size);

	Point.x = Size.cx;
	Point.y = Size.cy;

	::LPtoDP(hDC, &Point, 1);

	::RestoreDC(hDC, -1);

	::ReleaseDC(hWnd, hDC);

	pMeasureItemStruct->itemWidth = Point.x;
	pMeasureItemStruct->itemHeight = Point.y;

	return TRUE;
}

static LRESULT lOnVKeyToItem(
  HWND hWnd,
  INT nKeyCode,
  INT nCaretPos,
  HWND hListBox)
{
	TListBoxItemData* pItemData = (TListBoxItemData*)::SendMessage(hListBox, LB_GETITEMDATA, nCaretPos, 0);

	hWnd;

	if (nKeyCode == VK_SPACE)
	{
		lChangeItemState(pItemData);

		lSendChangeItemStateNotification(hWnd, nCaretPos, pItemData->dwState);

		return nCaretPos;
	}

	return -1;
}

static LRESULT lOnDeleteItem(
  HWND hWnd,
  INT nCtrlId,
  LPDELETEITEMSTRUCT pDeleteItem)
{
	TListBoxItemData* pItemData = (TListBoxItemData*)pDeleteItem->itemData;

	hWnd;
	nCtrlId;

	if (pItemData)
	{
		UtFreeMem(pItemData);
	}

	return TRUE;
}

static LRESULT lOnNotify(
  HWND hWnd,
  INT nControlId,
  LPNMHDR pHdr)
{
	nControlId;

	switch (pHdr->code)
	{
    	case NM_CLICK:
			lOnNotifyListBoxCtrlClick(hWnd, (TListBoxCtrlNMClick*)pHdr);
			break;
	}

	return 0;
}

static LRESULT lOnPaintMsg(
  HWND hWnd)
{
    HDC hDC;
    PAINTSTRUCT PaintStruct;

    hDC = ::BeginPaint(hWnd, &PaintStruct);

    ::EndPaint(hWnd, &PaintStruct);

    return 0;
}

static LRESULT lOnSizeMsg(
  HWND hWnd,
  INT nNewWidth,
  INT nNewHeight)
{
    TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	::SetWindowPos(pData->hListBox, NULL, 0, 0, nNewWidth, nNewHeight,
		           SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

    return 0;
}

static LRESULT lOnSetRedrawMsg(
  HWND hWnd,
  BOOL bRedraw)
{
    TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	::SendMessage(pData->hListBox, WM_SETREDRAW, bRedraw, 0);

    return 0;
}

static LRESULT lOnSetFocus(
  HWND hWnd,
  HWND hLostFocusWnd)
{
	hLostFocusWnd;

	::PostMessage(hWnd, CLBM_INTERNAL_SETFOCUS, 0, 0);

	return 0;
}

static LRESULT lOnThemeChangedMsg(
  HWND hWnd)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pData->hListBoxTheme)
    {
        ::CloseThemeData(pData->hListBoxTheme);

        pData->hListBoxTheme = NULL;
    }

	if (pData->hButtonTheme)
	{
		::CloseThemeData(pData->hButtonTheme);

		pData->hButtonTheme = NULL;
	}

    if (::IsThemeActive())
    {
        pData->hListBoxTheme = ::OpenThemeData(hWnd, CListBoxThemeClassName);
		pData->hButtonTheme = ::OpenThemeData(hWnd, CButtonThemeClassName);
	}

    return 0;
}

static LRESULT lOnGetFont(
  HWND hWnd)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	return (LRESULT)pData->hFont;
}

static LRESULT lOnSetFont(
  HWND hWnd,
  HFONT hFont,
  BOOL bRedraw)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pData->hFont)
	{
		::DeleteObject(pData->hFont);
	}

	pData->hFont = hFont;

	if (bRedraw)
	{
		::InvalidateRect(pData->hListBox, NULL, TRUE);
	}
	
	return 0;
}

static LRESULT lOnEnable(
  HWND hWnd,
  BOOL bEnable)
{
	TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	::EnableWindow(pData->hListBox, bEnable);

	return 0;
}

static LRESULT lOnCreateMsg(
  HWND hWnd,
  LPCREATESTRUCT pCreateStruct)
{
    TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)UtAllocMem(sizeof(TCheckedListBoxCtrlData));

    pCreateStruct;

    if (pData == NULL)
    {
        return -1;
    }

    ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

	pData->hListBox = ::CreateWindowW(CUiListBoxCtrlClass, NULL,
		                              WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_VISIBLE | WS_BORDER |
		                                  LBS_OWNERDRAWFIXED | LBS_WANTKEYBOARDINPUT | LBS_NOINTEGRALHEIGHT,
		                              pCreateStruct->x, pCreateStruct->y,
		                              pCreateStruct->cx, pCreateStruct->cy,
		                              hWnd, (HMENU)1, pCreateStruct->hInstance, NULL);
	pData->hFont = NULL;
    pData->hListBoxTheme = NULL;
	pData->hButtonTheme = NULL;

    if (::IsThemeActive())
    {
        pData->hListBoxTheme = ::OpenThemeData(hWnd, CListBoxThemeClassName);
		pData->hButtonTheme = ::OpenThemeData(hWnd, CButtonThemeClassName);
    }

    return 0;
}

static LRESULT lOnDestroyMsg(
  HWND hWnd)
{
    TCheckedListBoxCtrlData* pData = (TCheckedListBoxCtrlData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pData->hFont)
	{
		::DeleteObject(pData->hFont);
	}

	if (pData->hListBox)
	{
		::DestroyWindow(pData->hListBox);
	}

    if (pData->hListBoxTheme)
    {
        ::CloseThemeData(pData->hListBoxTheme);
    }

	if (pData->hButtonTheme)
	{
		::CloseThemeData(pData->hButtonTheme);
	}

    UtFreeMem(pData);

    return 0;
}

static LRESULT CALLBACK lCheckedListBoxCtrlWndProc(
  HWND hWnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
    switch (uMsg)
    {
	    case CLBM_ADDITEM:
			return lOnCheckedListBoxAddItem(hWnd, (LPCTSTR)lParam);
		case CLBM_INSERTITEM:
			return lOnCheckedListBoxInsertItem(hWnd, (INT)wParam, (LPCTSTR)lParam);
        case CLBM_DELETEITEM:
			return lOnCheckedListBoxDeleteItem(hWnd, (INT)wParam);
        case CLBM_DELETEALLITEMS:
			return lOnCheckedListBoxDeleteAllItems(hWnd);
		case CLBM_SETNAME:
			return lOnCheckedListBoxSetName(hWnd, (INT)wParam, (LPCTSTR)lParam);
		case CLBM_GETNAME:
			return lOnCheckedListBoxGetName(hWnd, (INT)wParam, (TUiCheckedListBoxGetName*)lParam);
		case CLBM_SETCHECKSTATE:
			return lOnCheckedListBoxSetCheckState(hWnd, (INT)wParam, (DWORD)lParam);
        case CLBM_GETCHECKSTATE:
			return lOnCheckedListBoxGetCheckState(hWnd, (INT)wParam);
		case CLBM_GETMINWIDTH:
			return lOnCheckedListBoxGetMinWidth(hWnd, (LPINT)lParam);
		case CLBM_INTERNAL_SETFOCUS:
			return lOnCheckedListBoxInternalSetFocus(hWnd);
	    case WM_DRAWITEM:
			return lOnDrawItem(hWnd, (INT)wParam, (LPDRAWITEMSTRUCT)lParam);
	    case WM_MEASUREITEM:
			return lOnMeasureItem(hWnd, (INT)wParam, (LPMEASUREITEMSTRUCT)lParam);
		case WM_VKEYTOITEM:
			return lOnVKeyToItem(hWnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		case WM_DELETEITEM:
			return lOnDeleteItem(hWnd, (INT)wParam, (LPDELETEITEMSTRUCT)lParam);
		case WM_NOTIFY:
			return lOnNotify(hWnd, (INT)wParam, (LPNMHDR)lParam);
        case WM_PAINT:
            return lOnPaintMsg(hWnd);
        case WM_SIZE:
            return lOnSizeMsg(hWnd, LOWORD(lParam), HIWORD(lParam));
        case WM_SETREDRAW:
            return lOnSetRedrawMsg(hWnd, (BOOL)wParam);
		case WM_SETFOCUS:
			return lOnSetFocus(hWnd, (HWND)wParam);
        case WM_THEMECHANGED:
            return lOnThemeChangedMsg(hWnd);
		case WM_GETFONT:
			return lOnGetFont(hWnd);
		case WM_SETFONT:
			return lOnSetFont(hWnd, (HFONT)wParam, LOWORD(lParam));
		case WM_ENABLE:
			return lOnEnable(hWnd, (BOOL)wParam);
        case WM_CREATE:
            return lOnCreateMsg(hWnd, (LPCREATESTRUCT)lParam);
        case WM_DESTROY:
            return lOnDestroyMsg(hWnd);
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#pragma endregion

VOID UiCheckedListBoxCtrlRegister(VOID)
{
    WNDCLASSEXW WndClassEx;

    WndClassEx.cbSize = sizeof(WndClassEx);
    WndClassEx.style = CS_GLOBALCLASS;
    WndClassEx.lpfnWndProc = lCheckedListBoxCtrlWndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = sizeof(TCheckedListBoxCtrlData*);
    WndClassEx.hInstance = UiPepCtrlGetInstance();
    WndClassEx.hIcon = NULL;
    WndClassEx.hCursor = ::LoadCursorW(NULL, IDC_ARROW);
    WndClassEx.hbrBackground = NULL;
    WndClassEx.lpszMenuName = NULL;
    WndClassEx.lpszClassName = CUiCheckedListBoxCtrlClass;
    WndClassEx.hIconSm = NULL;

    ::RegisterClassExW(&WndClassEx);
}

VOID UiCheckedListBoxCtrlUnregister(VOID)
{
    ::UnregisterClassW(CUiCheckedListBoxCtrlClass, UiPepCtrlGetInstance());
}

/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/
