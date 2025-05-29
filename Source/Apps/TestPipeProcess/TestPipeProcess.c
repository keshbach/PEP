/***************************************************************************/
/*  Copyright (C) 2019-2025 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <UtilsPep/UtPipeProcess.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

#include <Apps/Includes/UtCommandLineParser.inl>

#include <assert.h>

static	TUtPipeProcessHandle l_hPipeProcess = NULL;

static BOOL WINAPI lHandlerRoutine(
  DWORD dwCtrlType)
{
	wprintf(L"Console ctrl handler invoked.\n");

	switch (dwCtrlType)
	{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
			wprintf(L"Attempting to cancel the pipe process.\n");

			if (UtPipeProcessCancel(l_hPipeProcess))
			{
				wprintf(L"Cancelled the pipe process.\n");
			}
			else
			{
				wprintf(L"Failed to cancel the pipe process.\n");
			}
			break;
		case CTRL_CLOSE_EVENT:
			wprintf(L"Attempting to terminate the pipe process.\n");

			if (UtPipeProcessTerminate(l_hPipeProcess))
			{
				wprintf(L"Terminated the pipe process.\n");
			}
			else
			{
				wprintf(L"Failed to terminate the pipe process.\n");
			}
			break;
	}
	
	return TRUE;
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessBeginStatus(
  LPVOID pvTagData)
{
	pvTagData;

	wprintf(L"**** Pipe Process Begin Status Callback ****\n");
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessEndStatus(
  LPVOID pvTagData)
{
	pvTagData;

	wprintf(L"**** Pipe Process End Status Callback ****\n");
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessOutputData(
  LPVOID pvTagData,
  LPCSTR pszData,
  UINT32 nDataLen)
{
	pvTagData;

	for (UINT32 nIndex = 0; nIndex < nDataLen; ++nIndex)
	{
		wprintf(L"%c", pszData[nIndex]);
	}
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessErrorData(
  LPVOID pvTagData,
  LPCSTR pszData,
  UINT32 nDataLen)
{
	pvTagData;

	for (UINT32 nIndex = 0; nIndex < nDataLen; ++nIndex)
	{
		wprintf(L"%c", pszData[nIndex]);
	}
}

static int lTestPipeProcess(
  LPCWSTR pszApp,
  LPCWSTR pszArgs)
{
	TPipeProcessFuncs PipeProcessFuncs;

	PipeProcessFuncs.pBeginStatusFunc = lPipeProcessBeginStatus;
	PipeProcessFuncs.pEndStatusFunc = lPipeProcessEndStatus;
	PipeProcessFuncs.pOutputDataFunc = lPipeProcessOutputData;
	PipeProcessFuncs.pErrorDataFunc = lPipeProcessErrorData;

	wprintf(L"Attempting to initialize the library.\n");

	if (FALSE == UtPipeProcessInitialize())
	{
		wprintf(L"Failed to initialize the library.\n");

		return -1;
	}

	wprintf(L"Successfully initialized the library.\n");

	wprintf(L"Attempting to launch a pipe process.\n");

	l_hPipeProcess = UtPipeProcessLaunch(pszApp,
		                                 pszArgs,
		                                 NULL,
		                                 &PipeProcessFuncs);

	if (l_hPipeProcess != NULL)
	{
		wprintf(L"Pipe process launched.\n");

		wprintf(L"Attempting to close the pipe process.\n");

		if (UtPipeProcessClose(l_hPipeProcess))
		{
			wprintf(L"Pipe process closed.\n");
		}
		else
		{
			wprintf(L"Failed to close the pipe process.\n");
		}

		l_hPipeProcess = NULL;
	}
	else
	{
		wprintf(L"Failed to launch the pipe process.\n");
	}

	wprintf(L"Attempting to uninitialize the library.\n");

	if (FALSE == UtPipeProcessUninitialize())
	{
		wprintf(L"Failed to uninitialize the library.\n");

		return -1;
	}

	wprintf(L"Successfully uninitialized the library.\n");

	return 0;
}

static int lDisplayHelp(void)
{
	wprintf(L"\n");

	UtConsolePrintAppVersion();

	wprintf(L"\n");
	wprintf(L"TestPipeProcess [\"Application File\"] [\"Arguments\"]\n");
	wprintf(L"\n");
	wprintf(L"    \"Application File\" - Name of the application\n");
	wprintf(L"    \"Arguments\"        - Application arguments\n");
	wprintf(L"\n");

	return -1;
}

int __cdecl wmain(int argc, WCHAR* argv[])
{
	int nResult, nTotalNewArgs;
	LPCWSTR* ppszNewArgs;

	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

	UtCommandLineParserInitialize(argc, argv);

	nTotalNewArgs = UtCommandLineParserGetTotalArguments();
	ppszNewArgs = UtCommandLineParserGetArguments();

	if (nTotalNewArgs == 3)
	{
		nResult = lTestPipeProcess(ppszNewArgs[1], ppszNewArgs[2]);
	}
	else
	{
		nResult = lDisplayHelp();
	}

	UtCommandLineParserUninitialize();

	SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

	return nResult;
}

/***************************************************************************/
/*  Copyright (C) 2019-2025 Kevin Eshbach                                  */
/***************************************************************************/
