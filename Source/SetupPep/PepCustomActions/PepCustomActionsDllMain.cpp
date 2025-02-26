/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2023 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UtPepCustomActions.h"

BOOL APIENTRY DllMain(
  HMODULE hModule,
  DWORD ulReason,
  LPVOID pvReserved)
{
    pvReserved;

    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH:
            UtPepCustomActionsSetModule(hModule);

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
//  Copyright (C) 2014-2023 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
