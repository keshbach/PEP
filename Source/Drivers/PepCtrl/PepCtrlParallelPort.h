/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlParallelPort_H)
#define PepCtrlParallelPort_H

BOOLEAN TPEPCTRLAPI PepCtrlReadBitParallelPort(_In_ TPepCtrlObject* pObject,
                                               _Out_ PBOOLEAN pbValue);

BOOLEAN TPEPCTRLAPI PepCtrlWriteParallelPort(_In_ TPepCtrlObject* pObject,
                                             _In_ PUCHAR pucData,
                                             _In_ ULONG ulDataLen);

BOOLEAN TPEPCTRLAPI PepCtrlAllocParallelPort(_In_ TPepCtrlObject* pObject,
                                             _In_ LPCWSTR pszDeviceName);

BOOLEAN TPEPCTRLAPI PepCtrlFreeParallelPort(_In_ TPepCtrlObject* pObject);

LPGUID TPEPCTRLAPI PepCtrlGetParallelPortDevInterfaceGuid(VOID);

#endif /* !defined(PepCtrlParallelPort_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
