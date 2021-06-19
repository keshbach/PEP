/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepSetupInstallVcRedist.h"

#include "PepSetupResources.h"
#include "PepSetupModelessDialog.h"

#include "UtPepSetup.h"

#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>

#pragma region Constants

// Visual Studio Redistributable definitions

#if _MSC_VER != 1929
#error Check if Visual Studio Redistributable needs to be changed.
#endif

#define CVisualStudioVersionSubKeyName L"14.0"

#define CVisualStudioRedistMajorVersion 14
#define CVisualStudioRedistMinorVersion 29
#define CVisualStudioRedistBuildVersion 30038
#define CVisualStudioRedistReleaseBuildVersion 0

// Visual Studio Redistributable Command Line Arguments definitions

#define CVisualStudioRedistCommandLineArgs L"/install /quiet /norestart /log "

#define CVCRedistLogFileName L"vcredist.log"

#pragma endregion

#pragma region Local Functions

static BOOL lReadVisualStudioRedistRegValues(
  _In_ HKEY hKey,
  _Out_writes_bytes_(sizeof(DWORD)) LPDWORD pdwMajor,
  _Out_writes_bytes_(sizeof(DWORD)) LPDWORD pdwMinor,
  _Out_writes_bytes_(sizeof(DWORD)) LPDWORD pdwBuild,
  _Out_writes_bytes_(sizeof(DWORD)) LPDWORD pdwReleaseBuild)
{
    DWORD dwDataLen;

    *pdwMajor = 0;
    *pdwMinor = 0;
    *pdwBuild = 0;
    *pdwReleaseBuild = 0;

    dwDataLen = sizeof(DWORD);

    if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, L"Major", NULL, NULL, (LPBYTE)pdwMajor, &dwDataLen))
    {
        return FALSE;
    }

    dwDataLen = sizeof(DWORD);

    if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, L"Minor", NULL, NULL, (LPBYTE)pdwMinor, &dwDataLen))
    {
        return FALSE;
    }

    dwDataLen = sizeof(DWORD);

    if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, L"Bld", NULL, NULL, (LPBYTE)pdwBuild, &dwDataLen))
    {
        return FALSE;
    }

    dwDataLen = sizeof(DWORD);

    if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, L"Rbld", NULL, NULL, (LPBYTE)pdwReleaseBuild, &dwDataLen))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL lShouldInstallVisualStudioRedist(
  _In_ DWORD dwMajor,
  _In_ DWORD dwMinor,
  _In_ DWORD dwBuild,
  _In_ DWORD dwReleaseBuild)
{
    if (dwMajor > CVisualStudioRedistMajorVersion)
    {
        return FALSE;
    }

    if (dwMajor < CVisualStudioRedistMajorVersion)
    {
        return TRUE;
    }

    if (dwMinor > CVisualStudioRedistMinorVersion)
    {
        return FALSE;
    }

    if (dwMinor < CVisualStudioRedistMinorVersion)
    {
        return TRUE;
    }

    if (dwBuild > CVisualStudioRedistBuildVersion)
    {
        return FALSE;
    }

    if (dwBuild < CVisualStudioRedistBuildVersion)
    {
        return TRUE;
    }

    if (dwReleaseBuild > CVisualStudioRedistReleaseBuildVersion)
    {
        return FALSE;
    }

    if (dwReleaseBuild < CVisualStudioRedistReleaseBuildVersion)
    {
        return TRUE;
    }

    return FALSE;
}

#if defined(WIN32)
static BOOL lInstallVisualStudioRedist(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbInstallRedist)
{
    BOOL bResult = FALSE;
    WCHAR cSubKey[100];
    HKEY hKey;
    LSTATUS Status;
    DWORD dwMajor, dwMinor, dwBuild, dwReleaseBuild;

    *pbInstallRedist = FALSE;

    if (UtPepSetupIsWindows64Present())
    {
        ::StringCchCopyW(cSubKey, MArrayLen(cSubKey), L"SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\");
    }
    else
    {
        ::StringCchCopyW(cSubKey, MArrayLen(cSubKey), L"SOFTWARE\\Microsoft\\VisualStudio\\");
    }

    ::StringCchCatW(cSubKey, MArrayLen(cSubKey), CVisualStudioVersionSubKeyName);
    ::StringCchCatW(cSubKey, MArrayLen(cSubKey), L"\\VC\\Runtimes\\x86");

    Status = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, cSubKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

    switch (Status)
    {
        case ERROR_SUCCESS:
            if (lReadVisualStudioRedistRegValues(hKey, &dwMajor, &dwMinor, &dwBuild, &dwReleaseBuild))
            {
                *pbInstallRedist = lShouldInstallVisualStudioRedist(dwMajor, dwMinor, dwBuild, dwReleaseBuild);
            }

            ::RegCloseKey(hKey);

            bResult = TRUE;
            break;
        case ERROR_FILE_NOT_FOUND:
            *pbInstallRedist = TRUE;

            bResult = TRUE;
            break;
    }

    return bResult;
}
#elif defined(WIN64)
static BOOL lInstallVisualStudioRedist(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbInstallRedist)
{
    BOOL bResult = FALSE;
    WCHAR cSubKey[100];
    HKEY hKey;
    LSTATUS Status;
    DWORD dwMajor, dwMinor, dwBuild, dwReleaseBuild;

    *pbInstallRedist = FALSE;

    ::StringCchCopyW(cSubKey, MArrayLen(cSubKey), L"SOFTWARE\\Microsoft\\VisualStudio\\");

    ::StringCchCatW(cSubKey, MArrayLen(cSubKey), CVisualStudioVersionSubKeyName);
    ::StringCchCatW(cSubKey, MArrayLen(cSubKey), L"\\VC\\Runtimes\\x64");

    Status = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, cSubKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

    switch (Status)
    {
        case ERROR_SUCCESS:
            if (lReadVisualStudioRedistRegValues(hKey, &dwMajor, &dwMinor, &dwBuild, &dwReleaseBuild))
            {
                *pbInstallRedist = lInstallVisualStudioRedist(dwMajor, dwMinor, dwBuild, dwReleaseBuild);
            }

            ::RegCloseKey(hKey);

            bResult = TRUE;
            break;
        case ERROR_FILE_NOT_FOUND:
            *pbInstallRedist = TRUE;

            bResult = TRUE;
            break;
    }

    return bResult;
}
#else
#error Unsupported architecture
#endif

#pragma endregion

#pragma region Public Functions

BOOL PepSetupInstallVisualStudioRedist(
  _In_opt_z_ LPCWSTR pszLogFile)
{
    BOOL bResult = TRUE;
    LPCWSTR pszMessage;
    BOOL bInstallRedist;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    INT nCommandLineLen;
    LPWSTR pszCommandLine;
    WCHAR cTempLogFile[MAX_PATH];

    pszMessage = UtPepSetupAllocString(IDS_CHECKINGVISUALSTUDIOREDIST);

    if (pszMessage)
    {
        PepSetupModelessDialogDisplayMessage(pszMessage);

        UtPepSetupFreeString(pszMessage);
    }

    if (lInstallVisualStudioRedist(&bInstallRedist) && bInstallRedist)
    {
        pszMessage = UtPepSetupAllocString(IDS_INSTALLINGVISUALSTUDIOREDIST);

        if (pszMessage)
        {
            PepSetupModelessDialogDisplayMessage(pszMessage);

            UtPepSetupFreeString(pszMessage);
        }

        ::PathCombine(cTempLogFile, PepSetupGetTempPath(), CVCRedistLogFileName);

        nCommandLineLen = ::lstrlen(CVisualStudioRedistCommandLineArgs);
        nCommandLineLen += ::lstrlen(cTempLogFile);
        nCommandLineLen += 3;

        pszCommandLine = (LPWSTR)UtAllocMem(nCommandLineLen * sizeof(WCHAR));

        if (pszCommandLine == NULL)
        {
            return FALSE;
        }

        ::StringCchCopyW(pszCommandLine, nCommandLineLen, CVisualStudioRedistCommandLineArgs);
        ::StringCchCatW(pszCommandLine, nCommandLineLen, L"\"");
        ::StringCchCatW(pszCommandLine, nCommandLineLen, cTempLogFile);
        ::StringCchCatW(pszCommandLine, nCommandLineLen, L"\"");

        if (pszLogFile)
        {
            UtPepSetupAppendLine(pszLogFile, L"**** Installing the Visual Studio Redistributable with the command line arguments ****\r\n");
            UtPepSetupAppendLine(pszLogFile, pszCommandLine);
            UtPepSetupAppendLine(pszLogFile, L"\r\n");
            UtPepSetupAppendLine(pszLogFile, L"**** Begin Visual Studio Redistributable Install Log ****\r\n");
        }

        ::ZeroMemory(&StartupInfo, sizeof(StartupInfo));

        StartupInfo.cb = sizeof(StartupInfo);

        if (::CreateProcess(PepSetupGetVcRedistFile(), pszCommandLine, NULL, NULL,
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
            if (!UtPepSetupConvertAnsiLogFileToUnicodeLogFile(cTempLogFile, pszLogFile))
            {
                UtPepSetupAppendLine(pszLogFile, L"**** Failed to convert Visual Studio Install Log ****\r\n");
            }

            UtPepSetupAppendLine(pszLogFile, L"**** End Visual Studio Redistributable Install Log ****\r\n");
        }

        ::DeleteFile(cTempLogFile);
    }
    else
    {
        pszMessage = UtPepSetupAllocString(IDS_VISUALSTUDIOREDISTINSTALLED);

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
//  Copyright (C) 2018-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
