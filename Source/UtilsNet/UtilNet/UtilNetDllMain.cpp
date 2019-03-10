/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include <Windows.h>

static HINSTANCE l_hInstance = NULL;

HINSTANCE UtGetInstance(void)
{
	return l_hInstance;
}

BOOL WINAPI DllMain(
  HINSTANCE hInstance,
  DWORD dwReason,
  LPVOID pvReserved)
{
    pvReserved;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            l_hInstance = hInstance;

            ::DisableThreadLibraryCalls(hInstance);
            break;
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
