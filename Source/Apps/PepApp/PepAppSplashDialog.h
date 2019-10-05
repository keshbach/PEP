/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define PEPAPPSPLASHDIALOGEXECUTEAPI __stdcall

typedef VOID (PEPAPPSPLASHDIALOGEXECUTEAPI* TPepAppSplashDialogExecuteFunc)(_In_ LPVOID pvData);

BOOL PepAppSplashDialogCreate(_In_ HINSTANCE hInstance);
BOOL PepAppSplashDialogDestroy();

VOID PepAppSplashDialogMessagePump();

VOID PepAppSplashDialogQuitMessagePump();

VOID PepAppSplashDialogDisplayAppAlreadyRunning();
VOID PepAppSplashDialogDisplayUnsupportedOS();
VOID PepAppSplashDialogDisplayUnknownError();
VOID PepAppSplashDialogDisplayCommandLineHelp();

VOID PepAppSplashDialogDisplayMessage(_In_z_ LPCWSTR pszMessage);

VOID PepAppSplashDialogExecute(_In_ TPepAppSplashDialogExecuteFunc pExecute, _In_ LPVOID pvData);

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
