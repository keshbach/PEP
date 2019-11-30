/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepAnalyze_H)
#define UtPepAnalyze_H

#define UTPEPANALYZEAPI __stdcall

/* Bit Stuck flags */

#define CPepAnalyzeBitStuckHigh0 0x0001
#define CPepAnalyzeBitStuckHigh1 0x0002
#define CPepAnalyzeBitStuckHigh2 0x0004
#define CPepAnalyzeBitStuckHigh3 0x0008
#define CPepAnalyzeBitStuckHigh4 0x0010
#define CPepAnalyzeBitStuckHigh5 0x0020
#define CPepAnalyzeBitStuckHigh6 0x0040
#define CPepAnalyzeBitStuckHigh7 0x0080

#define CPepAnalyzeBitStuckLow0  0x0100
#define CPepAnalyzeBitStuckLow1  0x0200
#define CPepAnalyzeBitStuckLow2  0x0400
#define CPepAnalyzeBitStuckLow3  0x0800
#define CPepAnalyzeBitStuckLow4  0x1000
#define CPepAnalyzeBitStuckLow5  0x2000
#define CPepAnalyzeBitStuckLow6  0x4000
#define CPepAnalyzeBitStuckLow7  0x8000

#if defined(__cplusplus)
extern "C"
{
#endif

BOOL UTPEPANALYZEAPI UtPepAnalyzeIsEmpty(_In_ LPCBYTE pbyData, _In_ UINT32 nDataLen, _Out_ LPBOOL pbIsEmpty);

BOOL UTPEPANALYZEAPI UtPepAnalyzeCheckStuckBits(_In_ LPCBYTE pbyData, _In_ UINT32 nDataLen, _Out_ UINT32* pnBitStucks);

BOOL UTPEPANALYZEAPI UtPepAnalyzeIsOverdump(_In_ LPCBYTE pbyData, _In_ UINT32 nDataLen, _Out_ LPBOOL pbIsOverdump);

#if defined(__cplusplus)
};
#endif

#endif

/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/
