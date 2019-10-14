/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepAppSplashWindow.h"
#include "UtPepApp.h"

#include <Utils/UtHeapProcess.h>

#include <Includes/UtVersion.h>

#include <assert.h>

#pragma region Constants

#define CSplashWindowClassName L"SplashWindow"

#define CSplashWindowWidth 500
#define CSplashWindowHeight 300

#define WM_QUITSPLASH (WM_USER + 1)
#define WM_DISPLAYMESSAGEBOX (WM_USER + 2)
#define WM_EXECUTE (WM_USER + 3)

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
	embdPluginsLoadFailed
};

#pragma endregion

#pragma region "Structures"

typedef struct tagTPepAppSplashWindowData 
{
    HBITMAP hBitmap;
} TPepAppSplashWindowData;

typedef struct tagTPepAppSplashWindowExecuteData
{
	TPepAppSplashWindowExecuteFunc pExecute;
	LPVOID pvData;
	HANDLE hEvent;
} TPepAppSplashWindowExecuteData;

#pragma endregion

#pragma region Local Variables

static HWND l_hSplashWindow = NULL;

#pragma endregion

#pragma region Message Handlers

static LRESULT lHandlePaintMessage(
  _In_ HWND hWnd)
{
	PAINTSTRUCT PaintStruct;

	::BeginPaint(hWnd, &PaintStruct);
	
	::EndPaint(hWnd, &PaintStruct);

	return 0;
}

static LRESULT lHandleEraseBackgroundMessage(
  _In_ HWND hWnd,
  _In_ HDC hDC)
{
	TPepAppSplashWindowData* pData = (TPepAppSplashWindowData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	HDC hMemDC;

	if (pData)
	{
        hMemDC = ::CreateCompatibleDC(hDC);

        ::SaveDC(hMemDC);

        ::SelectObject(hMemDC, pData->hBitmap);

        ::BitBlt(hDC, 0, 0, CSplashWindowWidth, CSplashWindowHeight, hMemDC, 0, 0, SRCCOPY);

        ::RestoreDC(hMemDC, -1);

        ::DeleteDC(hMemDC);
	}

	return 1;
}

static LRESULT lHandleQuitSplash(
  _In_ HWND hWnd)
{
	hWnd;

	::PostQuitMessage(0);

	return 0;
}

static LRESULT lHandleDisplayMessageBoxMessage(
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
		case embdPluginsLoadFailed:
			nMessageId = IDS_PLUGINSLOADFAILED;
			break;
        default:
			assert(0);
            break;
    }

    pszMessage = UtPepAppAllocString(nMessageId);

    ::MessageBoxW(hWnd, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

    UtPepAppFreeString(pszAppTitle);
    UtPepAppFreeString(pszMessage);

	::PostQuitMessage(0);

    return 0;
}

static LRESULT lHandleExecuteMessage(
  _In_ TPepAppSplashWindowExecuteData* pExecuteData)
{
	pExecuteData->pExecute(pExecuteData->pvData);

	::SetEvent(pExecuteData->hEvent);

	return 0;
}

static LRESULT lHandleCreateMessage(
  _In_ HWND hWnd,
  _In_ LPCREATESTRUCT pCreateStruct)
{
	TPepAppSplashWindowData* pData = (TPepAppSplashWindowData*)UtAllocMem(sizeof(TPepAppSplashWindowData));

	pCreateStruct;

	if (pData != NULL)
	{
		pData->hBitmap = ::LoadBitmap((HINSTANCE)::GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                                      MAKEINTRESOURCE(IDB_SPLASH));

		if (pData->hBitmap == NULL)
		{
			UtFreeMem(pData);

			pData = NULL;
		}
	}

	::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

	return 0;
}

static LRESULT lHandleDestroyMessage(
  _In_ HWND hWnd)
{
	TPepAppSplashWindowData* pData = (TPepAppSplashWindowData*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pData)
	{
		::DeleteObject(pData->hBitmap);

		UtFreeMem(pData);
	}

	return 0;
}

#pragma endregion

#pragma region Window Callback

static LRESULT CALLBACK lWindowProc(
  _In_ HWND hWnd,
  _In_ UINT uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam)
{
	switch (uMsg)
	{
        case WM_PAINT:
			return lHandlePaintMessage(hWnd);
        case WM_ERASEBKGND:
			return lHandleEraseBackgroundMessage(hWnd, (HDC)wParam);
		case WM_QUITSPLASH:
			return lHandleQuitSplash(hWnd);
        case WM_DISPLAYMESSAGEBOX:
            return lHandleDisplayMessageBoxMessage(hWnd, (EMessageBoxDescription)wParam);
        case WM_EXECUTE:
            return lHandleExecuteMessage((TPepAppSplashWindowExecuteData*)lParam);
		case WM_CREATE:
			return lHandleCreateMessage(hWnd, (LPCREATESTRUCT)lParam);
		case WM_DESTROY:
			return lHandleDestroyMessage(hWnd);
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#pragma endregion

#pragma region Public Functions

BOOL PepAppSplashWindowCreate(
  _In_ HINSTANCE hInstance)
{
	WNDCLASSEXW WndClassEx;
	RECT Rect;

	::GetClientRect(::GetDesktopWindow(), &Rect);

	WndClassEx.cbSize = sizeof(WndClassEx);
	WndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
	WndClassEx.lpfnWndProc= lWindowProc;
	WndClassEx.cbClsExtra = 0;
	WndClassEx.cbWndExtra = sizeof(TPepAppSplashWindowData*);
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = NULL;
	WndClassEx.hCursor = ::LoadCursor(NULL, IDC_WAIT);
	WndClassEx.hbrBackground = NULL;
	WndClassEx.lpszMenuName = NULL;
	WndClassEx.lpszClassName = CSplashWindowClassName;
	WndClassEx.hIconSm = NULL;

	::RegisterClassExW(&WndClassEx);
		
	// How to create a top level window that does not show up in the task bar
	//
	// https://docs.microsoft.com/en-us/windows/win32/shell/taskbar

	l_hSplashWindow = ::CreateWindowExW(WS_EX_TOOLWINDOW, CSplashWindowClassName, NULL,
	                                    WS_POPUP | WS_VISIBLE,
                                        (Rect.right - CSplashWindowWidth) / 2,
                                        (Rect.bottom - CSplashWindowHeight) / 2,
		                                CSplashWindowWidth, CSplashWindowHeight,
		                                ::GetDesktopWindow(),
                                        NULL, hInstance, NULL);

    return (l_hSplashWindow != NULL) ? TRUE : FALSE;
}

BOOL PepAppSplashWindowDestroy(
  _In_ HINSTANCE hInstance)
{
    ::DestroyWindow(l_hSplashWindow);

	::UnregisterClassW(CSplashWindowClassName, hInstance);

	return TRUE;
}

VOID PepAppSplashWindowMessagePump()
{
	BOOL bQuit = FALSE;
	BOOL bResult;
    MSG Message;

	while (!bQuit)
	{
		bResult = ::GetMessage(&Message, NULL, 0, 0);

		if (bResult == TRUE)
		{
			::TranslateMessage(&Message);
    		::DispatchMessage(&Message);
		}
		else if (bResult == FALSE)
		{
			bQuit = TRUE;
		}
		else if (bResult == -1)
		{
			// Error returned
		}
	}
}

VOID PepAppSplashWindowQuitMessagePump()
{
	::PostMessage(l_hSplashWindow, WM_QUITSPLASH, 0, 0);
}

VOID PepAppSplashWindowDisplayAppAlreadyRunning()
{
    ::PostMessage(l_hSplashWindow, WM_DISPLAYMESSAGEBOX, embdAppAlreadyRunning, 0);
}

VOID PepAppSplashWindowDisplayUnsupportedOS()
{
    ::PostMessage(l_hSplashWindow, WM_DISPLAYMESSAGEBOX, embdUnsupportedOS, 0);
}

VOID PepAppSplashWindowDisplayUnknownError()
{
    ::PostMessage(l_hSplashWindow, WM_DISPLAYMESSAGEBOX, embdUnknownError, 0);
}

VOID PepAppSplashWindowDisplayCommandLineHelp()
{
    ::PostMessage(l_hSplashWindow, WM_DISPLAYMESSAGEBOX, embdCommandLineHelp, 0);
}

VOID PepAppSplashWindowDisplayPluginsLoadFailed()
{
	::PostMessage(l_hSplashWindow, WM_DISPLAYMESSAGEBOX, embdPluginsLoadFailed, 0);
}

VOID PepAppSplashWindowExecute(
  _In_ TPepAppSplashWindowExecuteFunc pExecute,
  _In_ LPVOID pvData)
{
	TPepAppSplashWindowExecuteData ExecuteData;

	ExecuteData.pExecute = pExecute;
	ExecuteData.pvData = pvData;
	ExecuteData.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	::PostMessage(l_hSplashWindow, WM_EXECUTE, 0, (LPARAM)&ExecuteData);

	::WaitForSingleObject(ExecuteData.hEvent, INFINITE);

	::CloseHandle(ExecuteData.hEvent);
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
