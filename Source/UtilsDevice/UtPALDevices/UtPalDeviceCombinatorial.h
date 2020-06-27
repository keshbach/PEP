/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDeviceCombinatorial_H)
#define UtPalDeviceCombinatorial_H

VOID UtPALDeviceCombinatorialRead(const TDeviceIOFuncs* pDeviceIOFuncs,
                                  UINT32 nChipEnableNanoseconds,
                                  UINT32 nOutputEnableNanoseconds,
								  const TPALData* pPALData,
							      LPBYTE pbyData,
                                  ULONG ulDataLen,
                                  BOOL bIsPal24);

VOID UtPALDeviceCombinatorialReadAdapter(const TDeviceIOFuncs* pDeviceIOFuncs,
										 UINT32 nChipEnableNanoseconds,
										 UINT32 nOutputEnableNanoseconds,
										 const TPALData* pPALData,
                                         LPBYTE pbyData,
                                         ULONG ulDataLen,
                                         LPCTSTR pszDeviceAdapter,
                                         UINT* pnMappedPins,
                                         UINT nMappedPinsLen);

#endif /* end of UtPalDeviceCombinatorial_H */

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
