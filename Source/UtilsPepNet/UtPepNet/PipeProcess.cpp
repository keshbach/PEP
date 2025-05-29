/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include "IPipeProcess.h"

#include <UtilsPep/UtPipeProcess.h>

#include "PipeProcess.h"

#define CPipeProcessLibrary L"UtPipeProcess.dll"

typedef BOOL (UTPIPEPROCESSAPI* TUtPipeProcessInitializeFunc)(VOID);
typedef BOOL (UTPIPEPROCESSAPI* TUtPipeProcessUninitializeFunc)(VOID);
typedef TUtPipeProcessHandle (UTPIPEPROCESSAPI* TUtPipeProcessLaunchFunc)(LPCWSTR pszApp, LPCWSTR pszArguments, LPVOID pvTagData, const TPipeProcessFuncs* pPipeProcessFuncs);
typedef BOOL (UTPIPEPROCESSAPI* TUtPipeProcessCancelFunc)(TUtPipeProcessHandle hPipeProcess);
typedef BOOL (UTPIPEPROCESSAPI* TUtPipeProcessTerminateFunc)(TUtPipeProcessHandle hPipeProcess);
typedef BOOL (UTPIPEPROCESSAPI* TUtPipeProcessCloseFunc)(TUtPipeProcessHandle hPipeProcess);

typedef struct tagTPipeProcessData
{
	LPVOID pvPipeProcess;
	HMODULE hModule;
	TUtPipeProcessInitializeFunc pPipeProcessInitializeFunc;
	TUtPipeProcessUninitializeFunc pPipeProcessUninitializeFunc;
	TUtPipeProcessLaunchFunc pPipeProcessLaunchFunc;
	TUtPipeProcessCancelFunc pPipeProcessCancelFunc;
	TUtPipeProcessTerminateFunc pPipeProcessTerminateFunc;
	TUtPipeProcessCloseFunc pPipeProcessCloseFunc;
	TUtPipeProcessHandle hPipeProcess;
} TPipeProcessData;

static LPVOID lPipeProcessToPointer(
  Pep::Programmer::IPipeProcess^ PipeProcess)
{
	System::Runtime::InteropServices::GCHandle Handle(System::Runtime::InteropServices::GCHandle::Alloc(PipeProcess));

	return System::Runtime::InteropServices::GCHandle::ToIntPtr(Handle).ToPointer();
}

static Pep::Programmer::IPipeProcess^ lPipeProcessFromPointer(
  LPVOID pvPipeProcess)
{
	System::IntPtr PipeProcessPtr(pvPipeProcess);
	System::Runtime::InteropServices::GCHandle Handle(System::Runtime::InteropServices::GCHandle::FromIntPtr(PipeProcessPtr));
	
	return (Pep::Programmer::IPipeProcess^)Handle.Target;
}

static VOID lFreePipeProcess(
  LPVOID pvPipeProcess)
{
	System::IntPtr PipeProcessPtr(pvPipeProcess);
	System::Runtime::InteropServices::GCHandle Handle(System::Runtime::InteropServices::GCHandle::FromIntPtr(PipeProcessPtr));

	Handle.Free();
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessBeginStatus(
  LPVOID pvTagData)
{
	TPipeProcessData* pPipeProcessData = (TPipeProcessData*)pvTagData;
	Pep::Programmer::IPipeProcess^ PipeProcess = lPipeProcessFromPointer(pPipeProcessData->pvPipeProcess);

	PipeProcess->BeginPipeProcess();
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessEndStatus(
  LPVOID pvTagData)
{
	TPipeProcessData* pPipeProcessData = (TPipeProcessData*)pvTagData;
	Pep::Programmer::IPipeProcess^ PipeProcess = lPipeProcessFromPointer(pPipeProcessData->pvPipeProcess);

	PipeProcess->EndPipeProcess();
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessOutputData(
  LPVOID pvTagData,
  LPCSTR pszData,
  UINT32 nDataLen)
{
	TPipeProcessData* pPipeProcessData = (TPipeProcessData*)pvTagData;
	Pep::Programmer::IPipeProcess^ PipeProcess = lPipeProcessFromPointer(pPipeProcessData->pvPipeProcess);
	array<System::Byte>^ DataBytes = gcnew array<System::Byte>(nDataLen);

	for (UINT32 nIndex = 0; nIndex < nDataLen; ++nIndex)
	{
		DataBytes[nIndex] = pszData[nIndex];
	}

	PipeProcess->OutputDataPipeProcess(DataBytes);
}

static VOID UTPIPEPROCESSCALLBACK lPipeProcessErrorData(
  LPVOID pvTagData,
  LPCSTR pszData,
  UINT32 nDataLen)
{
	TPipeProcessData* pPipeProcessData = (TPipeProcessData*)pvTagData;
	Pep::Programmer::IPipeProcess^ PipeProcess = lPipeProcessFromPointer(pPipeProcessData->pvPipeProcess);
	array<System::Byte>^ DataBytes = gcnew array<System::Byte>(nDataLen);

	for (UINT32 nIndex = 0; nIndex < nDataLen; ++nIndex)
	{
		DataBytes[nIndex] = pszData[nIndex];
	}

	PipeProcess->ErrorDataPipeProcess(DataBytes);
}

Pep::Programmer::PipeProcess::PipeProcess()
{
}

Pep::Programmer::PipeProcess::PipeProcess(
  IPipeProcess^ PipeProcess)
{
	TPipeProcessData* pPipeProcessData;

	m_PipeProcessData = System::IntPtr::Zero;

	try
	{
		m_PipeProcessData = System::Runtime::InteropServices::Marshal::AllocHGlobal(sizeof(TPipeProcessData));

		pPipeProcessData = (TPipeProcessData*)m_PipeProcessData.ToPointer();

		::ZeroMemory(pPipeProcessData, sizeof(TPipeProcessData));

		pPipeProcessData->pvPipeProcess = lPipeProcessToPointer(PipeProcess);
	}
	catch (System::OutOfMemoryException^)
	{
	}
}

Pep::Programmer::PipeProcess::~PipeProcess()
{
	this->!PipeProcess();
}

Pep::Programmer::PipeProcess::!PipeProcess()
{
	this->Close();
}

System::Boolean Pep::Programmer::PipeProcess::CaptureProcessOutput(
  System::String^ sApplication,
  System::String^ sArguments,
  System::String^% sErrorMessage)
{
	pin_ptr<const wchar_t> pszApplication = PtrToStringChars(sApplication);
	pin_ptr<const wchar_t> pszArguments = PtrToStringChars(sArguments);
	TPipeProcessData* pPipeProcessData;
	TPipeProcessFuncs PipeProcessFuncs;

	Common::Debug::Thread::IsUIThread();

	if (m_PipeProcessData == System::IntPtr::Zero)
	{
		sErrorMessage = gcnew System::String(L"Capture process output closed.");

		return false;
	}

	pPipeProcessData = (TPipeProcessData*)m_PipeProcessData.ToPointer();

	if (pPipeProcessData->hPipeProcess != NULL)
	{
		sErrorMessage = gcnew System::String(L"Capture process output already occurring.");

		return false;
	}

	if (pPipeProcessData->hModule == NULL)
	{
		pPipeProcessData->hModule = ::LoadLibrary(CPipeProcessLibrary);

		if (pPipeProcessData->hModule == NULL)
		{
			sErrorMessage = gcnew System::String(L"Could not find the UtPipeProcess.dll file.");

			return false;
		}

		pPipeProcessData->pPipeProcessInitializeFunc = (TUtPipeProcessInitializeFunc)::GetProcAddress(pPipeProcessData->hModule, "UtPipeProcessInitialize");
		pPipeProcessData->pPipeProcessUninitializeFunc = (TUtPipeProcessUninitializeFunc)::GetProcAddress(pPipeProcessData->hModule, "UtPipeProcessUninitialize");
		pPipeProcessData->pPipeProcessLaunchFunc = (TUtPipeProcessLaunchFunc)::GetProcAddress(pPipeProcessData->hModule, "UtPipeProcessLaunch");
		pPipeProcessData->pPipeProcessCancelFunc = (TUtPipeProcessCancelFunc)::GetProcAddress(pPipeProcessData->hModule, "UtPipeProcessCancel");
		pPipeProcessData->pPipeProcessTerminateFunc = (TUtPipeProcessTerminateFunc)::GetProcAddress(pPipeProcessData->hModule, "UtPipeProcessTerminate");
		pPipeProcessData->pPipeProcessCloseFunc = (TUtPipeProcessCloseFunc)::GetProcAddress(pPipeProcessData->hModule, "UtPipeProcessClose");

		if (pPipeProcessData->pPipeProcessInitializeFunc == NULL ||
			pPipeProcessData->pPipeProcessUninitializeFunc == NULL ||
			pPipeProcessData->pPipeProcessLaunchFunc == NULL ||
			pPipeProcessData->pPipeProcessCancelFunc == NULL ||
			pPipeProcessData->pPipeProcessTerminateFunc == NULL ||
			pPipeProcessData->pPipeProcessCloseFunc == NULL)
		{
			::FreeLibrary(pPipeProcessData->hModule);

			pPipeProcessData->hModule = NULL;

			sErrorMessage = gcnew System::String(L"Could not load the UtPipeProcess.dll file.");

			return false;
		}

		if (pPipeProcessData->pPipeProcessInitializeFunc() == FALSE)
		{
			::FreeLibrary(pPipeProcessData->hModule);

			pPipeProcessData->hModule = NULL;

			sErrorMessage = gcnew System::String(L"Could not initialize the UtPipeProcess.dll file.");

			return false;
		}
	}

	PipeProcessFuncs.pBeginStatusFunc = lPipeProcessBeginStatus;
	PipeProcessFuncs.pEndStatusFunc = lPipeProcessEndStatus;
	PipeProcessFuncs.pOutputDataFunc = lPipeProcessOutputData;
	PipeProcessFuncs.pErrorDataFunc = lPipeProcessErrorData;

	pPipeProcessData->hPipeProcess = pPipeProcessData->pPipeProcessLaunchFunc(
											  pszApplication,
                                              pszArguments,
                                              pPipeProcessData,
                                              &PipeProcessFuncs);

	if (pPipeProcessData->hPipeProcess == NULL)
	{
		sErrorMessage = gcnew System::String(L"Out of memory error.");

		return false;
	}

	return true;
}

System::Boolean Pep::Programmer::PipeProcess::StopCapture(
  System::Boolean bForce)
{
	TPipeProcessData* pPipeProcessData;

	Common::Debug::Thread::IsUIThread();

	if (m_PipeProcessData == System::IntPtr::Zero)
	{
		System::Diagnostics::Debug::Assert(false);

		return false;
	}

	pPipeProcessData = (TPipeProcessData*)m_PipeProcessData.ToPointer();

	if (pPipeProcessData->hModule == NULL)
	{
		System::Diagnostics::Debug::Assert(false);

		return false;
	}

	if (pPipeProcessData->hPipeProcess == NULL)
	{
		System::Diagnostics::Debug::Assert(false);

		return false;
	}

	if (!bForce)
	{
		return pPipeProcessData->pPipeProcessCancelFunc(pPipeProcessData->hPipeProcess);
	}

	return pPipeProcessData->pPipeProcessTerminateFunc(pPipeProcessData->hPipeProcess);
}

void Pep::Programmer::PipeProcess::Close()
{
	TPipeProcessData* pPipeProcessData;

	Common::Debug::Thread::IsAnyThread();

	if (m_PipeProcessData == System::IntPtr::Zero)
	{
		return;
	}

	pPipeProcessData = (TPipeProcessData*)m_PipeProcessData.ToPointer();

	if (pPipeProcessData->hPipeProcess)
	{
		pPipeProcessData->pPipeProcessCloseFunc(pPipeProcessData->hPipeProcess);

		pPipeProcessData->hPipeProcess = NULL;
	}

	if (pPipeProcessData->pvPipeProcess)
	{
		lFreePipeProcess(pPipeProcessData->pvPipeProcess);

		pPipeProcessData->pvPipeProcess = NULL;
	}

	if (pPipeProcessData->hModule)
	{
		if (pPipeProcessData->pPipeProcessUninitializeFunc())
		{
			::FreeLibrary(pPipeProcessData->hModule);
		}

		pPipeProcessData->hModule = NULL;
	}

	System::Runtime::InteropServices::Marshal::FreeHGlobal(m_PipeProcessData);

	m_PipeProcessData = System::IntPtr::Zero;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
