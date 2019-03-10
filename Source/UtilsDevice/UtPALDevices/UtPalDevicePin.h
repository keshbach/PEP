/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDevicePin_H)
#define UtPalDevicePin_H

/*
    Device pin type
*/

typedef enum EPALDevicePinType
{
    epdptInput      = 0x0001,
    epdptOutput     = 0x0002,
    epdptRegistered = 0x0004,
    epdptNotUsed    = 0x0008,
    epdptGround     = 0x0010,
    epdptVcc        = 0x0020
} EPALDevicePinType;

/*
    Device pin polarity
*/

typedef enum EPALDevicePinPolarity
{
    epdppActiveLow  = 0x0001,
    epdppActiveHigh = 0x0002
} EPALDevicePinPolarity;

EPALDevicePinType UtPALDeviceGetDevicePinType(UINT nPin);

EPALDevicePinPolarity UtPALDeviceGetDevicePinPolarity(UINT nPin);

ULONG UtPALDeviceGetTotalPinTypes(EPALDevicePinType PinType);

UINT UtPALDeviceGetDevicePinIndex(UINT nPin);

LPCWSTR UtPALDeviceGetConfigDevicePinsMsg(VOID);

BOOL UtPALDeviceRemoveFuseColumnsByPinType(EPALDevicePinType PinType,
                                           const TPALData* pPALData,
                                           LPBYTE pbyData,
                                           ULONG ulDataLen,
                                           BOOL bIsPal24);

BOOL UtPALDeviceSetDevicePinConfigs(const TPALData* pPALData,
                                    const TDevicePinConfig* pDevicePinConfigs,
                                    UINT nTotalDevicePinConfigs,
                                    BOOL bIsPal24);

#endif /* end of UtPalDevicePin_H */

/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/
