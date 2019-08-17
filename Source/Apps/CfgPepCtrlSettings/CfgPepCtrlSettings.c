/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <Config/UtPepCtrl.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

#pragma region Typedefs
typedef int(*TTestFunc)(LPCWSTR pszArg1, LPCWSTR pszArg2);
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

static int lReadDriverSettings(LPCWSTR pszArg1, LPCWSTR pszArg2);
static int lWriteDriverSettings(LPCWSTR pszArg1, LPCWSTR pszArg2);
static int lClearDriverSettings(LPCWSTR pszArg1, LPCWSTR pszArg2);

#pragma region Local Variables
static TTestData l_TestData[] = {
    {L"/config", L"read",  1, lReadDriverSettings},
    {L"/config", L"write", 3, lWriteDriverSettings},
    {L"/config", L"clear", 1, lClearDriverSettings},
    {NULL,       NULL,     0, NULL}
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

static int lReadDriverSettings(
  LPCWSTR pszArg1,
  LPCWSTR pszArg2)
{
    int nResult = 0;
    LPWSTR pszPortDeviceName;
    INT nPortDeviceNameLen;
    EUtPepCtrlPortType PortType;

    pszArg1;
    pszArg2;

    wprintf(L"CfgPepCtrlSettings: Attempting to retrieve the driver port settings.\n");

    if (!UtPepCtrlGetPortType(&PortType))
    {
        wprintf(L"CfgPepCtrlSettings: Could not retrieve the port type.\n");

        return -1;
    }

    wprintf(L"CfgPepCtrlSettings: Port Type - ");

    switch (PortType)
    {
        case eUtPepCtrlNonePortType:
            wprintf(L"None\n");
            break;
        case eUtPepCtrlParallelPortType:
            wprintf(L"Parallel\n");
            break;
        case eUtPepCtrlUsbPrintPortType:
            wprintf(L"USB Print\n");
            break;
        default:
            wprintf(L"Unknown\n");
            break;
    }

    if (!UtPepCtrlGetPortDeviceName(NULL, &nPortDeviceNameLen))
    {
        wprintf(L"CfgPepCtrlSettings: Could not retrieve the buffer size required for the port device name.\n");

        return -1;
    }

    pszPortDeviceName = (LPWSTR)UtAllocMem(nPortDeviceNameLen * sizeof(WCHAR));

    if (!UtPepCtrlGetPortDeviceName(pszPortDeviceName, &nPortDeviceNameLen))
    {
        wprintf(L"CfgPepCtrlSettings: Could not retrieve the port device name.\n");

        nResult = -1;

        goto EndRead;
    }

    wprintf(L"CfgPepCtrlSettings: Port Device Name - \"%s\".\n", pszPortDeviceName);

EndRead:
    UtFreeMem(pszPortDeviceName);

    return nResult;
}

static int lWriteDriverSettings(
  LPCWSTR pszPortType,
  LPCWSTR pszPortDeviceName)
{
    EUtPepCtrlPortType PortType;

    wprintf(L"CfgPepCtrlSettings: Attempting to change the driver port settings.\n");

    if (lstrcmpi(pszPortType, L"lpt") == 0)
    {
        PortType = eUtPepCtrlParallelPortType;
    }
    else if (lstrcmpi(pszPortType, L"usb") == 0)
    {
        PortType = eUtPepCtrlUsbPrintPortType;
    }
    else
    {
        wprintf(L"CfgPepCtrlSettings: Unknown port type given.\n");

        return -1;
    }

    if (!UtPepCtrlSetPortSettings(PortType, pszPortDeviceName))
    {
        wprintf(L"CfgPepCtrlSettings: Could not change the driver's port settings.\n");

        return -1;
    }

    wprintf(L"CfgPepCtrlSettings: Driver port settings changed.\n");

    return 0;
}

static int lClearDriverSettings(
  LPCWSTR pszArg1,
  LPCWSTR pszArg2)
{
    pszArg1;
    pszArg2;

    wprintf(L"CfgPepCtrlSettings: Attempting to remove the existing driver port settings.\n");

    if (!UtPepCtrlSetPortSettings(eUtPepCtrlNonePortType, L""))
    {
        wprintf(L"CfgPepCtrlSettings: Could not remove the existing driver's port settings.\n");

        return -1;
    }

    wprintf(L"CfgPepCtrlSettings: Driver port settings cleared.\n");

    return 0;
}

static VOID UTPEPCTRLAPI lDeviceChangeFunc(
  EUtPepCtrlDeviceChange DeviceChange)
{
    switch (DeviceChange)
    {
        case eUtPepCtrlDeviceArrived:
            wprintf(L"CfgPepCtrlSettings: ** Device arrived **\n");
            break;
        case eUtPepCtrlDeviceRemoved:
            wprintf(L"CfgPepCtrlSettings: ** Device removed **\n");
            break;
        default:
            wprintf(L"CfgPepCtrlSettings: ** Unknown device change **\n");
            break;
    }
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"CfgPepCtrlSettings [/config read]\n");
    wprintf(L"                   [/config write \"Port Type\" \"Device Name\"]\n");
    wprintf(L"                   [/config clear]\n");
    wprintf(L"\n");
    wprintf(L"    Port Configuration\n");
    wprintf(L"    /config\n");
    wprintf(L"        read          - Show the port settings\n");
    wprintf(L"    /config\n");
    wprintf(L"        write         - Change the port settings\n");
    wprintf(L"        \"Port Type\"   - \"lpt\" or \"usb\"\n");
    wprintf(L"        \"Device Name\" - Name of the device\n");
    wprintf(L"    /config\n");
    wprintf(L"        clear         - Remove the existing port settings\n");
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

            wprintf(L"CfgPepCtrlSettings: Initializing the Pep Control dll.\n");

            if (UtPepCtrlInitialize(lDeviceChangeFunc))
            {
                nResult = pTestData->TestFunc(pszArg1, pszArg2);

                wprintf(L"CfgPepCtrlSettings: Uninitializing the Pep Control dll.\n");

                if (!UtPepCtrlUninitialize())
                {
                    wprintf(L"CfgPepCtrlSettings: Could not uninitialize the Pep Control dll.\n");
                }
            }
            else
            {
                wprintf(L"CfgPepCtrlSettings: Could not initialize the Pep Control dll.\n");

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
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/
