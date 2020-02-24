/***************************************************************************/
/*  Copyright (C) 2010-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Windows.h>

#include "UiPepCtrlsUtil.h"

#include <Includes/UiMacros.h>

#pragma region "Local Variables"

static HINSTANCE l_hInstance = NULL;

#pragma endregion

VOID UiPepCtrlSetInstance(
  _In_ HINSTANCE hInstance)
{
	l_hInstance = hInstance;
}

HINSTANCE UiPepCtrlGetInstance(VOID)
{
	return l_hInstance;
}

VOID UiPepCtrlSelectTwipsMode(
  _In_ HDC hDC)
{
	::SetMapMode(hDC, MM_ANISOTROPIC);

	::SetWindowExtEx(hDC, MPtSizeToTwips(CPtSizesPerInch),
		             MPtSizeToTwips(CPtSizesPerInch), NULL);

	::SetViewportExtEx(hDC, ::GetDeviceCaps(hDC, LOGPIXELSX),
		               ::GetDeviceCaps(hDC, LOGPIXELSY), NULL);

	::SetWindowOrgEx(hDC, 0, 0, NULL);

	::SetViewportOrgEx(hDC, 0, 0, NULL);
}

VOID UiPepCtrlSelectDevUnitsMode(
  _In_ HDC hDC)
{
	::SetMapMode(hDC, MM_TEXT);

	::SetWindowOrgEx(hDC, 0, 0, NULL);
	::SetViewportOrgEx(hDC, 0, 0, NULL);
}

VOID UiPepCtrlFillSolidRect(
  _In_ HDC hDC,
  _In_ HBRUSH hBrush,
  _In_ INT nXPos,
  _In_ INT nYPos,
  _In_ INT nWidth,
  _In_ INT nHeight)
{
	RECT Rect;

	Rect.left = nXPos;
	Rect.top = nYPos;
	Rect.right = nXPos + nWidth;
	Rect.bottom = nYPos + nHeight;

	::FillRect(hDC, &Rect, hBrush);
}

VOID UiPepCtrlFillSolidRect(
  _In_ HDC hDC,
  _In_ COLORREF crBack,
  _In_ INT nXPos,
  _In_ INT nYPos,
  _In_ INT nWidth,
  _In_ INT nHeight)
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

/***************************************************************************/
/*  Copyright (C) 2010-2020 Kevin Eshbach                                  */
/***************************************************************************/
