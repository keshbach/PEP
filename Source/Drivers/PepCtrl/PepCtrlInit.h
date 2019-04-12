/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlInit_H)
#define PepCtrlInit_H

BOOLEAN PepCtrlInitPortData(IN PUNICODE_STRING pRegistryPath, IN TPepCtrlPortData* pPortData);

VOID PepCtrlUninitPortData(IN TPepCtrlPortData* pPortData);

VOID PepCtrlInitPortTypeFuncs(IN TPepCtrlPortData* pPortData);

#endif /* !defined(PepCtrlInit_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
