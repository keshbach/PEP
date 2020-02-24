/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include "UiListBoxCtrl.h"

#include "UiPepCtrlsUtil.h"

#include <Utils/UtHeap.h>

#include <commctrl.h>
#include <windowsx.h>

#pragma region "Constants"

#define CListBoxCtrlPropName L"ListBoxCtrlPropName"

#pragma endregion

#pragma region "Structures"

typedef struct tagTListBoxCtrlData
{
	BOOL bMouseCaptured;
	POINT Point;
	INT nIndex;
} TListBoxCtrlData;

#pragma endregion

#pragma region "Local Variables"

static WNDPROC l_ListBoxWndProc = NULL;

#pragma endregion

#pragma region "Local Functions"

static LRESULT lOnLeftButtonDownMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	TListBoxCtrlData* pData = (TListBoxCtrlData*)::GetProp(hWnd, CListBoxCtrlPropName);
	LRESULT lResult;

	dwKeys;

	if (::GetFocus() != hWnd)
	{
		::SetFocus(hWnd);
	}

	pData->bMouseCaptured = TRUE;
	pData->Point.x = iXPos;
	pData->Point.y = iYPos;
	pData->nIndex = -1;

	::SetCapture(hWnd);

	lResult = ::SendMessage(hWnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(iXPos, iYPos));

	if (HIWORD(lResult) == 0)
	{
		pData->nIndex = LOWORD(lResult);

		::SendMessage(hWnd, LB_SETCARETINDEX, pData->nIndex, FALSE);
		::SendMessage(hWnd, LB_SETCURSEL, pData->nIndex, 0);
	}
	else
	{
		::SendMessage(hWnd, LB_SETCURSEL, (WPARAM)-1, 0);
	}

	return 0;
}

static LRESULT lOnLeftButtonUpMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	TListBoxCtrlData* pData = (TListBoxCtrlData*)::GetProp(hWnd, CListBoxCtrlPropName);
    TListBoxCtrlNMClick ClickHdr;

	dwKeys;

	if (pData->bMouseCaptured)
	{
		::ReleaseCapture();

		pData->bMouseCaptured = FALSE;

		if (pData->Point.x == iXPos && pData->Point.y == iYPos && pData->nIndex != -1)
		{
			ClickHdr.Hdr.hwndFrom = hWnd;
			ClickHdr.Hdr.idFrom = ::GetDlgCtrlID(hWnd);
			ClickHdr.Hdr.code = NM_CLICK;
			ClickHdr.Point = pData->Point;
			ClickHdr.nIndex = pData->nIndex;

			::SendMessage(::GetParent(hWnd), WM_NOTIFY, ClickHdr.Hdr.idFrom, (LPARAM)&ClickHdr);
		}
	}

	return 0;
}

static LRESULT lOnLeftButtonDoubleClickMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnRightButtonDownMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnRightButtonUpMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnRightButtonDoubleClickMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnMiddleButtonDownMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnMiddleButtonUpMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnMiddleButtonDoubleClickMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnMouseMoveMsg(
  HWND hWnd,
  DWORD dwKeys,
  INT iXPos,
  INT iYPos)
{
	hWnd;
	dwKeys;
	iXPos;
	iYPos;

	return 0;
}

static LRESULT lOnCreateMsg(
  HWND hWnd,
  LPCREATESTRUCT pCreateStruct)
{
	TListBoxCtrlData* pData = (TListBoxCtrlData*)UtAllocMem(sizeof(TListBoxCtrlData));

	pCreateStruct;

	if (pData != NULL)
	{
		pData->bMouseCaptured = FALSE;

		::SetProp(hWnd, CListBoxCtrlPropName, pData);
	}

	return ::CallWindowProc(l_ListBoxWndProc, hWnd, WM_CREATE, 0, (LPARAM)pCreateStruct);
}

static LRESULT lOnDestroyMsg(
  HWND hWnd)
{
	TListBoxCtrlData* pData = (TListBoxCtrlData*)::RemoveProp(hWnd, CListBoxCtrlPropName);

	if (pData)
	{
		UtFreeMem(pData);
	}

	return ::CallWindowProc(l_ListBoxWndProc, hWnd, WM_DESTROY, 0, 0);
}

static LRESULT CALLBACK lListBoxCtrlWndProc(
  HWND hWnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
    switch (uMsg)
    {
    	case WM_LBUTTONDOWN:
	        return lOnLeftButtonDownMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        case WM_LBUTTONUP:
            return lOnLeftButtonUpMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_LBUTTONDBLCLK:
			return lOnLeftButtonDoubleClickMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_RBUTTONDOWN:
			return lOnRightButtonDownMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_RBUTTONUP:
			return lOnRightButtonUpMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_RBUTTONDBLCLK:
			return lOnRightButtonDoubleClickMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_MBUTTONDOWN:
			return lOnMiddleButtonDownMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_MBUTTONUP:
			return lOnMiddleButtonUpMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_MBUTTONDBLCLK:
			return lOnMiddleButtonDoubleClickMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_MOUSEMOVE:
			return lOnMouseMoveMsg(hWnd, DWORD(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		case WM_CREATE:
			return lOnCreateMsg(hWnd, (LPCREATESTRUCT)lParam);
		case WM_DESTROY:
			return lOnDestroyMsg(hWnd);
	}

	return ::CallWindowProc(l_ListBoxWndProc, hWnd, uMsg, wParam, lParam);
}

#pragma endregion

VOID UiListBoxCtrlRegister(VOID)
{
    WNDCLASSEXW WndClassEx;

	WndClassEx.cbSize = sizeof(WNDCLASSEX);

	::GetClassInfoExW(UiPepCtrlGetInstance(), WC_LISTBOX, &WndClassEx);

	l_ListBoxWndProc = WndClassEx.lpfnWndProc;

	WndClassEx.style |= CS_GLOBALCLASS;

	WndClassEx.lpfnWndProc = lListBoxCtrlWndProc;
	WndClassEx.hInstance = UiPepCtrlGetInstance();
	WndClassEx.lpszMenuName = NULL;
	WndClassEx.lpszClassName = CUiListBoxCtrlClass;

	::RegisterClassExW(&WndClassEx);
}

VOID UiListBoxCtrlUnregister(VOID)
{
    ::UnregisterClassW(CUiListBoxCtrlClass, UiPepCtrlGetInstance());
}

/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/
