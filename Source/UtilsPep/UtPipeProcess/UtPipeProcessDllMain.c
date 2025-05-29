/***************************************************************************/
/*  Copyright (C) 2008-2025 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include "UtPipeProcessUtil.h"

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

            UtPipeProcessSetInstance(hInstance);
            break;
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2008-2025 Kevin Eshbach                                  */
/***************************************************************************/
