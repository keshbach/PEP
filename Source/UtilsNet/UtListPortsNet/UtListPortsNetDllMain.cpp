/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include <Windows.h>

BOOL WINAPI DllMain(
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
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
