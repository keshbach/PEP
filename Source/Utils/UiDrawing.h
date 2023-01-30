/***************************************************************************/
/*  Copyright (C) 2010-2022 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiDrawing_H)
#define UiDrawing_H

#include <Includes/UtExternC.h>

#define UIDRAWINGAPI __stdcall

MExternC VOID UIDRAWINGAPI UiDrawingSelectTwipsMode(_In_ HDC hDC);
MExternC VOID UIDRAWINGAPI UiDrawingSelectDevUnitsMode(_In_ HDC hDC);

MExternC VOID UIDRAWINGAPI UiDrawingFillSolidRectWithBrush(_In_ HDC hDC, _In_ HBRUSH hBrush, _In_ INT nXPos, _In_ INT nYPos, _In_ INT nWidth, _In_ INT nHeight);
MExternC VOID UIDRAWINGAPI UiDrawingFillSolidRectWithRGB(_In_ HDC hDC, _In_ COLORREF crBack, _In_ INT nXPos, _In_ INT nYPos, _In_ INT nWidth, _In_ INT nHeight);

#endif /* end of UiDrawing_H */

/***************************************************************************/
/*  Copyright (C) 2010-2022 Kevin Eshbach                                  */
/***************************************************************************/
