/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PepSetup.h"
#include "UtPepSetup.h"

#include <cstdlib>

int CALLBACK wWinMain(
  _In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR pszCmdLine,
  _In_ int nCmdShow)
{
    hPrevInstance;
    pszCmdLine;
    nCmdShow;

    UtPepSetupSetInstance(hInstance);

    return PepSetupExecuteInstall(hInstance, __argc, __wargv);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
