/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlUsbPort_H)
#define PepCtrlUsbPort_H

BOOLEAN TPEPCTRLAPI PepCtrlReadBitUsbPort(IN TPepCtrlObject* pObject,
                                          OUT PBOOLEAN pbValue);

BOOLEAN TPEPCTRLAPI PepCtrlWriteUsbPort(IN TPepCtrlObject* pObject,
                                        IN PUCHAR pucData,
                                        IN ULONG ulDataLen);

BOOLEAN TPEPCTRLAPI PepCtrlAllocUsbPort(IN TPepCtrlObject* pObject,
                                        IN LPCWSTR pszDeviceName);

BOOLEAN TPEPCTRLAPI PepCtrlFreeUsbPort(IN TPepCtrlObject* pObject);

LPGUID TPEPCTRLAPI PepCtrlGetUsbPortDevInterfaceGuid(VOID);

#endif /* !defined(PepCtrlUsbPort_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
