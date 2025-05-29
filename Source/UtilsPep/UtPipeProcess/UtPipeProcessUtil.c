/***************************************************************************/
/*  Copyright (C) 2010-2025 Kevin Eshbach                                  */
/***************************************************************************/

#include <Windows.h>

#include "UtPipeProcessUtil.h"

#pragma region "Local Variables"

static HINSTANCE l_hInstance = NULL;

#pragma endregion

VOID UtPipeProcessSetInstance(
  _In_ HINSTANCE hInstance)
{
	l_hInstance = hInstance;
}

HINSTANCE UtPipeProcessGetInstance(VOID)
{
	return l_hInstance;
}

/***************************************************************************/
/*  Copyright (C) 2010-2025 Kevin Eshbach                                  */
/***************************************************************************/
