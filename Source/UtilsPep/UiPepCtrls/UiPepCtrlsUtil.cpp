/***************************************************************************/
/*  Copyright (C) 2010-2022 Kevin Eshbach                                  */
/***************************************************************************/

#include <Windows.h>

#include "UiPepCtrlsUtil.h"

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

/***************************************************************************/
/*  Copyright (C) 2010-2022 Kevin Eshbach                                  */
/***************************************************************************/
