/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepAppSplashDialog.h"
#include "UtPepApp.h"

#include "Utils/UtHeapProcess.h"

#include <Includes/UtVersion.h>

#pragma region Constants

#define WM_QUITDIALOG (WM_USER + 1)
#define WM_DISPLAYMESSAGEBOX (WM_USER + 2)
#define WM_DISPLAYMESSAGE (WM_USER + 3)
#define WM_EXECUTE (WM_USER + 4)

#define CMessageDelayMilliseconds (2 * 1000)

#pragma endregion

#pragma region Enumerations

enum EMessageBoxDescription
{
    embdAppAlreadyRunning,
    embdUnsupportedOS,
    embdUnknownError,
    embdFailedExtract,
    embdCommandLineHelp,
    embdInstallAlreadyRunning,
    embdInstallError
};

#pragma endregion

#pragma region "Structures"

typedef struct tagTPepAppSplashDialogExecuteData
{
	TPepAppSplashDialogExecuteFunc pExecute;
	LPVOID pvData;
	HANDLE hEvent;
} TPepAppSplashDialogExecuteData;

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
    LPCWSTR pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
    INT nMessageId = 0;
    LPCWSTR pszMessage = NULL;

    switch (MessageBoxDescription)
    {
        case embdAppAlreadyRunning:
            nMessageId = IDS_APPLICATIONALREADYRUNNING;
            break;
        case embdUnsupportedOS:
            nMessageId = IDS_UNSUPPORTEDWINDOWSVERSION;
            break;
        case embdUnknownError:
            nMessageId = IDS_UNKNOWNERROR;
            break;
        case embdCommandLineHelp:
            nMessageId = IDS_COMMANDLINEHELP;
            break;
        case embdInstallAlreadyRunning:
            nMessageId = IDS_ANOTHERINSTALLATIONRUNNING;
            break;
        case embdInstallError:
            nMessageId = IDS_INSTALLATIONERROR;
            break;
        default:
            break;
    }

    pszMessage = UtPepAppAllocString(nMessageId);

    ::MessageBoxW(hWnd, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

    UtPepAppFreeString(pszAppTitle);
    UtPepAppFreeString(pszMessage);

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

static INT_PTR lHandleExecuteMessage(
  TPepAppSplashDialogExecuteData* pExecuteData)
{
	pExecuteData->pExecute(pExecuteData->pvData);

	::SetEvent(pExecuteData->hEvent);

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
		case WM_EXECUTE:
			return lHandleExecuteMessage((TPepAppSplashDialogExecuteData*)lParam);
        case WM_INITDIALOG:
            return lHandleInitDialogMessage(hWnd);
    }

    return 0;
}

#pragma endregion

#pragma region Public Functions

BOOL PepAppSplashDialogCreate(
  _In_ HINSTANCE hInstance)
{
    l_hModelessDialog = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_PEPAPP),
                                            ::GetDesktopWindow(), lModelessDialogProc, 0);

    return (l_hModelessDialog != NULL) ? TRUE : FALSE;
}

BOOL PepAppSplashDialogDestroy()
{
    ::DestroyWindow(l_hModelessDialog);

    return TRUE;
}

VOID PepAppSplashDialogMessagePump()
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

VOID PepAppSplashDialogQuitMessagePump()
{
    if (!l_bQuitMessagePump)
    {
        ::PostMessage(l_hModelessDialog, WM_QUITDIALOG, 0, 0);
    }
}

VOID PepAppSplashDialogDisplayAppAlreadyRunning()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, embdAppAlreadyRunning, 0);
}

VOID PepAppSplashDialogDisplayUnsupportedOS()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, embdUnsupportedOS, 0);
}

VOID PepAppSplashDialogDisplayUnknownError()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, embdUnknownError, 0);
}

VOID PepAppSplashDialogDisplayCommandLineHelp()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, embdCommandLineHelp, 0);
}

VOID PepAppSplashDialogDisplayInstallAlreadyRunning()
{
    ::PostMessage(l_hModelessDialog, WM_DISPLAYMESSAGEBOX, embdInstallAlreadyRunning, 0);
}

VOID PepAppSplashDialogDisplayMessage(
  _In_z_ LPCWSTR pszMessage)
{
    ::SendMessage(l_hModelessDialog, WM_DISPLAYMESSAGE, 0, (LPARAM)pszMessage);

    ::Sleep(CMessageDelayMilliseconds);
}

VOID PepAppSplashDialogExecute(
  _In_ TPepAppSplashDialogExecuteFunc pExecute,
  _In_ LPVOID pvData)
{
	TPepAppSplashDialogExecuteData ExecuteData;

	ExecuteData.pExecute = pExecute;
	ExecuteData.pvData = pvData;
	ExecuteData.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	::PostMessage(l_hModelessDialog, WM_EXECUTE, 0, (LPARAM)&ExecuteData);

	::WaitForSingleObject(ExecuteData.hEvent, INFINITE);

	::CloseHandle(ExecuteData.hEvent);
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
