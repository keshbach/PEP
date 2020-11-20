/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <new>

#include "PepAppHostTask.h"

#include "Includes/UtMacros.h"

#pragma region "Constants"

#define CMaxPepAppHostTaskData 20

#pragma endregion

#pragma region "Structures"

typedef struct tagTPepAppTaskData
{
	DWORD dwThreadId;
	PepAppHostTask* pPepAppHostTask;
} TPepAppHostTaskData;

#pragma endregion

#pragma region "Local Variables"

static TPepAppHostTaskData l_PepAppHostTaskData[CMaxPepAppHostTaskData] = {0};
static INT l_nPepAppHostTaskDataLen = 0;

static CRITICAL_SECTION l_CriticalSection = {NULL};

#pragma endregion

BOOL UtPepAppHostTasksInitialize()
{
	::InitializeCriticalSection(&l_CriticalSection);

	return TRUE;
}

BOOL UtPepAppHostTasksUninitialize()
{
	::DeleteCriticalSection(&l_CriticalSection);

	return TRUE;
}

BOOL UtPepAppHostTasksCreate(
  DWORD dwThreadId,
  PepAppHostTask** ppHostTask)
{
	BOOL bResult = FALSE;
	TPepAppHostTaskData* pPepAppHostTaskData;

	::EnterCriticalSection(&l_CriticalSection);

	if (l_nPepAppHostTaskDataLen + 1 > MArrayLen(l_PepAppHostTaskData))
	{
		::LeaveCriticalSection(&l_CriticalSection);

		return FALSE;
	}

	pPepAppHostTaskData = &l_PepAppHostTaskData[l_nPepAppHostTaskDataLen];

	pPepAppHostTaskData->dwThreadId = dwThreadId;
	pPepAppHostTaskData->pPepAppHostTask = new (std::nothrow) PepAppHostTask(dwThreadId);

	if (pPepAppHostTaskData->pPepAppHostTask != NULL)
	{
		++l_nPepAppHostTaskDataLen;

		*ppHostTask = pPepAppHostTaskData->pPepAppHostTask;

		bResult = TRUE;
	}

	::LeaveCriticalSection(&l_CriticalSection);

	return bResult;
}

BOOL UtPepAppHostTasksCreate(
  DWORD dwStackSize,
  LPTHREAD_START_ROUTINE pStartAddress,
  PVOID pvParameter,
  IHostTask** ppHostTask)
{
	TPepAppHostTaskData* pPepAppHostTaskData;

	::EnterCriticalSection(&l_CriticalSection);

	if (l_nPepAppHostTaskDataLen + 1 > MArrayLen(l_PepAppHostTaskData))
	{
		::LeaveCriticalSection(&l_CriticalSection);

		return FALSE;
	}

	pPepAppHostTaskData = &l_PepAppHostTaskData[l_nPepAppHostTaskDataLen];

	pPepAppHostTaskData->pPepAppHostTask = new (std::nothrow) PepAppHostTask(dwStackSize, pStartAddress, pvParameter);

	if (pPepAppHostTaskData->pPepAppHostTask == NULL)
	{
		::LeaveCriticalSection(&l_CriticalSection);

		return FALSE;
	}

	if (pPepAppHostTaskData->pPepAppHostTask->GetThreadId() == 0)
	{
		delete pPepAppHostTaskData->pPepAppHostTask;

		::LeaveCriticalSection(&l_CriticalSection);

		return FALSE;
	}

	pPepAppHostTaskData->dwThreadId = pPepAppHostTaskData->pPepAppHostTask->GetThreadId();

	++l_nPepAppHostTaskDataLen;

	*ppHostTask = pPepAppHostTaskData->pPepAppHostTask;

	::LeaveCriticalSection(&l_CriticalSection);

	return TRUE;
}

BOOL UtPepAppHostTasksDestroy(
  DWORD dwThreadId)
{
	::EnterCriticalSection(&l_CriticalSection);

	for (INT nIndex = 0; nIndex < l_nPepAppHostTaskDataLen; ++nIndex)
	{
		if (l_PepAppHostTaskData[nIndex].dwThreadId == dwThreadId)
		{
			::MoveMemory(&l_PepAppHostTaskData[nIndex],
                                     &l_PepAppHostTaskData[nIndex + 1],
                                     (l_nPepAppHostTaskDataLen - (nIndex + 1)) * sizeof(TPepAppHostTaskData));

#if !defined(NDEBUG)
			::ZeroMemory(&l_PepAppHostTaskData[l_nPepAppHostTaskDataLen - 1],
                         sizeof(TPepAppHostTaskData));
#endif

			--l_nPepAppHostTaskDataLen;

			::LeaveCriticalSection(&l_CriticalSection);

			return TRUE;
		}
	}

	::LeaveCriticalSection(&l_CriticalSection);

	return FALSE;
}

BOOL UtPepAppHostTasksFind(
  DWORD dwThreadId,
  PepAppHostTask** ppHostTask)
{
	*ppHostTask = NULL;

	::EnterCriticalSection(&l_CriticalSection);

	for (INT nIndex = 0; nIndex < l_nPepAppHostTaskDataLen; ++nIndex)
	{
		if (l_PepAppHostTaskData[nIndex].dwThreadId == dwThreadId)
		{
			*ppHostTask = l_PepAppHostTaskData[nIndex].pPepAppHostTask;

			(*ppHostTask)->AddRef();

			::LeaveCriticalSection(&l_CriticalSection);

			return TRUE;
		}
	}

	::LeaveCriticalSection(&l_CriticalSection);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
