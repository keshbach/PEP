/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

BOOL PepAppSplashDialogCreate(_In_ HINSTANCE hInstance);
BOOL PepAppSplashDialogDestroy();

VOID PepAppSplashDialogMessagePump();

VOID PepAppSplashDialogQuitMessagePump();

VOID PepAppSplashDialogDisplayAppAlreadyRunning();
VOID PepAppSplashDialogDisplayUnsupportedOS();
VOID PepAppSplashDialogDisplayUnknownError();
VOID PepAppSplashDialogDisplayCommandLineHelp();

VOID PepAppSplashDialogDisplayMessage(_In_z_ LPCWSTR pszMessage);

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
