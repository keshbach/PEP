x/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UtPepAppHostUtility.h"

#pragma region "Local Functions"

static HRESULT lHRESULTFromWaitResult(
  _In_ DWORD dwWaitResult)
{
	switch (dwWaitResult)
	{
		case WAIT_OBJECT_0:
			return S_OK;
		case WAIT_ABANDONED_0:
			return HOST_E_ABANDONED;
		case WAIT_IO_COMPLETION:
			return HOST_E_INTERRUPTED;
		case WAIT_TIMEOUT:
			return HOST_E_TIMEOUT;
		case WAIT_FAILED:
			return HRESULT_FROM_WIN32(::GetLastError());
	}

    return E_FAIL;
}

#pragma endregion

HRESULT UtPepAppHostUtilityWait(
  _In_ HANDLE hWait,
  _In_ DWORD dwMilliseconds,
  _In_ DWORD dwOption)
{
	BOOL bAlertable = dwOption & WAIT_ALERTABLE;
	DWORD dwStatus, dwStartTickCount, dwCurrentTickCount, dwElapseTickCount, dwDelayMilliseconds;
	BOOL bQuit;
	MSG Msg;

	if (dwOption & WAIT_MSGPUMP)
	{
		bQuit = FALSE;
		dwStartTickCount = ::GetTickCount();

		while (!bQuit)
		{
			if (dwMilliseconds == INFINITE)
			{
				dwDelayMilliseconds = INFINITE;
			}
			else
			{
				dwCurrentTickCount = ::GetTickCount();

				if (dwCurrentTickCount >= dwStartTickCount)
				{
					dwElapseTickCount = dwCurrentTickCount - dwStartTickCount;
				}
				else
				{
					dwElapseTickCount = (MAXDWORD - dwStartTickCount) + dwCurrentTickCount;
				}

				if (dwElapseTickCount >= dwMilliseconds)
				{
					return HOST_E_TIMEOUT;
				}

				dwDelayMilliseconds = dwMilliseconds - dwElapseTickCount;
			}

			::SetLastError(ERROR_SUCCESS);

			dwStatus = ::MsgWaitForMultipleObjectsEx(1, &hWait, dwDelayMilliseconds,
				                                     QS_ALLINPUT,
				                                     MWMO_INPUTAVAILABLE | (bAlertable ? MWMO_ALERTABLE : 0));

			if (dwStatus == WAIT_OBJECT_0 + 1)
			{
				while (::PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
				{
					if (Msg.message == WM_QUIT)
					{
						::PostQuitMessage((int)Msg.wParam);

						return S_OK;
					}

					::TranslateMessage(&Msg);
					::DispatchMessage(&Msg);
				}
			}
			else if (dwStatus == WAIT_OBJECT_0 || dwStatus == WAIT_IO_COMPLETION)
			{
				return S_OK;
			}
			else if (dwStatus == WAIT_TIMEOUT)
			{
				return HOST_E_TIMEOUT;
			}
			else if (dwStatus == WAIT_FAILED)
			{
				return HRESULT_FROM_WIN32(::GetLastError());
			}
			else
			{
				bQuit = TRUE;
			}
		}

		return E_FAIL;
	}
	else
	{
		return lHRESULTFromWaitResult(::WaitForSingleObjectEx(hWait, dwMilliseconds, bAlertable));
	}
}

HRESULT UtPepAppHostUtilitySleep(
  _In_ DWORD dwMilliseconds,
  _In_ DWORD dwOption)
{
	BOOL bAlertable = dwOption & WAIT_ALERTABLE;
	DWORD dwStatus, dwStartTickCount, dwCurrentTickCount, dwElapseTickCount, dwDelayMilliseconds;
	BOOL bQuit;
	MSG Msg;

	if (dwOption & WAIT_MSGPUMP)
	{
		bQuit = FALSE;
		dwStartTickCount = ::GetTickCount();

		while (!bQuit)
		{
			if (dwMilliseconds == INFINITE)
			{
				dwDelayMilliseconds = INFINITE;
			}
			else
			{
				dwCurrentTickCount = ::GetTickCount();

				if (dwCurrentTickCount >= dwStartTickCount)
				{
					dwElapseTickCount = dwCurrentTickCount - dwStartTickCount;
				}
				else
				{
					dwElapseTickCount = (MAXDWORD - dwStartTickCount) + dwCurrentTickCount;
				}

				if (dwElapseTickCount >= dwMilliseconds)
				{
					return HOST_E_TIMEOUT;
				}

				dwDelayMilliseconds = dwMilliseconds - dwElapseTickCount;
			}

			::SetLastError(ERROR_SUCCESS);

			dwStatus = ::MsgWaitForMultipleObjectsEx(0, NULL, dwDelayMilliseconds,
				                                     QS_ALLINPUT,
				                                     MWMO_INPUTAVAILABLE | (bAlertable ? MWMO_ALERTABLE : 0));

			if (dwStatus == WAIT_OBJECT_0 + 1)
			{
				while (::PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
				{
					if (Msg.message == WM_QUIT)
					{
						::PostQuitMessage((int)Msg.wParam);

						return S_OK;
					}

					::TranslateMessage(&Msg);
					::DispatchMessage(&Msg);
				}
			}
			else if (dwStatus == WAIT_IO_COMPLETION)
			{
				return S_OK;
			}
			else if (dwStatus == WAIT_TIMEOUT)
			{
				return HOST_E_TIMEOUT;
			}
			else if (dwStatus == WAIT_FAILED)
			{
				return HRESULT_FROM_WIN32(::GetLastError());
			}
			else
			{
				bQuit = TRUE;
			}
		}
	}
	else
	{
		if (bAlertable)
		{
			dwStatus = ::SleepEx(dwMilliseconds, TRUE);

			if (dwStatus == 0)
			{
				return S_OK;
			}
			else if (dwStatus == WAIT_IO_COMPLETION)
			{
				return HOST_E_INTERRUPTED;
			}

			return HRESULT_FROM_WIN32(::GetLastError());
		}
		else
		{
			::Sleep(dwMilliseconds);

			return S_OK;
		}
	}

	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
