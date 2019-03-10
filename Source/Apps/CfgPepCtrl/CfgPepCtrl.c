/***************************************************************************/
/*  Copyright (C) 2006-2018 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <Config/UtPepCtrlCfg.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

#pragma region Typedefs
typedef int (*TTestFunc)(LPCWSTR pszArg1, LPCWSTR pszArg2);
#pragma endregion

#pragma region Structures
typedef struct tagTTestData
{
    LPCWSTR pszTestCommand;
    LPCWSTR pszTestSubCommand;
    INT nArgsRequired;
    TTestFunc TestFunc;
} TTestData;
#pragma endregion

static int lInstallDriver(LPCWSTR pszArg1, LPCWSTR pszArg2);
static int lUninstallDriver(LPCWSTR pszArg1, LPCWSTR pszArg2);
static int lStartDriver(LPCWSTR pszArg1, LPCWSTR pszArg2);
static int lStopDriver(LPCWSTR pszArg1, LPCWSTR pszArg2);
static int lReadDriverSettings(LPCWSTR pszArg1, LPCWSTR pszArg2);
static int lWriteDriverSettings(LPCWSTR pszArg1, LPCWSTR pszArg2);

#pragma region Local Variables
static TTestData l_TestData[] = {
    {L"/install",   L"driver", 2, lInstallDriver},
    {L"/uninstall", NULL,      0, lUninstallDriver},
    {L"/start",     NULL,      0, lStartDriver},
    {L"/stop",      NULL,      0, lStopDriver},
    {L"/config",    L"read",   1, lReadDriverSettings},
    {L"/config",    L"write",  3, lWriteDriverSettings},
    {NULL,          NULL,      0, NULL}
};
#pragma endregion

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

BOOL lFindTestRun(
  LPCWSTR pszTestCommand,
  LPCWSTR pszTestSubCommand,
  INT nArgsCount,
  TTestData** ppTestData)
{
    int nIndex = 0;

    *ppTestData = NULL;

    while (l_TestData[nIndex].pszTestCommand != NULL)
    {
        if (lstrcmpiW(pszTestCommand, l_TestData[nIndex].pszTestCommand) == 0)
        {
            if (l_TestData[nIndex].pszTestSubCommand)
            {
                if (lstrcmpiW(pszTestSubCommand, l_TestData[nIndex].pszTestSubCommand) == 0)
                {
                    if (l_TestData[nIndex].nArgsRequired == nArgsCount)
                    {
                        *ppTestData = &l_TestData[nIndex];

                        return TRUE;
                    }
                }
            }
            else
            {
                if (l_TestData[nIndex].nArgsRequired == nArgsCount)
                {
                    *ppTestData = &l_TestData[nIndex];

                    return TRUE;
                }
            }
        }
       
        ++nIndex;
    }

    return FALSE;
}

static VOID UTPEPCTRLCFGAPI lMsgFunc(
  LPCTSTR pszMsg)
{
    wprintf(L"CfgPepCtrl: %s\n", pszMsg);
}

static int lInstallDriver(
  LPCWSTR pszArg1,
  LPCWSTR pszArg2)
{
    pszArg1;
    pszArg2;

    if (UtPepCtrlCfgInstallDriver(pszArg1, &lMsgFunc))
    {
        return 0;
    }

    return -1;
}

static int lUninstallDriver(
  LPCWSTR pszArg1,
  LPCWSTR pszArg2)
{
    pszArg1;
    pszArg2;

    if (UtPepCtrlCfgUninstallDriver(&lMsgFunc))
    {
        return 0;
    }

    return -1;
}

static int lStartDriver(
  LPCWSTR pszArg1,
  LPCWSTR pszArg2)
{
    pszArg1;
    pszArg2;

    if (UtPepCtrlCfgStartDriver(&lMsgFunc))
    {
        return 0;
    }

    return -1;
}

static int lStopDriver(
  LPCWSTR pszArg1,
  LPCWSTR pszArg2)
{
    pszArg1;
    pszArg2;

    if (UtPepCtrlCfgStopDriver(&lMsgFunc))
    {
        return 0;
    }

    return -1;
}

static int lReadDriverSettings(
  LPCWSTR pszArg1,
  LPCWSTR pszArg2)
{
    int nResult = 0;
    LPWSTR pszPortDeviceName;
    INT nPortDeviceNameLen;
    EUtPepCtrlCfgPortType PortType;

    pszArg1;
    pszArg2;

    wprintf(L"CfgPepCtrl: Attempting to retrieve the driver port settings.\n");

    if (!UtPepCtrlCfgGetPortType(&PortType))
    {
        wprintf(L"CfgPepCtrl: Could not retrieve the port type.\n");

        return -1;
    }

    wprintf(L"CfgPepCtrl: Port Type - ");

    switch (PortType)
    {
        case eUtPepCtrlCfgParallelPortType:
            wprintf(L"Parallel\n");
            break;
        case eUtPepCtrlCfgUsbPrintPortType:
            wprintf(L"USB Print\n");
            break;
        default:
            wprintf(L"Unknown\n");
            break;
    }

    if (!UtPepCtrlCfgGetPortDeviceName(NULL, &nPortDeviceNameLen))
    {
        wprintf(L"CfgPepCtrl: Could not retrieve the buffer size required for the port device name.\n");

        return -1;
    }

    pszPortDeviceName = (LPWSTR)UtAllocMem(nPortDeviceNameLen);

    if (!UtPepCtrlCfgGetPortDeviceName(pszPortDeviceName, &nPortDeviceNameLen))
    {
        wprintf(L"CfgPepCtrl: Could not retrieve the port device name.\n");

        nResult = -1;

        goto EndRead;
    }

    wprintf(L"CfgPepCtrl: Port Device Name - \"%s\".\n", pszPortDeviceName);

EndRead:
    UtFreeMem(pszPortDeviceName);

    return nResult;
}

static int lWriteDriverSettings(
  LPCWSTR pszPortType,
  LPCWSTR pszPortDeviceName)
{
    EUtPepCtrlCfgPortType PortType;

    wprintf(L"CfgPepCtrl: Attempting to change the driver port settings.\n");

    if (lstrcmpi(pszPortType, L"lpt") == 0)
    {
        PortType = eUtPepCtrlCfgParallelPortType;
    }
    else if (lstrcmpi(pszPortType, L"usb") == 0)
    {
        PortType = eUtPepCtrlCfgUsbPrintPortType;
    }
    else
    {
        wprintf(L"CfgPepCtrl: Unknown port type given.\n");

        return -1;
    }

    if (!UtPepCtrlCfgSetPortSettings(PortType, pszPortDeviceName))
    {
        wprintf(L"CfgPepCtrl: Could not change the driver's port settings.\n");

        return -1;
    }

    wprintf(L"CfgPepCtrl: Driver port settings changed.\n");

    return 0;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"CfgPepCtrl [/install driver \"File\"]\n");
	wprintf(L"           [/uninstall]\n");
    wprintf(L"           [/start] [/stop]\n");
    wprintf(L"           [/config read]\n");
    wprintf(L"           [/config write \"Port Type\" \"Device Name\"]\n");
    wprintf(L"\n");
    wprintf(L"    /install      - Installs the driver\n");
	wprintf(L"         driver\n");
	wprintf(L"        \"File\" - Fully qualifed path to the driver file\n");
	wprintf(L"    /uninstall    - Uninstalls the driver\n");
    wprintf(L"\n");
    wprintf(L"    /start        - Starts the driver\n");
    wprintf(L"    /stop         - Stops the driver\n");
    wprintf(L"\n");
    wprintf(L"    Port Configuration\n");
    wprintf(L"    /config\n");
    wprintf(L"        read          - Show the port settings\n");
    wprintf(L"    /config\n");
    wprintf(L"        write         - Change the port settings\n");
    wprintf(L"        \"Port Type\"   - \"lpt\" or \"usb\"\n");
    wprintf(L"        \"Device Name\" - Name of the device\n");
    wprintf(L"\n");

    return -1;
}

int _cdecl wmain(int argc, WCHAR* argv[])
{
    int nResult;
    TTestData* pTestData;
    LPCWSTR pszTestSubCommand, pszArg1, pszArg2;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    if (argc > 1 && argc < 6)
    {
        if (argc == 2)
        {
            pszTestSubCommand = NULL;
        }
        else
        {
            pszTestSubCommand = argv[2];
        }

        if (lFindTestRun(argv[1], pszTestSubCommand, argc - 2, &pTestData))
        {
            if (argc >= 4)
            {
                pszArg1 = argv[3];
            }
            else
            {
                pszArg1 = NULL;
            }

            if (argc == 5)
            {
                pszArg2 = argv[4];
            }
            else
            {
                pszArg2 = NULL;
            }

            wprintf(L"CfgPepCtrl: Initializing the Pep Control Configuration dll.\n");

            if (UtPepCtrlCfgInitialize())
            {
                nResult = pTestData->TestFunc(pszArg1, pszArg2);

                wprintf(L"CfgPepCtrl: Uninitializing the Pep Control Configuration dll.\n");

                if (!UtPepCtrlCfgUninitialize())
                {
                    wprintf(L"CfgPepCtrl: Could not uninitialize the Pep Control Configuration dll.\n");
                }
            }
            else
            {
                wprintf(L"CfgPepCtrl: Could not initialize the Pep Control Configuration dll.\n");

                nResult = -1;
            }
        }
        else
        {
            nResult = lDisplayHelp();
        }
    }
    else
    {
        nResult = lDisplayHelp();
    }

    SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

    return nResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2018 Kevin Eshbach                                  */
/***************************************************************************/
