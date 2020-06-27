/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDeviceRegistered_H)
#define UtPalDeviceRegistered_H

VOID UtPALDeviceRegisteredRead(const TDeviceIOFuncs* pDeviceIOFuncs,
                               UINT32 nChipEnableNanoseconds,
                               UINT32 nOutputEnableNanoseconds,
                               const TPALData* pPALData,
                               LPBYTE pbyData,
                               ULONG ulDataLen,
                               BOOL bIsPal24);

#endif /* end of UtPalDeviceRegistered_H */

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
