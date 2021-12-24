/***************************************************************************/
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <Windows.h>

#include "UtPepCtrlUtil.h"

#pragma region "Local Variables"

static HINSTANCE l_hInstance = NULL;

#pragma endregion

VOID UtPepCtrlSetInstance(
	_In_ HINSTANCE hInstance)
{
	l_hInstance = hInstance;
}

HINSTANCE UtPepCtrlGetInstance(VOID)
{
	return l_hInstance;
}

/***************************************************************************/
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/
