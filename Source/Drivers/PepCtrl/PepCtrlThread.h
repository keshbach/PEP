/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlThread_H)
#define PepCtrlThread_H

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlThreadStart(_In_ PKSTART_ROUTINE pStartRoutine, _In_ PVOID pvContext, _Out_ PHANDLE phThread);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlThreadStop(_In_ HANDLE hThread);

#endif /* !defined(PepCtrlThread_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
