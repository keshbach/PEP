/***************************************************************************/
/*  Copyright (C) 2007-2009 Kevin Eshbach                                  */
/***************************************************************************/

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

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
            DisableThreadLibraryCalls(hInstance);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2007-2009 Kevin Eshbach                                  */
/***************************************************************************/
