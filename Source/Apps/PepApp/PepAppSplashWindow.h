/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define PEPAPPSPLASHWINDOWEXECUTEAPI __stdcall

typedef VOID (PEPAPPSPLASHWINDOWEXECUTEAPI* TPepAppSplashWindowExecuteFunc)(_In_ LPVOID pvData);

BOOL PepAppSplashWindowCreate(_In_ HINSTANCE hInstance);
BOOL PepAppSplashWindowDestroy(_In_ HINSTANCE hInstance);

VOID PepAppSplashWindowMessagePump();

VOID PepAppSplashWindowQuitMessagePump();

VOID PepAppSplashWindowDisplayAppAlreadyRunning();
VOID PepAppSplashWindowDisplayUnsupportedOS();
VOID PepAppSplashWindowDisplayUnknownError();
VOID PepAppSplashWindowDisplayCommandLineHelp();
VOID PepAppSplashWindowDisplayPluginsLoadFailed();

VOID PepAppSplashWindowExecute(_In_ TPepAppSplashWindowExecuteFunc pExecute, _In_ LPVOID pvData);

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
