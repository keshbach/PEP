/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Hosts/PepAppHostData.h>

#include "PepAppNet.h"

static VOID lExecutePepApp(
  TPepAppHostData* pPepAppHostData)
{
    Pep::Application::Startup^ Startup = gcnew Pep::Application::Startup();

    pPepAppHostData->dwExitCode = Startup->Execute();
}

extern "C"
{

#pragma unmanaged

HRESULT __stdcall PepAppHostNetExecuteInAppDomain(
  void* cookie)
{
    TPepAppHostData* pPepAppHostData = (TPepAppHostData*)cookie;

    lExecutePepApp(pPepAppHostData);

    return S_OK;
}

#pragma managed

}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
