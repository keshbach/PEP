/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

BOOL APIENTRY DllMain(
  HINSTANCE hInstance,
  DWORD dwReason,
  LPVOID pvReserved)
{
    pvReserved;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            ::DisableThreadLibraryCalls(hInstance);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/