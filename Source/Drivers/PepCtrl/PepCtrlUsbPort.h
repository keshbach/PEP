/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlUsbPort_H)
#define PepCtrlUsbPort_H

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlReadBitUsbPort(_In_ TPepCtrlObject* pObject,
                                          _Out_ PBOOLEAN pbValue);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlWriteUsbPort(_In_ TPepCtrlObject* pObject,
                                        _In_ PUCHAR pucData,
                                        _In_ ULONG ulDataLen,
	                                    _In_ ULONG ulWaitNanoSeconds);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlAllocUsbPort(_In_ TPepCtrlObject* pObject,
                                        _In_ LPCWSTR pszDeviceName);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlFreeUsbPort(_In_ TPepCtrlObject* pObject);

_IRQL_requires_max_(PASSIVE_LEVEL)
LPGUID TPEPCTRLAPI PepCtrlGetUsbPortDevInterfaceGuid(VOID);

#endif /* !defined(PepCtrlUsbPort_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
