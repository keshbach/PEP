/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlReg_H)
#define PepCtrlReg_H

BOOLEAN PepCtrlReadRegSettings(IN PUNICODE_STRING pRegistryPath, OUT PULONG pulPortType, OUT LPWSTR* ppszPortDeviceName);
BOOLEAN PepCtrlWriteRegSettings(IN PUNICODE_STRING pRegistryPath, IN ULONG ulPortType, IN LPWSTR pszPortDeviceName);

#endif /* !defined(PepCtrlReg_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
