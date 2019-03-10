/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/

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
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/
