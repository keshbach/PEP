/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <UtilsPep/UtPepAnalyze.h>

BOOL UTPEPANALYZEAPI UtPepAnalyzeIsEmpty(
  _In_ LPCBYTE pbyData,
  _In_ UINT32 nDataLen,
  _Out_ LPBOOL pbIsEmpty)
{
	*pbIsEmpty = TRUE;

	for (UINT32 nIndex = 0; nIndex < nDataLen; ++nIndex)
	{
		if (pbyData[nIndex] != 0xFF)
		{
			*pbIsEmpty = FALSE;

			break;
		}
	}

	return TRUE;
}

BOOL UTPEPANALYZEAPI UtPepAnalyzeCheckStuckBits(
  _In_ LPCBYTE pbyData,
  _In_ UINT32 nDataLen,
  _Out_ UINT32* pnBitsStuck)
{
	UINT32 nHighBitSetMask = 0;
	UINT32 nLowBitSetMask = 0;
	UINT32 nMask;

	*pnBitsStuck = 0;

	for (UINT32 nMaskPos = 0; nMaskPos < 8; ++nMaskPos)
	{
		nMask = 0x01 << nMaskPos;

		for (UINT32 nIndex = 0; nIndex < nDataLen; ++nIndex)
		{
			if (pbyData[nIndex] & nMask)
			{
				nHighBitSetMask |= nMask;
            }
			else
			{
				nLowBitSetMask |= nMask;
			}

			if ((nHighBitSetMask & nMask) && (nLowBitSetMask & nMask))
			{
				break;
			}
		}

		if ((nHighBitSetMask & nMask) && (nLowBitSetMask & nMask) == 0)
		{
			*pnBitsStuck |= nMask;
		}
		else if ((nHighBitSetMask & nMask) == 0 && (nLowBitSetMask & nMask))
		{
			*pnBitsStuck |= (nMask << 8);
		}
	}

	return TRUE;
}

BOOL UTPEPANALYZEAPI UtPepAnalyzeIsOverdump(
  _In_ LPCBYTE pbyData,
  _In_ UINT32 nDataLen,
  _Out_ LPBOOL pbIsOverdump)
{
	UINT32 nHalfDataLen = nDataLen / 2;

	for (UINT32 nIndex = 0; nIndex < nHalfDataLen; ++nIndex)
	{
		if (pbyData[nIndex] != pbyData[nIndex + nHalfDataLen])
		{
			*pbIsOverdump = FALSE;

			return TRUE;
		}
	}

	*pbIsOverdump = TRUE;

	return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/
