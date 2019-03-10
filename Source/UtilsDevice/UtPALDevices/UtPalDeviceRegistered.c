/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsDevice/UtPepDevices.h>

#include "UtPalDeviceRegistered.h"
#include "UtPalDeviceDefs.h"
#include "UtPalDevicePin.h"
#include "UtPalDeviceAdapter.h"
#include "UtPalDeviceDump.h"
#include "UtPalDeviceMsgs.h"

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPal.h>

#include <Config/UtPepCtrl.h>

typedef struct tagTRegisteredData
{
    ULONG ulInputData;
    ULONG ulTotalStates;
    TPALDeviceAdapterData* pOutputData; /* Output Data per state */
} TRegisteredData;

static VOID lGetClockAndOutputEnablePin(
  BOOL bIsPal24,
  UINT* pnClockPinPal24,
  UINT* pnOutputEnablePinPal24)
{
    if (bIsPal24)
    {
        *pnClockPinPal24 = 1;
        *pnOutputEnablePinPal24 = 13;
    }
    else
    {
        *pnClockPinPal24 = MPALDevice20PinTo24Pin(1);
        *pnOutputEnablePinPal24 = MPALDevice20PinTo24Pin(11);
    }
}

static BOOL lIsInputPin(
  BOOL bIsPal24,
  UINT nPin)
{
    UINT nPinPal24, nClockPinPal24, nOutputEnablePinPal24;

    if (bIsPal24)
    {
        nPinPal24 = nPin;
    }
    else
    {
        nPinPal24 = MPALDevice20PinTo24Pin(nPin);
    }

    lGetClockAndOutputEnablePin(bIsPal24, &nClockPinPal24,
                                &nOutputEnablePinPal24);

    if (nPinPal24 != nClockPinPal24 && nPinPal24 != nOutputEnablePinPal24)
    {
        return TRUE;
    }

    return FALSE;
}

static VOID lAllocRegisteredDataWithNoDuplicateStates(
  const TRegisteredData* pRegisteredData,
  ULONG ulRegisteredDataLen,
  TRegisteredData** ppNewRegisteredData,
  ULONG* pulNewRegisteredDataLen)
{
    ULONG ulIndex;

    *ppNewRegisteredData = NULL;
    *pulNewRegisteredDataLen = 0;

    for (ulIndex = 0; ulIndex < ulRegisteredDataLen; ++ulIndex)
    {
    }
}

static VOID lFreeRegisteredData(
  TRegisteredData* pRegisteredData,
  ULONG ulRegisteredDataLen)
{
    ULONG ulIndex;

    for (ulIndex = 0; ulIndex < ulRegisteredDataLen; ++ulIndex)
    {
        if (pRegisteredData[ulIndex].pOutputData)
        {
            UtFreeMem(pRegisteredData[ulIndex].pOutputData);
        }
    }

    UtFreeMem(pRegisteredData);
}

static BOOL lTranslateDataToRegisteredPalFuseMap(
  TUtPepDevicesContinueDeviceIOFunc pContinueDeviceIOFunc,
  TUtPepDevicesLogMessageDeviceIOFunc pLogMessageFunc,
  const TRegisteredData* pRegisteredData,
  ULONG ulRegisteredDataLen,
  const TPALData* pPALData,
  LPBYTE pbyFuseData,
  ULONG ulFuseDataLen,
  BOOL bIsPal24)
{
    BOOL bResult = FALSE;
    UINT nInputPinsPAL20[20], nOutputPinsPAL20[20];
    UINT nInputPinsPAL24[24], nOutputPinsPAL24[24];
    UINT nPinPAL20, nPinPAL24;
    ULONG ulInputPinsLen, ulOutputPinsLen, ulIndex;

    /* Pre-cache the input and output pins */

    ulInputPinsLen = 0;
    ulOutputPinsLen = 0;

    for (ulIndex = 0; ulIndex < pPALData->nDevicePinConfigValuesCount; ++ulIndex)
    {
        if (bIsPal24 == TRUE)
        {
            nPinPAL20 = 0;
            nPinPAL24 = pPALData->pDevicePinConfigValues[ulIndex].nPin;
        }
        else
        {
            nPinPAL20 = pPALData->pDevicePinConfigValues[ulIndex].nPin;
            nPinPAL24 = MPALDevice20PinTo24Pin(nPinPAL20);
        }

        switch (UtPALDeviceGetDevicePinType(nPinPAL24))
        {
            case epdptInput:
                if (lIsInputPin(bIsPal24, pPALData->pDevicePinConfigValues[ulIndex].nPin))
                {
                    if (bIsPal24 == FALSE)
                    {
                        nInputPinsPAL20[ulInputPinsLen] = nPinPAL20;
                    }

                    nInputPinsPAL24[ulInputPinsLen] = nPinPAL24;

                    ++ulInputPinsLen;
                }
                break;
            case epdptOutput:
            case epdptRegistered:
                if (bIsPal24 == FALSE)
                {
                    nOutputPinsPAL20[ulOutputPinsLen] = nPinPAL20;
                }

                nOutputPinsPAL24[ulOutputPinsLen] = nPinPAL24;

                ++ulOutputPinsLen;
                break;
        }
    }

    return bResult;
}

static ULONG lGetMaxTotalStates(
  const TPALData* pPALData,
  BOOL bIsPal24)
{
    ULONG ulIndex, ulTermCount, ulTmpTermCount;
    UINT nClockPinPAL24, nOutputEnablePAL24, nPinPAL24;

    ulTermCount = 0;

    lGetClockAndOutputEnablePin(bIsPal24, &nClockPinPAL24, &nOutputEnablePAL24);

    for (ulIndex = 0; ulIndex < pPALData->nDevicePinConfigValuesCount; ++ulIndex)
    {
        if (bIsPal24)
        {
            nPinPAL24 = pPALData->pDevicePinConfigValues[ulIndex].nPin;
        }
        else
        {
            nPinPAL24 = MPALDevice20PinTo24Pin(pPALData->pDevicePinConfigValues[ulIndex].nPin);
        }

        if (nPinPAL24 != nClockPinPAL24 &&
            nPinPAL24 != nOutputEnablePAL24 &&
            (epdptOutput == UtPALDeviceGetDevicePinType(nPinPAL24) ||
             epdptRegistered == UtPALDeviceGetDevicePinType(nPinPAL24)))
        {
            ulTmpTermCount = UtPALTermCount(pPALData->pDevicePinConfigValues[ulIndex].nPin,
                                            pPALData);

            if (ulTmpTermCount > ulTermCount)
            {
                ulTermCount = ulTmpTermCount;
            }
        }
    }

    return ulTermCount;
}

static ULONG lTranslateRegisteredAddress(
  ULONG ulAddress,
  const TPALData* pPALData,
  BOOL bClockOn,
  BOOL bIsPal24)
{
    ULONG ulNewAddress, ulIndex, ulData;
    UINT nClockPinPAL24, nOutputEnablePAL24, nPinPAL24;

    ulNewAddress = 0;

    lGetClockAndOutputEnablePin(bIsPal24, &nClockPinPAL24, &nOutputEnablePAL24);

    for (ulIndex = 0; ulIndex < pPALData->nDevicePinConfigValuesCount; ++ulIndex)
    {
        if (bIsPal24)
        {
            nPinPAL24 = pPALData->pDevicePinConfigValues[ulIndex].nPin;
        }
        else
        {
            nPinPAL24 = MPALDevice20PinTo24Pin(pPALData->pDevicePinConfigValues[ulIndex].nPin);
        }

        if (nPinPAL24 != nClockPinPAL24 &&
            nPinPAL24 != nOutputEnablePAL24 &&
            epdptInput == UtPALDeviceGetDevicePinType(nPinPAL24))
        {
            if ((ulAddress & 0x01) == 0x01)
            {
                ulData = 1 << UtPALDeviceGetDevicePinIndex(nPinPAL24);

                ulNewAddress |= ulData;
            }

            ulAddress >>= 1;
        }
    }

    ulData = 1 << UtPALDeviceGetDevicePinIndex(nOutputEnablePAL24);

    ulNewAddress |= ulData;

    if (bClockOn)
    {
        ulData = 1 << UtPALDeviceGetDevicePinIndex(nClockPinPAL24);

        ulNewAddress |= ulData;
    }

    return ulNewAddress;
}

static BOOL lConfigureRegisteredPalFuseMap(
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BOOL bIsPal24)
{
    /*
        The assumption is made that an output pin is not fed back into another
        term.
    */

    if (UtPALDeviceRemoveFuseColumnsByPinType(epdptNotUsed, pPALData, pbyData,
                                              ulDataLen, bIsPal24) &&
        UtPALDeviceRemoveFuseColumnsByPinType(epdptOutput, pPALData, pbyData,
                                              ulDataLen, bIsPal24))
    {
        return TRUE;
    }

    return FALSE;
}

VOID UtPALDeviceRegisteredRead(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BOOL bIsPal24)
{
    BOOL bErrorOccurred = FALSE;
    ULONG* pulInputData = NULL;
    TRegisteredData* pRegisteredData = NULL;
    TPALDeviceAdapterData* pOutputData;
    ULONG ulFuseSize, ulAddress, ulTmpAddress, ulTotalAddresses, ulStateIndex;
    ULONG ulTotalStates;

    ulTotalAddresses = 1 << UtPALDeviceGetTotalPinTypes(epdptInput);

    ulTotalStates = lGetMaxTotalStates(pPALData, bIsPal24) + 1;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulTotalAddresses, edoRead);

    if (!UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        ulDataLen != ulFuseSize ||
        FALSE == UtPALClearFuseMap(pbyData, ulDataLen) ||
        FALSE == lConfigureRegisteredPalFuseMap(pPALData, pbyData, ulDataLen, bIsPal24))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode) ||
        FALSE == UtPALDeviceAdapterZeroInputs())
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    pulInputData = (ULONG*)UtAllocMem(ulTotalAddresses * sizeof(ULONG));
    pRegisteredData = (TRegisteredData*)UtAllocMem(ulTotalAddresses * sizeof(TRegisteredData));

    if (pulInputData == NULL || pRegisteredData == NULL)
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulTotalAddresses;  ++ulAddress)
    {
        pRegisteredData[ulAddress].ulInputData = 
            lTranslateRegisteredAddress(ulAddress, pPALData, FALSE, bIsPal24);
        pRegisteredData[ulAddress].ulTotalStates = ulTotalStates;
        pRegisteredData[ulAddress].pOutputData =
            (TPALDeviceAdapterData*)UtAllocMem(
                ulTotalStates * sizeof(TPALDeviceAdapterData));
    }

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOnVccSwitchMsg);

    pDeviceIOFuncs->pLogMessageDeviceIOFunc(UtPALDeviceGetConfigDevicePinsMsg());
    pDeviceIOFuncs->pShowMessageDeviceIOFunc(UtPALDeviceGetConfigDevicePinsMsg());

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CInsertPalMsg);

    for (ulAddress = 0;
         bErrorOccurred == FALSE && ulAddress < ulTotalAddresses;
         ++ulAddress)
    {
        if (FALSE == UtPALDeviceAdapterEnablePALVcc(FALSE) ||
            FALSE == UtPALDeviceAdapterZeroInputs() ||
            FALSE == UtPALDeviceAdapterEnablePALVcc(TRUE))
        {
            bErrorOccurred = TRUE;
        }

        for (ulStateIndex = 0;
             bErrorOccurred == FALSE && ulStateIndex < pRegisteredData[ulAddress].ulTotalStates;
             ++ulStateIndex)
        {
            ulTmpAddress = lTranslateRegisteredAddress(ulAddress, pPALData, FALSE, bIsPal24);

            if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
                FALSE == UtPALDeviceAdapterWriteData(ulTmpAddress))
            {
                bErrorOccurred = TRUE;
            }

            if (bErrorOccurred == FALSE && ulStateIndex > 0)
            {
                ulTmpAddress = lTranslateRegisteredAddress(ulAddress, pPALData, TRUE, bIsPal24);

                if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
                    FALSE == UtPALDeviceAdapterWriteData(ulTmpAddress))
                {
                    bErrorOccurred = TRUE;
                }
            }

            if (bErrorOccurred == FALSE)
            {
                pOutputData = &pRegisteredData[ulAddress].pOutputData[ulStateIndex];

                if (FALSE == UtPALDeviceAdapterReadData(pOutputData))
                {
                    bErrorOccurred = TRUE;
                }
            }
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    UtPALDeviceAdapterEnablePALVcc(FALSE);

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    if (TRUE== bErrorOccurred)
    {
        goto End;
    }

    pDeviceIOFuncs->pLogMessageDeviceIOFunc(CTranslatingDataMsg);

#if !defined(NDEBUG)
    for (ulAddress = 0; ulAddress < ulTotalAddresses; ++ulAddress)
    {
        for (ulStateIndex = 0;
             ulStateIndex < pRegisteredData[ulAddress].ulTotalStates;
             ++ulStateIndex)
        {
            pOutputData = &pRegisteredData[ulAddress].pOutputData[ulStateIndex];

            UtPALDeviceDumpOutputData(&pRegisteredData[ulAddress].ulInputData,
                                      pOutputData, 1,
                                      ulAddress == 0 && ulStateIndex == 0 ? TRUE : FALSE);
        }

        OutputDebugStringW(L"\n");
    }
#endif

    if (!lTranslateDataToRegisteredPalFuseMap(pDeviceIOFuncs->pContinueDeviceIOFunc,
                                              pDeviceIOFuncs->pLogMessageDeviceIOFunc,
                                              pRegisteredData, ulTotalAddresses,
                                              pPALData, pbyData, ulDataLen,
                                              bIsPal24))
    {
        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CAllDataNotTranslatedMsg);
    }

End:
    if (pRegisteredData)
    {
        lFreeRegisteredData(pRegisteredData, ulTotalAddresses);
    }

    if (pulInputData)
    {
        UtFreeMem(pulInputData);
    }

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
