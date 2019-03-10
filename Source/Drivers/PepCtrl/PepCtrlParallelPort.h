/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlParallelPort_H)
#define PepCtrlParallelPort_H

BOOLEAN TPEPCTRLAPI PepCtrlReadBitParallelPort(IN TPepCtrlObject* pObject,
                                               OUT PBOOLEAN pbValue);

BOOLEAN TPEPCTRLAPI PepCtrlWriteParallelPort(IN TPepCtrlObject* pObject,
                                             IN PUCHAR pucData,
                                             IN ULONG ulDataLen);

BOOLEAN TPEPCTRLAPI PepCtrlAllocParallelPort(IN TPepCtrlObject* pObject,
                                             IN LPCWSTR pszDeviceName);

BOOLEAN TPEPCTRLAPI PepCtrlFreeParallelPort(IN TPepCtrlObject* pObject);

LPGUID TPEPCTRLAPI PepCtrlGetParallelPortDevInterfaceGuid(VOID);

#endif /* !defined(PepCtrlParallelPort_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
