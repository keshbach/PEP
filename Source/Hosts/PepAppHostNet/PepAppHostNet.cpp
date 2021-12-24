/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Hosts/PepAppHostData.h>

#include "PepAppNet.h"

static VOID lExecutePepApp(
  _In_ TPepAppHostData* pPepAppHostData)
{
    Pep::Application::Startup^ Startup = gcnew Pep::Application::Startup();

    pPepAppHostData->dwExitCode = Startup->Execute(pPepAppHostData->bUseParallelPort);
}

extern "C"
{

#pragma unmanaged

HRESULT __stdcall PepAppHostNetExecuteInAppDomain(
  _In_ void* cookie)
{
    TPepAppHostData* pPepAppHostData = (TPepAppHostData*)cookie;

    lExecutePepApp(pPepAppHostData);

    return S_OK;
}

#pragma managed

}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
