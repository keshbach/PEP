/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlInit_H)
#define PepCtrlInit_H

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlInitPortData(_In_ PDRIVER_OBJECT pDriverObject, _In_ PDEVICE_OBJECT pDeviceObject, _In_ PUNICODE_STRING pRegistryPath, _In_ TPepCtrlPortData* pPortData);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlUninitPortData(_In_ TPepCtrlPortData* pPortData);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlInitPortTypeFuncs(_In_ TPepCtrlPortData* pPortData);

#endif /* !defined(PepCtrlInit_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
