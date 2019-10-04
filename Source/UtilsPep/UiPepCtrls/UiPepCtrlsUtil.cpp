/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <Windows.h>

#include "UiPepCtrlsUtil.h"

static HINSTANCE l_hInstance = NULL;

VOID UiPepCtrlSetInstance(
  HINSTANCE hInstance)
{
	l_hInstance = hInstance;
}

HINSTANCE UiPepCtrlGetInstance(VOID)
{
	return l_hInstance;
}

/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/
