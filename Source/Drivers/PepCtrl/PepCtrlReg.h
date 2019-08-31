/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlReg_H)
#define PepCtrlReg_H

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlReadRegSettings(_In_ PUNICODE_STRING pRegistryPath, _Out_ PULONG pulPortType, _Out_ LPWSTR* ppszPortDeviceName);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlWriteRegSettings(_In_ PUNICODE_STRING pRegistryPath, _In_ ULONG ulPortType, _In_ LPWSTR pszPortDeviceName);

#endif /* !defined(PepCtrlReg_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
