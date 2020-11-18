/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <new>

#include "PepAppHostTask.h"

#pragma region "Structures"

typedef struct tagTPepAppTaskData
{
	DWORD dwThreadId;
	PepAppHostTask* pPepAppHostTask;
} TPepAppHostTaskData;

#pragma endregion

#pragma region "Local Variables"

static TPepAppHostTaskData l_PepAppHostTaskData[50] = {0};
static INT l_nPepAppHostTaskDataLen = 0;

#pragma endregion

BOOL UtPepAppHostTasksInitialize()
{
	return TRUE;
}

BOOL UtPepAppHostTasksUninitialize()
{
	return TRUE;
}

BOOL UtPepAppHostTasksCreate(
  DWORD dwThreadId,
  PepAppHostTask** ppHostTask)
{
	TPepAppHostTaskData* pPepAppHostTaskData = &l_PepAppHostTaskData[l_nPepAppHostTaskDataLen];

	pPepAppHostTaskData->dwThreadId = dwThreadId;
	pPepAppHostTaskData->pPepAppHostTask = new (std::nothrow) PepAppHostTask(dwThreadId);

	if (pPepAppHostTaskData->pPepAppHostTask == NULL)
	{
		return FALSE;
	}

	++l_nPepAppHostTaskDataLen;

	*ppHostTask = pPepAppHostTaskData->pPepAppHostTask;

	return TRUE;
}

BOOL UtPepAppHostTasksCreate(
  DWORD dwStackSize,
  LPTHREAD_START_ROUTINE pStartAddress,
  PVOID pvParameter,
  IHostTask** ppHostTask)
{
	TPepAppHostTaskData* pPepAppHostTaskData = &l_PepAppHostTaskData[l_nPepAppHostTaskDataLen];

	pPepAppHostTaskData->pPepAppHostTask = new (std::nothrow) PepAppHostTask(dwStackSize, pStartAddress, pvParameter);

	if (pPepAppHostTaskData->pPepAppHostTask == NULL)
	{
		return FALSE;
	}

	if (pPepAppHostTaskData->pPepAppHostTask->GetThreadId() == 0)
	{
		delete pPepAppHostTaskData->pPepAppHostTask;

		return FALSE;
	}

	pPepAppHostTaskData->dwThreadId = pPepAppHostTaskData->pPepAppHostTask->GetThreadId();

	++l_nPepAppHostTaskDataLen;

	*ppHostTask = pPepAppHostTaskData->pPepAppHostTask;

	return TRUE;
}

BOOL UtPepAppHostTasksDestroy(
  DWORD dwThreadId)
{
	for (INT nIndex = 0; nIndex < l_nPepAppHostTaskDataLen; ++nIndex)
	{
		if (l_PepAppHostTaskData[nIndex].dwThreadId == dwThreadId)
		{
			::MoveMemory(&l_PepAppHostTaskData[nIndex],
                                     &l_PepAppHostTaskData[nIndex + 1],
                                     (l_nPepAppHostTaskDataLen - (nIndex + 1)) * sizeof(TPepAppHostTaskData));

			--l_nPepAppHostTaskDataLen;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL UtPepAppHostTasksFind(
  DWORD dwThreadId,
  PepAppHostTask** ppHostTask)
{
	*ppHostTask = NULL;

	for (INT nIndex = 0; nIndex < l_nPepAppHostTaskDataLen; ++nIndex)
	{
		if (l_PepAppHostTaskData[nIndex].dwThreadId == dwThreadId)
		{
			*ppHostTask = l_PepAppHostTaskData[nIndex].pPepAppHostTask;

			return TRUE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
