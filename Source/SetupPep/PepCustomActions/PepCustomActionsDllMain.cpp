/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

static HMODULE l_hModule = NULL;

HMODULE UtGetInstance(VOID)
{
    return l_hModule;
}

BOOL APIENTRY DllMain(
  HMODULE hModule,
  DWORD ulReason,
  LPVOID pvReserved)
{
    pvReserved;

    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH:
            l_hModule = hModule;

            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
	}

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
