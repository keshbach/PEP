/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDeviceAdapter_H)
#define UtPalDeviceAdapter_H

/*
    Data read from a PAL
*/

typedef struct tagTPALDeviceAdapterData
{
    WORD wOutputData;
    WORD wPinActiveData;
} TPALDeviceAdapterData;

BOOL UtPALDeviceAdapterInit(VOID);

BOOL UtPALDeviceAdapterEnablePALVcc(BOOL bEnable);

BOOL UtPALDeviceAdapterZeroInputs(VOID);

BOOL UtPALDeviceAdapterWriteData(ULONG ulData);

BOOL UtPALDeviceAdapterReadData(TPALDeviceAdapterData* pDeviceAdapterData);

#endif /* end of UtPalDeviceAdapter_H */

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
