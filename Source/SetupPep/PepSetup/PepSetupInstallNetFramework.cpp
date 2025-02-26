/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepSetupInstallNetFramework.h"

#include "PepSetupResources.h"
#include "PepSetupModelessDialog.h"

#include "UtPepSetup.h"

#include <Utils/UtHeapProcess.h>

#pragma region Constants

// .NET Framework 4 Command Line Arguments definitions

#define CNetFramework4CommandLineArgs L"/q /norestart"

#pragma endregion

#pragma region Local Functions

static BOOL lReadNetFrameworkRegValues(
  _In_ HKEY hKey,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbInstall)
{
    DWORD dwData, dwDataLen;

    *pbInstall = FALSE;

    dwDataLen = sizeof(DWORD);

    if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, L"Install", NULL, NULL, (LPBYTE)&dwData, &dwDataLen))
    {
        return FALSE;
    }

    *pbInstall = dwData ? FALSE : TRUE;

    return TRUE;
}

static BOOL lInstallNetFramework(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbInstallFramework)
{
    BOOL bResult = FALSE;
    LPCWSTR pszSubKey = L"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full";
    HKEY hKey;
    LSTATUS Status;

    *pbInstallFramework = FALSE;

    Status = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszSubKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

    switch (Status)
    {
        case ERROR_SUCCESS:
            if (lReadNetFrameworkRegValues(hKey, pbInstallFramework))
            {
                bResult = TRUE;
            }

            ::RegCloseKey(hKey);
            break;
        case ERROR_FILE_NOT_FOUND:
            *pbInstallFramework = TRUE;

            bResult = TRUE;
            break;
    }

    return bResult;
}

#pragma endregion

#pragma region Public Functions

BOOL PepSetupInstallNetFrameworkRedist(
  _In_opt_z_ LPCWSTR pszLogFile)
{
    BOOL bResult = TRUE;
    LPCWSTR pszMessage;
    BOOL bInstallFramework;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    LPWSTR pszCommandLine;
    INT nCommandLineLen;

    pszMessage = UtPepSetupAllocString(IDS_CHECKINGNETFRAMEWORK);

    if (pszMessage)
    {
        PepSetupModelessDialogDisplayMessage(pszMessage);

        UtPepSetupFreeString(pszMessage);
    }

    if (lInstallNetFramework(&bInstallFramework) && bInstallFramework)
    {
        pszMessage = UtPepSetupAllocString(IDS_INSTALLINGNETFRAMEWORK);

        if (pszMessage)
        {
            PepSetupModelessDialogDisplayMessage(pszMessage);

            UtPepSetupFreeString(pszMessage);
        }

        nCommandLineLen = ::lstrlen(CNetFramework4CommandLineArgs) + 1;

        pszCommandLine = (LPWSTR)UtAllocMem(nCommandLineLen * sizeof(WCHAR));

        if (pszCommandLine == NULL)
        {
            return FALSE;
        }

        ::StringCchCopy(pszCommandLine, nCommandLineLen, CNetFramework4CommandLineArgs);

        if (pszLogFile)
        {
            UtPepSetupAppendLine(pszLogFile, L"**** Installing the .NET Framework 4 with the command line arguments ****\r\n");
            UtPepSetupAppendLine(pszLogFile, pszCommandLine);
            UtPepSetupAppendLine(pszLogFile, L"\r\n");
            UtPepSetupAppendLine(pszLogFile, L"**** Begin .NET Framework 4 Install Log ****\r\n");
        }

        ::ZeroMemory(&StartupInfo, sizeof(StartupInfo));

        StartupInfo.cb = sizeof(StartupInfo);

        if (::CreateProcess(PepSetupGetNetFrameworkFile(), pszCommandLine, NULL, NULL,
                            FALSE, 0, NULL, NULL, &StartupInfo,
                            &ProcessInformation))
        {
            ::WaitForSingleObject(ProcessInformation.hProcess, INFINITE);

            ::CloseHandle(ProcessInformation.hProcess);
            ::CloseHandle(ProcessInformation.hThread);
        }
        else
        {
            bResult = FALSE;
        }

        UtFreeMem(pszCommandLine);

        if (pszLogFile)
        {
            UtPepSetupAppendLine(pszLogFile, L"**** End .NET Framework 4 Install Log ****\r\n");
        }
    }
    else
    {
        pszMessage = UtPepSetupAllocString(IDS_NETFRAMEWORKINSTALLED);

        if (pszMessage)
        {
            PepSetupModelessDialogDisplayMessage(pszMessage);

            UtPepSetupFreeString(pszMessage);
        }
    }

    return bResult;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
