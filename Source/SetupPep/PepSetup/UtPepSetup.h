/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

VOID UtPepSetupSetInstance(_In_ HINSTANCE hInstance);

_Ret_maybenull_z_ LPCWSTR UtPepSetupAllocString(_In_ INT nResourceId);

VOID UtPepSetupFreeString(_In_opt_z_ LPCWSTR pszData);

BOOL UtPepSetupAppendLine(_In_z_ LPCWSTR pszLogFile, _In_z_ LPCWSTR pszMessage);

BOOL UtPepSetupConvertAnsiLogFileToUnicodeLogFile(_In_z_ LPCWSTR pszAnsiLogFile, _In_z_ LPCWSTR pszUnicodeLogFile);

BOOL UtPepSetupGenerateTempDirectoryName(_Out_writes_z_(nPathLen) LPWSTR pszPath, _In_ INT nPathLen);

BOOL UtPepSetupIsWindows64Present();

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
