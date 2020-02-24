/***************************************************************************/
/*  Copyright (C) 2010-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiPepCtrlsUtil_H)
#define UiPepCtrlsUtil_H

VOID UiPepCtrlSetInstance(_In_ HINSTANCE hInstance);
HINSTANCE UiPepCtrlGetInstance(VOID);

VOID UiPepCtrlSelectTwipsMode(_In_ HDC hDC);
VOID UiPepCtrlSelectDevUnitsMode(_In_ HDC hDC);

VOID UiPepCtrlFillSolidRect(_In_ HDC hDC, _In_ HBRUSH hBrush, _In_ INT nXPos, _In_ INT nYPos, _In_ INT nWidth, _In_ INT nHeight);
VOID UiPepCtrlFillSolidRect(_In_ HDC hDC, _In_ COLORREF crBack, _In_ INT nXPos, _In_ INT nYPos, _In_ INT nWidth, _In_ INT nHeight);

#endif

/***************************************************************************/
/*  Copyright (C) 2010-2020 Kevin Eshbach                                  */
/***************************************************************************/
