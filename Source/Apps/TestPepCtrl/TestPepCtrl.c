/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <Config/UtPepCtrl.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

typedef int (*TTestWithAddressFunc)(int);
typedef int (*TTestFunc)(void);

typedef struct tagTTestData
{
    LPCWSTR pszTestCommand;
    TTestWithAddressFunc TestWithAddressFunc;
    TTestFunc TestFunc;
} TTestData;

static int lRunAllTest(int nAddress);
static int lRunProgramTest(int nAddress);
static int lRunReadTest(int nAddress);
static int lRunAddressTest(int nAddress);

static int lRunDataTest(int nAddress);

static int lRunStatusTest(void);
static int lRunDeviceChangeTest(void);

static int lRunVcc5Test(void);
static int lRunVcc6Test(void);
static int lRunVEN08Test(void);
static int lRunNegVP5Test(void);
static int lRunVpp12Test(void);
static int lRunVpp21Test(void);
static int lRunVpp25Test(void);

static TTestData l_TestData[] = {
    {L"all",          lRunAllTest,     NULL},
    {L"read",         lRunReadTest,    NULL},
    {L"program",      lRunProgramTest, NULL},
    {L"address",      lRunAddressTest, NULL},
    {L"data",         lRunDataTest,    NULL},
    {L"status",       NULL,            lRunStatusTest},
    {L"devicechange", NULL,            lRunDeviceChangeTest},
    {L"vcc5",         NULL,            lRunVcc5Test},
    {L"vcc6",         NULL,            lRunVcc6Test},
    {L"ven08",        NULL,            lRunVEN08Test},
    {L"~vp5",         NULL,            lRunNegVP5Test},
    {L"vpp12",        NULL,            lRunVpp12Test},
    {L"vpp21",        NULL,            lRunVpp21Test},
    {L"vpp25",        NULL,            lRunVpp25Test},
    {NULL,            NULL,            NULL} };

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

static VOID UTPEPCTRLAPI lDeviceChangeFunc(
  EUtPepCtrlDeviceChange DeviceChange)
{
    switch (DeviceChange)
    {
        case eUtPepCtrlDeviceArrived:
            wprintf(L"TestPepCtrl: ** Device arrived **\n");
            break;
        case eUtPepCtrlDeviceRemoved:
            wprintf(L"TestPepCtrl: ** Device removed **\n");
            break;
        default:
            wprintf(L"TestPepCtrl: ** Unknown device change **\n");
            break;
    }
}

static int lParseNumber(
  LPCWSTR pszNumber)
{
    int nNumberLen = lstrlenW(pszNumber);
    int nNumber = 0;
    int nMultiplier = 1;
    int nNumberPos;

    if (nNumberLen > 2 && (pszNumber[0] == L'0' && pszNumber[2] == L'x'))
    {
        return -1;
    }
    else
    {
        for (nNumberPos = 1; nNumberPos < nNumberLen; ++nNumberPos)
        {
            nMultiplier *= 10;
        }

        for (nNumberPos = 0; nNumberPos < nNumberLen; ++nNumberPos)
        {
            switch (pszNumber[nNumberPos])
            {
                case L'0':
                case L'1':
                case L'2':
                case L'3':
                case L'4':
                case L'5':
                case L'6':
                case L'7':
                case L'8':
                case L'9':
                    nNumber += (nMultiplier * (pszNumber[nNumberPos] - L'0'));
                    break;
                default:
                    return -1;
            }

            nMultiplier /= 10;
        }
    }

    return nNumber;
}

BOOL lFindTestRun(
  LPCWSTR pszTestCommand,
  TTestWithAddressFunc* pTestWithAddressFunc,
  TTestFunc* pTestFunc)
{
    int nIndex = 0;

    *pTestWithAddressFunc = NULL;
    *pTestFunc = NULL;

    while (l_TestData[nIndex].pszTestCommand != NULL)
    {
        if (lstrcmpi(pszTestCommand, l_TestData[nIndex].pszTestCommand) == 0)
        {
            *pTestWithAddressFunc = l_TestData[nIndex].TestWithAddressFunc;
            *pTestFunc = l_TestData[nIndex].TestFunc;

            return TRUE;
        }

        ++nIndex;
    }

    return FALSE;
}

static int lRunAllTest(
  int nAddress)
{
    int nResult;

    nResult = lRunProgramTest(nAddress);

    if (nResult)
    {
        nResult = lRunReadTest(nAddress);
    }

    return nResult;
}

static int lRunProgramTest(
  int nAddress)
{
    int nResult = 0;
    ULONG ulAddress = nAddress;
    BYTE byData[10] = {255, 128, 64, 32, 16, 8, 4, 2, 1, 0};
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

    return nResult;
}

static int lRunReadTest(
  int nAddress)
{
    int nResult = 0;
    ULONG ulAddress = nAddress;
    BYTE byData[10] = {0};
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

    return nResult;
}

static int lRunAddressTest(
  int nAddress)
{
    int nResult = 0;
    ULONG ulAddress = nAddress;
    BYTE byData[1] = {0};

    wprintf(L"TestPepCtrl: Running the address test. (All dips should be set to off.)\n");

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the programmer's read mode.\n");

        return 0;
    }

    if (!UtPepCtrlReadData(ulAddress, byData, sizeof(byData)))
    {
        wprintf(L"TestPepCtrl: Could not set the address.\n");

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

    return nResult;
}

static int lRunDataTest(
  int nAddress)
{
    int nResult = 0;
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

    return nResult;
}

static int lRunStatusTest(void)
{
    BOOL bPresent;

    wprintf(L"TestPepCtrl: Retrieving if the device is present.\n");

    if (!UtPepCtrlIsDevicePresent(&bPresent))
    {
        wprintf(L"TestPepCtrl: Could not retrieve the device status.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: %s\n",
            bPresent ? L"Device is present" : L"Device is not present");

    return 1;
}

static int lRunDeviceChangeTest(void)
{
    wprintf(L"TestPepCtrl: Waiting for a device change notification.\n");
    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    return 1;
}

static int lRunVcc5Test(void)
{
    int nResult = 0;

    wprintf(L"TestPepCtrl: Outputting +5VDC on Pin 32.\n");

    if (!UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +5VDC Vpp mode.\n");

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

    return nResult;
}

static int lRunVcc6Test(void)
{
    int nResult = 0;

    wprintf(L"TestPepCtrl: Outputting +6.25VDC on Pin 32.\n");

    if (!UtPepCtrlSetVccMode(eUtPepCtrl625VDCMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +6.25VDC Vpp mode.\n");

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

    return nResult;
}

static int lRunVEN08Test(void)
{
    int nResult = 0;

    wprintf(L"TestPepCtrl: Outputting +12VDC on Pin 23.\n");
    wprintf(L"TestPepCtrl: Outputting -5VDC on Pin 25.\n");

    if (!UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode))
    {
        wprintf(L"TestPepCtrl: Could not enable the VEN08 pin mode.\n");

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

    return nResult;
}

static int lRunNegVP5Test(void)
{
    int nResult = 0;

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

    return nResult;
}

static int lRunVpp12Test(void)
{
    wprintf(L"TestPepCtrl: Outputting +12VDC on the Vpp Accessory Connector.\n");

    if (!UtPepCtrlSetVppMode(eUtPepCtrl12VDCVppMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +12VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    return 1;
}

static int lRunVpp21Test(void)
{
    wprintf(L"TestPepCtrl: Outputting +21VDC on the Vpp Accessory Connector.\n");

    if (!UtPepCtrlSetVppMode(eUtPepCtrl21VDCVppMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +21VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    return 1;
}

static int lRunVpp25Test(void)
{
    wprintf(L"TestPepCtrl: Outputting +25VDC on the Vpp Accessory Connector.\n");

    if (!UtPepCtrlSetVppMode(eUtPepCtrl25VDCVppMode))
    {
        wprintf(L"TestPepCtrl: Could not enable the +25VDC Vpp mode.\n");

        return 0;
    }

    wprintf(L"TestPepCtrl: Press the \"Enter\" key to end the test.\n");

    _getch();

    return 1;
}

static int lRunTest(
  TTestWithAddressFunc TestWithAddressFunc,
  TTestFunc TestFunc,
  int nAddress)
{
    int nResult = 0;

    wprintf(L"TestPepCtrl: Initializing the Pep Control dll.\n");

    if (!UtPepCtrlInitialize(&lDeviceChangeFunc))
    {
        wprintf(L"TestPepCtrl: Could not initialize the Pep Control dll.\n");

        return 0;
    }

    if (TestWithAddressFunc)
    {
        nResult = TestWithAddressFunc(nAddress);
    }
    else if (TestFunc)
    {
        nResult = TestFunc();
    }

    wprintf(L"TestPepCtrl: Uninitializing the Pep Control dll.\n");

    if (!UtPepCtrlUninitialize())
    {
        wprintf(L"TestPepCtrl: Could not uninitialize the Pep Control dll.\n");
    }

    return nResult;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"TestPepCtrl [/test (all | address) \"Address\"]\n");
    wprintf(L"            [/test data \"Byte\"]\n");
    wprintf(L"            [/test (read | program) \"Address\"]\n");
    wprintf(L"            [/test (vcc5 | vcc6)]\n");
    wprintf(L"            [/test (ven08 | ~vp5)]\n");
    wprintf(L"            [/test (we08 | vpp16 | vpp32 | vpp64)]\n");
    wprintf(L"            [/test status]\n");
    wprintf(L"            [/test devicechange]\n");
    wprintf(L"\n");
    wprintf(L"    Programmer Tests\n");
    wprintf(L"    /test\n");
    wprintf(L"        all          - Run the write (program), then read test\n");
    wprintf(L"        read         - Run the read byte test\n");
    wprintf(L"        program      - Run the program byte test\n");
    wprintf(L"        address      - Selects the given address\n");
    wprintf(L"        data         - Selects the byte to output\n");
    wprintf(L"        status       - Retrieves whether the device is present or not\n");
    wprintf(L"        devicechange - Run the device change notification test\n");
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
    int nAddress = 0;
    TTestWithAddressFunc TestWithAddressFunc;
    TTestFunc TestFunc;
    int nResult;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    if (lstrcmpi(argv[1], L"/test") == 0 && (argc == 3 || argc == 4))
    {
        if (!lFindTestRun(argv[2], &TestWithAddressFunc, &TestFunc))
        {
            wprintf(L"TestPepCtrl: Unrecognized test command.\n");

            nResult = -4;

            goto End;
        }

        if (TestWithAddressFunc)
        {
            if (argc != 4)
            {
                wprintf(L"TestPepCtrl: Address argument missing.\n");

                nResult = -4;

                goto End;
            }

            nAddress = lParseNumber(argv[3]);

            if (nAddress == -1)
            {
                wprintf(L"TestPepCtrl: The address is not a valid number.\n");

                nResult = -4;

                goto End;
            }
        }

        nResult = lRunTest(TestWithAddressFunc, TestFunc, nAddress);
    }
    else
    {
        nResult = lDisplayHelp();
    }

End:
    SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

    return nResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
