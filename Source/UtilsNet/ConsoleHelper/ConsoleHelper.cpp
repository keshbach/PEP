/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

static BOOL WINAPI lHandlerRoutine(
  DWORD dwCtrlType)
{
    dwCtrlType;

    return TRUE;
}

int _tmain(
  int argc,
  _TCHAR* argv[])
{
    int nResult = 0;
    BOOL bQuit = FALSE;
    INT nArgumentLen = 1;
    LPWSTR pszArgument;
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    HANDLE hCancelEvent;
    HANDLE hHandles[2];

    ::SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    if (argc <= 2)
    {
        _tprintf(TEXT("Application to run is missing.\n"));

        return -1;
    }

    hCancelEvent = ::CreateEvent(NULL, TRUE, FALSE, argv[1]);

    if (::GetLastError() != ERROR_ALREADY_EXISTS)
    {
        ::CloseHandle(hCancelEvent);

        _tprintf(TEXT("Cancel event not found.\n"));

        return -1;
    }

    for (int nIndex = 2; nIndex < argc; ++nIndex)
    {
        nArgumentLen += ::lstrlen(argv[nIndex]) + 3;
    }

    ++nArgumentLen;

    pszArgument = (LPWSTR)::HeapAlloc(::GetProcessHeap(), 0, nArgumentLen * sizeof(WCHAR));

    *pszArgument = 0;

    for (int nIndex = 2; nIndex < argc; ++nIndex)
    {
        ::StringCchCatW(pszArgument, nArgumentLen, L"\"");
        ::StringCchCatW(pszArgument, nArgumentLen, argv[nIndex]);
        ::StringCchCatW(pszArgument, nArgumentLen, L"\" ");
    }

    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.lpReserved = NULL;
    StartupInfo.lpDesktop = NULL;
    StartupInfo.lpTitle = NULL;
    StartupInfo.dwFlags = 0;
    StartupInfo.cbReserved2 = 0;
    StartupInfo.lpReserved2 = NULL;

    if (::CreateProcessW(NULL, pszArgument, NULL, NULL, TRUE,
                         CREATE_DEFAULT_ERROR_MODE,
                         NULL, NULL, &StartupInfo, &ProcessInformation))
    {
        hHandles[0] = hCancelEvent;
        hHandles[1] = ProcessInformation.hProcess;

        while (bQuit == FALSE)
        {
            switch (::WaitForMultipleObjects(sizeof(hHandles) / sizeof(hHandles[0]),
                                             hHandles, FALSE, INFINITE))
            {
                case WAIT_OBJECT_0:
                    ::ResetEvent(hCancelEvent);
                    ::GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
                    break;
                case WAIT_OBJECT_0 + 1:
                    bQuit = TRUE;
                    break;
                default:
                    _tprintf(TEXT("Unknown wait object return code.\n"));
                    break;
            }   
        }

        ::CloseHandle(ProcessInformation.hProcess);
        ::CloseHandle(ProcessInformation.hThread);
    }
    else
    {
        _tprintf(TEXT("Error launching the application.\n"));

        nResult = 1;
    }

    ::CloseHandle(hCancelEvent);

    ::HeapFree(::GetProcessHeap(), 0, pszArgument);

	return nResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
