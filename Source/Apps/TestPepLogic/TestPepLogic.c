/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <UtilsPep/UtPepLogic.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

#include <Apps/Includes/UtCommandLineParser.inl>

#define CMaxDecimalNumberDigits 10
#define CMaxHexNumberDigits 8

typedef int (*TTestWithValueFunc)(int);
typedef int (*TTestFunc)(void);

typedef struct tagTTestData
{
    LPCWSTR pszTestCommand;
    TTestWithValueFunc TestWithValueFunc;
    TTestFunc TestFunc;
} TTestData;

static int lRunProgramTest(int nAddress);
static int lRunReadTest(int nAddress);
static int lRunAddressTest(int nAddress);

static int lRunDataTest(int nAddress);

static int lRunVcc5Test(void);
static int lRunVcc6Test(void);
static int lRunVEN08Test(void);
static int lRunNegVP5Test(void);
static int lRunVpp12Test(void);
static int lRunVpp21Test(void);
static int lRunVpp25Test(void);

static TTestData l_TestData[] = {
    {L"read",    lRunReadTest,    NULL},
    {L"program", lRunProgramTest, NULL},
    {L"address", lRunAddressTest, NULL},
    {L"data",    lRunDataTest,    NULL},
    {L"vcc5",    NULL,            lRunVcc5Test},
    {L"vcc6",    NULL,            lRunVcc6Test},
    {L"ven08",   NULL,            lRunVEN08Test},
    {L"~vp5",    NULL,            lRunNegVP5Test},
    {L"vpp12",   NULL,            lRunVpp12Test},
    {L"vpp21",   NULL,            lRunVpp21Test},
    {L"vpp25",   NULL,            lRunVpp25Test},
    {NULL,       NULL,            NULL} };

static TUtPepLogicData l_PepLogicData;

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

static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(
  _Out_ PBOOLEAN pbValue)
{
    *pbValue = FALSE;

    return TRUE;
}

static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(
  _In_ PUINT8 pnData,
  _In_ UINT32 nDataLen,
  _In_ UINT32 nWaitNanoSeconds)
{
    UINT8 nDataIndex;

    for (nDataIndex = 0; nDataIndex < nDataLen; ++nDataIndex)
    {
        wprintf(L"TestPepLogic: Write Port Data - 0x%X.\n", pnData[nDataIndex]);
    }

    return TRUE;
}

static BOOL lParseDecimalNumber(
  LPCWSTR pszNumber,
  int* pnDecimalNumber)
{
    int nNumberLen = lstrlenW(pszNumber);
    int nMultiplier;

    *pnDecimalNumber = 0;

    for (int nNumberPos = 0; nNumberPos < nNumberLen; ++nNumberPos)
    {
        if (pszNumber[nNumberPos] < L'0' || pszNumber[nNumberPos] > L'9')
        {
            return FALSE;
        }
    }

    if (nNumberLen > CMaxDecimalNumberDigits)
    {
        return FALSE;
    }

    nMultiplier = 10;

    for (int nNumberPos = 1; nNumberPos < nNumberLen; ++nNumberPos)
    {
        nMultiplier *= 10;
    }

    for (int nNumberPos = 0; nNumberPos < nNumberLen; ++nNumberPos)
    {
        *pnDecimalNumber += ((pszNumber[nNumberPos] - L'0') * nMultiplier);

        nMultiplier /= 10;
    }

    return TRUE;
}

static BOOL lParseHexNumber(
  LPCWSTR pszNumber,
  int* pnHexNumber)
{
    int nNumberLen = lstrlenW(pszNumber);
    int nValue;

    *pnHexNumber = 0;

    if (nNumberLen < 3 || pszNumber[0] != L'0' || pszNumber[1] != L'x')
    {
        return FALSE;
    }

    for (int nNumberPos = 2; nNumberPos < nNumberLen; ++nNumberPos)
    {
        if (!((pszNumber[nNumberPos] >= L'0' && pszNumber[nNumberPos] <= L'9') ||
            (pszNumber[nNumberPos] >= L'A' && pszNumber[nNumberPos] <= L'F') ||
            (pszNumber[nNumberPos] >= L'a' && pszNumber[nNumberPos] <= L'f')))
        {
            return FALSE;
        }
    }

    if (nNumberLen > CMaxHexNumberDigits + 2)
    {
        return FALSE;
    }

    for (int nNumberPos = 2; nNumberPos < nNumberLen; ++nNumberPos)
    {
        if (pszNumber[nNumberPos] >= '0' && pszNumber[nNumberPos] <= '9')
        {
            nValue = pszNumber[nNumberPos] - '0';
        }
        else if (pszNumber[nNumberPos] >= 'a' && pszNumber[nNumberPos] <= 'f')
        {
            nValue = pszNumber[nNumberPos] - 'a' + 10;
        }
        else if (pszNumber[nNumberPos] >= 'A' && pszNumber[nNumberPos] <= 'F')
        {
            nValue = pszNumber[nNumberPos] - 'A' + 10;
        }
        else
        {
            nValue = 0;
        }

        *pnHexNumber = (*pnHexNumber << 4) | (nValue & 0xF);
    }

    return TRUE;
}

static BOOL lParseNumber(
  LPCWSTR pszNumber,
  int* pnNumber)
{
    if (lParseDecimalNumber(pszNumber, pnNumber) ||
        lParseHexNumber(pszNumber, pnNumber))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL lFindTestRun(
  LPCWSTR pszTestCommand,
  TTestWithValueFunc* pTestWithValueFunc,
  TTestFunc* pTestFunc)
{
    int nIndex = 0;

    *pTestWithValueFunc = NULL;
    *pTestFunc = NULL;

    while (l_TestData[nIndex].pszTestCommand != NULL)
    {
        if (lstrcmpi(pszTestCommand, l_TestData[nIndex].pszTestCommand) == 0)
        {
            *pTestWithValueFunc = l_TestData[nIndex].TestWithValueFunc;
            *pTestFunc = l_TestData[nIndex].TestFunc;

            return TRUE;
        }

        ++nIndex;
    }

    return FALSE;
}

static int lRunProgramTest(
  int nAddress)
{
    /*int nResult = 0;
    ULONG ulAddress = nAddress;
    BYTE byData[10] = { 255, 128, 64, 32, 16, 8, 4, 2, 1, 0 };
    ULONG ulIndex;

    wprintf(L"TestPepCtrl: Running the program byte test.\n");

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerWriteMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the programmer's write mode.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: Data Program at 0x%X: ", ulAddress);

    for (ulIndex = 0; ulIndex < sizeof(byData); ++ulIndex)
    {
        wprintf(L"%02X ", byData[ulIndex]);
    }

    wprintf(L"\n");

    if (UtPepCtrlProgramData(ulAddress, byData, sizeof(byData)))
    {
        nResult = 1;
    }
    else
    {
        wprintf(L"TestPepCtrl: Could not write data.\n");
    }

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode))
    {
        wprintf(L"TestPepCtrl: Could not clear the programmer's mode.\n");

        return 0;
    }

    return nResult;*/
    return 0;
}

static int lRunReadTest(
  int nAddress)
{
    /*int nResult = 0;
    ULONG ulAddress = nAddress;
    BYTE byData[10] = { 0 };
    ULONG ulIndex;

    wprintf(L"TestPepCtrl: Running the read byte test.\n");

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the programmer's read mode.\n");

        return 0;
    }

    if (!UtPepCtrlReadData(ulAddress, byData, sizeof(byData)))
    {
        wprintf(L"TestPepCtrl: Could not read data.\n");

        goto EndTest;
    }

    wprintf(L"TestPepCtrl: Data Read at 0x%08X: ", ulAddress);

    for (ulIndex = 0; ulIndex < sizeof(byData); ++ulIndex)
    {
        wprintf(L"%02X ", (ULONG)byData[ulIndex]);
    }

    wprintf(L"\n");

    nResult = 1;

EndTest:
    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode))
    {
        wprintf(L"TestPepCtrl: Could not clear the programmer's mode.\n");

        return 0;
    }

    return nResult;*/
    return 0;
}

static int lRunAddressTest(
  int nAddress)
{
    int nResult = 0;

    wprintf(L"TestPepLogic: Running the address test. (All dips should be set to off.)\n");

    wprintf(L"TestPepLogic: Setting programmer to read mode.\n");

    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerReadMode))
    {
        wprintf(L"TestPepLogic: Could not enable the programmer's read mode.\n");

        return 0;
    }

    wprintf(L"TestPepLogic: Setting programmer address.\n");

    if (!UtPepLogicSetAddress(&l_PepLogicData, nAddress))
    {
        wprintf(L"TestPepCtrl: Could not set the address.\n");

        goto EndTest;
    }

    nResult = 1;

EndTest:
    wprintf(L"TestPepLogic: Setting programmer to none mode.\n");

    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerNoneMode))
    {
        wprintf(L"TestPepLogic: Could not enable the programmer's none mode.\n");

        return 0;
    }

    return nResult;
}

static int lRunDataTest(
  int nAddress)
{
    /*int nResult = 0;
    BYTE byData[1];

    wprintf(L"TestPepCtrl: Running the data test.\n");

    byData[0] = (BYTE)nAddress;

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerWriteMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the programmer's write mode.\n");

        return 0;
    }

    if (!UtPepCtrlProgramData(0, byData, sizeof(byData)))
    {
        wprintf(L"TestPepCtrl: Could not output the data.\n");

        goto EndTest;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    nResult = 1;

EndTest:
    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode))
    {
        wprintf(L"TestPepCtrl: Could not clear the programmer's mode.\n");

        return 0;
    }

    return nResult;*/
    return 0;
}

static int lRunVcc5Test(void)
{
    int nResult = 0;

    wprintf(L"TestPepLogic: Outputting +5VDC on Pin 32.\n");

    if (!UtPepLogicSetVccMode(&l_PepLogicData, CUtPepLogic5VDCMode))
    {
        wprintf(L"TestPepLogic: Could not enable the +5VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepLogic: Setting programmer to read mode.\n");

    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerReadMode))
    {
        wprintf(L"TestPepLogic: Could not enable the programmer's read mode.\n");

        goto EndTest;
    }

    nResult = 1;

EndTest:
    wprintf(L"TestPepLogic: Setting programmer to none mode.\n");

    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerNoneMode))
    {
        wprintf(L"TestPepLogic: Could not enable the programmer's none mode.\n");

        return 0;
    }

    return nResult;
}

static int lRunVcc6Test(void)
{
    int nResult = 0;

    wprintf(L"TestPepLogic: Outputting +6.25VDC on Pin 32.\n");

    if (!UtPepLogicSetVccMode(&l_PepLogicData, CUtPepLogic625VDCMode))
    {
        wprintf(L"TestPepLogic: Could not enable the +6.25VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepLogic: Setting programmer to read mode.\n");

    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerReadMode))
    {
        wprintf(L"TestPepLogic: Could not enable the programmer's read mode.\n");

        goto EndTest;
    }

    nResult = 1;

EndTest:
    wprintf(L"TestPepLogic: Setting programmer to none mode.\n");

    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerNoneMode))
    {
        wprintf(L"TestPepLogic: Could not enable the programmer's none mode.\n");

        return 0;
    }

    return nResult;
}

static int lRunVEN08Test(void)
{
    int nResult = 0;

    wprintf(L"TestPepLogic: Outputting +12VDC on Pin 23.\n");
    wprintf(L"TestPepLogic: Outputting -5VDC on Pin 25.\n");

    if (!UtPepLogicSetPinPulseMode(&l_PepLogicData, CUtPepLogicPinPulse1Mode))
    {
        wprintf(L"TestPepLogic: Could not enable the VEN08 pin mode.\n");

        return 0;
    }

    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerReadMode))
    {
        wprintf(L"TestPepLogic: Could not enable the programmer's read mode.\n");

        goto EndTest;
    }

    nResult = 1;

EndTest:
    if (!UtPepLogicSetProgrammerMode(&l_PepLogicData, CUtPepLogicProgrammerNoneMode))
    {
        wprintf(L"TestPepLogic: Could not clear the programmer's mode.\n");

        return 0;
    }

    return nResult;
}

static int lRunNegVP5Test(void)
{
    /*int nResult = 0;

    wprintf(L"TestPepCtrl: Outputting +5VDC on Pin 25.\n");

    if (!UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse2Mode))
    {
        wprintf(L"TestPepCtrl: Could not enable the ~VP5 pin mode.\n");

        return 0;
    }

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the programmer's read mode.\n");

        goto EndTest;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    nResult = 1;

EndTest:
    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode))
    {
        wprintf(L"TestPepCtrl: Could not clear the programmer's mode.\n");

        return 0;
    }

    return nResult;*/
    return 0;
}

static int lRunVpp12Test(void)
{
    /*wprintf(L"TestPepCtrl: Outputting +12VDC on the Vpp Accessory Connector.\n");

    if (!UtPepCtrlSetVppMode(eUtPepCtrl12VDCVppMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +12VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    return 1;*/
    return 0;
}

static int lRunVpp21Test(void)
{
    /*wprintf(L"TestPepCtrl: Outputting +21VDC on the Vpp Accessory Connector.\n");

    if (!UtPepCtrlSetVppMode(eUtPepCtrl21VDCVppMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +21VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    return 1;*/
    return 0;
}

static int lRunVpp25Test(void)
{
    /*wprintf(L"TestPepCtrl: Outputting +25VDC on the Vpp Accessory Connector.\n");

    if (!UtPepCtrlSetVppMode(eUtPepCtrl25VDCVppMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +25VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    return 1;*/
    return 0;
}

static int lRunTest(
  TTestWithValueFunc TestWithValueFunc,
  TTestFunc TestFunc,
  int nAddress)
{
    int nResult = 0;

    l_PepLogicData.pvLogicContext = UtPepLogicAllocLogicContext();
    l_PepLogicData.pReadBitPortFunc = lPepLogicReadBitPort;
    l_PepLogicData.pWritePortFunc = lPepLogicWritePort;

    if (TestWithValueFunc)
    {
        nResult = TestWithValueFunc(nAddress);
    }
    else if (TestFunc)
    {
        nResult = TestFunc();
    }

    UtPepLogicFreeLogicContext(l_PepLogicData.pvLogicContext);

    return nResult;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"TestPepLogic [/test address \"Address\"]\n");
    wprintf(L"             [/test data \"Byte\"]\n");
    wprintf(L"             [/test (read | program) \"Address\"]\n");
    wprintf(L"             [/test (vcc5 | vcc6)]\n");
    wprintf(L"             [/test (ven08 | ~vp5)]\n");
    wprintf(L"             [/test (we08 | vpp16 | vpp32 | vpp64)]\n");
    wprintf(L"\n");
    wprintf(L"    Programmer Tests\n");
    wprintf(L"    /test\n");
    wprintf(L"        read         - Run the read byte test\n");
    wprintf(L"        program      - Run the program byte test\n");
    wprintf(L"        address      - Selects the given address\n");
    wprintf(L"        data         - Selects the byte to output\n");
    wprintf(L"        \"Address\"    - Address to start with/select\n");
    wprintf(L"                       (Enter 0x1f for hexadecimal or 39 for decimal)\n");
    wprintf(L"        \"Byte\"       - 8-bit value to output\n");
    wprintf(L"                       (Enter 0x1f for hexadecimal or 39 for decimal)\n");
    wprintf(L"\n");
    wprintf(L"    VCC Mode Tests (Dips: 1 - On, 2 to 8 - Off)\n");
    wprintf(L"    /test\n");
    wprintf(L"        vcc5         - Set Pin 32 to +5VDC\n");
    wprintf(L"        vcc6         - Set Pin 32 to +6.25VDC\n");
    wprintf(L"\n");
    wprintf(L"    Pin Read Mode Tests (Dips: 1 - On, 2 to 8 - Off)\n");
    wprintf(L"    /test\n");
    wprintf(L"        ven08        - Set the VEN08 pin mode\n");
    wprintf(L"        ~vp5         - Set the ~VP5 pin mode\n");
    wprintf(L"\n");
    wprintf(L"    Vpp Mode Tests (Dips: 1 - On, 2 to 8 - Off)\n");
    wprintf(L"    /test\n");
    wprintf(L"        vpp12        - Set Vpp Accessory Connector to +12VDC\n");
    wprintf(L"        vpp21        - Set Vpp Accessory Connector to +21VDC\n");
    wprintf(L"        vpp25        - Set Vpp Accessory Connector to +25VDC\n");
    wprintf(L"\n");

    return -1;
}

int _cdecl wmain(int argc, WCHAR* argv[])
{
    int nValue = 0;
    TTestWithValueFunc TestWithValueFunc;
    TTestFunc TestFunc;
    int nResult, nTotalNewArgs;
    LPCWSTR* ppszNewArgs;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    UtCommandLineParserInitialize(argc, argv);

    nTotalNewArgs = UtCommandLineParserGetTotalArguments();
    ppszNewArgs = UtCommandLineParserGetArguments();

    if ((nTotalNewArgs == 3 || nTotalNewArgs == 4) && lstrcmpiW(ppszNewArgs[1], L"/test") == 0)
    {
        if (!lFindTestRun(ppszNewArgs[2], &TestWithValueFunc, &TestFunc))
        {
            wprintf(L"TestPepLogic: Unrecognized test command.\n");

            nResult = -4;

            goto End;
        }

        if (TestWithValueFunc)
        {
            if (nTotalNewArgs != 4)
            {
                wprintf(L"TestPepLogic: Value argument missing.\n");

                nResult = -4;

                goto End;
            }

            if (!lParseNumber(ppszNewArgs[3], &nValue))
            {
                wprintf(L"TestPepLogic: Invalid number provided.\n");

                nResult = -4;

                goto End;
            }
        }

        nResult = lRunTest(TestWithValueFunc, TestFunc, nValue);
    }
    else
    {
        nResult = lDisplayHelp();
    }

End:
    UtCommandLineParserUninitialize();

    SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

    return nResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
