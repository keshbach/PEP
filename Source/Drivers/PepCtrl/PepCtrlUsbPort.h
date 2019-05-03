/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlUsbPort_H)
#define PepCtrlUsbPort_H

BOOLEAN TPEPCTRLAPI PepCtrlReadBitUsbPort(_In_ TPepCtrlObject* pObject,
                                          _Out_ PBOOLEAN pbValue);

BOOLEAN TPEPCTRLAPI PepCtrlWriteUsbPort(_In_ TPepCtrlObject* pObject,
                                        _In_ PUCHAR pucData,
                                        _In_ ULONG ulDataLen);

BOOLEAN TPEPCTRLAPI PepCtrlAllocUsbPort(_In_ TPepCtrlObject* pObject,
                                        _In_ LPCWSTR pszDeviceName);

BOOLEAN TPEPCTRLAPI PepCtrlFreeUsbPort(_In_ TPepCtrlObject* pObject);

LPGUID TPEPCTRLAPI PepCtrlGetUsbPortDevInterfaceGuid(VOID);

#endif /* !defined(PepCtrlUsbPort_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
