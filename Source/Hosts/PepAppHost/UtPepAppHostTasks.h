/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

class PepAppHostTask;

BOOL UtPepAppHostTasksInitialize();
BOOL UtPepAppHostTasksUninitialize();

BOOL UtPepAppHostTasksCreate(DWORD dwThreadId, PepAppHostTask** ppHostTask);

BOOL UtPepAppHostTasksCreate(DWORD dwStackSize, LPTHREAD_START_ROUTINE pStartAddress, PVOID pvParameter, IHostTask** ppHostTask);

BOOL UtPepAppHostTasksDestroy(DWORD dwThreadId);

BOOL UtPepAppHostTasksFind(DWORD dwThreadId, PepAppHostTask** ppHostTask);

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
