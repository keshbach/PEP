/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Process.h"

#include "resource.h"

#include "UtDll.h"

#include "Includes/UtExtractResource.inl"

typedef struct tagTLaunchProcessData
{
	PROCESS_INFORMATION ProcessInformation;
	HANDLE hStdInputRead;
	HANDLE hStdInputWrite;
	HANDLE hStdOutputRead;
	HANDLE hStdOutputWrite;
	HANDLE hStdErrorRead;
	HANDLE hStdErrorWrite;
	HANDLE hCancelEvent;
	WCHAR cConsoleHelperExe[MAX_PATH];
} TLaunchProcessData;

private ref struct TRunProcessThreadData
{
public:
	Common::Process::IRunProcess^ RunProcess;
};

private ref struct TRunProcessReadPipeThreadData
{
public:
	Common::Process::IRunProcess^ RunProcess;
	System::Boolean bReadStdOut;
};

#pragma unmanaged

static TLaunchProcessData l_LaunchProcessData = { { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 };

static bool lUnmanagedExtractConsoleHelper(void)
{
	::GetTempPathW(sizeof(l_LaunchProcessData.cConsoleHelperExe) / sizeof(l_LaunchProcessData.cConsoleHelperExe[0]),
                   l_LaunchProcessData.cConsoleHelperExe);

	::StringCchCatW(l_LaunchProcessData.cConsoleHelperExe,
		            sizeof(l_LaunchProcessData.cConsoleHelperExe) / sizeof(l_LaunchProcessData.cConsoleHelperExe[0]),
		            L"\\ConsoleHelper.exe");

    return UtExtractResource(UtGetInstance(), RT_EXECUTABLE, IDE_CONSOLEHELPER,
                             l_LaunchProcessData.cConsoleHelperExe) ? true : false;
}

static BOOL lUnmanagedCreateUniqueEvent(
	LPWSTR pszEventName,
	INT nEventNameLen,
	PHANDLE phEvent)
{
	BOOL bNameFound = FALSE;
	GUID guid;
	WCHAR cGuid[50], cEventName[MAX_PATH];
	HANDLE hEvent;

	while (bNameFound == FALSE)
	{
		::CoCreateGuid(&guid);

		::StringFromGUID2(guid, cGuid, sizeof(cGuid) / sizeof(cGuid[0]));

		::StringCchCopyW(cEventName, sizeof(cEventName) / sizeof(cEventName[0]),
			             L"Local\\Event_");

		::StringCchCatW(cEventName, sizeof(cEventName) / sizeof(cEventName[0]),
			            cGuid);

		if (S_OK != ::StringCchCopyW(pszEventName, nEventNameLen, cEventName))
		{
			return FALSE;
		}

		hEvent = ::CreateEventW(NULL, TRUE, FALSE, cEventName);

		if (hEvent)
		{
			if (::GetLastError() != ERROR_ALREADY_EXISTS)
			{
				*phEvent = hEvent;

				bNameFound = TRUE;
			}
			else
			{
				::CloseHandle(hEvent);
			}
		}
	}

	return TRUE;
}

static BOOL lUnmanagedReadPipe(
	HANDLE hPipe,
	LPBYTE pbyData)
{
	DWORD dwBytesRead;

	return ::ReadFile(hPipe, pbyData, 1, &dwBytesRead, NULL);
}

static VOID lUnmanagedWaitForObject(
	HANDLE hObject)
{
	::WaitForSingleObject(hObject, INFINITE);
}

static VOID lUnmanagedCreatePipes(
	TLaunchProcessData* pLaunchProcessData)
{
	SECURITY_ATTRIBUTES SecurityAttributes;

	SecurityAttributes.nLength = sizeof(SecurityAttributes);
	SecurityAttributes.lpSecurityDescriptor = NULL;
	SecurityAttributes.bInheritHandle = TRUE;

	::CreatePipe(&pLaunchProcessData->hStdInputRead,
		         &pLaunchProcessData->hStdInputWrite,
		         &SecurityAttributes, 0);

	::SetHandleInformation(pLaunchProcessData->hStdInputWrite, HANDLE_FLAG_INHERIT, 0);

	::CreatePipe(&pLaunchProcessData->hStdOutputRead,
		         &pLaunchProcessData->hStdOutputWrite,
		         &SecurityAttributes, 0);

	::SetHandleInformation(pLaunchProcessData->hStdOutputRead, HANDLE_FLAG_INHERIT, 0);

	::CreatePipe(&pLaunchProcessData->hStdErrorRead,
		         &pLaunchProcessData->hStdErrorWrite,
		         &SecurityAttributes, 0);

	::SetHandleInformation(pLaunchProcessData->hStdErrorRead, HANDLE_FLAG_INHERIT, 0);
}

static BOOL lUnmanagedCreateProcess(
	LPCWSTR pszArguments,
	HANDLE hStdInput,
	HANDLE hStdOutput,
	HANDLE hStdError,
	LPPROCESS_INFORMATION pProcessInformation)
{
	STARTUPINFOW StartupInfo;

	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.lpReserved = NULL;
	StartupInfo.lpDesktop = NULL;
	StartupInfo.lpTitle = NULL;
	StartupInfo.dwFlags = STARTF_USESTDHANDLES;
	StartupInfo.cbReserved2 = 0;
	StartupInfo.lpReserved2 = NULL;
	StartupInfo.hStdInput = hStdInput;
	StartupInfo.hStdOutput = hStdOutput;
	StartupInfo.hStdError = hStdError;

	return ::CreateProcessW(NULL, (LPWSTR)pszArguments, NULL, NULL, TRUE,
		                    CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP |
		                    CREATE_UNICODE_ENVIRONMENT | CREATE_NO_WINDOW,
		                    NULL, NULL, &StartupInfo, pProcessInformation);
}

#pragma managed

static void lCreateUniqueEvent(
  System::String^% sEventName,
  PHANDLE phEvent)
{
	WCHAR cEventName[MAX_PATH];

	lUnmanagedCreateUniqueEvent(cEventName, sizeof(cEventName) / sizeof(cEventName[0]),
		                        phEvent);

	sEventName = gcnew System::String(cEventName);
}

static void lRunProcessReadPipeThreadFunc(
  System::Object^ object)
{
	TRunProcessReadPipeThreadData^ ReadPipeThreadData = (TRunProcessReadPipeThreadData^)object;
	System::Text::StringBuilder sb;
	BYTE byData;
	HANDLE hPipe;

	if (ReadPipeThreadData->bReadStdOut)
	{
		hPipe = l_LaunchProcessData.hStdOutputRead;
	}
	else
	{
		hPipe = l_LaunchProcessData.hStdErrorRead;
	}

	while (lUnmanagedReadPipe(hPipe, &byData))
	{
		switch (byData)
		{
		    case 0x0D:
			    break;
		    case 0x0A:
			    if (ReadPipeThreadData->bReadStdOut)
			    {
				    ReadPipeThreadData->RunProcess->RunProcessStdOutData(sb.ToString());
			    }
			    else
			    {
				    ReadPipeThreadData->RunProcess->RunProcessStdErrData(sb.ToString());
			    }

			    sb.Length = 0;
			    break;
		    default:
			    sb.Append((System::Char)byData);
			    break;
		}
	}

	if (sb.Length > 0)
	{
		if (ReadPipeThreadData->bReadStdOut)
		{
			ReadPipeThreadData->RunProcess->RunProcessStdOutData(sb.ToString());
		}
		else
		{
			ReadPipeThreadData->RunProcess->RunProcessStdErrData(sb.ToString());
		}
	}
}

static void lRunProcessThreadFunc(
  System::Object^ object)
{
	TRunProcessThreadData^ RunProcessThreadData = (TRunProcessThreadData^)object;
	System::Threading::ParameterizedThreadStart^ ThreadStart;
	System::Threading::Thread^ StdOutThread;
	System::Threading::Thread^ StdErrThread;
	TRunProcessReadPipeThreadData^ StdOutReadPipeThreadData;
	TRunProcessReadPipeThreadData^ StdErrReadPipeThreadData;

	RunProcessThreadData->RunProcess->RunProcessBegin();

	StdOutReadPipeThreadData = gcnew TRunProcessReadPipeThreadData;
	StdErrReadPipeThreadData = gcnew TRunProcessReadPipeThreadData;

	StdOutReadPipeThreadData->RunProcess = RunProcessThreadData->RunProcess;
	StdOutReadPipeThreadData->bReadStdOut = true;

	StdErrReadPipeThreadData->RunProcess = RunProcessThreadData->RunProcess;
	StdErrReadPipeThreadData->bReadStdOut = false;

	ThreadStart = gcnew System::Threading::ParameterizedThreadStart(lRunProcessReadPipeThreadFunc);

	StdOutThread = gcnew System::Threading::Thread(ThreadStart);
	StdErrThread = gcnew System::Threading::Thread(ThreadStart);

	StdOutThread->Name = L"Read Std Output Thread";
	StdErrThread->Name = L"Read Std Error Thread";

	StdOutThread->Start(StdOutReadPipeThreadData);
	StdErrThread->Start(StdErrReadPipeThreadData);

	lUnmanagedWaitForObject(l_LaunchProcessData.ProcessInformation.hProcess);

	::CloseHandle(l_LaunchProcessData.ProcessInformation.hThread);
	::CloseHandle(l_LaunchProcessData.ProcessInformation.hProcess);

	::DeleteFileW(l_LaunchProcessData.cConsoleHelperExe);

	::CloseHandle(l_LaunchProcessData.hStdInputRead);
	::CloseHandle(l_LaunchProcessData.hStdOutputWrite);
	::CloseHandle(l_LaunchProcessData.hStdErrorWrite);

	StdOutThread->Join();
	StdErrThread->Join();

    delete StdOutThread;
    delete StdErrThread;
    delete ThreadStart;
    delete StdOutReadPipeThreadData;
    delete StdErrReadPipeThreadData;

	::CloseHandle(l_LaunchProcessData.hStdInputWrite);
	::CloseHandle(l_LaunchProcessData.hStdOutputRead);
	::CloseHandle(l_LaunchProcessData.hStdErrorRead);

	::CloseHandle(l_LaunchProcessData.hCancelEvent);

	::ZeroMemory(&l_LaunchProcessData, sizeof(l_LaunchProcessData));

	RunProcessThreadData->RunProcess->RunProcessEnd();

    delete RunProcessThreadData;
}

Common::Process::Running::Running()
{
}

Common::Process::Running::Running(
  System::String^ sAppName) :
  m_bAlreadyRunning(false)
{
	System::Boolean bCreatedNew = false;

	m_AppMutex = gcnew System::Threading::Mutex(false, sAppName, bCreatedNew);

	if (bCreatedNew == false)
	{
		m_bAlreadyRunning = true;
	}
}

Common::Process::Running::~Running()
{
#if !defined(NDEBUG)
    if (m_AppMutex != nullptr)
    {
        throw gcnew System::ApplicationException(L"Common::Process::Running() was not destroyed.");
    }
#endif

    Close();
}

void Common::Process::Running::Close()
{
	if (m_AppMutex != nullptr)
	{
		m_AppMutex->Close();

        delete m_AppMutex;

		m_AppMutex = nullptr;
	}
}

System::Boolean Common::Process::LaunchAndCaptureProcessOutput(
	System::String^ sApplication,
	System::String^ sArguments,
	Common::Process::IRunProcess^ RunProcess,
	System::String^% sErrorMsg)
{
	System::Boolean bResult = false;
	pin_ptr<const wchar_t> pszNewArguments;
	TRunProcessThreadData^ RunProcessThreadData;
	System::Threading::ParameterizedThreadStart^ ThreadStart;
	System::Threading::Thread^ RunProcessThread;
	System::String^ sEventName;
	System::Text::StringBuilder sb;

	sErrorMsg = L"";

	if (l_LaunchProcessData.ProcessInformation.hProcess)
	{
		sErrorMsg = L"An application is already being launched.";

		return false;
	}

	if (false == lUnmanagedExtractConsoleHelper())
	{
		sErrorMsg = L"The ConsoleHelper application could not be extracted.";

		return false;
	}

	lCreateUniqueEvent(sEventName, &l_LaunchProcessData.hCancelEvent);

	sb.Append(L"\"");
	sb.Append(gcnew System::String(l_LaunchProcessData.cConsoleHelperExe));
	sb.Append(L"\" \"");
	sb.Append(sEventName);
	sb.Append(L"\" \"");
	sb.Append(sApplication);
	sb.Append(L"\" ");
	sb.Append(sArguments);

    delete sEventName;

    sEventName = nullptr;

	pszNewArguments = PtrToStringChars(sb.ToString());

	lUnmanagedCreatePipes(&l_LaunchProcessData);

	::ZeroMemory(&l_LaunchProcessData.ProcessInformation,
		         sizeof(l_LaunchProcessData.ProcessInformation));

	if (lUnmanagedCreateProcess(pszNewArguments, l_LaunchProcessData.hStdInputRead,
	                            l_LaunchProcessData.hStdOutputWrite,
		                        l_LaunchProcessData.hStdErrorWrite,
		                        &l_LaunchProcessData.ProcessInformation))
	{
		RunProcessThreadData = gcnew TRunProcessThreadData;

		RunProcessThreadData->RunProcess = RunProcess;

		ThreadStart = gcnew System::Threading::ParameterizedThreadStart(lRunProcessThreadFunc);

		RunProcessThread = gcnew System::Threading::Thread(ThreadStart);

		RunProcessThread->Name = L"Run Process Thread";

		RunProcessThread->Start(RunProcessThreadData);

		bResult = true;
	}
	else
	{
		::CloseHandle(l_LaunchProcessData.hStdInputRead);
		::CloseHandle(l_LaunchProcessData.hStdInputWrite);
		::CloseHandle(l_LaunchProcessData.hStdOutputRead);
		::CloseHandle(l_LaunchProcessData.hStdOutputWrite);
		::CloseHandle(l_LaunchProcessData.hStdErrorRead);
		::CloseHandle(l_LaunchProcessData.hStdErrorWrite);
		::CloseHandle(l_LaunchProcessData.hCancelEvent);

		::ZeroMemory(&l_LaunchProcessData, sizeof(l_LaunchProcessData));

		sErrorMsg = L"The process could not be launched.";
	}

	return bResult;
}

System::Boolean Common::Process::StopCaptureAppOutput()
{
	if (l_LaunchProcessData.ProcessInformation.hProcess == NULL)
	{
		return false;
	}

	::SetEvent(l_LaunchProcessData.hCancelEvent);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
