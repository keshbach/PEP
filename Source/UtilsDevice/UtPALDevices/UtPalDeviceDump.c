/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <assert.h>

#include "UtPalDeviceAdapter.h"
#include "UtPalDeviceDump.h"

#include <Includes/UtMacros.h>

static VOID lPrintNumberToBinary(
  ULONG ulNumber,
  ULONG ulPrecision)
{
#if defined(NDEBUG)
    ulNumber;
    ulPrecision;
#else
    ULONG ulIndex;
    WCHAR cBuffer[33];

    assert(ulPrecision <= MArrayLen(cBuffer) - 1);

    for (ulIndex = 0; ulIndex < MArrayLen(cBuffer) - 1 && ulIndex < ulPrecision;
         ++ulIndex)
    {
        cBuffer[ulIndex] = (ulNumber & 0x01) ? L'1' : L'0';

        ulNumber = ulNumber >> 1;
    }

    cBuffer[ulIndex] = 0;

    OutputDebugStringW(cBuffer);
#endif
}

VOID UtPALDeviceDumpTerms(
  LPCWSTR* ppszTerms)
{
#if defined(NDEBUG)
    ppszTerms;
#else
    ULONG ulIndex;

    OutputDebugStringW(L"Terms\n");
    OutputDebugStringW(L"------------\n");

    for (ulIndex = 0; ppszTerms[ulIndex] != NULL; ++ulIndex)
    {
        OutputDebugStringW(ppszTerms[ulIndex]);
        OutputDebugStringW(L"\n");
    }
#endif
}

VOID UtPALDeviceDumpOutputData(
  ULONG* pulInputData,
  TPALDeviceAdapterData* pOutputData,
  ULONG ulTotalData,
  BOOL bIncludeHeader)
{
#if defined(NDEBUG)
    pulInputData;
    pOutputData;
    ulTotalData;
    bIncludeHeader;
#else
    ULONG ulCurData;

    if (bIncludeHeader)
    {
        OutputDebugStringW(L"Index                               Input Data                          Output Data     Pin Active\n");
        OutputDebugStringW(L"--------------------------------    --------------------------------    ------------    ------------\n");
    }

    for (ulCurData = 0; ulCurData < ulTotalData; ++ulCurData)
    {
        lPrintNumberToBinary(ulCurData, 32);

        OutputDebugStringW(L"    ");

        lPrintNumberToBinary(pulInputData[ulCurData], 32);

        OutputDebugStringW(L"    ");

        lPrintNumberToBinary(pOutputData[ulCurData].wOutputData, 12);

        OutputDebugStringW(L"    ");

        lPrintNumberToBinary(pOutputData[ulCurData].wPinActiveData, 12);

        OutputDebugStringW(L"\n");
    }
#endif
}

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
