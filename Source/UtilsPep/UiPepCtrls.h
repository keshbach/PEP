/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiPepCtrls_H)
#define UiPepCtrls_H

#include <windows.h>

#define UIPEPCTRLSAPI __stdcall

#if defined(__cplusplus)
extern "C"
{
#endif

VOID UIPEPCTRLSAPI UiPepCtrlsInitialize(VOID);
VOID UIPEPCTRLSAPI UiPepCtrlsUninitialize(VOID);

#if defined(__cplusplus)
};
#endif

#endif

/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/
