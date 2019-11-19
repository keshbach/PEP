/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

BOOL PepSetupModelessDialogCreate(_In_ HINSTANCE hInstance);
BOOL PepSetupModelessDialogDestroy();

VOID PepSetupModelessDialogMessagePump();

VOID PepSetupModelessDialogQuitMessagePump();

VOID PepSetupModelessDialogDisplayAppAlreadyRunning();
VOID PepSetupModelessDialogDisplayUnknownError();
VOID PepSetupModelessDialogDisplayFailedExtractError();
VOID PepSetupModelessDialogDisplayInstallAlreadyRunning();
VOID PepSetupModelessDialogDisplayInstallError();

VOID PepSetupModelessDialogDisplayMessage(_In_z_ LPCWSTR pszMessage);

VOID PepSetupModelessDialogDisplayRebootPrompt(_Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbRebootAllowed);

VOID PepSetupModelessDialogDisplayRebootFailed(_In_ DWORD dwErrorCode);

VOID PepSetupModelessDialogDisplayUninstallPrompt(_Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbUninstall);

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
