/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsDevice/UtPepDevices.h>

#include "UtPalDeviceAdapter.h"
#include "UtPalDevicePin.h"
#include "UtPalDeviceDefs.h"

#include <Includes/UtMacros.h>

#include <Config/UtPepCtrl.h>

/*
    Pins Data macro

    A0 - Data value
    A1 - Enable switch data 
*/

#define CPinDataOff 0
#define CPinDataOn 1

#define CPinDataEnable 1
#define CPinDataDisable 0

#define MPinData(data, enable) \
    (ULONG)((data & 0x01) | ((enable & 0x01) << 1))

/*
    Pin Trigger Data Output Update macro

    A2 - Trigger Data Output Update
         (Data load triggered on the positive edge of the clock.)
*/

#define CPinEnableDataUpdate 1
#define CPinDisableDataUpdate 0

#define MPinEnableDataUpdate(enable) \
    (ULONG)((enable & 0x01) << 2)

/*
    Pin Select Data Bits macro

    A3 - Select Data Bits 0-7 or 8-11
*/

#define CPinSelectDataBits0_7 0
#define CPinSelectDataBits8_11 1

#define MPinSelectDataBits(select) \
    (ULONG)((select & 0x01) << 3)

/*
    Pin Check Active Data Bits macro

    A4 - Pin Check Active Output
    A5 - Pin Check Active Output Enable
*/

#define CPinCheckActiveEnable 1
#define CPinCheckActiveDisable 0

#define MPinCheckActiveModeData(state, enable) \
    (ULONG)(((state & 0x01) << 4) | ((enable & 0x01) << 5))

/*
    PAL VCC data bit
*/

#define CPalVccEnable 1
#define CPalVccDisable 0

#define MPalVccData(enable) \
    (ULONG)((enable & 0x01) << 23)

#define CTotalPinData 24

static BOOL l_bEnablePALVcc = FALSE;
static ULONG l_ulLastWriteData = 0;

static ULONG lSetNotUsedConfigurablePinsToZero(
  ULONG ulInputData)
{
    static UINT nPins[] = {11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    ULONG ulIndex, ulTmpData;

    for (ulIndex = 0; ulIndex < MArrayLen(nPins); ++ulIndex)
    {
        if (epdptNotUsed == UtPALDeviceGetDevicePinType(nPins[ulIndex]))
        {
            ulTmpData = 1 << UtPALDeviceGetDevicePinIndex(nPins[ulIndex]);

            ulInputData &= ~ulTmpData;
        }
    }

    return ulInputData;
}

static BOOL lReadPinActiveData(
  WORD* pwActiveData)
{
    TUtPepCtrlReadUserDataWithDelay ReadUserDataWithDelay[8];
    BYTE byData[1];
    UINT nIndex, nIndex2;
    ULONG ulInputData, ulTmpData;
    WORD wPinActive;

    *pwActiveData = 0;

    for (nIndex = 0; nIndex < CPALDeviceTotalConfigurablePins; ++nIndex)
    {
        ulInputData = nIndex;

        for (nIndex2 = 0; nIndex2 < 4; ++nIndex2)
        {
            ulTmpData = (ulInputData & 0x08) ? 1 : 0;

            ReadUserDataWithDelay[nIndex2 * 2].nAddress =
                MPinData(CPinDataOff, CPinDataEnable) |
                MPinEnableDataUpdate(CPinEnableDataUpdate) |
                MPinSelectDataBits(CPinSelectDataBits8_11) |
                MPinCheckActiveModeData(ulTmpData, CPinCheckActiveDisable);
            ReadUserDataWithDelay[nIndex2 * 2].bPerformRead = FALSE;
			ReadUserDataWithDelay[nIndex2 * 2].nDelayNanoSeconds = 0;

            ReadUserDataWithDelay[(nIndex2 * 2) + 1].nAddress =
                MPinData(CPinDataOff, CPinDataEnable) |
                MPinEnableDataUpdate(CPinEnableDataUpdate) |
                MPinSelectDataBits(CPinSelectDataBits8_11) |
                MPinCheckActiveModeData(ulTmpData, CPinCheckActiveEnable);
            ReadUserDataWithDelay[(nIndex2 * 2) + 1].bPerformRead = FALSE;
			ReadUserDataWithDelay[(nIndex2 * 2) + 1].nDelayNanoSeconds = 0;

            ulInputData = ulInputData << 1;
        }

        ReadUserDataWithDelay[MArrayLen(ReadUserDataWithDelay) - 1].bPerformRead = TRUE;

        if (!UtPepCtrlReadUserDataWithDelay(ReadUserDataWithDelay, MArrayLen(ReadUserDataWithDelay),
                                            byData, MArrayLen(byData)))
        {
            return FALSE;
        }

        wPinActive = (byData[0] & 0x10) ? 0 : 1;
        *pwActiveData = *pwActiveData | (wPinActive << nIndex);
    }

    return TRUE;
}

BOOL lWriteData(
  ULONG ulData)
{
    TUtPepCtrlReadUserDataWithDelay ReadUserDataWithDelay[(CTotalPinData * 2) + 2];
    UINT nIndex;

    /* Setup data for the inputs */

    for (nIndex = 0; nIndex < CTotalPinData;  ++nIndex)
    {
        ReadUserDataWithDelay[nIndex * 2].nAddress =
            MPinData(((ulData & 0x00800000) ? CPinDataOn : CPinDataOff),
                     CPinDataDisable) |
            MPinEnableDataUpdate(CPinEnableDataUpdate) |
            MPinSelectDataBits(CPinSelectDataBits0_7) |
            MPinCheckActiveModeData(0, CPinCheckActiveDisable);
        ReadUserDataWithDelay[nIndex * 2].bPerformRead = FALSE;
		ReadUserDataWithDelay[nIndex * 2].nDelayNanoSeconds = 0;

        ReadUserDataWithDelay[(nIndex * 2) + 1].nAddress =
            MPinData(((ulData & 0x00800000) ? CPinDataOn : CPinDataOff),
                     CPinDataEnable) |
            MPinEnableDataUpdate(CPinEnableDataUpdate) |
            MPinSelectDataBits(CPinSelectDataBits0_7) |
            MPinCheckActiveModeData(0, CPinCheckActiveDisable);
        ReadUserDataWithDelay[(nIndex * 2) + 1].bPerformRead = FALSE;
		ReadUserDataWithDelay[(nIndex * 2) + 1].nDelayNanoSeconds = 0;

        ulData = ulData << 1;
    }

    /* Setup the trigger for a data output */

    ReadUserDataWithDelay[nIndex * 2].nAddress = MPinData(CPinDataOff, CPinDataEnable) |
                                                 MPinEnableDataUpdate(CPinDisableDataUpdate) |
                                                 MPinSelectDataBits(CPinSelectDataBits0_7) |
                                                 MPinCheckActiveModeData(0, CPinCheckActiveDisable);
    ReadUserDataWithDelay[nIndex * 2].bPerformRead = FALSE;
	ReadUserDataWithDelay[nIndex * 2].nDelayNanoSeconds = 0;

    ReadUserDataWithDelay[(nIndex * 2) + 1].nAddress = MPinData(CPinDataOff, CPinDataEnable) |
                                                       MPinEnableDataUpdate(CPinEnableDataUpdate) |
                                                       MPinSelectDataBits(CPinSelectDataBits0_7) |
                                                       MPinCheckActiveModeData(0, CPinCheckActiveDisable);
    ReadUserDataWithDelay[(nIndex * 2) + 1].bPerformRead = FALSE;
	ReadUserDataWithDelay[(nIndex * 2) + 1].nDelayNanoSeconds = 0;

    if (UtPepCtrlReadUserDataWithDelay(ReadUserDataWithDelay,
		                               MArrayLen(ReadUserDataWithDelay),
		                               NULL, 0))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL UtPALDeviceAdapterInit(VOID)
{
    TUtPepCtrlReadUserDataWithDelay ReadUserDataWithDelay[1];

    ReadUserDataWithDelay[0].nAddress = MPinData(CPinDataOff, CPinDataEnable) |
                                        MPinEnableDataUpdate(CPinEnableDataUpdate) |
                                        MPinSelectDataBits(CPinSelectDataBits0_7) |
                                        MPinCheckActiveModeData(0, CPinCheckActiveDisable);
    ReadUserDataWithDelay[0].bPerformRead = FALSE;
	ReadUserDataWithDelay[0].nDelayNanoSeconds = 0;

    l_bEnablePALVcc = FALSE;

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode) ||
        !UtPepCtrlReadUserDataWithDelay(ReadUserDataWithDelay, 
			                            MArrayLen(ReadUserDataWithDelay),
			                            NULL, 0) ||
        !UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UtPALDeviceAdapterEnablePALVcc(
  BOOL bEnable)
{
    ULONG ulData;

    if (bEnable != l_bEnablePALVcc)
    {
        l_bEnablePALVcc = bEnable;

        ulData = l_ulLastWriteData;
        ulData |= MPalVccData(bEnable ? CPalVccEnable : CPalVccDisable);

        return lWriteData(ulData);
    }

    return TRUE;
}

BOOL UtPALDeviceAdapterZeroInputs(VOID)
{
    if (l_bEnablePALVcc)
    {
        return FALSE;
    }

    return lWriteData(0 | MPalVccData(CPalVccDisable));
}

BOOL UtPALDeviceAdapterWriteData(
  ULONG ulData)
{
    ulData = lSetNotUsedConfigurablePinsToZero(ulData);

    l_ulLastWriteData = ulData;

    ulData |= MPalVccData(l_bEnablePALVcc ? CPalVccEnable : CPalVccDisable);

    return lWriteData(ulData);
}

BOOL UtPALDeviceAdapterReadData(
  TPALDeviceAdapterData* pDeviceAdapterData)
{
    TUtPepCtrlReadUserDataWithDelay ReadUserDataWithDelay[2];
    BYTE byData[2];

    pDeviceAdapterData->wOutputData = 0;
    pDeviceAdapterData->wPinActiveData = 0;

    if (l_bEnablePALVcc == FALSE)
    {
        return FALSE;
    }

    /* Read low/high data */

    ReadUserDataWithDelay[0].nAddress = MPinData(CPinDataOff, CPinDataDisable) |
                                        MPinEnableDataUpdate(CPinEnableDataUpdate) |
                                        MPinSelectDataBits(CPinSelectDataBits0_7) |
                                        MPinCheckActiveModeData(0, CPinCheckActiveDisable);
    ReadUserDataWithDelay[0].bPerformRead = TRUE;
	ReadUserDataWithDelay[0].nDelayNanoSeconds = 0;

    ReadUserDataWithDelay[1].nAddress = MPinData(CPinDataOff, CPinDataDisable) |
                                        MPinEnableDataUpdate(CPinEnableDataUpdate) |
                                        MPinSelectDataBits(CPinSelectDataBits8_11) |
                                        MPinCheckActiveModeData(0, CPinCheckActiveDisable);
    ReadUserDataWithDelay[1].bPerformRead = TRUE;
	ReadUserDataWithDelay[1].nDelayNanoSeconds = 0;

    if (!UtPepCtrlReadUserDataWithDelay(ReadUserDataWithDelay,
		                                MArrayLen(ReadUserDataWithDelay),
		                                byData, MArrayLen(byData)))
    {
        return FALSE;
    }

    byData[1] &= 0x0F; /* Clear out upper four bits since they are not used */

    pDeviceAdapterData->wOutputData = (WORD)byData[0];
    pDeviceAdapterData->wOutputData |= ((WORD)byData[1] << 8);

    lReadPinActiveData(&pDeviceAdapterData->wPinActiveData);

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
