/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PepSetup.h"
#include "UtPepSetup.h"

int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPSTR pszCmdLine,
  _In_ int nCmdShow)
{
    hPrevInstance;
    pszCmdLine;
    nCmdShow;

    UtPepSetupSetInstance(hInstance);

    return PepSetupExecuteInstall(hInstance, ::GetCommandLineW());
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////