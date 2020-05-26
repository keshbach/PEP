/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiMacros_H)
#define UiMacros_H

#define CPtSizesPerInch 72

#define MPtSizeToTwips(nPtSize) (nPtSize * 20)
#define MTwipsToPtSize(nTwipsSize) (nTwipsSize / 20)

#define MRectWidth(rect) ((rect).right - (rect).left)
#define MRectHeight(rect) ((rect).bottom - (rect).top)

#endif /* end of UiMacros_H */

/***************************************************************************/
/*  Copyright (C) 2020-2020 Kevin Eshbach                                  */
/***************************************************************************/
