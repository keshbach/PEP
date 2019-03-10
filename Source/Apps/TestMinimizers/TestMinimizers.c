/***************************************************************************/
/*  Copyright (C) 2012-2018 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <Utils/UtConsole.h>

#include <UtilsDevice/UtPepDevices.h>

#include <UtilsDevice/UtPAL.h>
#include <UtilsDevice/UtPALDefs.h>

#include <Includes/UtMacros.h>

static LPCWSTR l_ppszSampleTerms1[] = {
    L"0111111100",
    TEXT("0111111110"),
    TEXT("0111111101"),
    TEXT("0000000011"),
    TEXT("1000000011"),
    TEXT("0100000011"),
    TEXT("1100000011"),
    TEXT("0010000011"),
    TEXT("1010000011"),
    TEXT("0110000011"),
    TEXT("1110000011"),
    TEXT("0001000011"),
    TEXT("1001000011"),
    TEXT("0101000011"),
    TEXT("1101000011"),
    TEXT("0011000011"),
    TEXT("1011000011"),
    TEXT("0111000011"),
    TEXT("1111000011"),
    TEXT("0000100011"),
    TEXT("1000100011"),
    TEXT("0100100011"),
    TEXT("1100100011"),
    TEXT("0010100011"),
    TEXT("1010100011"),
    TEXT("0110100011"),
    TEXT("1110100011"),
    TEXT("0001100011"),
    TEXT("1001100011"),
    TEXT("0101100011"),
    TEXT("1101100011"),
    TEXT("0011100011"),
    TEXT("1011100011"),
    TEXT("0111100011"),
    TEXT("1111100011"),
    TEXT("0000010011"),
    TEXT("1000010011"),
    TEXT("0100010011"),
    TEXT("1100010011"),
    TEXT("0010010011"),
    TEXT("1010010011"),
    TEXT("0110010011"),
    TEXT("1110010011"),
    TEXT("0001010011"),
    TEXT("1001010011"),
    TEXT("0101010011"),
    TEXT("1101010011"),
    TEXT("0011010011"),
    TEXT("1011010011"),
    TEXT("0111010011"),
    TEXT("1111010011"),
    TEXT("0000110011"),
    TEXT("1000110011"),
    TEXT("0100110011"),
    TEXT("1100110011"),
    TEXT("0010110011"),
    TEXT("1010110011"),
    TEXT("0110110011"),
    TEXT("1110110011"),
    TEXT("0001110011"),
    TEXT("1001110011"),
    TEXT("0101110011"),
    TEXT("1101110011"),
    TEXT("0011110011"),
    TEXT("1011110011"),
    TEXT("0111110011"),
    TEXT("1111110011"),
    TEXT("0000001011"),
    TEXT("1000001011"),
    TEXT("0100001011"),
    TEXT("1100001011"),
    TEXT("0010001011"),
    TEXT("1010001011"),
    TEXT("0110001011"),
    TEXT("1110001011"),
    TEXT("0001001011"),
    TEXT("1001001011"),
    TEXT("0101001011"),
    TEXT("1101001011"),
    TEXT("0011001011"),
    TEXT("1011001011"),
    TEXT("0111001011"),
    TEXT("1111001011"),
    TEXT("0000101011"),
    TEXT("1000101011"),
    TEXT("0100101011"),
    TEXT("1100101011"),
    TEXT("0010101011"),
    TEXT("1010101011"),
    TEXT("0110101011"),
    TEXT("1110101011"),
    TEXT("0001101011"),
    TEXT("1001101011"),
    TEXT("0101101011"),
    TEXT("1101101011"),
    TEXT("0011101011"),
    TEXT("1011101011"),
    TEXT("0111101011"),
    TEXT("1111101011"),
    TEXT("0000011011"),
    TEXT("1000011011"),
    TEXT("0100011011"),
    TEXT("1100011011"),
    TEXT("0010011011"),
    TEXT("1010011011"),
    TEXT("0110011011"),
    TEXT("1110011011"),
    TEXT("0001011011"),
    TEXT("1001011011"),
    TEXT("0101011011"),
    TEXT("1101011011"),
    TEXT("0011011011"),
    TEXT("1011011011"),
    TEXT("0111011011"),
    TEXT("1111011011"),
    TEXT("0000111011"),
    TEXT("1000111011"),
    TEXT("0100111011"),
    TEXT("1100111011"),
    TEXT("0010111011"),
    TEXT("1010111011"),
    TEXT("0110111011"),
    TEXT("1110111011"),
    TEXT("0001111011"),
    TEXT("1001111011"),
    TEXT("0101111011"),
    TEXT("1101111011"),
    TEXT("0011111011"),
    TEXT("1011111011"),
    TEXT("0111111011"),
    TEXT("1111111011"),
    TEXT("0000000111"),
    TEXT("1000000111"),
    TEXT("0100000111"),
    TEXT("1100000111"),
    TEXT("0010000111"),
    TEXT("1010000111"),
    TEXT("0110000111"),
    TEXT("1110000111"),
    TEXT("0001000111"),
    TEXT("1001000111"),
    TEXT("0101000111"),
    TEXT("1101000111"),
    TEXT("0011000111"),
    TEXT("1011000111"),
    TEXT("0111000111"),
    TEXT("1111000111"),
    TEXT("0000100111"),
    TEXT("1000100111"),
    TEXT("0100100111"),
    TEXT("1100100111"),
    TEXT("0010100111"),
    TEXT("1010100111"),
    TEXT("0110100111"),
    TEXT("1110100111"),
    TEXT("0001100111"),
    TEXT("1001100111"),
    TEXT("0101100111"),
    TEXT("1101100111"),
    TEXT("0011100111"),
    TEXT("1011100111"),
    TEXT("0111100111"),
    TEXT("1111100111"),
    TEXT("0000010111"),
    TEXT("1000010111"),
    TEXT("0100010111"),
    TEXT("1100010111"),
    TEXT("0010010111"),
    TEXT("1010010111"),
    TEXT("0110010111"),
    TEXT("1110010111"),
    TEXT("0001010111"),
    TEXT("1001010111"),
    TEXT("0101010111"),
    TEXT("1101010111"),
    TEXT("0011010111"),
    TEXT("1011010111"),
    TEXT("0111010111"),
    TEXT("1111010111"),
    TEXT("0000110111"),
    TEXT("1000110111"),
    TEXT("0100110111"),
    TEXT("1100110111"),
    TEXT("1010110111"),
    TEXT("0110110111"),
    TEXT("1110110111"),
    TEXT("0001110111"),
    TEXT("1001110111"),
    TEXT("0101110111"),
    TEXT("1101110111"),
    TEXT("0011110111"),
    TEXT("1011110111"),
    TEXT("0111110111"),
    TEXT("1111110111"),
    TEXT("0000001111"),
    TEXT("1000001111"),
    TEXT("0100001111"),
    TEXT("1100001111"),
    TEXT("0010001111"),
    TEXT("1010001111"),
    TEXT("0110001111"),
    TEXT("1110001111"),
    TEXT("0001001111"),
    TEXT("1001001111"),
    TEXT("0101001111"),
    TEXT("1101001111"),
    TEXT("0011001111"),
    TEXT("1011001111"),
    TEXT("0111001111"),
    TEXT("1111001111"),
    TEXT("0000101111"),
    TEXT("1000101111"),
    TEXT("0100101111"),
    TEXT("1100101111"),
    TEXT("0010101111"),
    TEXT("1010101111"),
    TEXT("0110101111"),
    TEXT("1110101111"),
    TEXT("0001101111"),
    TEXT("1001101111"),
    TEXT("0101101111"),
    TEXT("1101101111"),
    TEXT("0011101111"),
    TEXT("1011101111"),
    TEXT("0111101111"),
    TEXT("1111101111"),
    TEXT("0000011111"),
    TEXT("1000011111"),
    TEXT("0100011111"),
    TEXT("1100011111"),
    TEXT("0010011111"),
    TEXT("1010011111"),
    TEXT("0110011111"),
    TEXT("1110011111"),
    TEXT("0001011111"),
    TEXT("1001011111"),
    TEXT("0101011111"),
    TEXT("1101011111"),
    TEXT("0011011111"),
    TEXT("1011011111"),
    TEXT("0111011111"),
    TEXT("1111011111"),
    TEXT("0000111111"),
    TEXT("1000111111"),
    TEXT("0100111111"),
    TEXT("1100111111"),
    TEXT("0010111111"),
    TEXT("1010111111"),
    TEXT("0110111111"),
    TEXT("1110111111"),
    TEXT("0001111111"),
    TEXT("1001111111"),
    TEXT("0101111111"),
    TEXT("1101111111"),
    TEXT("0011111111"),
    TEXT("1011111111"),
    TEXT("0111111111"),
    TEXT("1111111111"),
    NULL};

static BOOL WINAPI lHandlerRoutine(
  DWORD dwCtrlType)
{
    wprintf(L"Console ctrl handler invoked.\n");

    switch (dwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            break;
        case CTRL_CLOSE_EVENT:
            break;
    }

    return TRUE;
}

static LPCWSTR lGetAlgoDescription(
  INT nAlgo)
{
    switch (nAlgo)
    {
        case CQuine_McCluskyMinimizer:
            return L"Quine\\McClusky Minimizer";
        case CEspressoMinimizer:
            return L"Espresso Minimizer";
    }

    return L"Unknown Minimizer";
}

static void lPrintTerms(
  INT nAlgo,
  LPWSTR* ppszTerms)
{
    INT nIndex;

    wprintf(L"\n");
    wprintf(L"%s\n", lGetAlgoDescription(nAlgo));
    wprintf(L"----------------------------------------\n");

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        wprintf(L"%s\n", ppszTerms[nIndex]);
    }

    wprintf(L"\n");
}

static void lRunMinimizerTest(
  LPCWSTR* ppszTestTerms)
{
    INT nAlgos[] = {CQuine_McCluskyMinimizer,
                    CEspressoMinimizer};
    LPWSTR* ppszTerms;
    INT nIndex;

    for (nIndex = 0; nIndex < MArrayLen(nAlgos); ++nIndex)
    {
        ppszTerms = UtPALAllocMinimizedTerms(nAlgos[nIndex], ppszTestTerms);

        lPrintTerms(nAlgos[nIndex], ppszTerms);

        UtPALFreeMinimizedTerms(nAlgos[nIndex], ppszTerms);
    }
}

static int lRunTest1()
{
    if (UtPALInitialize() == FALSE)
    {
        wprintf(L"UtPAL could not be initialized.\n");

        return -1;
    }

    lRunMinimizerTest(l_ppszSampleTerms1);

    if (UtPALUninitialize() == FALSE)
    {
        wprintf(L"UtPAL could not be uninitialized.\n");
    }

    return 0;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"Espresso Logic Minimizer\n");
    wprintf(L"Copyright (c) 1988, 1989, Regents of the University of California.\n");
    wprintf(L"All rights reserved.\n");
    wprintf(L"\n");
    wprintf(L"TestMinimizers [/test1]\n");
    wprintf(L"\n");
    wprintf(L"    /test1 - Run minimization test 1\n");
    wprintf(L"\n");

    return -1;
}

int _cdecl wmain(int argc, WCHAR* argv[])
{
    int result;

    argv;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    if (argc == 2)
    {
        if (lstrcmpi(argv[1], L"/test1") == 0)
        {
            result = lRunTest1();
        }
        else
        {
            result = lDisplayHelp();
        }
    }
    else
    {
        result = lDisplayHelp();
    }

    SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

    return result;
}

/***************************************************************************/
/*  Copyright (C) 2012-2018 Kevin Eshbach                                  */
/***************************************************************************/
