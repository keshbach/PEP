/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPAL_H)
#define UtPAL_H

#include <Includes/UtExternC.h>

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#define UTPALAPI __stdcall

/*
    Product term minimizer algorithms
*/

#define CQuine_McCluskyMinimizer 0x01
#define CEspressoMinimizer       0x02

typedef struct tagTPALPinDefined
{
    UINT nPin;
    BOOL bDefined;
} TPALPinDefined;

MExternC BOOL UTPALAPI UtPALInitialize(VOID);

MExternC BOOL UTPALAPI UtPALUninitialize(VOID);

MExternC BOOL UTPALAPI UtPALClearFuseMap(LPBYTE pbyData,
                                         ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALGetFuseMapSize(const TPALData* pPALData,
                                           PULONG pulFuseSize);

MExternC BOOL UTPALAPI UtPALBlowFuse(ULONG nFuseRow,
                                     ULONG nFuseColumn,
                                     const TPALData* pPALData,
                                     LPBYTE pbyData,
                                     ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALBlowFuseRange(ULONG nStartFuseRow,
                                          ULONG nStartFuseColumn,
                                          ULONG nEndFuseRow,
                                          ULONG nEndFuseColumn,
                                          const TPALData* pPALData,
                                          LPBYTE pbyData,
                                          ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALBlowFuseRow(ULONG nFuseRow,
                                        const TPALData* pPALData,
                                        LPBYTE pbyData,
                                        ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALBlowFuseRows(ULONG nStartFuseRow,
                                         ULONG nEndFuseRow,
                                         const TPALData* pPALData,
                                         LPBYTE pbyData,
                                         ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALBlowFuseColumn(ULONG nFuseColumn,
                                           const TPALData* pPALData,
                                           LPBYTE pbyData,
                                           ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALBlowFuseColumns(ULONG nStartFuseColumn,
                                            ULONG nEndFuseColumn,
                                            const TPALData* pPALData,
                                            LPBYTE pbyData,
                                            ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALRestoreFuse(ULONG nFuseRow,
                                        ULONG nFuseColumn,
                                        const TPALData* pPALData,
                                        LPBYTE pbyData,
                                        ULONG ulDataLen);

MExternC BOOL UTPALAPI UtPALRestoreFuseRow(ULONG nFuseRow,
                                           const TPALData* pPALData,
                                           LPBYTE pbyData,
                                           ULONG ulDataLen);

MExternC ULONG UTPALAPI UtPALFuseNumberToFuseRow(ULONG nFuseNumber,
                                                 const TPALData* pPALData);

MExternC ULONG UTPALAPI UtPALFuseColumnFromPin(UINT nPin,
                                               BOOL bHighFuseColumn,
                                               const TPALData* pPALData);

MExternC BOOL UTPALAPI UtPALIsFuseColumnPresent(UINT nPin,
                                                const TPALData* pPALData);

MExternC ULONG UTPALAPI UtPALTermCount(UINT nPin,
                                       const TPALData* pPALData);

MExternC ULONG UTPALAPI UtPALTermRowToFuseRow(UINT nPin,
                                              ULONG nTerm,
                                              const TPALData* pPALData);

MExternC BOOL UTPALAPI UtPALIsOutputEnablePresent(UINT nPin,
                                                  const TPALData* pPALData);

MExternC ULONG UTPALAPI UtPALOutputEnableFuseRow(UINT nPin,
                                                 const TPALData* pPALData);

MExternC BOOL UTPALAPI UtPALWriteFuseMapToJEDFile(const TPALData* pPALData,
                                                  LPBYTE pbyData,
                                                  ULONG ulDataLen,
                                                  LPCWSTR pszPALType,
                                                  UINT nPinCount,
                                                  LPCWSTR pszFile);

MExternC BOOL UTPALAPI UtPALFuseMapText(const TPALData* pPALData,
                                        LPBYTE pbyData,
                                        ULONG ulDataLen,
                                        LPCWSTR pszPALType,
                                        UINT nPinCount,
                                        LPWSTR pszText,
                                        PULONG pulTextLen);

MExternC BOOL UTPALAPI UtPALFuseMapSampleText(const TPALData* pPALData,
                                              LPWSTR pszText,
                                              PULONG pulTextLen);

MExternC BOOL UTPALAPI UtPALDevicePinConfigToPinDefined(const TPALData* pPALData,
                                                        const TDevicePinConfig* pDevicePinConfigs,
                                                        ULONG nTotalDevicePinConfigs,
                                                        TPALPinDefined* pPALPinDefined,
                                                        ULONG nTotalPALPinDefined);

MExternC BOOL UTPALAPI UtPALVerifyDevicePinConfig(const TPALData* pPALData,
                                                  const TDevicePinConfig* pDevicePinConfigs,
                                                  ULONG nTotalDevicePinConfigs,
                                                  LPBOOL pbValid);

MExternC LPWSTR* UTPALAPI UtPALAllocMinimizedTerms(INT nMinimizerAlgo,
                                                   LPCWSTR* ppszTerms);

MExternC BOOL UTPALAPI UtPALFreeMinimizedTerms(INT nMinimizerAlgo,
                                               LPWSTR* ppszMinimizedTerms);

MExternC BOOL UTPALAPI UtPALAreSimiliarTermsPresent(LPCWSTR* ppszTerms, LPCWSTR* ppszTermsMatch);

MExternC LPWSTR* UTPALAPI UtPALAllocMergedTerms(LPCWSTR* ppszTerms, LPCWSTR* ppszTermsMatch);

MExternC VOID UTPALAPI UtPALFreeMergedTerms(LPWSTR* ppszTerms);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* end of UtPAL_H */

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
