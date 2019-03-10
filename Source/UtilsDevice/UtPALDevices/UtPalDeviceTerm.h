/***************************************************************************/
/*  Copyright (C) 2007-2012 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDeviceTerm_H)
#define UtPalDeviceTerm_H

/*
    Pin Terms
*/

typedef struct tagTPALDevicePinTerms
{
    BOOL bOutputEnableDone;
    BOOL bOutputDone;
    BOOL bOutputEnableTermsSet;
    BOOL bOutputTermsSet;
    LPWSTR* ppszOutputEnableTerms;
    LPWSTR* ppszOutputTerms;
    LPWSTR pszOutputEnableEquations;
    LPWSTR pszOutputEquations;
} TPALDevicePinTerms;

LPWSTR* UtPALDeviceAllocOutputEnableTerms(const ULONG* pulInputData,
                                          const TPALDeviceAdapterData* pOutputData,
                                          ULONG ulOutputDataLen,
                                          UINT* pnInputPins,
                                          ULONG ulInputPinsLen,
                                          UINT nOutputPin);

VOID UtPALDeviceFreeOutputEnableTerms(LPWSTR* ppszTerms);

LPWSTR* UtPALDeviceAllocDataTerms(const ULONG* pulInputData,
                                  const TPALDeviceAdapterData* pOutputData,
                                  ULONG ulOutputDataLen,
                                  UINT* pnInputPins,
                                  ULONG ulInputPinsLen,
                                  UINT nOutputPin,
                                  BOOL bPinDataOn);

VOID UtPALDeviceFreeDataTerms(LPWSTR* ppszTerms);

LPWSTR UtPALDeviceAllocMinimizedTermsToEquations(UINT* pnInputPins,
                                                 UINT nInputPinsLen,
                                                 LPCWSTR* ppszMinimizedTerms);

LPWSTR UtPALDeviceAllocMinimizedTermsWithOutputPinToEquations(UINT* pnInputPins,
                                                              UINT nInputPinsLen,
                                                              LPCWSTR* ppszMinimizedTerms,
                                                              UINT nOutputPin,
                                                              BOOL bOutputPinHigh);

LPWSTR UtPALDeviceAllocOutputEnableAlwaysOnEquation();

ULONG UtPALDeviceGetTotalTerms(LPCWSTR* ppszTerms);

BOOL UtPALDeviceTranslateMinimizedOutputEnableTerms(UINT* pnInputPins,
                                                    UINT nInputPinsCount,
                                                    UINT nOutputPin,
                                                    LPWSTR* ppszMinimizedTerms,
                                                    const TPALData* pPALData,
                                                    LPBYTE pbyFuseData,
                                                    ULONG ulFuseDataLen);

BOOL UtPALDeviceTranslateMinimizedOutputEnableTermsWithOutputPin(UINT* pnInputPins,
                                                                 UINT nInputPinsCount,
                                                                 UINT nOutputPin,
                                                                 LPWSTR* ppszMinimizedTerms,
                                                                 UINT nMergeOutputPin,
                                                                 const TPALData* pPALData,
                                                                 LPBYTE pbyFuseData,
                                                                 ULONG ulFuseDataLen);

BOOL UtPALDeviceTranslateMinimizedDataTerms(UINT* pnInputPins,
                                            UINT nInputPinsCount,
                                            UINT nOutputPin,
                                            LPWSTR* ppszMinimizedTerms,
                                            const TPALData* pPALData,
                                            LPBYTE pbyFuseData,
                                            ULONG ulFuseDataLen);

BOOL UtPALDeviceTranslateMinimizedDataTermsWithOutputPin(UINT* pnInputPins,
                                                         UINT nInputPinsCount,
                                                         UINT nOutputPin,
                                                         LPWSTR* ppszMinimizedTerms,
                                                         UINT nMergeOutputPin,
                                                         const TPALData* pPALData,
                                                         LPBYTE pbyFuseData,
                                                         ULONG ulFuseDataLen);

INT UtPALDeviceFindPinTermIndexUtilizedByEquation(LPCWSTR* ppszTerms,
                                                  const TPALDevicePinTerms* pPinTerms,
                                                  ULONG ulPinTermsLen);

#endif /* end of UtPalDeviceTerm_H */

/***************************************************************************/
/*  Copyright (C) 2007-2012 Kevin Eshbach                                  */
/***************************************************************************/
