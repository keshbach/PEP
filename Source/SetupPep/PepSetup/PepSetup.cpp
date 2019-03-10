/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepSetup.h"

#include "PepSetupModelessDialog.h"
#include "PepSetupResources.h"
#include "PepSetupInstallVcRedist.h"
#include "PepSetupInstallNetFramework.h"
#include "PepSetupUpdateMsi.h"

#include "UtPepSetup.h"

#include <Includes/UtThread.inl>

#include <Includes/UtVersion.h>
#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>

#pragma region Constants

// Command Line Arguments definitions

#define CExtractMsiArgument L"/extractmsi"
#define CLogSetupArgument L"/logsetup"

// MSI Command Line Arguments definitions

#define CMsiNoRestartArgs L"/norestart "
#define CMsiLogArgs L"/L*v+ "

#define CProductCodeQueryStatement L"SELECT * FROM Property WHERE Property = 'ProductCode'"

#define CPepSetupAppRunningMutexName L"PepSetupAppRunningMutex"

#pragma endregion

#pragma region Enumerations

enum EInstallResult
{
    eirNone,
    eirSuccessRebootRequired,
    eirSuccessRebootInitiated,
    eirCancelledByUser,
    eirInstallAlreadyRunning,
    eirUnknownError
};

#pragma endregion

#pragma region Structures

typedef struct tagTInstallData
{
    HINSTANCE hInstance;
    LPCWSTR pszArguments;
} TInstallData;

#pragma endregion

#pragma region Global Variables

static HANDLE l_hAppRunningMutex = NULL;

#pragma endregion

#pragma region Local Functions

static VOID lDisplayRebootFailureMessage(
  _In_ DWORD dwErrorCode)
{
    LPCWSTR pszMessage;
    LPWSTR pszFormattedMessage;
    INT nFormattedMessageLen;

    pszMessage = UtPepSetupAllocString(IDS_REBOOTFAILED);

    if (pszMessage)
    {
        nFormattedMessageLen = ::lstrlen(pszMessage) + 20;

        pszFormattedMessage = (LPWSTR)UtAllocMem(nFormattedMessageLen * sizeof(WCHAR));

        if (pszFormattedMessage)
        {
            ::StringCbPrintf(pszFormattedMessage, nFormattedMessageLen, pszMessage, dwErrorCode);

            PepSetupModelessDialogDisplayMessage(pszFormattedMessage);

            UtFreeMem(pszFormattedMessage);
        }

        UtPepSetupFreeString(pszMessage);
    }
}

static BOOL lGetMsiProductCode(
  _In_z_ LPCWSTR pszDatabase,
  _Out_writes_bytes_(sizeof(GUID)) LPGUID pProductGuid)
{
    BOOL bResult(TRUE);
    MSIHANDLE hDatabase;
    MSIHANDLE hView(NULL), hRecord(NULL);
    WCHAR cBuffer[40];
    DWORD dwBufferLen;

    ::ZeroMemory(pProductGuid, sizeof(GUID));

    if (ERROR_SUCCESS != ::MsiOpenDatabase(pszDatabase, MSIDBOPEN_READONLY, &hDatabase))
    {
        return FALSE;
    }

    dwBufferLen = MArrayLen(cBuffer);

    if (ERROR_SUCCESS == ::MsiDatabaseOpenView(hDatabase, CProductCodeQueryStatement, &hView) &&
        ERROR_SUCCESS == ::MsiViewExecute(hView, 0) &&
        ERROR_SUCCESS == ::MsiViewFetch(hView, &hRecord) &&
        ERROR_SUCCESS == ::MsiRecordGetString(hRecord, 2, cBuffer, &dwBufferLen) &&
        NOERROR == ::CLSIDFromString(cBuffer, pProductGuid))
    {
        bResult = TRUE;
    }

    if (hView)
    {
        ::MsiCloseHandle(hView);

        hView = NULL;
    }

    if (hRecord)
    {
        ::MsiCloseHandle(hRecord);

        hRecord = NULL;
    }

    if (ERROR_SUCCESS != ::MsiCloseHandle(hDatabase))
    {
        bResult = FALSE;
    }

    return bResult;
}

static VOID lUninstallProduct(
  _In_ LPGUID pProductGuid)
{
    LPOLESTR pBuffer;
    UINT nResult;

    if (NOERROR != ::StringFromCLSID(*pProductGuid, &pBuffer))
    {
        PepSetupModelessDialogDisplayUnknownError();

        return;
    }

    nResult = ::MsiConfigureProduct(pBuffer, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT);

    switch (nResult)
    {
        case ERROR_INVALID_PARAMETER:
            PepSetupModelessDialogDisplayUnknownError();
            break;
        case ERROR_SUCCESS:
            break;
        case ERROR_INSTALL_USEREXIT:
            break;
        default:
            PepSetupModelessDialogDisplayUnknownError();
            break;
    }

    ::CoTaskMemFree(pBuffer);
}

static EInstallResult lProcessInstallExitCode(
  _In_ DWORD dwExitCode,
  _In_opt_z_ LPCWSTR pszLogFile)
{
    EInstallResult InstallResult = eirUnknownError;
    WCHAR cMessage[50];

    switch (dwExitCode)
    {
        case ERROR_SUCCESS:
            InstallResult = eirSuccessRebootRequired;

            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"The installation was successfully.\r\n");
            }
            break;
        case ERROR_SUCCESS_REBOOT_INITIATED:
            InstallResult = eirSuccessRebootInitiated;

            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"The installation was successfully and a reboot has been initiated.\r\n");
            }
            break;
        case ERROR_SUCCESS_REBOOT_REQUIRED:
            InstallResult = eirSuccessRebootRequired;

            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"The installation was successfully but a reboot is required.\r\n");
            }
            break;
        case ERROR_INSTALL_USEREXIT:
            InstallResult = eirCancelledByUser;

            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"The user cancelled the installation.\r\n");
            }
            break;
        case ERROR_INSTALL_ALREADY_RUNNING:
            InstallResult = eirInstallAlreadyRunning;

            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"An installation is already in progress.\r\n");
            }
            break;
        case ERROR_INSTALL_LOG_FAILURE:
            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"The install log file could not be created.\r\n");
            }
            break;
        case ERROR_INSTALL_TEMP_UNWRITABLE:
            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"The install temp directory is unwritable.\r\n");
            }
            break;
        default:
            if (pszLogFile)
            {
                ::StringCchPrintf(cMessage, MArrayLen(cMessage), L"Unknown install error code: %d\r\n", dwExitCode);

                UtPepSetupAppendLine(pszLogFile, cMessage);
            }
            break;
    }

    return InstallResult;
}

static BOOL lExecuteInstall(
  _In_z_ LPCWSTR pszInstallFile,
  _In_opt_z_ LPCWSTR pszLogFile,
  _Out_ EInstallResult* pInstallResult)
{
    BOOL bResult(FALSE);
    LPWSTR pszParameters(NULL);
    INT nParametersLen(0);
    LPCWSTR pszMessage;
    SHELLEXECUTEINFO ShellExecuteInfo;
    DWORD dwExitCode;

    *pInstallResult = eirUnknownError;

    pszMessage = UtPepSetupAllocString(IDS_INSTALLINGFILES);

    if (pszMessage)
    {
        PepSetupModelessDialogDisplayMessage(pszMessage);

        UtPepSetupFreeString(pszMessage);
    }

    nParametersLen += lstrlen(CMsiNoRestartArgs);

    if (pszLogFile != NULL)
    {
        nParametersLen += lstrlen(CMsiLogArgs) + 2;
        nParametersLen += lstrlen(pszLogFile);
    }

    nParametersLen += 1;

    pszParameters = (LPWSTR)UtAllocMem(nParametersLen * sizeof(WCHAR));

    if (pszParameters == NULL)
    {
        return FALSE;
    }

    *pszParameters = 0;

    ::StringCchCat(pszParameters, nParametersLen, CMsiNoRestartArgs);

    if (pszLogFile != NULL)
    {
        ::StringCchCat(pszParameters, nParametersLen, CMsiLogArgs);
        ::StringCchCat(pszParameters, nParametersLen, L"\"");
        ::StringCchCat(pszParameters, nParametersLen, pszLogFile);
        ::StringCchCat(pszParameters, nParametersLen, L"\"");
    }

    ShellExecuteInfo.cbSize = sizeof(ShellExecuteInfo);
    ShellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    ShellExecuteInfo.hwnd = NULL;
    ShellExecuteInfo.lpVerb = L"open";
    ShellExecuteInfo.lpFile = pszInstallFile;
    ShellExecuteInfo.lpParameters = pszParameters;
    ShellExecuteInfo.lpDirectory = NULL;
    ShellExecuteInfo.nShow = SW_SHOWDEFAULT;

    if (::ShellExecuteEx(&ShellExecuteInfo) && ShellExecuteInfo.hProcess != NULL)
    {
        ::WaitForSingleObject(ShellExecuteInfo.hProcess, INFINITE);

        if (::GetExitCodeProcess(ShellExecuteInfo.hProcess, &dwExitCode))
        {
            *pInstallResult = lProcessInstallExitCode(dwExitCode, pszLogFile);
        }
        else
        {
            if (pszLogFile)
            {
                UtPepSetupAppendLine(pszLogFile, L"Failed to get the exit code of the install process.\r\n");
            }
        }

        ::CloseHandle(ShellExecuteInfo.hProcess);

        bResult = TRUE;
    }

    UtFreeMem(pszParameters);

    return bResult;
}

static BOOL lIsSupportedOperatingSystem(VOID)
{
    DWORDLONG nConditionMask(0);
    OSVERSIONINFOEX VersionInfoEx;

    VersionInfoEx.dwOSVersionInfoSize = sizeof(VersionInfoEx);
    VersionInfoEx.dwMajorVersion = 6; // set to 6 for vista and above
    VersionInfoEx.dwMinorVersion = 1; // set for Windows 7 and above

    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MAJORVERSION,
                                           VER_GREATER_EQUAL);
    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MINORVERSION,
                                           VER_GREATER_EQUAL);

    return ::VerifyVersionInfo(&VersionInfoEx, VER_MAJORVERSION | VER_MINORVERSION,
                               nConditionMask);
}

static BOOL lIsProductAlreadyInstalled(
  _In_ LPGUID pProductGuid,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbAlreadyInstalled)
{
    LPOLESTR pBuffer;
    DWORD dwValueLen;

    *pbAlreadyInstalled = FALSE;

    if (NOERROR != ::StringFromCLSID(*pProductGuid, &pBuffer))
    {
        return FALSE;
    }

    dwValueLen = 0;

    if (ERROR_SUCCESS == ::MsiGetProductInfo(pBuffer,
                                             INSTALLPROPERTY_INSTALLLOCATION,
                                             NULL, &dwValueLen))
    {
        *pbAlreadyInstalled = TRUE;
    }

    ::CoTaskMemFree(pBuffer);

    return TRUE;
}

static BOOL lInitialize(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbAlreadyRunning)
{
    BOOL bResult = FALSE;

    *pbAlreadyRunning = FALSE;

    // Check if the application is already running

    l_hAppRunningMutex = ::CreateMutex(NULL, FALSE, CPepSetupAppRunningMutexName);

    if (l_hAppRunningMutex)
    {
        if (::GetLastError() == ERROR_ALREADY_EXISTS)
        {
            *pbAlreadyRunning = TRUE;
        }

        bResult = TRUE;
    }
    else
    {
        if (::GetLastError() == ERROR_INVALID_HANDLE)
        {
            // name matches a different object
        }
        else if (::GetLastError() == ERROR_ACCESS_DENIED)
        {
            *pbAlreadyRunning = TRUE;

            bResult = TRUE;
        }
    }

    return bResult;
}

static VOID lUninitialize()
{
    ::CloseHandle(l_hAppRunningMutex);
}

static BOOL lAddRebootPrivileges(
  _In_opt_z_ LPCWSTR pszLogFile)
{
    HANDLE hProcess = ::GetCurrentProcess();
    HANDLE hToken;
    TOKEN_PRIVILEGES TokenPrivileges;
    LUID luid;
    DWORD dwError;
    WCHAR cBuffer[100];

    if (pszLogFile)
    {
        UtPepSetupAppendLine(pszLogFile, L"Attempting to grant shutdown privileges to the process");
        UtPepSetupAppendLine(pszLogFile, L"Calling OpenProcessToken\r\n");
    }

    if (!::OpenProcessToken(hProcess, TOKEN_WRITE | TOKEN_READ, &hToken))
    {
        if (pszLogFile)
        {
            ::StringCchPrintf(cBuffer, MArrayLen(cBuffer), L"OpenProcessToken failed (Error Code: 0x%x)\r\n", ::GetLastError());

            UtPepSetupAppendLine(pszLogFile, cBuffer);
        }

        return FALSE;
    }

    if (pszLogFile)
    {
        UtPepSetupAppendLine(pszLogFile, L"Calling LookupPrivilegeValue\r\n");
    }

    if (!::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid))
    {
        if (pszLogFile)
        {
            ::StringCchPrintf(cBuffer, MArrayLen(cBuffer), L"LookupPrivilegeValue failed (Error Code: 0x%x)\r\n", ::GetLastError());

            UtPepSetupAppendLine(pszLogFile, cBuffer);
        }

        ::CloseHandle(hToken);

        return FALSE;
    }

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = luid;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (pszLogFile)
    {
        UtPepSetupAppendLine(pszLogFile, L"Calling AdjustTokenPrivileges\r\n");
    }

    if (!::AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TokenPrivileges),
                                 NULL, NULL))
    {
        if (pszLogFile)
        {
            ::StringCchPrintf(cBuffer, MArrayLen(cBuffer), L"AdjustTokenPrivileges failed (Error Code: 0x%x)\r\n", ::GetLastError());

            UtPepSetupAppendLine(pszLogFile, cBuffer);
        }

        ::CloseHandle(hToken);

        return FALSE;
    }

    dwError = ::GetLastError();

    if (pszLogFile)
    {
        UtPepSetupAppendLine(pszLogFile, L"Closing the token handle");
    }

    ::CloseHandle(hToken);

    if (dwError != ERROR_SUCCESS)
    {
        if (pszLogFile)
        {
            ::StringCchPrintf(cBuffer, MArrayLen(cBuffer), L"Failed to adjust token privileges (Error Code: 0x%x)\r\n", dwError);

            UtPepSetupAppendLine(pszLogFile, cBuffer);
        }

        return FALSE;
    }

    if (pszLogFile)
    {
        UtPepSetupAppendLine(pszLogFile, L"Shutdown privileges granted to the process");
    }

    return TRUE;
}

static DWORD WINAPI lRunInstallThreadProc(
  _In_ LPVOID pvParameter)
{
    DWORD dwResult = 0;
    TInstallData* pInstallData = (TInstallData*)pvParameter;
    EInstallResult InstallResult = eirNone;
    LPWSTR* ppszArgs;
    INT nTotalArgs;
    BOOL bAlreadyRunning, bAlreadyInstalled, bRebootAllowed, bUninstall;
    LPCWSTR pszMessage;
    GUID ProductCodeGuid;

    if (!lIsSupportedOperatingSystem())
    {
        PepSetupModelessDialogDisplayUnsupportedOS();

        return 1;
    }

    if (!lInitialize(&bAlreadyRunning))
    {
        PepSetupModelessDialogDisplayUnknownError();

        return 1;
    }

    if (bAlreadyRunning)
    {
        PepSetupModelessDialogDisplayAppAlreadyRunning();

        lUninitialize();

        return 1;
    }

    ppszArgs = ::CommandLineToArgvW(pInstallData->pszArguments, &nTotalArgs);

    if (ppszArgs == NULL)
    {
        PepSetupModelessDialogDisplayUnknownError();

        lUninitialize();

        return 1;
    }

    pszMessage = UtPepSetupAllocString(IDS_EXTRACTINGSETUPFILES);

    if (pszMessage)
    {
        PepSetupModelessDialogDisplayMessage(pszMessage);

        UtPepSetupFreeString(pszMessage);
    }

    if (FALSE == PepSetupResourcesInitialize(pInstallData->hInstance))
    {
        PepSetupModelessDialogDisplayUnknownError();

        ::LocalFree(ppszArgs);

        lUninitialize();

        return 1;
    }

    if (lGetMsiProductCode(PepSetupGetPepSetupMsiFile(), &ProductCodeGuid) &&
        lIsProductAlreadyInstalled(&ProductCodeGuid, &bAlreadyInstalled) &&
        PepSetupUpdateMsi(PepSetupGetPepSetupMsiFile()))
    {
        if (nTotalArgs == 3)
        {
            if (::lstrcmpi(ppszArgs[1], CExtractMsiArgument) == 0)
            {
                if (!::CopyFile(PepSetupGetPepSetupMsiFile(), ppszArgs[2], TRUE))
                {
                    PepSetupModelessDialogDisplayFailedExtractError();

                    dwResult = 1;
                }
            }
            else if (::lstrcmpi(ppszArgs[1], CLogSetupArgument) == 0)
            {
                if (!bAlreadyInstalled)
                {
                    lAddRebootPrivileges(ppszArgs[2]);

                    if (!PepSetupInstallNetFrameworkRedist(ppszArgs[2]) ||
                        !PepSetupInstallVisualStudioRedist(ppszArgs[2]) ||
                        !lExecuteInstall(PepSetupGetPepSetupMsiFile(), ppszArgs[2], &InstallResult))
                    {
                        dwResult = 1;
                    }
                }
                else
                {
                    PepSetupModelessDialogDisplayUninstallPrompt(&bUninstall);

                    if (bUninstall)
                    {
                        pszMessage = UtPepSetupAllocString(IDS_UNINSTALLING);

                        if (pszMessage)
                        {
                            PepSetupModelessDialogDisplayMessage(pszMessage);

                            UtPepSetupFreeString(pszMessage);
                        }

                        lUninstallProduct(&ProductCodeGuid);
                    }

                    dwResult = 1;
                }
            }
            else
            {
                PepSetupModelessDialogDisplayCommandLineHelp();

                dwResult = 1;
            }
        }
        else if (nTotalArgs == 1)
        {
            if (!bAlreadyInstalled)
            {
                lAddRebootPrivileges(NULL);

                if (!PepSetupInstallNetFrameworkRedist(NULL) ||
                    !PepSetupInstallVisualStudioRedist(NULL) ||
                    !lExecuteInstall(PepSetupGetPepSetupMsiFile(), NULL, &InstallResult))
                {
                    dwResult = 1;
                }
            }
            else
            {
                PepSetupModelessDialogDisplayUninstallPrompt(&bUninstall);

                if (bUninstall)
                {
                    pszMessage = UtPepSetupAllocString(IDS_UNINSTALLING);

                    if (pszMessage)
                    {
                        PepSetupModelessDialogDisplayMessage(pszMessage);

                        UtPepSetupFreeString(pszMessage);
                    }

                    lUninstallProduct(&ProductCodeGuid);
                }

                dwResult = 1;
            }
        }
        else
        {
            PepSetupModelessDialogDisplayCommandLineHelp();

            dwResult = 1;
        }
    }
    else
    {
        PepSetupModelessDialogDisplayUnknownError();

        dwResult = 1;
    }

    PepSetupResourcesUninitialize();

    ::LocalFree(ppszArgs);

    if (dwResult == 0)
    {
        switch (InstallResult)
        {
            case eirSuccessRebootRequired:
                PepSetupModelessDialogDisplayRebootPrompt(&bRebootAllowed);

                if (bRebootAllowed)
                {
#pragma warning (disable:28159)
                    if (FALSE == ::InitiateSystemShutdownEx(NULL, (LPWSTR)L"Pep Driver Installed",
                                                            0, FALSE, TRUE,
                                                            SHTDN_REASON_MAJOR_SOFTWARE |
                                                                SHTDN_REASON_MINOR_HARDWARE_DRIVER |
                                                                SHTDN_REASON_MINOR_INSTALLATION))
                    {
                        lDisplayRebootFailureMessage(::GetLastError());
                    }
#pragma warning (default:28159)
                }
                break;
            case eirSuccessRebootInitiated:
            case eirCancelledByUser:
                // Do nothing
                break;
            case eirInstallAlreadyRunning:
                PepSetupModelessDialogDisplayInstallAlreadyRunning();
                break;
            case eirUnknownError:
                PepSetupModelessDialogDisplayInstallError();
                break;
            default:
                ::DebugBreak();
                break;
        }
    }

    PepSetupModelessDialogQuitMessagePump();

    lUninitialize();

    return dwResult;
}

#pragma endregion

#pragma region Public Functions

INT PepSetupExecuteInstall(
  _In_ HINSTANCE hInstance,
  _In_z_ LPCWSTR pszArguments)
{
    HANDLE hThread;
    TInstallData InstallData;
    DWORD dwThreadId, dwExitCode;
    LPCWSTR pszAppTitle, pszMessage;

    if (!PepSetupModelessDialogCreate(hInstance))
    {
        pszAppTitle = UtPepSetupAllocString(IDS_APPTITLE);
        pszMessage = UtPepSetupAllocString(IDS_CANNOTCREATEAPPWINDOW);

        ::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

        UtPepSetupFreeString(pszAppTitle);
        UtPepSetupFreeString(pszMessage);

        return 1;
    }

    InstallData.hInstance = hInstance;
    InstallData.pszArguments = pszArguments;

    hThread = ::CreateThread(NULL, 0, lRunInstallThreadProc, &InstallData, 0, &dwThreadId);

    if (hThread == NULL)
    {
        pszAppTitle = UtPepSetupAllocString(IDS_APPTITLE);
        pszMessage = UtPepSetupAllocString(IDS_CANNOTCREATEINSTALLTHREAD);

        ::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

        UtPepSetupFreeString(pszAppTitle);
        UtPepSetupFreeString(pszMessage);

        PepSetupModelessDialogDestroy();

        return 1;
    }

    UtSetThreadName(dwThreadId, "Install Thread");

    PepSetupModelessDialogMessagePump();

    ::WaitForSingleObject(hThread, INFINITE);

    ::GetExitCodeThread(hThread, &dwExitCode);

    ::CloseHandle(hThread);

    PepSetupModelessDialogDestroy();

    return dwExitCode;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
