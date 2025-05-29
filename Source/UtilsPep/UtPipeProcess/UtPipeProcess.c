/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/

#define STRICT

#include <windows.h>
#include <strsafe.h>

#include <UtilsPep/UtPipeProcess.h>

#include <Utils/UtHeap.h>

#include <Includes/UtMacros.h>
#include <Includes/UtExtractResource.inl>
#include <Includes/UtThread.inl>

#include "UtPipeProcessUtil.h"

#include "resource.h"

#include <UtilsPep/Includes/UtPipeDefs.h>
#include <UtilsPep/Includes/UtPipeFuncs.inl>

#define CProcessSignalledWaitDelayMs 50

#define CPipeSignalledWaitDelayMs 50

#define CPipeCancelledWaitDelayMs 100

#define CConnectPipeWaitDelayMs 500

typedef enum tagEOutputPipeState
{
    eopsInitialize = 0,
    eopsCreated,
    eopsDestroyed,
    eopsConnected,
    eopsDisconnected,
    eopsPeeking, // Waiting for data to read
    eopsReading,
    eopsReadingFailed,
    eopsReadingNoData,
    eopsPeekingError
} EOutputPipeState;

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
    CHAR cData[100];
} TPipeOverlappedData;

typedef struct tagOutputPipeData {
    EOutputPipeState OutputPipeState;
    HANDLE hNamedPipe;
    HANDLE hFilePipe;
    TPipeOverlappedData* pPipeOverlappedData;
} TOutputPipeData;

typedef struct tagInputPipeData {
    EInputPipeState InputPipeState;
    HANDLE hNamedPipe;
    HANDLE hFilePipe;
    TPipeOverlappedData* pPipeOverlappedData;
} TInputPipeData;

typedef struct tagTLaunchProcessData
{
    LPVOID pvTagData;
    TPipeProcessFuncs PipeProcessFuncs;
    PROCESS_INFORMATION ProcessInformation;
    WCHAR cConsoleHelperExe[MAX_PATH];
    TOutputPipeData OutputPipeData;
    TOutputPipeData ErrorPipeData;
    TInputPipeData InputPipeData;
    HANDLE hThread;
    HANDLE hKillEvent;
} TLaunchProcessData;

#pragma region "Local Functions"

static BOOL lExtractConsoleHelper(
  TLaunchProcessData* pLaunchProcessData)
{
    WCHAR cGuid[50];
    GUID guid;

    if (GetTempPathW(MArrayLen(pLaunchProcessData->cConsoleHelperExe),
                     pLaunchProcessData->cConsoleHelperExe) &&
        S_OK == CoCreateGuid(&guid) &&
        StringFromGUID2(&guid, cGuid, MArrayLen(cGuid)) &&
        S_OK == StringCchCatW(pLaunchProcessData->cConsoleHelperExe,
                              MArrayLen(pLaunchProcessData->cConsoleHelperExe),
                              L"\\ConsoleHelper") &&
        S_OK == StringCchCatW(pLaunchProcessData->cConsoleHelperExe,
                              MArrayLen(pLaunchProcessData->cConsoleHelperExe),
                              cGuid) &&
        S_OK == StringCchCatW(pLaunchProcessData->cConsoleHelperExe,
                              MArrayLen(pLaunchProcessData->cConsoleHelperExe),
                              L".exe"))
    {
        return UtExtractResource(UtPipeProcessGetInstance(), RT_EXECUTABLE, IDE_CONSOLEHELPER,
                                 pLaunchProcessData->cConsoleHelperExe);
    }

    return FALSE;
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
    StartupInfo.hStdOutput = pLaunchProcessData->OutputPipeData.hFilePipe;
    StartupInfo.hStdError = pLaunchProcessData->ErrorPipeData.hFilePipe;

    return CreateProcessW(NULL, (LPWSTR)pszArguments, NULL, NULL, TRUE,
                          CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP |
                          CREATE_UNICODE_ENVIRONMENT | CREATE_NO_WINDOW,
                          NULL, NULL, &StartupInfo,
                          &pLaunchProcessData->ProcessInformation);
}

static TPipeOverlappedData* lAllocPipeOverlappedData(VOID)
{
    TPipeOverlappedData* pOverlappedData = (TPipeOverlappedData*)UtAllocMem(sizeof(TPipeOverlappedData));

    if (pOverlappedData == NULL)
    {
        return NULL;
    }

    pOverlappedData->Overlapped.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    if (pOverlappedData->Overlapped.hEvent == NULL)
    {
        UtFreeMem(pOverlappedData);

        pOverlappedData = NULL;
    }

    return pOverlappedData;
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

static void lFreeLaunchProcessDataOverlappedData(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    // Output pipe

    switch (pLaunchProcessData->OutputPipeData.OutputPipeState)
    {
        case eopsInitialize:
        case eopsCreated:
        case eopsDestroyed:
        case eopsConnected:
        case eopsDisconnected:
        case eopsPeeking:
        case eopsReading:
        case eopsReadingNoData:
        case eopsPeekingError:
            lFreePipeOverlappedData(pLaunchProcessData->OutputPipeData.pPipeOverlappedData);
            break;
        case eopsReadingFailed:
            if (lCancelOverlapped(pLaunchProcessData->OutputPipeData.hNamedPipe,
                                  &pLaunchProcessData->OutputPipeData.pPipeOverlappedData->Overlapped,
                                  CPipeCancelledWaitDelayMs))
            {
                lFreePipeOverlappedData(pLaunchProcessData->OutputPipeData.pPipeOverlappedData);
            }
            break;
    }

    // Error pipe

    switch (pLaunchProcessData->ErrorPipeData.OutputPipeState)
    {
        case eopsInitialize:
        case eopsCreated:
        case eopsDestroyed:
        case eopsConnected:
        case eopsDisconnected:
        case eopsPeeking:
        case eopsReading:
        case eopsReadingNoData:
        case eopsPeekingError:
            lFreePipeOverlappedData(pLaunchProcessData->ErrorPipeData.pPipeOverlappedData);
            break;
        case eopsReadingFailed:
            if (lCancelOverlapped(pLaunchProcessData->ErrorPipeData.hNamedPipe,
                                  &pLaunchProcessData->ErrorPipeData.pPipeOverlappedData->Overlapped,
                                  CPipeCancelledWaitDelayMs))
            {
                lFreePipeOverlappedData(pLaunchProcessData->ErrorPipeData.pPipeOverlappedData);
            }
            break;
    }

    // Input pipe

    switch (pLaunchProcessData->InputPipeData.InputPipeState)
    {
        case eipsInitialize:
        case eipsCreated:
        case eipsDestroyed:
        case eipsConnected:
        case eipsDisconnected:
        case eipsWritingNoData:
            lFreePipeOverlappedData(pLaunchProcessData->InputPipeData.pPipeOverlappedData);
            break;
        case eipsWriting:
        case eipsWritingFailed:
            if (lCancelOverlapped(pLaunchProcessData->InputPipeData.hNamedPipe,
                                  &pLaunchProcessData->InputPipeData.pPipeOverlappedData->Overlapped,
                                  CPipeCancelledWaitDelayMs))
            {
                lFreePipeOverlappedData(pLaunchProcessData->InputPipeData.pPipeOverlappedData);
            }
            break;
    }
}

static VOID lDestroyPipes(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    HANDLE hHandles[] = {
        pLaunchProcessData->InputPipeData.hNamedPipe,
        pLaunchProcessData->InputPipeData.hFilePipe,
        pLaunchProcessData->OutputPipeData.hNamedPipe,
        pLaunchProcessData->OutputPipeData.hFilePipe,
        pLaunchProcessData->ErrorPipeData.hNamedPipe,
        pLaunchProcessData->ErrorPipeData.hFilePipe};

    for (INT nIndex = 0; nIndex < MArrayLen(hHandles); ++nIndex)
    {
        if (hHandles[nIndex] &&
            hHandles[nIndex] != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hHandles[nIndex]);
        }
    }

    pLaunchProcessData->OutputPipeData.OutputPipeState = eopsDestroyed;
    pLaunchProcessData->ErrorPipeData.OutputPipeState = eopsDestroyed;
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

    // Output pipe

    if (!lCreatePipeName(L"Out", cPipeName, MArrayLen(cPipeName)))
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    pLaunchProcessData->OutputPipeData.hNamedPipe = CreateNamedPipeW(cPipeName,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED | WRITE_OWNER,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
        PIPE_UNLIMITED_INSTANCES, 64, 64, 50, &SecurityAttributes);

    if (pLaunchProcessData->OutputPipeData.hNamedPipe == INVALID_HANDLE_VALUE)
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    pLaunchProcessData->OutputPipeData.hFilePipe = CreateFileW(
        cPipeName, GENERIC_WRITE, 0,
        &SecurityAttributes, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL);

    if (pLaunchProcessData->OutputPipeData.hFilePipe == INVALID_HANDLE_VALUE)
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    pLaunchProcessData->OutputPipeData.OutputPipeState = eopsCreated;

    // Error pipe

    if (!lCreatePipeName(L"Err", cPipeName, MArrayLen(cPipeName)))
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    pLaunchProcessData->ErrorPipeData.hNamedPipe = CreateNamedPipeW(cPipeName,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED | WRITE_OWNER,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
        PIPE_UNLIMITED_INSTANCES, 64, 64, 50, &SecurityAttributes);

    if (pLaunchProcessData->ErrorPipeData.hNamedPipe == INVALID_HANDLE_VALUE)
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    pLaunchProcessData->ErrorPipeData.hFilePipe = CreateFileW(
        cPipeName, GENERIC_WRITE, 0,
        &SecurityAttributes, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL);

    if (pLaunchProcessData->ErrorPipeData.hFilePipe == INVALID_HANDLE_VALUE)
    {
        lDestroyPipes(pLaunchProcessData);

        return FALSE;
    }

    pLaunchProcessData->ErrorPipeData.OutputPipeState = eopsInitialize;

    return TRUE;
}

static VOID lDisconnectNamedPipes(
    _In_ TLaunchProcessData* pLaunchProcessData)
{
    HANDLE hHandles[] = {
        pLaunchProcessData->InputPipeData.hNamedPipe,
        pLaunchProcessData->OutputPipeData.hNamedPipe,
        pLaunchProcessData->ErrorPipeData.hNamedPipe};

    for (INT nIndex = 0; nIndex < MArrayLen(hHandles); ++nIndex)
    {
        if (hHandles[nIndex] &&
            hHandles[nIndex] != INVALID_HANDLE_VALUE)
        {
            DisconnectNamedPipe(hHandles[nIndex]);
        }
    }

    pLaunchProcessData->OutputPipeData.OutputPipeState = eopsDisconnected;
    pLaunchProcessData->ErrorPipeData.OutputPipeState = eopsDisconnected;
    pLaunchProcessData->InputPipeData.InputPipeState = eipsDisconnected;
}

static BOOL lConnectNamedPipes(
  _In_ TLaunchProcessData* pLaunchProcessData)
{
    HANDLE hHandles[] = {
        pLaunchProcessData->InputPipeData.hNamedPipe,
        pLaunchProcessData->OutputPipeData.hNamedPipe,
        pLaunchProcessData->ErrorPipeData.hNamedPipe};
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

    pLaunchProcessData->OutputPipeData.OutputPipeState = eopsConnected;
    pLaunchProcessData->ErrorPipeData.OutputPipeState = eopsConnected;
    pLaunchProcessData->InputPipeData.InputPipeState = eipsConnected;

    return TRUE;
}

static void lQueueReadPipe(
  TOutputPipeData* pOutputPipeData)
{
    if (ReadFile(pOutputPipeData->hNamedPipe,
                 pOutputPipeData->pPipeOverlappedData->cData,
                 MArrayLen(pOutputPipeData->pPipeOverlappedData->cData),
                 NULL,
                 &pOutputPipeData->pPipeOverlappedData->Overlapped))
    {
        pOutputPipeData->OutputPipeState = eopsReading;

        return;
    }

    switch (GetLastError())
    {
        case ERROR_IO_PENDING:
            pOutputPipeData->OutputPipeState = eopsReading;
            break;
        case ERROR_NO_DATA:
            pOutputPipeData->OutputPipeState = eopsReadingNoData;
            break;
        default:
            pOutputPipeData->OutputPipeState = eopsReadingFailed;
            break;
    }
}

static void lReadPipeData(
  TOutputPipeData* pOutputPipeData,
  TUtPipeProcessDataFunc pPipeProcessDataFunc,
  LPVOID pvTagData)
{
    DWORD dwNumberOfBytesTransferred;

    if (GetOverlappedResult(pOutputPipeData->hNamedPipe,
                            &pOutputPipeData->pPipeOverlappedData->Overlapped,
                            &dwNumberOfBytesTransferred,
                            TRUE))
    {
        pPipeProcessDataFunc(pvTagData,
                             pOutputPipeData->pPipeOverlappedData->cData,
                             dwNumberOfBytesTransferred);

        lZeroOverlapped(&pOutputPipeData->pPipeOverlappedData->Overlapped);

        ResetEvent(pOutputPipeData->pPipeOverlappedData->Overlapped.hEvent);

        lQueueReadPipe(pOutputPipeData);
    }
    else
    {
        ResetEvent(pOutputPipeData->pPipeOverlappedData->Overlapped.hEvent);

        pOutputPipeData->OutputPipeState = eopsReadingFailed;
    }
}

static VOID lProcessPipeData(
  TOutputPipeData* pOutputPipeData,
  TUtPipeProcessDataFunc pPipeProcessDataFunc,
  LPVOID pvTagData)
{
    DWORD dwBytesAvailable;

    switch (pOutputPipeData->OutputPipeState)
    {
        case eopsPeeking:
            if (PeekNamedPipe(pOutputPipeData->hNamedPipe,
                              NULL, 0, NULL, &dwBytesAvailable, NULL))
            {
                if (dwBytesAvailable > 0)
                {
                    lQueueReadPipe(pOutputPipeData);
                }
            }
            else
            {
                pOutputPipeData->OutputPipeState = eopsPeekingError;
            }
            break;
        case eopsReading:
            switch (WaitForSingleObject(pOutputPipeData->pPipeOverlappedData->Overlapped.hEvent,
                                        CPipeSignalledWaitDelayMs))
            {
                case WAIT_OBJECT_0:
                    lReadPipeData(pOutputPipeData, pPipeProcessDataFunc, pvTagData);
                    break;
                case WAIT_TIMEOUT:
                    break;
                case WAIT_FAILED:
                    pOutputPipeData->OutputPipeState = eopsReadingFailed;
                    break;
            }
            break;
        case eopsReadingFailed:
            break;
        case eopsReadingNoData:
            pOutputPipeData->OutputPipeState = eopsPeeking;
            break;
        case eopsPeekingError:
            break;
        default:
            break;
    }
}

static BOOL lIsPipeReadingData(
  TOutputPipeData* pOutputPipeData)
{
    BOOL bResult = FALSE;

    switch (pOutputPipeData->OutputPipeState)
    {
        case eopsPeeking:
            break;
        case eopsReading:
            bResult = TRUE;
            break;
        case eopsReadingFailed:
        case eopsReadingNoData:
        case eopsPeekingError:
        default:
            break;
    }

    return bResult;
}

static BOOL lSendProcessPipeCommand(
  TInputPipeData* pInputPipeData,
  LPCSTR pszCommand,
  INT nCommandLen)
{
    BOOL bResult = FALSE;

    switch (pInputPipeData->InputPipeState)
    {
        case eipsConnected:
        case eipsWritingNoData:
            break;
        default:
            return FALSE;
    }

    StringCchCopyA(pInputPipeData->pPipeOverlappedData->cData,
                   MArrayLen(pInputPipeData->pPipeOverlappedData->cData),
                   pszCommand);

    ResetEvent(pInputPipeData->pPipeOverlappedData->Overlapped.hEvent);

    if (WriteFile(pInputPipeData->hNamedPipe,
                  pInputPipeData->pPipeOverlappedData->cData,
                  nCommandLen,
                  NULL,
                  &pInputPipeData->pPipeOverlappedData->Overlapped))
    {
        pInputPipeData->InputPipeState = eipsWriting;
    }
    else
    {
        pInputPipeData->InputPipeState = eipsWritingFailed;

        return FALSE;
    }

    switch (WaitForSingleObject(pInputPipeData->pPipeOverlappedData->Overlapped.hEvent,
                                CPipeSignalledWaitDelayMs))
    {
        case WAIT_OBJECT_0:
            pInputPipeData->InputPipeState = eipsWritingNoData;

            bResult = TRUE;
            break;
        case WAIT_ABANDONED_0:
            break;
        case WAIT_TIMEOUT:
            if (lCancelOverlapped(pInputPipeData->hNamedPipe,
                                  &pInputPipeData->pPipeOverlappedData->Overlapped,
                                  CPipeCancelledWaitDelayMs))
            {
                pInputPipeData->InputPipeState = eipsWritingNoData;
            }
            else
            {
                pInputPipeData->InputPipeState = eipsWritingFailed;
            }
            break;
        case WAIT_FAILED:
            break;
        default:
            break;
    }

    return bResult;
}

static DWORD WINAPI lProcessPipeProcessThread(
  _In_ LPVOID pvParameter)
{
    TLaunchProcessData* pLaunchProcessData = (TLaunchProcessData*)pvParameter;
    BOOL bQuit, bProcessRunning;
    HANDLE hHandles[2];

    if (pLaunchProcessData->OutputPipeData.OutputPipeState == eopsConnected)
    {
        pLaunchProcessData->OutputPipeData.OutputPipeState = eopsPeeking;
    }

    if (pLaunchProcessData->ErrorPipeData.OutputPipeState == eopsConnected)
    {
        pLaunchProcessData->ErrorPipeData.OutputPipeState = eopsPeeking;
    }

    pLaunchProcessData->PipeProcessFuncs.pBeginStatusFunc(pLaunchProcessData->pvTagData);

    bQuit = FALSE;
    bProcessRunning = TRUE;

    hHandles[0] = pLaunchProcessData->ProcessInformation.hProcess;
    hHandles[1] = pLaunchProcessData->hKillEvent;

    while (!bQuit)
    {
        lProcessPipeData(&pLaunchProcessData->OutputPipeData,
                         pLaunchProcessData->PipeProcessFuncs.pOutputDataFunc,
                         pLaunchProcessData->pvTagData);
        lProcessPipeData(&pLaunchProcessData->ErrorPipeData,
                         pLaunchProcessData->PipeProcessFuncs.pErrorDataFunc,
                         pLaunchProcessData->pvTagData);

        if (bProcessRunning)
        {
            switch (WaitForMultipleObjects(MArrayLen(hHandles),
                                           hHandles,
                                           FALSE,
                                           CProcessSignalledWaitDelayMs))
            {
                case WAIT_OBJECT_0:
                    bProcessRunning = FALSE;
                    break;
                case WAIT_OBJECT_0 + 1:
                    bQuit = TRUE;
                    break;
                case WAIT_TIMEOUT:
                    break;
                case WAIT_FAILED:
                    bQuit = TRUE;
                    break;
                default:
                    break;
            }
        }
        else
        {
            if (!lIsPipeReadingData(&pLaunchProcessData->OutputPipeData) &&
                !lIsPipeReadingData(&pLaunchProcessData->ErrorPipeData))
            {
                bQuit = TRUE;
            }
            else
            {
                Sleep(CProcessSignalledWaitDelayMs);
            }
        }
    }

    pLaunchProcessData->PipeProcessFuncs.pEndStatusFunc(pLaunchProcessData->pvTagData);

    return 0;
}

#pragma endregion

BOOL UTPIPEPROCESSAPI UtPipeProcessInitialize(VOID)
{
	return UtInitHeap();
}

BOOL UTPIPEPROCESSAPI UtPipeProcessUninitialize(VOID)
{
	return UtUninitHeap();
}

TUtPipeProcessHandle UTPIPEPROCESSAPI UtPipeProcessLaunch(
  LPCWSTR pszApp,
  LPCWSTR pszArguments,
  LPVOID pvTagData,
  const TPipeProcessFuncs* pPipeProcessFuncs)
{
    TLaunchProcessData* pLaunchProcessData;
    LPWSTR pszNewArguments;
    INT nNewArgumentsLen;
    DWORD dwThreadId;

    if (pszApp == NULL ||
        pszArguments == NULL ||
        pPipeProcessFuncs == NULL ||
        pPipeProcessFuncs->pBeginStatusFunc == NULL ||
        pPipeProcessFuncs->pEndStatusFunc == NULL ||
        pPipeProcessFuncs->pOutputDataFunc == NULL ||
        pPipeProcessFuncs->pErrorDataFunc == NULL)
    {
        return NULL;
    }

    pLaunchProcessData = (TLaunchProcessData*)UtAllocMem(sizeof(TLaunchProcessData));

    if (pLaunchProcessData == NULL)
    {
        return NULL;
    }

    ZeroMemory(pLaunchProcessData, sizeof(pLaunchProcessData));

    if (!lExtractConsoleHelper(pLaunchProcessData))
    {
        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    nNewArgumentsLen = lstrlenW(pLaunchProcessData->cConsoleHelperExe) +
                       lstrlenW(pszApp) +
                       lstrlenW(pszArguments) + 10;

    pszNewArguments = (LPWSTR)UtAllocMem(nNewArgumentsLen * sizeof(WCHAR));

    if (pszNewArguments == NULL)
    {
        DeleteFileW(pLaunchProcessData->cConsoleHelperExe);

        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    StringCchCopyW(pszNewArguments, nNewArgumentsLen, L"\"");
    StringCchCatW(pszNewArguments, nNewArgumentsLen, pLaunchProcessData->cConsoleHelperExe);
    StringCchCatW(pszNewArguments, nNewArgumentsLen, L"\" \"");
    StringCchCatW(pszNewArguments, nNewArgumentsLen, pszApp);
    StringCchCatW(pszNewArguments, nNewArgumentsLen, L"\" ");
    StringCchCatW(pszNewArguments, nNewArgumentsLen, pszArguments);

    CopyMemory(&pLaunchProcessData->PipeProcessFuncs, pPipeProcessFuncs, sizeof(*pPipeProcessFuncs));

    pLaunchProcessData->pvTagData = pvTagData;
    pLaunchProcessData->OutputPipeData.OutputPipeState = eopsInitialize;
    pLaunchProcessData->ErrorPipeData.OutputPipeState = eopsInitialize;
    pLaunchProcessData->InputPipeData.InputPipeState = eipsInitialize;

    pLaunchProcessData->OutputPipeData.pPipeOverlappedData = lAllocPipeOverlappedData();
    pLaunchProcessData->ErrorPipeData.pPipeOverlappedData = lAllocPipeOverlappedData();
    pLaunchProcessData->InputPipeData.pPipeOverlappedData = lAllocPipeOverlappedData();

    if (pLaunchProcessData->OutputPipeData.pPipeOverlappedData == NULL ||
        pLaunchProcessData->ErrorPipeData.pPipeOverlappedData == NULL ||
        pLaunchProcessData->InputPipeData.pPipeOverlappedData == NULL)
    {
        lFreeLaunchProcessDataOverlappedData(pLaunchProcessData);

        UtFreeMem(pszNewArguments);

        DeleteFileW(pLaunchProcessData->cConsoleHelperExe);

        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    pLaunchProcessData->hKillEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    if (pLaunchProcessData->hKillEvent == NULL)
    {
        lFreeLaunchProcessDataOverlappedData(pLaunchProcessData);

        UtFreeMem(pszNewArguments);

        DeleteFileW(pLaunchProcessData->cConsoleHelperExe);

        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    if (!lCreatePipes(pLaunchProcessData))
    {
        CloseHandle(pLaunchProcessData->hKillEvent);

        lFreeLaunchProcessDataOverlappedData(pLaunchProcessData);

        UtFreeMem(pszNewArguments);

        DeleteFileW(pLaunchProcessData->cConsoleHelperExe);

        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    if (!lConnectNamedPipes(pLaunchProcessData))
    {
        lDestroyPipes(pLaunchProcessData);

        CloseHandle(pLaunchProcessData->hKillEvent);

        lFreeLaunchProcessDataOverlappedData(pLaunchProcessData);

        UtFreeMem(pszNewArguments);

        DeleteFileW(pLaunchProcessData->cConsoleHelperExe);

        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    if (!lCreateProcess(pszNewArguments, pLaunchProcessData))
    {
        lDisconnectNamedPipes(pLaunchProcessData);

        lDestroyPipes(pLaunchProcessData);

        CloseHandle(pLaunchProcessData->hKillEvent);

        lFreeLaunchProcessDataOverlappedData(pLaunchProcessData);

        UtFreeMem(pszNewArguments);

        DeleteFileW(pLaunchProcessData->cConsoleHelperExe);

        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    UtFreeMem(pszNewArguments);

    pLaunchProcessData->hThread = CreateThread(NULL,
                                               0,
                                               lProcessPipeProcessThread,
                                               pLaunchProcessData,
                                               0,
                                               &dwThreadId);

    if (pLaunchProcessData->hThread == NULL)
    {
        lDisconnectNamedPipes(pLaunchProcessData);

        lDestroyPipes(pLaunchProcessData);

        CloseHandle(pLaunchProcessData->hKillEvent);

        lFreeLaunchProcessDataOverlappedData(pLaunchProcessData);

        DeleteFileW(pLaunchProcessData->cConsoleHelperExe);

        UtFreeMem(pLaunchProcessData);

        return NULL;
    }

    UtSetThreadName(dwThreadId, "Pipe Process");

    return pLaunchProcessData;
}

BOOL UTPIPEPROCESSAPI UtPipeProcessPostData(
  TUtPipeProcessHandle hPipeProcess,
  LPCSTR pszData,
  UINT32 nDataLen)
{
    TLaunchProcessData* pLaunchProcessData = (TLaunchProcessData*)hPipeProcess;

    if (hPipeProcess == NULL)
    {
        return FALSE;
    }

    pLaunchProcessData;
    pszData;
    nDataLen;

    // Remember to escape the : character

    return FALSE;
}

BOOL UTPIPEPROCESSAPI UtPipeProcessCancel(
  TUtPipeProcessHandle hPipeProcess)
{
    TLaunchProcessData* pLaunchProcessData = (TLaunchProcessData*)hPipeProcess;

    if (hPipeProcess == NULL)
    {
        return FALSE;
    }

    return lSendProcessPipeCommand(&pLaunchProcessData->InputPipeData,
                                   CUtPipeQuitCommand,
                                   CUtPipeQuitCommandLen);
}

BOOL UTPIPEPROCESSAPI UtPipeProcessTerminate(
  TUtPipeProcessHandle hPipeProcess)
{
    TLaunchProcessData* pLaunchProcessData = (TLaunchProcessData*)hPipeProcess;
    BOOL bResult;

    if (hPipeProcess == NULL)
    {
        return FALSE;
    }

    bResult = lSendProcessPipeCommand(&pLaunchProcessData->InputPipeData,
                                      CUtPipeKillCommand,
                                      CUtPipeKillCommandLen);

    if (bResult)
    {
        SetEvent(pLaunchProcessData->hKillEvent);
    }

    return bResult;
}

BOOL UTPIPEPROCESSAPI UtPipeProcessClose(
  TUtPipeProcessHandle hPipeProcess)
{
    TLaunchProcessData* pLaunchProcessData = (TLaunchProcessData*)hPipeProcess;

    if (hPipeProcess == NULL)
    {
        return FALSE;
    }

    switch (WaitForSingleObject(pLaunchProcessData->hThread,
                                INFINITE))
    {
        case WAIT_OBJECT_0:
            break;
        case WAIT_ABANDONED_0:
            break;
        case WAIT_TIMEOUT:
            break;
        case WAIT_FAILED:
            break;
        default:
            break;
    }

    CloseHandle(pLaunchProcessData->ProcessInformation.hThread);
    CloseHandle(pLaunchProcessData->ProcessInformation.hProcess);

    lFreeLaunchProcessDataOverlappedData(pLaunchProcessData);

    lDisconnectNamedPipes(pLaunchProcessData);

    lDestroyPipes(pLaunchProcessData);

    CloseHandle(pLaunchProcessData->hThread);
    CloseHandle(pLaunchProcessData->hKillEvent);

    DeleteFile(pLaunchProcessData->cConsoleHelperExe);

    UtFreeMem(pLaunchProcessData);

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/
