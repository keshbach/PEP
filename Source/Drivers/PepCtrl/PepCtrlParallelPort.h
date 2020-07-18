/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlParallelPort_H)
#define PepCtrlParallelPort_H

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlReadBitParallelPort(_In_ TPepCtrlObject* pObject,
                                               _Out_ PBOOLEAN pbValue);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlWriteParallelPort(_In_ TPepCtrlObject* pObject,
                                             _In_ PUCHAR pucData,
                                             _In_ ULONG ulDataLen,
	                                         _In_ ULONG ulWaitNanoSeconds);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlAllocParallelPort(_In_ TPepCtrlObject* pObject,
                                             _In_ LPCWSTR pszDeviceName);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TPEPCTRLAPI PepCtrlFreeParallelPort(_In_ TPepCtrlObject* pObject);

_IRQL_requires_max_(PASSIVE_LEVEL)
LPGUID TPEPCTRLAPI PepCtrlGetParallelPortDevInterfaceGuid(VOID);

#endif /* !defined(PepCtrlParallelPort_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
