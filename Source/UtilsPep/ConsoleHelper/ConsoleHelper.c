/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#define STRICT

#include <windows.h>
#include <strsafe.h>

#include <Utils/UtHeapProcess.h>

#include <Includes/UtMacros.h>

#include <UtilsPep/Includes/UtPipeDefs.h>
#include <UtilsPep/Includes/UtPipeFuncs.inl>

#define CProcessSignalledWaitDelayMs 50

#define CPipeSignalledWaitDelayMs 50

#define CPipeCancelledWaitDelayMs 100

#define CConnectPipeWaitDelayMs 500

typedef enum tagEInputPipeState
{
    eipsInitialize = 0,
    eipsCreated,
    eipsDestroyed,
    eipsConnected,
    eipsDisconnected,
    eipsWriting, // data written to pipe
    eipsWritingFailed,
    eipsWritingNoData // no data pending to be wrote
} EInputPipeState;

typedef struct tagPipeOverlappedData {
    OVERLAPPED Overlapped;
    CHAR cData[50];
} TPipeOverlappedData;

typedef struct tagInputPipeData {
    EInputPipeState InputPipeState;
    HANDLE hNamedPipe;
    HANDLE hFilePipe;
    TPipeOverlappedData* pPipeOverlappedData;
} TInputPipeData;

typedef struct tagTLaunchProcessData
{
    PROCESS_INFORMATION ProcessInformation;
    TInputPipeData InputPipeData;
} TLaunchProcessData;

static BOOL WINAPI lHandlerRoutine(
  DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            break;
        case CTRL_CLOSE_EVENT:
            break;
    }

    return TRUE;
}

static BOOL lArgNeedsQuotes(
  LPCWSTR pszArg)
{
    while (*pszArg)
    {
        if (*pszArg == L' ')
        {
            return TRUE;
        }

        ++pszArg;
    }

    return FALSE;
}

static LPWSTR lBuildArgs(
  INT argc,
  LPCWSTR argv[])
{
    INT nArgumentLen = 1;
    LPWSTR pszArgument;
    BOOL bNeedQuotes;

    for (INT nIndex = 1; nIndex < argc; ++nIndex)
    {
        nArgumentLen += lstrlenW(argv[nIndex]) + 1;

        if (lArgNeedsQuotes(argv[nIndex]))
        {
            nArgumentLen += 2;
        }
    }

    ++nArgumentLen;

    pszArgument = (LPWSTR)UtAllocMem(nArgumentLen * sizeof(WCHAR));

    if (pszArgument == NULL)
    {
        return NULL;
    }

    *pszArgument = 0;

    for (INT nIndex = 1; nIndex < argc; ++nIndex)
    {
        bNeedQuotes = lArgNeedsQuotes(argv[nIndex]);

        if (bNeedQuotes)
        {
            StringCchCatW(pszArgument, nArgumentLen, L"\"");
        }

        StringCchCatW(pszArgument, nArgumentLen, argv[nIndex]);

        if (bNeedQuotes)
        {
            StringCchCatW(pszArgument, nArgumentLen, L"\"");
        }

        StringCchCatW(pszArgument, nArgumentLen, L" ");
    }

    return pszArgument;
}

static BOOL lCreateProcess(
  LPCWSTR pszArguments,
  TLaunchProcessData* pLaunchProcessData)
{
    STARTUPINFOW StartupInfo;

    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.lpReserved = NULL;
    StartupInfo.lpDesktop = NULL;
    StartupInfo.lpTitle = NULL;
    StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    StartupInfo.cbReserved2 = 0;
    StartupInfo.lpReserved2 = NULL;
    StartupInfo.hStdInput = pLaunchProcessData->InputPipeData.hFilePipe;
    StartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    StartupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    return CreateProcessW(NULL, (LPWSTR)pszArguments, NULL, NULL, TRUE,
                          CREATE_DEFAULT_ERROR_MODE,
                          NULL, NULL, &StartupInfo,
                          &pLaunchProcessData->ProcessInformation);
}

static TPipeOverlappedData* lAllocPipeOverlappedData(VOID)
{
    TPipeOverlappedData* pPipeOverlappedData = (TPipeOverlappedData*)UtAllocMem(sizeof(TPipeOverlappedData));

    if (pPipeOverlappedData == NULL)
    {
        return NULL;
    }

    pPipeOverlappedData->Overlapped.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    if (pPipeOverlappedData->Overlapped.hEvent == NULL)
    {
        UtFreeMem(pPipeOverlappedData);

        pPipeOverlappedData = NULL;
    }

    return pPipeOverlappedData;
}

static VOID lFreePipeOverlappedData(
  _In_ TPipeOverlappedData* pPipeOverlappedData)
{
    if (pPipeOverlappedData)
    {
        CloseHandle(pPipeOverlappedData->Overlapped.hEvent);

        UtFreeMem(pPipeOverlappedData);
    }
}

static VOID lZeroOverlapped(
  _In_ LPOVERLAPPED pOverlapped)
{
    HANDLE hEvent = pOverlapped->hEvent;

    ZeroMemory(pOverlapped, sizeof(OVERLAPPED));

    pOverlapped->hEvent = hEvent;
}

_Success_(return)
static BOOL lCancelOverlapped(
  _In_ HANDLE hPipe,
  _In_ LPOVERLAPPED pOverlapped,
  _In_ INT nTimeoutMs)
{
    DWORD dwError, dwNumberOfBytesTransferred;

    if (CancelIo(hPipe))
    {
        if (GetOverlappedResultEx(hPipe, pOverlapped, &dwNumberOfBytesTransferred,
                                  nTimeoutMs, TRUE))
        {
            return TRUE;
        }

        dwError = GetLastError();

        switch (dwError)
        {
            case ERROR_OPERATION_ABORTED:
                return TRUE;
            case ERROR_IO_INCOMPLETE:
                break;
        }
    }

    // Need to alloc new overlapped data and event

    return FALSE;
}

static VOID lDestroyPipes(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    HANDLE hHandles[] = {
        pLaunchProcessData->InputPipeData.hNamedPipe,
        pLaunchProcessData->InputPipeData.hFilePipe};

    for (INT nIndex = 0; nIndex < MArrayLen(hHandles); ++nIndex)
    {
        if (hHandles[nIndex] &&
            hHandles[nIndex] != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hHandles[nIndex]);
        }
    }

    pLaunchProcessData->InputPipeData.InputPipeState = eipsDestroyed;
}

static BOOL lCreatePipes(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    WCHAR cPipeName[MAX_PATH];
    SECURITY_ATTRIBUTES SecurityAttributes;

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL;
    SecurityAttributes.bInheritHandle = TRUE;

    // Input pipe

    if (!lCreatePipeName(L"In", cPipeName, MArrayLen(cPipeName)))
    {
        return FALSE;
    }

    pLaunchProcessData->InputPipeData.hNamedPipe = CreateNamedPipeW(cPipeName,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED | WRITE_OWNER,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
        PIPE_UNLIMITED_INSTANCES, 64, 64, 50, &SecurityAttributes);

    if (pLaunchProcessData->InputPipeData.hNamedPipe == INVALID_HANDLE_VALUE)
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    pLaunchProcessData->InputPipeData.hFilePipe = CreateFileW(
        cPipeName, GENERIC_READ, 0,
        &SecurityAttributes, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL);

    if (pLaunchProcessData->InputPipeData.hFilePipe == INVALID_HANDLE_VALUE)
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    return TRUE;
}

static VOID lDisconnectNamedPipes(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    HANDLE hHandles[] = {
        pLaunchProcessData->InputPipeData.hNamedPipe};

    for (INT nIndex = 0; nIndex < MArrayLen(hHandles); ++nIndex)
    {
        if (hHandles[nIndex] &&
            hHandles[nIndex] != INVALID_HANDLE_VALUE)
        {
            DisconnectNamedPipe(hHandles[nIndex]);
        }
    }

    pLaunchProcessData->InputPipeData.InputPipeState = eipsDisconnected;
}

static BOOL lConnectNamedPipes(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    HANDLE hHandles[] = {
        pLaunchProcessData->InputPipeData.hNamedPipe};
    LPOVERLAPPED pOverlapped;

    for (INT nIndex = 0; nIndex < MArrayLen(hHandles); ++nIndex)
    {
        if (hHandles[nIndex] &&
            hHandles[nIndex] != INVALID_HANDLE_VALUE)
        {
            pOverlapped = (LPOVERLAPPED)UtAllocMem(sizeof(OVERLAPPED));

            if (pOverlapped == NULL)
            {
                lDisconnectNamedPipes(pLaunchProcessData);

                return FALSE;
            }

            ZeroMemory(pOverlapped, sizeof(OVERLAPPED));

            pOverlapped->hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

            if (pOverlapped->hEvent == NULL)
            {
                UtFreeMem(pOverlapped);

                lDisconnectNamedPipes(pLaunchProcessData);

                return FALSE;
            }

            if (!ConnectNamedPipe(hHandles[nIndex], pOverlapped))
            {
                switch (GetLastError())
                {
                    case ERROR_PIPE_CONNECTED:
                        // Client connected

                        CloseHandle(pOverlapped->hEvent);

                        UtFreeMem(pOverlapped);

                        continue;
                    default:
                        CloseHandle(pOverlapped->hEvent);

                        UtFreeMem(pOverlapped);

                        lDisconnectNamedPipes(pLaunchProcessData);

                        return FALSE;
                }
            }

            switch (WaitForSingleObject(pOverlapped->hEvent,
                                        CConnectPipeWaitDelayMs))
            {
                case WAIT_OBJECT_0:
                    CloseHandle(pOverlapped->hEvent);

                    UtFreeMem(pOverlapped);
                    break;
                case WAIT_ABANDONED_0:
                    lDisconnectNamedPipes(pLaunchProcessData);

                    return FALSE;
                case WAIT_TIMEOUT:
                    if (CancelIo(hHandles[nIndex]))
                    {
                        CloseHandle(pOverlapped->hEvent);

                        UtFreeMem(pOverlapped);
                    }

                    lDisconnectNamedPipes(pLaunchProcessData);

                    return FALSE;
                case WAIT_FAILED:
                    if (CancelIo(hHandles[nIndex]))
                    {
                        CloseHandle(pOverlapped->hEvent);

                        UtFreeMem(pOverlapped);
                    }

                    lDisconnectNamedPipes(pLaunchProcessData);

                    return FALSE;
            }
        }
    }

    pLaunchProcessData->InputPipeData.InputPipeState = eipsConnected;

    return TRUE;
}

static BOOL lCompareStrings(
  LPCSTR psData1,
  INT nData1Len,
  LPCSTR psData2,
  INT nData2Len)
{
    if (nData1Len != nData2Len)
    {
        return FALSE;
    }

    for (INT nIndex = 0; nIndex < nData1Len; ++nIndex)
    {
        if (psData1[nIndex] != psData2[nIndex])
        {
            return FALSE;
        }
    }

    return TRUE;
}

static void lProcessInputData(
  LPCSTR psData,
  INT nDataLen,
  LPBOOL pbKillProcess)
{
    *pbKillProcess = FALSE;

    if (lCompareStrings(psData, nDataLen,
                        CUtPipeQuitCommand, CUtPipeQuitCommandLen))
    {
        GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
    }
    else if (lCompareStrings(psData, nDataLen,
                             CUtPipeKillCommand, CUtPipeKillCommandLen))
    {
        *pbKillProcess = TRUE;
    }
    else {
        // Pass data to the running process's std input
    }
}

static void lProcessPipeOverlappedData(
  HANDLE hPipe,
  TPipeOverlappedData* pPipeOverlappedData,
  LPBOOL pbKillProcess)
{
    DWORD dwNumberOfBytesTransferred;

    *pbKillProcess = FALSE;

    if (GetOverlappedResultEx(hPipe,
                              &pPipeOverlappedData->Overlapped,
                              &dwNumberOfBytesTransferred,
                              0,
                              TRUE))
    {
        lProcessInputData(pPipeOverlappedData->cData,
                          dwNumberOfBytesTransferred,
                          pbKillProcess);
    }
}

static int lProcessPipeProcess(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    BOOL bQuit = FALSE;
    HANDLE hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    TPipeOverlappedData* pPipeOverlappedData = NULL;
    DWORD dwBytesAvailable, dwExitCode;
    BOOL bKillProcess;

    while (!bQuit)
    {
        switch (WaitForSingleObject(pLaunchProcessData->ProcessInformation.hProcess,
                                    CProcessSignalledWaitDelayMs))
        {
            case WAIT_OBJECT_0:
                bQuit = TRUE;
                break;
            case WAIT_TIMEOUT:
                break;
            case WAIT_FAILED:
                break;
            default:
                break;
        }

        if (pPipeOverlappedData)
        {
            switch (WaitForSingleObject(pPipeOverlappedData->Overlapped.hEvent,
                                        CPipeSignalledWaitDelayMs))
            {
                case WAIT_OBJECT_0:
                    lProcessPipeOverlappedData(hStdInput,
                                               pPipeOverlappedData,
                                               &bKillProcess);

                    lFreePipeOverlappedData(pPipeOverlappedData);

                    pPipeOverlappedData = NULL;

                    if (bKillProcess)
                    {
                        bQuit = TRUE;
                    }
                    break;
                case WAIT_TIMEOUT:
                    break;
                case WAIT_FAILED:
                    break;
                default:
                    break;
            }
        }
        else
        {
            if (!PeekNamedPipe(hStdInput, NULL, 0, NULL, &dwBytesAvailable, NULL))
            {
                continue;
            }

            if (dwBytesAvailable == 0)
            {
                continue;
            }

            pPipeOverlappedData = lAllocPipeOverlappedData();

            if (pPipeOverlappedData == NULL)
            {
                continue;
            }

            if (!ReadFile(hStdInput,
                          pPipeOverlappedData->cData,
                          MArrayLen(pPipeOverlappedData->cData),
                          NULL,
                          &pPipeOverlappedData->Overlapped))
            {
                lFreePipeOverlappedData(pPipeOverlappedData);

                pPipeOverlappedData = NULL;
            }
        }
    }

    if (pPipeOverlappedData)
    {
        if (lCancelOverlapped(hStdInput,
                              &pPipeOverlappedData->Overlapped,
                              CPipeCancelledWaitDelayMs))
        {
            lFreePipeOverlappedData(pPipeOverlappedData);

            pPipeOverlappedData = NULL;
        }
    }

    switch (pLaunchProcessData->InputPipeData.InputPipeState)
    {
        case eipsWriting:
        case eipsWritingFailed:
            if (lCancelOverlapped(pLaunchProcessData->InputPipeData.hNamedPipe,
                                  &pLaunchProcessData->InputPipeData.pPipeOverlappedData->Overlapped,
                                  CPipeCancelledWaitDelayMs))
            {
                lFreePipeOverlappedData(pLaunchProcessData->InputPipeData.pPipeOverlappedData);

                pLaunchProcessData->InputPipeData.pPipeOverlappedData = NULL;
            }
            break;
        case eipsWritingNoData:
            lFreePipeOverlappedData(pLaunchProcessData->InputPipeData.pPipeOverlappedData);
            break;
    }

    if (GetExitCodeProcess(pLaunchProcessData->ProcessInformation.hProcess,
                           &dwExitCode))
    {
        return dwExitCode;
    }

    return -1;
}

int wmain(
  int argc,
  LPWSTR argv[])
{
    int nResult = 0;
    TLaunchProcessData* pLaunchProcessData;
    LPWSTR pszArgument;

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    /*
    BOOL bEndLoop = FALSE;

    while (!bEndLoop)
    {
        Sleep(500);
    }
    */

    if (argc < 2)
    {
        wprintf(L"Application to run is missing.\n");

        return -1;
    }

    pszArgument = lBuildArgs(argc, argv);

    if (pszArgument == NULL)
    {
        wprintf(L"Out of memory (arguments).\n");

        return -1;
    }

    pLaunchProcessData = (TLaunchProcessData*)UtAllocMem(sizeof(TLaunchProcessData));

    if (pLaunchProcessData == NULL)
    {
        UtFreeMem(pszArgument);

        wprintf(L"Out of memory (launch process data).\n");

        return -1;
    }

    ZeroMemory(pLaunchProcessData, sizeof(*pLaunchProcessData));

    pLaunchProcessData->InputPipeData.InputPipeState = eipsInitialize;

    if (!lCreatePipes(pLaunchProcessData))
    {
        UtFreeMem(pLaunchProcessData);
        UtFreeMem(pszArgument);

        wprintf(L"Could not create pipes.\n");

        return -1;
    }

    if (!lConnectNamedPipes(pLaunchProcessData))
    {
        lDestroyPipes(pLaunchProcessData);

        UtFreeMem(pLaunchProcessData);
        UtFreeMem(pszArgument);

        wprintf(L"Could not connect named pipes.\n");

        return -1;
    }

    if (lCreateProcess(pszArgument, pLaunchProcessData))
    {
        nResult = lProcessPipeProcess(pLaunchProcessData);

        CloseHandle(pLaunchProcessData->ProcessInformation.hProcess);
        CloseHandle(pLaunchProcessData->ProcessInformation.hThread);
    }
    else
    {
        wprintf(L"Error launching the application.\n");

        nResult = -1;
    }

    lDestroyPipes(pLaunchProcessData);

    UtFreeMem(pLaunchProcessData);
    UtFreeMem(pszArgument);

    return nResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
