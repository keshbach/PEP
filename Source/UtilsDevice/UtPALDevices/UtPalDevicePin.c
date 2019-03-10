/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <assert.h>

#include <UtilsDevice/UtPepDevices.h>

#include "UtPalDevicePin.h"
#include "UtPalDeviceDefs.h"

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPal.h>

static EPALDevicePinType l_PinType[] = {
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptGround,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptNotUsed,
    epdptVcc};

static EPALDevicePinPolarity l_PinPolarity[] = {
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow,
    epdppActiveLow
};

static UINT l_nNonConfigurablePins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static WCHAR l_cConfigDevicePinsMsg[25 * CPALDeviceTotalConfigurablePins] = {0};

static UINT l_nPAL20ConfigurablePins[] = {
    MPALDevice20PinTo24Pin(9),
    MPALDevice20PinTo24Pin(11),
    MPALDevice20PinTo24Pin(12),
    MPALDevice20PinTo24Pin(13),
    MPALDevice20PinTo24Pin(14),
    MPALDevice20PinTo24Pin(15),
    MPALDevice20PinTo24Pin(16),
    MPALDevice20PinTo24Pin(17),
    MPALDevice20PinTo24Pin(18),
    MPALDevice20PinTo24Pin(19)};
static UINT l_nPAL24ConfigurablePins[] = {11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};

static VOID lSetDevicePinType(
  UINT nPin,
  EPALDevicePinType PinType)
{
    if (nPin == 0 || nPin > MArrayLen(l_PinType))
    {
        assert(0);
    }

    l_PinType[nPin - 1] = PinType;
}

static VOID lSetDevicePinPolarity(
  UINT nPin,
  EPALDevicePinPolarity PinPolarity)
{
    if (nPin == 11)
    {
        l_PinPolarity[0] = PinPolarity;
    }
    else if (nPin >= 13 && nPin <= 23)
    {
        l_PinPolarity[nPin - 12] = PinPolarity;
    }
    else
    {
        assert(0);
    }
}

static const TDevicePinConfig* lFindDevicePinConfig(
  UINT nPin,
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    UINT nIndex;

    for (nIndex = 0; nIndex < nTotalDevicePinConfigs; ++nIndex)
    {
        if (pDevicePinConfigs[nIndex].nPin == nPin)
        {
            return &pDevicePinConfigs[nIndex];
        }
    }

    return NULL;
}

static const TDevicePinConfigValues* lFindDevicePinConfigValues(
  UINT nPin,
  const TDevicePinConfigValues* pDevicePinConfigValues,
  UINT nTotalDevicePinConfigValues)
{
    UINT nIndex;

    for (nIndex = 0; nIndex < nTotalDevicePinConfigValues; ++nIndex)
    {
        if (pDevicePinConfigValues[nIndex].nPin == nPin)
        {
            return &pDevicePinConfigValues[nIndex];
        }
    }

    return NULL;
}

static VOID lSaveNonConfigurableDevicePinType(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    const TDevicePinConfig* pTmpDevicePinConfig;
    UINT nIndex, nPin;

    for (nIndex = 0; nIndex < MArrayLen(l_nNonConfigurablePins); ++nIndex)
    {
        nPin = l_nNonConfigurablePins[nIndex];

        pTmpDevicePinConfig = lFindDevicePinConfig(nPin, pDevicePinConfigs,
                                                   nTotalDevicePinConfigs);

        if (pTmpDevicePinConfig)
        {
            if (0 == lstrcmpiW(pTmpDevicePinConfig->pszType, CInputPinType))
            {
                lSetDevicePinType(nPin, epdptInput);
            }
            else if (0 == lstrcmpiW(pTmpDevicePinConfig->pszType, CNotUsedPinType))
            {
                lSetDevicePinType(nPin, epdptNotUsed);
            }
            else if (0 == lstrcmpiW(pTmpDevicePinConfig->pszType, COutputPinType))
            {
                lSetDevicePinType(nPin, epdptOutput);
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            lSetDevicePinType(nPin, epdptNotUsed);
        }
    }
}

static VOID lSaveConfigurableDevicePinType(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs,
  UINT* pnConfigurablePins,
  UINT nTotalConfigurablePins)
{
    const TDevicePinConfig* pTmpDevicePinConfig;
    UINT nIndex, nPin;

    for (nIndex = 0; nIndex < nTotalConfigurablePins; ++nIndex)
    {
        nPin = pnConfigurablePins[nIndex];

        pTmpDevicePinConfig = lFindDevicePinConfig(nPin, pDevicePinConfigs,
                                                   nTotalDevicePinConfigs);

        if (0 == lstrcmpiW(pTmpDevicePinConfig->pszType, CInputPinType))
        {
            lSetDevicePinType(nPin, epdptInput);
        }
        else if (0 == lstrcmpiW(pTmpDevicePinConfig->pszType, CNotUsedPinType))
        {
            lSetDevicePinType(nPin, epdptNotUsed);
        }
        else if (0 == lstrcmpiW(pTmpDevicePinConfig->pszType, COutputPinType))
        {
            lSetDevicePinType(nPin, epdptOutput);
        }
        else if (0 == lstrcmpiW(pTmpDevicePinConfig->pszType, CRegisteredPinType))
        {
            lSetDevicePinType(nPin, epdptRegistered);
        }
        else
        {
            assert(0);
        }
    }
}

static TDevicePinConfig* lAllocPAL20ToPAL24DevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    TDevicePinConfig* pNewDevicePinConfigs;
    ULONG ulIndex;

    pNewDevicePinConfigs = (TDevicePinConfig*)UtAllocMem(
                               sizeof(TDevicePinConfig) * nTotalDevicePinConfigs);

    if (pNewDevicePinConfigs)
    {
        for (ulIndex = 0; ulIndex < nTotalDevicePinConfigs; ++ulIndex)
        {
            pNewDevicePinConfigs[ulIndex].nPin = MPALDevice20PinTo24Pin(pDevicePinConfigs[ulIndex].nPin);
            pNewDevicePinConfigs[ulIndex].pszType = pDevicePinConfigs[ulIndex].pszType;
            pNewDevicePinConfigs[ulIndex].pszPolarity = pDevicePinConfigs[ulIndex].pszPolarity;
        }
    }

    return pNewDevicePinConfigs;
}

static VOID lFreePAL20ToPAL24DevicePinConfigs(
  TDevicePinConfig* pDevicePinConfigs)
{
    UtFreeMem(pDevicePinConfigs);
}

static BOOL lSetDevicePinTypes(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs,
  UINT* pnConfigurablePins,
  UINT nTotalConfigurablePins)
{
    UINT nIndex, nPin, nDeviceIndex;
    WCHAR cBuffer[5];

    lSaveNonConfigurableDevicePinType(pDevicePinConfigs, nTotalDevicePinConfigs);

    lSaveConfigurableDevicePinType(pDevicePinConfigs, nTotalDevicePinConfigs,
                                   pnConfigurablePins, nTotalConfigurablePins);

    l_cConfigDevicePinsMsg[0] = 0;

    for (nIndex = 0; nIndex < nTotalConfigurablePins; ++nIndex)
    {
        nPin = pnConfigurablePins[nIndex];

        nDeviceIndex = UtPALDeviceGetDevicePinIndex(nPin);

        StringCchPrintfW(cBuffer, sizeof(cBuffer) / sizeof(cBuffer[0]), L"%d",
                         nIndex + 1);

        StringCchCatW(l_cConfigDevicePinsMsg,
                      sizeof(l_cConfigDevicePinsMsg) / sizeof(l_cConfigDevicePinsMsg[0]),
                      L"Set Switch ");
        StringCchCatW(l_cConfigDevicePinsMsg,
                      sizeof(l_cConfigDevicePinsMsg) / sizeof(l_cConfigDevicePinsMsg[0]),
                      cBuffer);
        StringCchCatW(l_cConfigDevicePinsMsg,
                      sizeof(l_cConfigDevicePinsMsg) / sizeof(l_cConfigDevicePinsMsg[0]),
                      L" to ");

        switch (UtPALDeviceGetDevicePinType(nPin))
        {
            case epdptInput:
            case epdptNotUsed:
                StringCchCatW(l_cConfigDevicePinsMsg,
                              sizeof(l_cConfigDevicePinsMsg) / sizeof(l_cConfigDevicePinsMsg[0]),
                              L"Input\n");
                break;
            case epdptOutput:
            case epdptRegistered:
                StringCchCatW(l_cConfigDevicePinsMsg,
                              sizeof(l_cConfigDevicePinsMsg) / sizeof(l_cConfigDevicePinsMsg[0]),
                              L"Output\n");
                break;
            default:
                assert(0);
                break;
        }
    }

    return TRUE;
}

static BOOL lSetDevicePinPolarities(
  const TDevicePinConfigValues* pDevicePinConfigValues,
  UINT nTotalDevicePinConfigValues,
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs,
  UINT* pnConfigurablePins,
  UINT nTotalConfigurablePins)
{
    UINT nIndex;
    const TDevicePinConfig* pDevicePinConfig;
    const TDevicePinConfigValues* pTmpDevicePinConfigValues;
    LPCTSTR pszPolarity;

    for (nIndex = 0; nIndex < nTotalConfigurablePins; ++nIndex)
    {
        pDevicePinConfig = lFindDevicePinConfig(pnConfigurablePins[nIndex],
                                                pDevicePinConfigs,
                                                nTotalDevicePinConfigs);

        if (pDevicePinConfig &&
            (0 == lstrcmpiW(pDevicePinConfig->pszType, COutputPinType) ||
             0 == lstrcmpiW(pDevicePinConfig->pszType, CRegisteredPinType)))
        {
            pszPolarity = pDevicePinConfig->pszPolarity;

            if (pszPolarity == NULL)
            {
                pTmpDevicePinConfigValues = lFindDevicePinConfigValues(
                                                pnConfigurablePins[nIndex],
                                                pDevicePinConfigValues,
                                                nTotalDevicePinConfigValues);

                if (pTmpDevicePinConfigValues)
                {
                    pszPolarity = pTmpDevicePinConfigValues->ppszPolarities[0];
                }
            }

            if (pszPolarity)
            {
                if (0 == lstrcmpiW(pszPolarity, CActiveLowPinPolarity))
                {
                    lSetDevicePinPolarity(pnConfigurablePins[nIndex],
                                          epdppActiveLow);
                }
                else if (0 == lstrcmpiW(pszPolarity, CActiveHighPinPolarity))
                {
                    lSetDevicePinPolarity(pnConfigurablePins[nIndex],
                                          epdppActiveHigh);
                }
                else
                {
                    assert(0);
                }
            }
        }
    }

    return TRUE;
}

static VOID lRemoveFuseColumnExcludeNotUsed(
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BOOL bIsPal24,
  ULONG ulFuseColumn)
{
    ULONG ulFuseRowIndex;
    UINT nPinPAL24;
    TDevicePinFuseRows* pDevicePinFuseRows;

    for (ulFuseRowIndex = 0;
         ulFuseRowIndex < pPALData->nDevicePinFuseRowsCount;
         ++ulFuseRowIndex)
    {
        pDevicePinFuseRows = &pPALData->pDevicePinFuseRows[ulFuseRowIndex];

        if (bIsPal24)
        {
            nPinPAL24 = pDevicePinFuseRows->nPin;
        }
        else
        {
            nPinPAL24 = MPALDevice20PinTo24Pin(pDevicePinFuseRows->nPin);
        }

        if (epdptOutput == UtPALDeviceGetDevicePinType(nPinPAL24) ||
            epdptRegistered == UtPALDeviceGetDevicePinType(nPinPAL24))
        {
            if (CNoOutputEnableFuseRow != pDevicePinFuseRows->nFuseRowOutputEnable)
            {
                UtPALBlowFuse(UtPALFuseNumberToFuseRow(
                                  pDevicePinFuseRows->nFuseRowOutputEnable,
                                  pPALData),
                              ulFuseColumn, pPALData, pbyData, ulDataLen);
            }

            UtPALBlowFuseRange(UtPALFuseNumberToFuseRow(
                                   pDevicePinFuseRows->nFuseRowTermStart,
                                   pPALData),
                               ulFuseColumn,
                               UtPALFuseNumberToFuseRow(
                                   pDevicePinFuseRows->nFuseRowTermEnd,
                                   pPALData),
                               ulFuseColumn,
                               pPALData, pbyData, ulDataLen);
        }
    }
}

static BOOL lIsPinConfigurable(
  UINT nPin)
{
    UINT nIndex;

    for (nIndex = 0; nIndex < MArrayLen(l_nPAL24ConfigurablePins); ++nIndex)
    {
        if (nPin == l_nPAL24ConfigurablePins[nIndex])
        {
            return TRUE;
        }
    }

    return FALSE;
}

EPALDevicePinType UtPALDeviceGetDevicePinType(
  UINT nPin)
{
    if (nPin == 0 || nPin > MArrayLen(l_PinType))
    {
        assert(0);

        return l_PinType[0];
    }

    return l_PinType[nPin - 1];
}

EPALDevicePinPolarity UtPALDeviceGetDevicePinPolarity(
  UINT nPin)
{
    if (nPin == 11)
    {
        return l_PinPolarity[0];
    }

    if (nPin >= 13 && nPin <= 23)
    {
        return l_PinPolarity[nPin - 12];
    }

    assert(0);

    return l_PinPolarity[0];
}

ULONG UtPALDeviceGetTotalPinTypes(
  EPALDevicePinType PinType)
{
    ULONG ulIndex, ulTotalTypes;

    ulTotalTypes = 0;

    for (ulIndex = 0; ulIndex < MArrayLen(l_PinType); ++ulIndex)
    {
        if (l_PinType[ulIndex] == PinType)
        {
            ++ulTotalTypes;
        }
    }

    return ulTotalTypes;
}

UINT UtPALDeviceGetDevicePinIndex(
  UINT nPin)
{
    if (nPin < 12)
    {
        return nPin - 1;
    }
    else if (nPin < 24)
    {
        return nPin - 2;
    }

    /* Invalid pin */

    assert(0);

    return 0;
}

LPCWSTR UtPALDeviceGetConfigDevicePinsMsg(VOID)
{
    return l_cConfigDevicePinsMsg;
}

BOOL UtPALDeviceRemoveFuseColumnsByPinType(
  EPALDevicePinType PinType,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BOOL bIsPal24)
{
    ULONG ulFuseColumnIndex, ulFuseColumn;
    UINT nPinPAL24;

    for (ulFuseColumnIndex = 0;
         ulFuseColumnIndex < pPALData->nDevicePinFuseColumnsCount;
         ++ulFuseColumnIndex)
    {
        if (bIsPal24)
        {
            nPinPAL24 = pPALData->pDevicePinFuseColumns[ulFuseColumnIndex].nPin;
        }
        else
        {
            nPinPAL24 = MPALDevice20PinTo24Pin(pPALData->pDevicePinFuseColumns[ulFuseColumnIndex].nPin);
        }

        if (PinType == UtPALDeviceGetDevicePinType(nPinPAL24))
        {
            ulFuseColumn = pPALData->pDevicePinFuseColumns[ulFuseColumnIndex].nLowFuseColumn;

            lRemoveFuseColumnExcludeNotUsed(pPALData, pbyData, ulDataLen,
                                            bIsPal24, ulFuseColumn);

            ulFuseColumn = pPALData->pDevicePinFuseColumns[ulFuseColumnIndex].nHighFuseColumn;

            lRemoveFuseColumnExcludeNotUsed(pPALData, pbyData, ulDataLen,
                                            bIsPal24, ulFuseColumn);
        }
    }

    return TRUE;
}

BOOL UtPALDeviceSetDevicePinConfigs(
  const TPALData* pPALData,
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs,
  BOOL bIsPal24)
{
    BOOL bResult = FALSE;
    TDevicePinConfig* pNewDevicePinConfigs;
    BOOL bIsValid;

    if (bIsPal24)
    {
        if (FALSE == UtPALVerifyDevicePinConfig(pPALData, pDevicePinConfigs,
                                                nTotalDevicePinConfigs, &bIsValid) ||
            bIsValid == FALSE)
        {
            return FALSE;
        }

        if (lSetDevicePinTypes(pDevicePinConfigs, nTotalDevicePinConfigs,
                               l_nPAL24ConfigurablePins,
                               MArrayLen(l_nPAL24ConfigurablePins)) &&
            lSetDevicePinPolarities(pPALData->pDevicePinConfigValues,
                                    pPALData->nDevicePinConfigValuesCount,
                                    pDevicePinConfigs, nTotalDevicePinConfigs,
                                    l_nPAL24ConfigurablePins,
                                    MArrayLen(l_nPAL24ConfigurablePins)))
       {
           bResult = TRUE;
       }
    }
    else
    {
        if (FALSE == UtPALVerifyDevicePinConfig(pPALData, pDevicePinConfigs,
                                                nTotalDevicePinConfigs, &bIsValid) ||
            bIsValid == FALSE)
        {
            return FALSE;
        }

        pNewDevicePinConfigs = lAllocPAL20ToPAL24DevicePinConfigs(pDevicePinConfigs,
                                                                  nTotalDevicePinConfigs);

        if (pNewDevicePinConfigs)
        {
            if (lSetDevicePinTypes(pNewDevicePinConfigs, nTotalDevicePinConfigs,
                                   l_nPAL20ConfigurablePins,
                                   MArrayLen(l_nPAL20ConfigurablePins)) &&
                lSetDevicePinPolarities(pPALData->pDevicePinConfigValues,
                                        pPALData->nDevicePinConfigValuesCount,
                                        pNewDevicePinConfigs,
                                        nTotalDevicePinConfigs,
                                        l_nPAL20ConfigurablePins,
                                        MArrayLen(l_nPAL20ConfigurablePins)))
            {
                bResult = TRUE;
            }

            lFreePAL20ToPAL24DevicePinConfigs(pNewDevicePinConfigs);
        }
    }

    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/
