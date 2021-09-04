/***************************************************************************/
/*  Copyright (C) 2018-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if defined(_MANAGED)
#pragma managed(push, off)
#endif

#pragma region Functions

static BOOL UtIsKeyDown(
  _In_ INT nKeyCode)
{
    return (::GetKeyState(nKeyCode) & 0x8000) ? TRUE : FALSE;
}

#pragma endregion

#if defined(_MANAGED)
#pragma managed(pop)
#endif

/***************************************************************************/
/*  Copyright (C) 2018-2021 Kevin Eshbach                                  */
/***************************************************************************/
