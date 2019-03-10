/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <UtilsDevice/UtPepDevices.h>

#include "UtPalDeviceCombinatorial.h"
#include "UtPalDeviceDefs.h"
#include "UtPalDevicePin.h"
#include "UtPalDeviceAdapter.h"
#include "UtPalDeviceTerm.h"
#include "UtPalDeviceDump.h"
#include "UtPalDeviceMsgs.h"

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPal.h>
#include <UtilsDevice/UtPALDefs.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#include <assert.h>

static BOOL lIsPinPresent(
  UINT nPin,
  UINT* pnPins,
  UINT nPinsLen) 
{
    UINT nIndex;

    for (nIndex = 0; nIndex < nPinsLen; ++nIndex)
    {
        if (nPin == pnPins[nIndex])
        {
            return TRUE;
        }
    }

    return FALSE;
}

static TPALDevicePinTerms* lAllocPinTerms(
  UINT nTotalPinTerms)
{
    TPALDevicePinTerms* pPinTerms;
    UINT nIndex;

    pPinTerms = (TPALDevicePinTerms*)UtAllocMem(nTotalPinTerms * sizeof(TPALDevicePinTerms));

    for (nIndex = 0; nIndex < nTotalPinTerms; ++nIndex)
    {
        pPinTerms[nIndex].bOutputEnableDone = FALSE;
        pPinTerms[nIndex].bOutputDone = FALSE;
        pPinTerms[nIndex].bOutputEnableTermsSet = FALSE;
        pPinTerms[nIndex].bOutputTermsSet = FALSE;
        pPinTerms[nIndex].pszOutputEnableEquations = NULL;
        pPinTerms[nIndex].pszOutputEquations = NULL;
    }

    return pPinTerms;
}

static VOID lFreePinTerms(
  TPALDevicePinTerms* pPinTerms,
  UINT nPinTermsLen)
{
    UINT nIndex;

    for (nIndex = 0; nIndex < nPinTermsLen; ++nIndex)
    {
        if (pPinTerms[nIndex].pszOutputEnableEquations)
        {
            UtFreeMem(pPinTerms[nIndex].pszOutputEnableEquations);
        }

        if (pPinTerms[nIndex].pszOutputEquations)
        {
            UtFreeMem(pPinTerms[nIndex].pszOutputEquations);
        }
    }

    UtFreeMem(pPinTerms);
}

static BOOL lTranslateDataToCombinatorialPalFuseMap(
  TUtPepDevicesContinueDeviceIOFunc pContinueDeviceIOFunc,
  TUtPepDevicesLogMessageDeviceIOFunc pLogMessageFunc,
  const ULONG* pulInputData,
  const TPALDeviceAdapterData* pOutputData,
  ULONG ulOutputDataLen,
  const TPALData* pPALData,
  LPBYTE pbyFuseData,
  ULONG ulFuseDataLen,
  BOOL bIsPal24)
{
    BOOL bResult = FALSE;
    UINT nInputPinsPAL20[20], nOutputPinsPAL20[20];
    UINT nInputPinsPAL24[24], nOutputPinsPAL24[24];
    UINT nPinPAL20, nPinPAL24, nPin;
    ULONG ulInputPinsLen, ulOutputPinIndex, ulOutputPinsLen, ulIndex, ulFuseRow;
    LPWSTR* ppszTerms;
    LPWSTR* ppszMinimizedTerms;
    LPWSTR* ppszMergedTerms;
    LPWSTR pszEquations;
    WCHAR cMessage[100];
    TPALDevicePinTerms* pPinTerms;
    TPALDevicePinTerms* pPinTermsMatch;
    INT nPinTermMatchIndex;
    BOOL bPinDataOn;

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
                if (bIsPal24 == FALSE)
                {
                    nInputPinsPAL20[ulInputPinsLen] = nPinPAL20;
                }

                nInputPinsPAL24[ulInputPinsLen] = nPinPAL24;

                ++ulInputPinsLen;
                break;
            case epdptOutput:
                if (bIsPal24 == FALSE)
                {
                    nOutputPinsPAL20[ulOutputPinsLen] = nPinPAL20;
                }

                nOutputPinsPAL24[ulOutputPinsLen] = nPinPAL24;

                ++ulOutputPinsLen;
                break;
        }
    }

    pPinTerms = lAllocPinTerms(ulOutputPinsLen);

    /* Pass 1 - Generating minimized terms */

    pLogMessageFunc(L"Pass 1 - Generating the minimized product terms for all output pins");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        if (bIsPal24 == TRUE)
        {
            nPin = nOutputPinsPAL24[ulOutputPinIndex];
        }
        else
        {
            nPin = nOutputPinsPAL20[ulOutputPinIndex];
        }

        /* Output enable product terms */

        if (FALSE == pContinueDeviceIOFunc())
        {
            goto End;
        }

        if (UtPALIsOutputEnablePresent(nPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Generating the minimized product terms for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            ppszTerms = UtPALDeviceAllocOutputEnableTerms(pulInputData, pOutputData,
                                                          ulOutputDataLen, nInputPinsPAL24,
                                                          ulInputPinsLen,
                                                          nOutputPinsPAL24[ulOutputPinIndex]);

            if (UtPALDeviceGetTotalTerms(ppszTerms) != ulOutputDataLen)
            {
                ppszMinimizedTerms = UtPALAllocMinimizedTerms(CEspressoMinimizer,
                                                              ppszTerms);

                pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet = TRUE;
                pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms = ppszMinimizedTerms;

                UtPALDeviceDumpTerms(ppszMinimizedTerms);
            }

            UtPALDeviceFreeOutputEnableTerms(ppszTerms);
        }

        /* Output product terms */

        if (FALSE == pContinueDeviceIOFunc())
        {
            goto End;
        }

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Generating the minimized product terms for the output pin %d.",
                         nPin);

        pLogMessageFunc(cMessage);


        switch (UtPALDeviceGetDevicePinPolarity(nOutputPinsPAL24[ulOutputPinIndex]))
        {
            case epdppActiveLow:
                bPinDataOn = FALSE;
                break;
            case epdppActiveHigh:
                bPinDataOn = TRUE;
                break;
            default:
                assert(0);

                bPinDataOn = FALSE;
                break;
        }

        ppszTerms = UtPALDeviceAllocDataTerms(pulInputData, pOutputData,
                                              ulOutputDataLen,
                                              nInputPinsPAL24, ulInputPinsLen,
                                              nOutputPinsPAL24[ulOutputPinIndex],
                                              bPinDataOn);

        ppszMinimizedTerms = UtPALAllocMinimizedTerms(CEspressoMinimizer,
                                                      ppszTerms);

        pPinTerms[ulOutputPinIndex].bOutputTermsSet = TRUE;
        pPinTerms[ulOutputPinIndex].ppszOutputTerms = ppszMinimizedTerms;

        UtPALDeviceDumpTerms(ppszMinimizedTerms);

        UtPALDeviceFreeDataTerms(ppszTerms);
    }

    pLogMessageFunc(L"");

    /* Pass 2 - Converting minimized product terms into the fuse map */

    pLogMessageFunc(L"Pass 2 - Converting the minimized product terms into the fuse map");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        if (bIsPal24 == TRUE)
        {
            nPin = nOutputPinsPAL24[ulOutputPinIndex];
        }
        else
        {
            nPin = nOutputPinsPAL20[ulOutputPinIndex];
        }

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Converting the product terms for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            if (pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet == FALSE)
            {
                ulFuseRow = UtPALOutputEnableFuseRow(
                                bIsPal24 ? nOutputPinsPAL24[ulOutputPinIndex] :
                                    nOutputPinsPAL20[ulOutputPinIndex],
                                pPALData);

                UtPALBlowFuseRow(ulFuseRow, pPALData, pbyFuseData, ulFuseDataLen);

                pszEquations = UtPALDeviceAllocOutputEnableAlwaysOnEquation();

                pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
                pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
            }
            else
            {
                if (UtPALDeviceTranslateMinimizedOutputEnableTerms(
                        bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20, ulInputPinsLen,
                        bIsPal24 ? nOutputPinsPAL24[ulOutputPinIndex] : nOutputPinsPAL20[ulOutputPinIndex],
                        pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                        pPALData, pbyFuseData, ulFuseDataLen))
                {
                    pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                       bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20,
                                       ulInputPinsLen,
                                       pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);

                    pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
                    pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
                }
            }
        }

        /* Output */

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Converting the product terms for the output of pin %d.",
                         nPin);

        pLogMessageFunc(cMessage);

        if (UtPALDeviceTranslateMinimizedDataTerms(
                bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20, ulInputPinsLen,
                bIsPal24 ? nOutputPinsPAL24[ulOutputPinIndex] : nOutputPinsPAL20[ulOutputPinIndex],
                pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                pPALData, pbyFuseData, ulFuseDataLen))
        {
            pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                               bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20,
                               ulInputPinsLen,
                               pPinTerms[ulOutputPinIndex].ppszOutputTerms);

            pPinTerms[ulOutputPinIndex].bOutputDone = TRUE;
            pPinTerms[ulOutputPinIndex].pszOutputEquations = pszEquations;
        }
    }

    pLogMessageFunc(L"");

    /* Pass 3 - Attempting to convert any unused minimized product terms into the fuse map */

    pLogMessageFunc(L"Pass 3 - Attempting to convert any unused minimized product terms into the fuse map");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        if (bIsPal24 == TRUE)
        {
            nPin = nOutputPinsPAL24[ulOutputPinIndex];
        }
        else
        {
            nPin = nOutputPinsPAL20[ulOutputPinIndex];
        }

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nPin, pPALData) &&
            pPinTerms[ulOutputPinIndex].bOutputEnableDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Attempting to convert the product terms for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            nPinTermMatchIndex = UtPALDeviceFindPinTermIndexUtilizedByEquation(
                                     pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                                     pPinTerms, ulOutputPinsLen);

            if (nPinTermMatchIndex != -1)
            {
                pPinTermsMatch =  &pPinTerms[nPinTermMatchIndex];

                ppszMergedTerms = UtPALAllocMergedTerms(
                                      pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                                      pPinTermsMatch->ppszOutputTerms);

                if (!UtPALDeviceTranslateMinimizedOutputEnableTermsWithOutputPin(
                        bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20, ulInputPinsLen,
                        bIsPal24 ? nOutputPinsPAL24[ulOutputPinIndex] : nOutputPinsPAL20[ulOutputPinIndex],
                        ppszMergedTerms,
                        bIsPal24 ? nOutputPinsPAL24[nPinTermMatchIndex] : nOutputPinsPAL20[nPinTermMatchIndex],
                        pPALData, pbyFuseData, ulFuseDataLen))
                {
                    StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                     L"Output enable of pin %d could not be translated.",
                                     nPin);

                    pLogMessageFunc(cMessage);
                }

                pszEquations = UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(
                                   bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20,
                                   ulInputPinsLen, ppszMergedTerms,
                                   bIsPal24 ? nOutputPinsPAL24[nPinTermMatchIndex] : nOutputPinsPAL20[nPinTermMatchIndex],
                                   FALSE);

                UtPALFreeMergedTerms(ppszMergedTerms);
            }
            else
            {
                pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                   bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20,
                                   ulInputPinsLen,
                                   pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);
            }

            pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
            pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
        }

        /* Output */

        if (pPinTerms[ulOutputPinIndex].bOutputDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Attempting to convert the product terms for the output of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            nPinTermMatchIndex = UtPALDeviceFindPinTermIndexUtilizedByEquation(
                                     pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                                     pPinTerms, ulOutputPinsLen);

            if (nPinTermMatchIndex != -1 &&
                UtPALIsFuseColumnPresent(
                    bIsPal24 ? nOutputPinsPAL24[nPinTermMatchIndex] : nOutputPinsPAL20[nPinTermMatchIndex], 
                    pPALData))
            {
                pPinTermsMatch =  &pPinTerms[nPinTermMatchIndex];

                ppszMergedTerms = UtPALAllocMergedTerms(
                                      pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                                      pPinTermsMatch->ppszOutputTerms);

                if (UtPALDeviceTranslateMinimizedDataTermsWithOutputPin(
                       bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20, ulInputPinsLen,
                       bIsPal24 ? nOutputPinsPAL24[ulOutputPinIndex] : nOutputPinsPAL20[ulOutputPinIndex],
                       ppszMergedTerms,
                       bIsPal24 ? nOutputPinsPAL24[nPinTermMatchIndex] : nOutputPinsPAL20[nPinTermMatchIndex],
                       pPALData, pbyFuseData, ulFuseDataLen))
                {
                    pPinTerms[ulOutputPinIndex].bOutputDone = TRUE;
                }
                else
                {
                    StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                     L"Output of pin %d could not be translated.",
                                     nPin);

                    pLogMessageFunc(cMessage);
                }

                pszEquations = UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(
                                   bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20,
                                   ulInputPinsLen, ppszMergedTerms,
                                   bIsPal24 ? nOutputPinsPAL24[nPinTermMatchIndex] : nOutputPinsPAL20[nPinTermMatchIndex],
                                   FALSE);

                UtPALFreeMergedTerms(ppszMergedTerms);
            }
            else
            {
                StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                    L"Output of pin %d could not be translated.",
                                    nPin);

                pLogMessageFunc(cMessage);

                pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                   bIsPal24 ? nInputPinsPAL24 : nInputPinsPAL20,
                                   ulInputPinsLen,
                                   pPinTerms[ulOutputPinIndex].ppszOutputTerms);
            }

            pPinTerms[ulOutputPinIndex].pszOutputEquations = pszEquations;
        }
    }

    pLogMessageFunc(L"");

    bResult = TRUE;

    /* Report any unconverted output pins */

    pLogMessageFunc(L"Reporting any unconverted output pins.");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        if (bIsPal24 == TRUE)
        {
            nPin = nOutputPinsPAL24[ulOutputPinIndex];
        }
        else
        {
            nPin = nOutputPinsPAL20[ulOutputPinIndex];
        }

        if (UtPALIsOutputEnablePresent(nPin, pPALData) &&
            pPinTerms[ulOutputPinIndex].bOutputEnableDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                L"Could not convert the product terms for the output enable of pin %d.",
                                nPin);

            pLogMessageFunc(cMessage);

            bResult = FALSE;
        }

        if (pPinTerms[ulOutputPinIndex].bOutputDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Could not convert the product terms for the output pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            bResult = FALSE;
        }
    }

    pLogMessageFunc(L"");

    /* Dumping all logic equations */

    pLogMessageFunc(L"Dumping all logic equations.");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        if (bIsPal24 == TRUE)
        {
            nPin = nOutputPinsPAL24[ulOutputPinIndex];
        }
        else
        {
            nPin = nOutputPinsPAL20[ulOutputPinIndex];
        }

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Dumping logic equations for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);
            pLogMessageFunc(pPinTerms[ulOutputPinIndex].pszOutputEnableEquations);
        }

        /* Output */

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Dumping logic equations for the output of pin %d.",
                         nPin);

        pLogMessageFunc(cMessage);
        pLogMessageFunc(pPinTerms[ulOutputPinIndex].pszOutputEquations);
    }

End:
    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        if (pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet == TRUE)
        {
            UtPALFreeMinimizedTerms(CEspressoMinimizer,
                                    pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);
        }

        if (pPinTerms[ulOutputPinIndex].bOutputTermsSet == TRUE)
        {
            UtPALFreeMinimizedTerms(CEspressoMinimizer,
                                    pPinTerms[ulOutputPinIndex].ppszOutputTerms);
        }
    }

    lFreePinTerms(pPinTerms, ulOutputPinsLen);

    return bResult;
}

static BOOL lTranslateDataToCombinatorialPalFuseMap2(
  TUtPepDevicesContinueDeviceIOFunc pContinueDeviceIOFunc,
  TUtPepDevicesLogMessageDeviceIOFunc pLogMessageFunc,
  const ULONG* pulInputData,
  const TPALDeviceAdapterData* pOutputData,
  ULONG ulOutputDataLen,
  const ULONG* pulInputDataMapped,
  const TPALDeviceAdapterData* pOutputDataMapped,
  ULONG ulOutputDataMappedLen,
  const TPALData* pPALData,
  LPBYTE pbyFuseData,
  ULONG ulFuseDataLen,
  UINT* pnMappedPins,
  UINT nMappedPinsLen)
{
    BOOL bResult = FALSE;
    UINT nInputPinsPAL24[24], nOutputPinsPAL24[24];
    UINT nPinPAL24, nPin, nMappedPin, nCurMappedPinLen;
    ULONG ulInputPinsLen, ulOutputPinIndex, ulOutputPinsLen, ulIndex, ulFuseRow;
    LPWSTR* ppszTerms;
    LPWSTR* ppszMinimizedTerms;
    LPWSTR* ppszMergedTerms;
    LPWSTR pszEquations;
    WCHAR cMessage[100];
    TPALDevicePinTerms* pPinTerms;
    TPALDevicePinTerms* pPinTermsMatch;
    INT nPinTermMatchIndex;
    BOOL bPinDataOn;

    /* Non-mapped set of data */

    /* Pre-cache the input and output pins */

    ulInputPinsLen = 0;
    ulOutputPinsLen = 0;

    for (ulIndex = 0; ulIndex < pPALData->nDevicePinConfigValuesCount; ++ulIndex)
    {
        nPinPAL24 = pPALData->pDevicePinConfigValues[ulIndex].nPin;

        switch (UtPALDeviceGetDevicePinType(nPinPAL24))
        {
            case epdptInput:
                nInputPinsPAL24[ulInputPinsLen] = nPinPAL24;

                ++ulInputPinsLen;
                break;
            case epdptOutput:
                if (!lIsPinPresent(nPinPAL24, pnMappedPins, nMappedPinsLen))
                {
                    nOutputPinsPAL24[ulOutputPinsLen] = nPinPAL24;

                    ++ulOutputPinsLen;
                }
                break;
        }
    }

    pPinTerms = lAllocPinTerms(ulOutputPinsLen);

    /* Pass 1 - Generating minimized terms */

    pLogMessageFunc(L"Pass 1 - Generating the minimized product terms for all output pins");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
        ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];

        /* Output enable product terms */

        if (FALSE == pContinueDeviceIOFunc())
        {
            goto End;
        }

        if (UtPALIsOutputEnablePresent(nPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Generating the minimized product terms for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            ppszTerms = UtPALDeviceAllocOutputEnableTerms(pulInputData, pOutputData,
                                                          ulOutputDataLen, nInputPinsPAL24,
                                                          ulInputPinsLen,
                                                          nOutputPinsPAL24[ulOutputPinIndex]);

            if (UtPALDeviceGetTotalTerms(ppszTerms) != ulOutputDataLen)
            {
                ppszMinimizedTerms = UtPALAllocMinimizedTerms(CEspressoMinimizer,
                                                              ppszTerms);

                pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet = TRUE;
                pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms = ppszMinimizedTerms;

                UtPALDeviceDumpTerms(ppszMinimizedTerms);
            }

            UtPALDeviceFreeOutputEnableTerms(ppszTerms);
        }

        /* Output product terms */

        if (FALSE == pContinueDeviceIOFunc())
        {
            goto End;
        }

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Generating the minimized product terms for the output pin %d.",
                         nPin);

        pLogMessageFunc(cMessage);

        switch (UtPALDeviceGetDevicePinPolarity(nOutputPinsPAL24[ulOutputPinIndex]))
        {
            case epdppActiveLow:
                bPinDataOn = FALSE;
                break;
            case epdppActiveHigh:
                bPinDataOn = TRUE;
                break;
            default:
                assert(0);

                bPinDataOn = FALSE;
                break;
        }

        ppszTerms = UtPALDeviceAllocDataTerms(pulInputData, pOutputData,
                                              ulOutputDataLen,
                                              nInputPinsPAL24, ulInputPinsLen,
                                              nOutputPinsPAL24[ulOutputPinIndex],
                                              bPinDataOn);

        ppszMinimizedTerms = UtPALAllocMinimizedTerms(CEspressoMinimizer,
                                                      ppszTerms);

        pPinTerms[ulOutputPinIndex].bOutputTermsSet = TRUE;
        pPinTerms[ulOutputPinIndex].ppszOutputTerms = ppszMinimizedTerms;

        UtPALDeviceDumpTerms(ppszMinimizedTerms);

        UtPALDeviceFreeDataTerms(ppszTerms);
    }

    pLogMessageFunc(L"");

    /* Pass 2 - Converting minimized product terms into the fuse map */

    pLogMessageFunc(L"Pass 2 - Converting the minimized product terms into the fuse map");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
        ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Converting the product terms for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            if (pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet == FALSE)
            {
                ulFuseRow = UtPALOutputEnableFuseRow(
                                nOutputPinsPAL24[ulOutputPinIndex],
                                pPALData);

                UtPALBlowFuseRow(ulFuseRow, pPALData, pbyFuseData, ulFuseDataLen);

                pszEquations = UtPALDeviceAllocOutputEnableAlwaysOnEquation();

                pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
                pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
            }
            else
            {
                if (UtPALDeviceTranslateMinimizedOutputEnableTerms(
                        nInputPinsPAL24, ulInputPinsLen,
                        nOutputPinsPAL24[ulOutputPinIndex],
                        pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                        pPALData, pbyFuseData, ulFuseDataLen))
                {
                    pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                       nInputPinsPAL24,
                                       ulInputPinsLen,
                                       pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);

                    pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
                    pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
                }
            }
        }

        /* Output */

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Converting the product terms for the output of pin %d.",
                         nPin);

        pLogMessageFunc(cMessage);

        if (UtPALDeviceTranslateMinimizedDataTerms(
                nInputPinsPAL24, ulInputPinsLen,
                nOutputPinsPAL24[ulOutputPinIndex],
                pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                pPALData, pbyFuseData, ulFuseDataLen))
        {
            pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                               nInputPinsPAL24,
                               ulInputPinsLen,
                               pPinTerms[ulOutputPinIndex].ppszOutputTerms);

            pPinTerms[ulOutputPinIndex].bOutputDone = TRUE;
            pPinTerms[ulOutputPinIndex].pszOutputEquations = pszEquations;
        }
    }

    pLogMessageFunc(L"");

    /* Pass 3 - Attempting to convert any unused minimized product terms into the fuse map */

    pLogMessageFunc(L"Pass 3 - Attempting to convert any unused minimized product terms into the fuse map");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nPin, pPALData) &&
            pPinTerms[ulOutputPinIndex].bOutputEnableDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Attempting to convert the product terms for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            nPinTermMatchIndex = UtPALDeviceFindPinTermIndexUtilizedByEquation(
                                     pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                                     pPinTerms, ulOutputPinsLen);

            if (nPinTermMatchIndex != -1)
            {
                pPinTermsMatch = &pPinTerms[nPinTermMatchIndex];

                ppszMergedTerms = UtPALAllocMergedTerms(
                                      pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                                      pPinTermsMatch->ppszOutputTerms);

                if (!UtPALDeviceTranslateMinimizedOutputEnableTermsWithOutputPin(
                         nInputPinsPAL24, ulInputPinsLen,
                         nOutputPinsPAL24[ulOutputPinIndex],
                         ppszMergedTerms,
                         nOutputPinsPAL24[nPinTermMatchIndex],
                         pPALData, pbyFuseData, ulFuseDataLen))
                {
                    StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                     L"Output enable of pin %d could not be translated.",
                                     nPin);

                    pLogMessageFunc(cMessage);
                }

                pszEquations = UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(
                                   nInputPinsPAL24,
                                   ulInputPinsLen, ppszMergedTerms,
                                   nOutputPinsPAL24[nPinTermMatchIndex],
                                   FALSE);

                UtPALFreeMergedTerms(ppszMergedTerms);
            }
            else
            {
                pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                   nInputPinsPAL24,
                                   ulInputPinsLen,
                                   pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);
            }

            pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
            pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
        }

        /* Output */

        if (pPinTerms[ulOutputPinIndex].bOutputDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Attempting to convert the product terms for the output of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            nPinTermMatchIndex = UtPALDeviceFindPinTermIndexUtilizedByEquation(
                                     pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                                     pPinTerms, ulOutputPinsLen);

            if (nPinTermMatchIndex != -1 &&
                UtPALIsFuseColumnPresent(nOutputPinsPAL24[nPinTermMatchIndex],
                                         pPALData))
            {
                pPinTermsMatch = &pPinTerms[nPinTermMatchIndex];

                ppszMergedTerms = UtPALAllocMergedTerms(
                                      pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                                      pPinTermsMatch->ppszOutputTerms);

                if (UtPALDeviceTranslateMinimizedDataTermsWithOutputPin(
                        nInputPinsPAL24, ulInputPinsLen,
                        nOutputPinsPAL24[ulOutputPinIndex],
                        ppszMergedTerms,
                        nOutputPinsPAL24[nPinTermMatchIndex],
                        pPALData, pbyFuseData, ulFuseDataLen))
                {
                    pPinTerms[ulOutputPinIndex].bOutputDone = TRUE;
                }
                else
                {
                    StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                     L"Output of pin %d could not be translated.",
                                     nPin);

                    pLogMessageFunc(cMessage);
                }

                pszEquations = UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(
                                   nInputPinsPAL24,
                                   ulInputPinsLen, ppszMergedTerms,
                                   nOutputPinsPAL24[nPinTermMatchIndex],
                                   FALSE);

                UtPALFreeMergedTerms(ppszMergedTerms);
            }
            else
            {
                StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                 L"Output of pin %d could not be translated.",
                                 nPin);

                pLogMessageFunc(cMessage);

                pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                   nInputPinsPAL24,
                                   ulInputPinsLen,
                                   pPinTerms[ulOutputPinIndex].ppszOutputTerms);
            }

            pPinTerms[ulOutputPinIndex].pszOutputEquations = pszEquations;
        }
    }

    pLogMessageFunc(L"");

    bResult = TRUE;

    /* Report any unconverted output pins */

    pLogMessageFunc(L"Reporting any unconverted output pins.");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];

        if (UtPALIsOutputEnablePresent(nPin, pPALData) &&
            pPinTerms[ulOutputPinIndex].bOutputEnableDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Could not convert the product terms for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            bResult = FALSE;
        }

        if (pPinTerms[ulOutputPinIndex].bOutputDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Could not convert the product terms for the output pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);

            bResult = FALSE;
        }
    }

    pLogMessageFunc(L"");

    /* Dumping all logic equations */

    pLogMessageFunc(L"Dumping all logic equations.");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Dumping logic equations for the output enable of pin %d.",
                             nPin);

            pLogMessageFunc(cMessage);
            pLogMessageFunc(pPinTerms[ulOutputPinIndex].pszOutputEnableEquations);
        }

        /* Output */

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Dumping logic equations for the output of pin %d.",
                         nPin);

        pLogMessageFunc(cMessage);
        pLogMessageFunc(pPinTerms[ulOutputPinIndex].pszOutputEquations);
    }

End:
    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
        ++ulOutputPinIndex)
    {
        if (pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet == TRUE)
        {
            UtPALFreeMinimizedTerms(CEspressoMinimizer,
                                    pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);
        }

        if (pPinTerms[ulOutputPinIndex].bOutputTermsSet == TRUE)
        {
            UtPALFreeMinimizedTerms(CEspressoMinimizer,
                                    pPinTerms[ulOutputPinIndex].ppszOutputTerms);
        }
    }

    lFreePinTerms(pPinTerms, ulOutputPinsLen);

    if (!bResult)
    {
        return FALSE;
    }

    /* Mapped set of data */

    bResult = FALSE;

    /* Pre-cache the input and output pins */

    ulInputPinsLen = 0;
    ulOutputPinsLen = 0;
    nCurMappedPinLen = 0;

    for (ulIndex = 0; ulIndex < pPALData->nDevicePinConfigValuesCount; ++ulIndex)
    {
        nPinPAL24 = pPALData->pDevicePinConfigValues[ulIndex].nPin;

        switch (UtPALDeviceGetDevicePinType(nPinPAL24))
        {
            case epdptInput:
                nInputPinsPAL24[ulInputPinsLen] = nPinPAL24;

                ++ulInputPinsLen;
                break;
            case epdptOutput:
                if (lIsPinPresent(nPinPAL24, pnMappedPins, nMappedPinsLen))
                {
                    nOutputPinsPAL24[ulOutputPinsLen] = 13 + nCurMappedPinLen;

                    ++ulOutputPinsLen;
                    ++nCurMappedPinLen;
                }
                break;
        }
    }

    pPinTerms = lAllocPinTerms(ulOutputPinsLen);

    /* Pass 4 - Generating minimized terms for the mapped pins */

    pLogMessageFunc(L"Pass 4 - Generating the minimized product terms for all output pins");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];
        nMappedPin = pnMappedPins[ulOutputPinIndex];

        /* Output enable product terms */

        if (FALSE == pContinueDeviceIOFunc())
        {
            goto EndMapped;
        }

        if (UtPALIsOutputEnablePresent(nMappedPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Generating the minimized product terms for the output enable of pin %d.",
                             nMappedPin);

            pLogMessageFunc(cMessage);

            ppszTerms = UtPALDeviceAllocOutputEnableTerms(pulInputDataMapped, pOutputDataMapped,
                                                          ulOutputDataMappedLen, nInputPinsPAL24,
                                                          ulInputPinsLen,
                                                          nOutputPinsPAL24[ulOutputPinIndex]);

            if (UtPALDeviceGetTotalTerms(ppszTerms) != ulOutputDataMappedLen)
            {
                ppszMinimizedTerms = UtPALAllocMinimizedTerms(CEspressoMinimizer,
                                                              ppszTerms);

                pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet = TRUE;
                pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms = ppszMinimizedTerms;

                UtPALDeviceDumpTerms(ppszMinimizedTerms);
            }

            UtPALDeviceFreeOutputEnableTerms(ppszTerms);
        }

        /* Output product terms */

        if (FALSE == pContinueDeviceIOFunc())
        {
            goto EndMapped;
        }

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Generating the minimized product terms for the output pin %d.",
                         nMappedPin);

        pLogMessageFunc(cMessage);

        switch (UtPALDeviceGetDevicePinPolarity(nOutputPinsPAL24[ulOutputPinIndex]))
        {
            case epdppActiveLow:
                bPinDataOn = FALSE;
                break;
            case epdppActiveHigh:
                bPinDataOn = TRUE;
                break;
            default:
                assert(0);

                bPinDataOn = FALSE;
                break;
        }

        ppszTerms = UtPALDeviceAllocDataTerms(pulInputDataMapped, pOutputDataMapped,
                                              ulOutputDataMappedLen,
                                              nInputPinsPAL24, ulInputPinsLen,
                                              nOutputPinsPAL24[ulOutputPinIndex],
                                              bPinDataOn);

        ppszMinimizedTerms = UtPALAllocMinimizedTerms(CEspressoMinimizer,
                                                      ppszTerms);

        pPinTerms[ulOutputPinIndex].bOutputTermsSet = TRUE;
        pPinTerms[ulOutputPinIndex].ppszOutputTerms = ppszMinimizedTerms;

        UtPALDeviceDumpTerms(ppszMinimizedTerms);

        UtPALDeviceFreeDataTerms(ppszTerms);
    }

    pLogMessageFunc(L"");

    /* Pass 5 - Converting minimized product terms into the fuse map */

    pLogMessageFunc(L"Pass 5 - Converting the minimized product terms into the fuse map");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];
        nMappedPin = pnMappedPins[ulOutputPinIndex];

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nMappedPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Converting the product terms for the output enable of pin %d.",
                             nMappedPin);

            pLogMessageFunc(cMessage);

            if (pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet == FALSE)
            {
                ulFuseRow = UtPALOutputEnableFuseRow(nMappedPin, pPALData);

                UtPALBlowFuseRow(ulFuseRow, pPALData, pbyFuseData, ulFuseDataLen);

                pszEquations = UtPALDeviceAllocOutputEnableAlwaysOnEquation();

                pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
                pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
            }
            else
            {
                if (UtPALDeviceTranslateMinimizedOutputEnableTerms(
                        nInputPinsPAL24, ulInputPinsLen,
                        nMappedPin,
                        pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                        pPALData, pbyFuseData, ulFuseDataLen))
                {
                    pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                       nInputPinsPAL24, ulInputPinsLen,
                                       pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);

                    pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
                    pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
                }
            }
        }

        /* Output */

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Converting the product terms for the output of pin %d.",
                         nMappedPin);

        pLogMessageFunc(cMessage);

        if (UtPALDeviceTranslateMinimizedDataTerms(
                nInputPinsPAL24, ulInputPinsLen, nMappedPin,
                pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                pPALData, pbyFuseData, ulFuseDataLen))
        {
            pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                               nInputPinsPAL24,
                               ulInputPinsLen,
                               pPinTerms[ulOutputPinIndex].ppszOutputTerms);

            pPinTerms[ulOutputPinIndex].bOutputDone = TRUE;
            pPinTerms[ulOutputPinIndex].pszOutputEquations = pszEquations;
        }
    }

    pLogMessageFunc(L"");

    /* Pass 6 - Attempting to convert any unused minimized product terms into the fuse map */

    pLogMessageFunc(L"Pass 6 - Attempting to convert any unused minimized product terms into the fuse map");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];
        nMappedPin = pnMappedPins[ulOutputPinIndex];

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nMappedPin, pPALData) &&
            pPinTerms[ulOutputPinIndex].bOutputEnableDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Attempting to convert the product terms for the output enable of pin %d.",
                             nMappedPin);

            pLogMessageFunc(cMessage);

            nPinTermMatchIndex = UtPALDeviceFindPinTermIndexUtilizedByEquation(
                                     pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                                     pPinTerms, ulOutputPinsLen);

            if (nPinTermMatchIndex != -1)
            {
                pPinTermsMatch = &pPinTerms[nPinTermMatchIndex];

                ppszMergedTerms = UtPALAllocMergedTerms(
                                      pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms,
                                      pPinTermsMatch->ppszOutputTerms);

                if (!UtPALDeviceTranslateMinimizedOutputEnableTermsWithOutputPin(
                         nInputPinsPAL24, ulInputPinsLen, nMappedPin,
                         ppszMergedTerms, nMappedPin,
                         pPALData, pbyFuseData, ulFuseDataLen))
                {
                    StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                     L"Output enable of pin %d could not be translated.",
                                     nMappedPin);

                    pLogMessageFunc(cMessage);
                }

                pszEquations = UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(
                                   nInputPinsPAL24, ulInputPinsLen, ppszMergedTerms,
                                   nMappedPin, FALSE);

                UtPALFreeMergedTerms(ppszMergedTerms);
            }
            else
            {
                pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                   nInputPinsPAL24, ulInputPinsLen,
                                   pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);
            }

            pPinTerms[ulOutputPinIndex].bOutputEnableDone = TRUE;
            pPinTerms[ulOutputPinIndex].pszOutputEnableEquations = pszEquations;
        }

        /* Output */

        if (pPinTerms[ulOutputPinIndex].bOutputDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Attempting to convert the product terms for the output of pin %d.",
                             nMappedPin);

            pLogMessageFunc(cMessage);

            nPinTermMatchIndex = UtPALDeviceFindPinTermIndexUtilizedByEquation(
                                     pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                                     pPinTerms, ulOutputPinsLen);

            if (nPinTermMatchIndex != -1 &&
                UtPALIsFuseColumnPresent(nMappedPin, pPALData))
            {
                pPinTermsMatch = &pPinTerms[nPinTermMatchIndex];

                ppszMergedTerms = UtPALAllocMergedTerms(
                                      pPinTerms[ulOutputPinIndex].ppszOutputTerms,
                                      pPinTermsMatch->ppszOutputTerms);

                if (UtPALDeviceTranslateMinimizedDataTermsWithOutputPin(
                        nInputPinsPAL24, ulInputPinsLen, nMappedPin,
                        ppszMergedTerms, nMappedPin,
                        pPALData, pbyFuseData, ulFuseDataLen))
                {
                    pPinTerms[ulOutputPinIndex].bOutputDone = TRUE;
                }
                else
                {
                    StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                     L"Output of pin %d could not be translated.",
                                     nMappedPin);

                    pLogMessageFunc(cMessage);
                }

                pszEquations = UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(
                                   nInputPinsPAL24, ulInputPinsLen, ppszMergedTerms,
                                   nMappedPin, FALSE);

                UtPALFreeMergedTerms(ppszMergedTerms);
            }
            else
            {
                StringCchPrintfW(cMessage, MArrayLen(cMessage),
                                 L"Output of pin %d could not be translated.",
                                 nMappedPin);

                pLogMessageFunc(cMessage);

                pszEquations = UtPALDeviceAllocMinimizedTermsToEquations(
                                   nInputPinsPAL24, ulInputPinsLen,
                                   pPinTerms[ulOutputPinIndex].ppszOutputTerms);
            }

            pPinTerms[ulOutputPinIndex].pszOutputEquations = pszEquations;
        }
    }

    pLogMessageFunc(L"");

    bResult = TRUE;

    /* Report any unconverted output pins */

    pLogMessageFunc(L"Reporting any unconverted output pins.");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];
        nMappedPin = pnMappedPins[ulOutputPinIndex];

        if (UtPALIsOutputEnablePresent(nMappedPin, pPALData) &&
            pPinTerms[ulOutputPinIndex].bOutputEnableDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Could not convert the product terms for the output enable of pin %d.",
                             nMappedPin);

            pLogMessageFunc(cMessage);

            bResult = FALSE;
        }

        if (pPinTerms[ulOutputPinIndex].bOutputDone == FALSE)
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Could not convert the product terms for the output pin %d.",
                             nMappedPin);

            pLogMessageFunc(cMessage);

            bResult = FALSE;
        }
    }

    pLogMessageFunc(L"");

    /* Dumping all logic equations */

    pLogMessageFunc(L"Dumping all logic equations.");

    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        nPin = nOutputPinsPAL24[ulOutputPinIndex];
        nMappedPin = pnMappedPins[ulOutputPinIndex];

        /* Output enable */

        if (UtPALIsOutputEnablePresent(nMappedPin, pPALData))
        {
            StringCchPrintfW(cMessage, MArrayLen(cMessage),
                             L"Dumping logic equations for the output enable of pin %d.",
                             nMappedPin);

            pLogMessageFunc(cMessage);
            pLogMessageFunc(pPinTerms[ulOutputPinIndex].pszOutputEnableEquations);
        }

        /* Output */

        StringCchPrintfW(cMessage, MArrayLen(cMessage),
                         L"Dumping logic equations for the output of pin %d.",
                         nMappedPin);

        pLogMessageFunc(cMessage);
        pLogMessageFunc(pPinTerms[ulOutputPinIndex].pszOutputEquations);
    }

EndMapped:
    for (ulOutputPinIndex = 0; ulOutputPinIndex < ulOutputPinsLen;
         ++ulOutputPinIndex)
    {
        if (pPinTerms[ulOutputPinIndex].bOutputEnableTermsSet == TRUE)
        {
            UtPALFreeMinimizedTerms(CEspressoMinimizer,
                                    pPinTerms[ulOutputPinIndex].ppszOutputEnableTerms);
        }

        if (pPinTerms[ulOutputPinIndex].bOutputTermsSet == TRUE)
        {
            UtPALFreeMinimizedTerms(CEspressoMinimizer,
                                    pPinTerms[ulOutputPinIndex].ppszOutputTerms);
        }
    }

    lFreePinTerms(pPinTerms, ulOutputPinsLen);

    return bResult;
}

static ULONG lTranslateCombinatorialAddress(
  ULONG ulAddress,
  const TPALData* pPALData,
  BOOL bIsPal24)
{
    ULONG ulNewAddress, ulIndex, ulData;
    UINT nPinPAL24;

    ulNewAddress = 0;

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

        if (epdptInput == UtPALDeviceGetDevicePinType(nPinPAL24))
        {
            if ((ulAddress & 0x01) == 0x01)
            {
                ulData = 1 << UtPALDeviceGetDevicePinIndex(nPinPAL24);

                ulNewAddress |= ulData;
            }

            ulAddress >>= 1;
        }
    }

    return ulNewAddress;
}

static BOOL lConfigureCombinatorialPalFuseMap(
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BOOL bIsPal24)
{
    if (pPALData->pConfigFuseMapFunc)
    {
        pPALData->pConfigFuseMapFunc(pbyData);
    }

    /*
        The assumption is made that an output pin is not fed back into another
        term.
    */

    if (!UtPALDeviceRemoveFuseColumnsByPinType(epdptNotUsed, pPALData, pbyData,
                                               ulDataLen, bIsPal24) ||
        !UtPALDeviceRemoveFuseColumnsByPinType(epdptOutput, pPALData, pbyData,
                                               ulDataLen, bIsPal24))
    {
        return FALSE;
    }

    return TRUE;
}

VOID UtPALDeviceCombinatorialRead(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  BOOL bIsPal24)
{
    BOOL bErrorOccurred = FALSE;
    ULONG* pulInputData = NULL;
    TPALDeviceAdapterData* pOutputData = NULL;
    ULONG ulFuseSize, ulAddress, ulTmpAddress, ulTotalAddresses;

    ulTotalAddresses = 1 << UtPALDeviceGetTotalPinTypes(epdptInput);

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulTotalAddresses, edoRead);

    if (!UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        ulDataLen != ulFuseSize ||
        FALSE == UtPALClearFuseMap(pbyData, ulDataLen) ||
        FALSE == lConfigureCombinatorialPalFuseMap(pPALData, pbyData, ulDataLen, bIsPal24))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    pulInputData = (ULONG*)UtAllocMem(ulTotalAddresses * sizeof(ULONG));
    pOutputData = (TPALDeviceAdapterData*)UtAllocMem(ulTotalAddresses * sizeof(TPALDeviceAdapterData));

    if (pulInputData == NULL || pOutputData == NULL)
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOnVccSwitchMsg);

    if (FALSE == UtPALDeviceAdapterZeroInputs())
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    pDeviceIOFuncs->pLogMessageDeviceIOFunc(UtPALDeviceGetConfigDevicePinsMsg());
    pDeviceIOFuncs->pShowMessageDeviceIOFunc(UtPALDeviceGetConfigDevicePinsMsg());

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CInsertPalMsg);

    if (FALSE == UtPALDeviceAdapterEnablePALVcc(TRUE))
    {
        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0;
         bErrorOccurred == FALSE && ulAddress < ulTotalAddresses;
         ++ulAddress)
    {
        ulTmpAddress = lTranslateCombinatorialAddress(ulAddress, pPALData, bIsPal24);

        pulInputData[ulAddress] = ulTmpAddress;

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPALDeviceAdapterWriteData(ulTmpAddress) ||
            FALSE == UtPALDeviceAdapterReadData(&pOutputData[ulAddress]))
        {
            bErrorOccurred = TRUE;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    UtPALDeviceAdapterEnablePALVcc(FALSE);

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    if (TRUE == bErrorOccurred)
    {
        goto End;
    }

    pDeviceIOFuncs->pLogMessageDeviceIOFunc(CTranslatingDataMsg);

    UtPALDeviceDumpOutputData(pulInputData, pOutputData, ulTotalAddresses, TRUE);

    if (!lTranslateDataToCombinatorialPalFuseMap(pDeviceIOFuncs->pContinueDeviceIOFunc,
                                                 pDeviceIOFuncs->pLogMessageDeviceIOFunc,
                                                 pulInputData, pOutputData, ulTotalAddresses,
                                                 pPALData, pbyData, ulDataLen, bIsPal24))
    {
        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CAllDataNotTranslatedMsg);
    }

End:
    if (pulInputData)
    {
        UtFreeMem(pulInputData);
    }

    if (pOutputData)
    {
        UtFreeMem(pOutputData);
    }

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

VOID UtPALDeviceCombinatorialReadAdapter(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  LPCTSTR pszDeviceAdapter,
  UINT* pnMappedPins,
  UINT nMappedPinsLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG* pulInputData = NULL;
    ULONG* pulInputDataMapped = NULL;
    TPALDeviceAdapterData* pOutputData = NULL;
    TPALDeviceAdapterData* pOutputDataMapped = NULL;
    ULONG ulFuseSize, ulAddress, ulTmpAddress, ulTotalAddresses;
    UINT nPin, nShiftAddressTotal;
    WCHAR cAdapterMsg[100];

    ulTotalAddresses = 1 << UtPALDeviceGetTotalPinTypes(epdptInput);

    nPin = 1;
    nShiftAddressTotal = 0;

    while (epdptOutput == UtPALDeviceGetDevicePinType(nPin))
    {
        ++nPin;
        ++nShiftAddressTotal;
    }

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulTotalAddresses, edoRead);

    if (!UtPALGetFuseMapSize(pPALData, &ulFuseSize) ||
        ulDataLen != ulFuseSize ||
        FALSE == UtPALClearFuseMap(pbyData, ulDataLen) ||
        FALSE == lConfigureCombinatorialPalFuseMap(pPALData, pbyData, ulDataLen, TRUE))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    pulInputData = (ULONG*)UtAllocMem(ulTotalAddresses * sizeof(ULONG));
    pulInputDataMapped = (ULONG*)UtAllocMem(ulTotalAddresses * sizeof(ULONG));
    pOutputData = (TPALDeviceAdapterData*)UtAllocMem(ulTotalAddresses * sizeof(TPALDeviceAdapterData));
    pOutputDataMapped = (TPALDeviceAdapterData*)UtAllocMem(ulTotalAddresses * sizeof(TPALDeviceAdapterData));

    if (pulInputData == NULL || pulInputDataMapped == NULL ||
        pOutputData == NULL || pOutputDataMapped == NULL)
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOnVccSwitchMsg);

    if (FALSE == UtPALDeviceAdapterZeroInputs())
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    pDeviceIOFuncs->pLogMessageDeviceIOFunc(UtPALDeviceGetConfigDevicePinsMsg());
    pDeviceIOFuncs->pShowMessageDeviceIOFunc(UtPALDeviceGetConfigDevicePinsMsg());

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CInsertPalMsg);

    if (FALSE == UtPALDeviceAdapterEnablePALVcc(TRUE))
    {
        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0;
        bErrorOccurred == FALSE && ulAddress < ulTotalAddresses;
        ++ulAddress)
    {
        ulTmpAddress = ulAddress << nShiftAddressTotal;

        pulInputData[ulAddress] = ulTmpAddress;

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPALDeviceAdapterWriteData(ulTmpAddress) ||
            FALSE == UtPALDeviceAdapterReadData(&pOutputData[ulAddress]))
        {
            bErrorOccurred = TRUE;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    UtPALDeviceAdapterEnablePALVcc(FALSE);

    if (TRUE == bErrorOccurred)
    {
        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

        UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

        goto End;
    }

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CRemovePalMsg);

    if (FALSE == UtPALDeviceAdapterZeroInputs())
    {
        bErrorOccurred = TRUE;

        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

        UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

        goto End;
    }

    StringCchPrintfW(cAdapterMsg, MArrayLen(cAdapterMsg),
                     L"Please insert the adapter \"%s\" into the PAL Cracker.", pszDeviceAdapter);

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(cAdapterMsg);
    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CInsertPalMsg);

    if (FALSE == UtPALDeviceAdapterEnablePALVcc(TRUE))
    {
        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

        UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0;
        bErrorOccurred == FALSE && ulAddress < ulTotalAddresses;
        ++ulAddress)
    {
        ulTmpAddress = ulAddress << nShiftAddressTotal;

        pulInputDataMapped[ulAddress] = ulTmpAddress;

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPALDeviceAdapterWriteData(ulTmpAddress) ||
            FALSE == UtPALDeviceAdapterReadData(&pOutputDataMapped[ulAddress]))
        {
            bErrorOccurred = TRUE;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    UtPALDeviceAdapterEnablePALVcc(FALSE);

    pDeviceIOFuncs->pShowMessageDeviceIOFunc(CTurnOffVccSwitchMsg);

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    if (TRUE == bErrorOccurred)
    {
        goto End;
    }

    pDeviceIOFuncs->pLogMessageDeviceIOFunc(CTranslatingDataMsg);

    UtPALDeviceDumpOutputData(pulInputData, pOutputData, ulTotalAddresses, TRUE);
    UtPALDeviceDumpOutputData(pulInputDataMapped, pOutputDataMapped, ulTotalAddresses, TRUE);

    if (!lTranslateDataToCombinatorialPalFuseMap2(pDeviceIOFuncs->pContinueDeviceIOFunc,
                                                  pDeviceIOFuncs->pLogMessageDeviceIOFunc,
                                                  pulInputData, pOutputData, ulTotalAddresses,
                                                  pulInputDataMapped, pOutputDataMapped, ulTotalAddresses,
                                                  pPALData, pbyData, ulDataLen,
                                                  pnMappedPins, nMappedPinsLen))
    {
        pDeviceIOFuncs->pShowMessageDeviceIOFunc(CAllDataNotTranslatedMsg);
    }

End:
    if (pulInputData)
    {
        UtFreeMem(pulInputData);
    }

    if (pulInputDataMapped)
    {
        UtFreeMem(pulInputDataMapped);
    }

    if (pOutputData)
    {
        UtFreeMem(pOutputData);
    }

    if (pOutputDataMapped)
    {
        UtFreeMem(pOutputDataMapped);
    }

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
