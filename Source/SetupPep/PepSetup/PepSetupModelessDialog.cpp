/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepSetupModelessDialog.h"
#include "UtPepSetup.h"

#include "Utils/UtHeapProcess.h"

#include <Includes/UtVersion.h>

#pragma region Constants

#define WM_QUITDIALOG (WM_USER + 1)
#define WM_DISPLAYMESSAGEBOX (WM_USER + 2)
#define WM_DISPLAYMESSAGE (WM_USER + 3)
#define WM_DISPLAYREBOOTPROMPT (WM_USER + 4)
#define WM_DISPLAYREBOOTFAILED (WM_USER + 5)
#define WM_DISPLAYUNINSTALLPROMPT (WM_USER + 6)

#define CMessageDelayMilliseconds (2 * 1000)

#pragma endregion

#pragma region Enumerations

enum class  EMessageBoxDescription
{
    AppAlreadyRunning,
    UnknownError,
    FailedExtract,
    InstallAlreadyRunning,
    InstallError
};

#pragma endregion

#pragma region Local Variables

static HWND l_hModelessDialog = NULL;
static BOOL l_bQuitMessagePump = FALSE;

#pragma endregion

#pragma region Message Handlers

static INT_PTR lHandleInitDialogMessage(
  _In_ HWND hWnd)
{
    HWND hProgressBar = ::GetDlgItem(hWnd, IDC_PROGRESSBAR);

    ::SetWindowLongPtr(hProgressBar, GWL_STYLE, ::GetWindowLongPtr(hProgressBar, GWL_STYLE) | PBS_MARQUEE);

    ::SendMessage(hProgressBar, PBM_SETMARQUEE, TRUE, 0);

    return TRUE;
}

static INT_PTR lHandleQuitDialogMessage()
{
    l_bQuitMessagePump = TRUE;

    return 0;
}

static INT_PTR lHandleDisplayMessageBoxMessage(
  _In_ HWND hWnd,
  _In_ EMessageBoxDescription MessageBoxDescription)
{
    LPCWSTR pszAppTitle = UtPepSetupAllocString(IDS_APPTITLE);
    INT nMessageId = 0;
    LPCWSTR pszMessage = NULL;

    switch (MessageBoxDescription)
    {
        case EMessageBoxDescription::AppAlreadyRunning:
            nMessageId = IDS_APPLICATIONALREADYRUNNING;
            break;
        case EMessageBoxDescription::UnknownError:
            nMessageId = IDS_UNKNOWNERROR;
            break;
        case EMessageBoxDescription::FailedExtract:
            nMessageId = IDS_MSICOULDNOTBECOPIED;
            break;
        case EMessageBoxDescription::InstallAlreadyRunning:
            nMessageId = IDS_ANOTHERINSTALLATIONRUNNING;
            break;
        case EMessageBoxDescription::InstallError:
            nMessageId = IDS_INSTALLATIONERROR;
            break;
        default:
            break;
    }

    pszMessage = UtPepSetupAllocString(nMessageId);

    ::MessageBoxW(hWnd, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

    UtPepSetupFreeString(pszAppTitle);
    UtPepSetupFreeString(pszMessage);

    l_bQuitMessagePump = TRUE;

    return 0;
}

static INT_PTR lHandleDisplayMessageMessage(
  _In_ HWND hWnd,
  _In_z_ LPCWSTR pszMessage)
{
    HWND hStatusLabel = ::GetDlgItem(hWnd, IDC_STATUSLABEL);

    ::SetWindowText(hStatusLabel, pszMessage);

    return 0;
}

static INT_PTR lHandleDisplayRebootPromptMessage(
  _In_ HWND hWnd,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbRebootAllowed)
{
    LPCWSTR pszAppTitle = UtPepSetupAllocString(IDS_APPTITLE);
    LPCWSTR pszMessage = UtPepSetupAllocString(IDS_REBOOTREQUIRED);

    *pbRebootAllowed = FALSE;

    switch (::MessageBox(hWnd, pszMessage, pszAppTitle, MB_YESNO | MB_ICONWARNING))
    {
        case IDYES:
            *pbRebootAllowed = TRUE;
            break;
        case IDNO:
            *pbRebootAllowed = FALSE;
            break;
    }

    UtPepSetupFreeString(pszAppTitle);
    UtPepSetupFreeString(pszMessage);

    return 0;
}

static INT_PTR lHandleDisplayRebootFailedMessage(
  _In_ HWND hWnd,
  _In_ DWORD dwErrorCode)
{
    LPCWSTR pszAppTitle = UtPepSetupAllocString(IDS_APPTITLE);
    LPCWSTR pszMessage = UtPepSetupAllocString(IDS_REBOOTFAILED);
    LPWSTR pszFormattedMessage;
    INT nFormattedMessageLen;

    if (pszAppTitle && pszMessage)
    {
        nFormattedMessageLen = ::lstrlen(pszMessage) + 20;

        pszFormattedMessage = (LPWSTR)UtAllocMem(nFormattedMessageLen * sizeof(WCHAR));

        if (pszFormattedMessage)
        {
            ::StringCbPrintf(pszFormattedMessage, nFormattedMessageLen, pszMessage, dwErrorCode);

            ::MessageBox(hWnd, pszFormattedMessage, pszAppTitle, MB_OK | MB_ICONERROR);

            UtFreeMem(pszFormattedMessage);
        }
    }

    UtPepSetupFreeString(pszAppTitle);
    UtPepSetupFreeString(pszMessage);

    return 0;
}

static INT_PTR lHandleDisplayUninstallPromptMessage(
  _In_ HWND hWnd,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbUninstall)
{
    LPCWSTR pszAppTitle = UtPepSetupAllocString(IDS_APPTITLE);
    LPCWSTR pszMessage = UtPepSetupAllocString(IDS_UNINSTALL);

    *pbUninstall = FALSE;

    switch (::MessageBox(hWnd, pszMessage, pszAppTitle, MB_YESNO | MB_ICONWARNING))
    {
        case IDYES:
            *pbUninstall = TRUE;
            break;
        case IDNO:
            *pbUninstall = FALSE;
            break;
    }

    UtPepSetupFreeString(pszAppTitle);
    UtPepSetupFreeString(pszMessage);

    return 0;
}

#pragma endregion

#pragma region Dialog Callback

static INT_PTR CALLBACK lModelessDialogProc(
  _In_ HWND hWnd,
  _In_ UINT nMessage,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam)
{
    lParam;

    switch (nMessage)
    {
        case WM_QUITDIALOG:
            return lHandleQuitDialogMessage();
        case WM_DISPLAYMESSAGEBOX:
            return lHandleDisplayMessageBoxMessage(hWnd, (EMessageBoxDescription)wParam);
        case WM_DISPLAYMESSAGE:
            return lHandleDisplayMessageMessage(hWnd, (LPCWSTR)lParam);
        case WM_DISPLAYREBOOTPROMPT:
            return lHandleDisplayRebootPromptMessage(hWnd, (LPBOOL)lParam);
        case WM_DISPLAYREBOOTFAILED:
            return lHandleDisplayRebootFailedMessage(hWnd, (DWORD)lParam);
        case WM_DISPLAYUNINSTALLPROMPT:
            return lHandleDisplayUninstallPromptMessage(hWnd, (LPBOOL)lParam);
        case WM_INITDIALOG:
            return lHandleInitDialogMessage(hWnd);
    }

    return 0;
}

#pragma endregion

#pragma region Public Functions

BOOL PepSetupModelessDialogCreate(
  _In_ HINSTANCE hInstance)
{
    l_hModelessDialog = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_PEPSETUP),
                                           ::GetDesktopWindow(), lModelessDialogProc, 0);

    return (l_hModelessDialog != NULL) ? TRUE : FALSE;
}

BOOL PepSetupModelessDialogDestroy()
{
    ::DestroyWindow(l_hModelessDialog);

    return TRUE;
}

VOID PepSetupModelessDialogMessagePump()
{
    MSG Message;

    while (!l_bQuitMessagePump)
    {
        switch (::GetMessage(&Message, NULL, 0, 0))
        {
            case 0:
                l_bQuitMessagePump = TRUE;
                break;
            case -1:
                // Ignore error
                break;
            default:
                if (!::IsDialogMessage(l_hModelessDialog, &Message))
                {
                    ::TranslateMessage(&Message);
                    ::DispatchMessage(&Message);
                }
                break;
        }
    }
}

VOID PepSetupModelessDialogQuitMessagePump()
{
    if (!l_bQuitMessagePump)
    {
        ::PostMessage(l_hModelessDialog, WM_QUITDIALOG, 0, 0);
    }
}

VOID PepSetupModelessDialogDisplayAppAlreadyRunning()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, (int)EMessageBoxDescription::AppAlreadyRunning, 0);
}

VOID PepSetupModelessDialogDisplayUnknownError()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, (int)EMessageBoxDescription::UnknownError, 0);
}

VOID PepSetupModelessDialogDisplayFailedExtractError()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, (int)EMessageBoxDescription::FailedExtract, 0);
}

VOID PepSetupModelessDialogDisplayInstallAlreadyRunning()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, (int)EMessageBoxDescription::InstallAlreadyRunning, 0);
}

VOID PepSetupModelessDialogDisplayInstallError()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, (int)EMessageBoxDescription::InstallError, 0);
}

VOID PepSetupModelessDialogDisplayMessage(
  _In_z_ LPCWSTR pszMessage)
{
    ::SendMessage(l_hModelessDialog, WM_DISPLAYMESSAGE, 0, (LPARAM)pszMessage);

    ::Sleep(CMessageDelayMilliseconds);
}

VOID PepSetupModelessDialogDisplayRebootPrompt(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbRebootAllowed)
{
    ::SendMessage(l_hModelessDialog, WM_DISPLAYREBOOTPROMPT, 0, (LPARAM)pbRebootAllowed);
}

VOID PepSetupModelessDialogDisplayRebootFailed(
  _In_ DWORD dwErrorCode)
{
    ::SendMessage(l_hModelessDialog, WM_DISPLAYREBOOTFAILED, 0, dwErrorCode);
}

VOID PepSetupModelessDialogDisplayUninstallPrompt(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbUninstall)
{
    ::SendMessage(l_hModelessDialog, WM_DISPLAYUNINSTALLPROMPT, 0, (LPARAM)pbUninstall);
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
