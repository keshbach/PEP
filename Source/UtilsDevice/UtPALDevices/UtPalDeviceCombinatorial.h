/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDeviceCombinatorial_H)
#define UtPalDeviceCombinatorial_H

VOID UtPALDeviceCombinatorialRead(const TDeviceIOFuncs* pDeviceIOFuncs,
                                  const TPALData* pPALData,
                                  LPBYTE pbyData,
                                  ULONG ulDataLen,
                                  BOOL bIsPal24);

VOID UtPALDeviceCombinatorialReadAdapter(const TDeviceIOFuncs* pDeviceIOFuncs,
                                         const TPALData* pPALData,
                                         LPBYTE pbyData,
                                         ULONG ulDataLen,
                                         LPCTSTR pszDeviceAdapter,
                                         UINT* pnMappedPins,
                                         UINT nMappedPinsLen);

#endif /* end of UtPalDeviceCombinatorial_H */

/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/
