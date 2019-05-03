/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlInit_H)
#define PepCtrlInit_H

BOOLEAN PepCtrlInitPortData(_In_ PUNICODE_STRING pRegistryPath, _In_ TPepCtrlPortData* pPortData);

VOID PepCtrlUninitPortData(_In_ TPepCtrlPortData* pPortData);

VOID PepCtrlInitPortTypeFuncs(_In_ TPepCtrlPortData* pPortData);

#endif /* !defined(PepCtrlInit_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
