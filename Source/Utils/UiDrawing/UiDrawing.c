/***************************************************************************/
/*  Copyright (C) 2010-2022 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <Includes/UtMacros.h>

#include <Utils/UiDrawing.h>

#include <Includes/UiMacros.h>

VOID UIDRAWINGAPI UiDrawingSelectTwipsMode(
  _In_ HDC hDC)
{
	SetMapMode(hDC, MM_ANISOTROPIC);

	SetWindowExtEx(hDC, MPtSizeToTwips(CPtSizesPerInch),
	               MPtSizeToTwips(CPtSizesPerInch), NULL);

	SetViewportExtEx(hDC, GetDeviceCaps(hDC, LOGPIXELSX),
		             GetDeviceCaps(hDC, LOGPIXELSY), NULL);

	SetWindowOrgEx(hDC, 0, 0, NULL);

	SetViewportOrgEx(hDC, 0, 0, NULL);
}

VOID UIDRAWINGAPI UiDrawingSelectDevUnitsMode(
  _In_ HDC hDC)
{
	SetMapMode(hDC, MM_TEXT);

	SetWindowOrgEx(hDC, 0, 0, NULL);
	SetViewportOrgEx(hDC, 0, 0, NULL);
}

VOID UIDRAWINGAPI UiDrawingFillSolidRectWithBrush(
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

	FillRect(hDC, &Rect, hBrush);
}

VOID UIDRAWINGAPI UiDrawingFillSolidRectWithRGB(
  _In_ HDC hDC,
  _In_ COLORREF crBack,
  _In_ INT nXPos,
  _In_ INT nYPos,
  _In_ INT nWidth,
  _In_ INT nHeight)
{
	HBRUSH hBrush = CreateSolidBrush(crBack);
	RECT Rect;

	Rect.left = nXPos;
	Rect.top = nYPos;
	Rect.right = nXPos + nWidth;
	Rect.bottom = nYPos + nHeight;

	FillRect(hDC, &Rect, hBrush);

	DeleteObject(hBrush);
}

/***************************************************************************/
/*  Copyright (C) 2010-2022 Kevin Eshbach                                  */
/***************************************************************************/
