/***************************************************************************/
/*  Copyright (C) 2007-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <Config/UtListPorts.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

typedef struct tagTPortData
{
    INT nPortData;
    LPCWSTR pszPortDataString;
} TPortData;

typedef int (*TTestFunc)(void);

typedef struct tagTTestData
{
    LPCWSTR pszTestCommand;
    TTestFunc TestFunc;
} TTestData;

static int lRunAllTest(void);
static int lRunLptTest(void);
static int lRunUsbPrintTest(void);
static int lRunUsbTest(void);

static TTestData l_TestData[] = {
    {L"all",      lRunAllTest},
    {L"lpt",      lRunLptTest},
    {L"usbprint", lRunUsbPrintTest},
    {L"usb",      lRunUsbTest},
    {NULL,        NULL} };

static TPortData l_PortData[] = {
    {epdLocation,                 L"    Location:               %s\n"},
    {epdPhysicalDeviceObjectName, L"    Physical Device Object: %s\n"},
    {0,                           NULL} };

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
  _In_z_ LPCWSTR pszTestCommand,
  TTestFunc* pTestFunc)
{
    int nIndex = 0;

    *pTestFunc = NULL;

    while (l_TestData[nIndex].pszTestCommand != NULL)
    {
        if (lstrcmpiW(pszTestCommand, l_TestData[nIndex].pszTestCommand) == 0)
        {
            *pTestFunc = l_TestData[nIndex].TestFunc;

            return TRUE;
        }

        ++nIndex;
    }

    return FALSE;
}

static int lRunAllTest(void)
{
    int nResult;

    nResult = lRunLptTest();

    if (nResult)
    {
        nResult = lRunUsbPrintTest();
    }

    if (nResult)
    {
        nResult = lRunUsbTest();
    }

    return nResult;
}

static int lRunLptTest(void)
{
    BOOL bQuit;
    INT nCount, nIndex, nDataIndex, nDataLen;
    LPWSTR pszData;

    wprintf(L"TestListPorts: Running the list lpt ports test.\n");

    if (!UtListPortsGetLptPortCount(&nCount))
    {
        wprintf(L"TestListPorts: Could not retrieve the lpt port count.\n");

        return 0;
    }

    wprintf(L"\n");

    for (nIndex = 0; nIndex < nCount; ++nIndex)
    {
        bQuit = FALSE;
        nDataIndex = 0;

        while (bQuit == FALSE)
        {
            pszData = NULL;

            if (!UtListPortsGetLptPortData(nIndex, l_PortData[nDataIndex].nPortData,
                                           NULL, &nDataLen))
            {
                wprintf(L"TestListPorts: Could not retrieve the required buffer size.\n");

                return 0;
            }

            pszData = (LPWSTR)UtAllocMem(nDataLen);

            if (!UtListPortsGetLptPortData(nIndex, l_PortData[nDataIndex].nPortData,
                                           pszData, &nDataLen))
            {
                wprintf(L"TestListPorts: Could not retrieve the data.\n");

                return 0;
            }

            wprintf(l_PortData[nDataIndex].pszPortDataString, pszData);

            UtFreeMem(pszData);

            ++nDataIndex;

            if (l_PortData[nDataIndex].pszPortDataString == NULL)
            {
                bQuit = TRUE;
            }
        }

        wprintf(L"\n");
    }

    return 1;
}

static int lRunUsbPrintTest(void)
{
    BOOL bQuit;
    INT nCount, nIndex, nDataIndex, nDataLen;
    LPWSTR pszData;

    wprintf(L"TestListPorts: Running the list usb print port test.\n");

    if (!UtListPortsGetUsbPrintPortCount(&nCount))
    {
        wprintf(L"TestListPorts: Could not retrieve the usb print port count.\n");

        return 0;
    }

    wprintf(L"\n");

    for (nIndex = 0; nIndex < nCount; ++nIndex)
    {
        bQuit = FALSE;
        nDataIndex = 0;

        while (bQuit == FALSE)
        {
            pszData = NULL;

            if (!UtListPortsGetUsbPrintPortData(nIndex, l_PortData[nDataIndex].nPortData,
                                                NULL, &nDataLen))
            {
                wprintf(L"TestListPorts: Could not retrieve the required buffer size.\n");

                return 0;
            }

            pszData = (LPWSTR)UtAllocMem(nDataLen);

            if (!UtListPortsGetUsbPrintPortData(nIndex, l_PortData[nDataIndex].nPortData,
                                                pszData, &nDataLen))
            {
                wprintf(L"TestListPorts: Could not retrieve the data.\n");

                return 0;
            }

            wprintf(l_PortData[nDataIndex].pszPortDataString, pszData);

            UtFreeMem(pszData);

            ++nDataIndex;

            if (l_PortData[nDataIndex].pszPortDataString == NULL)
            {
                bQuit = TRUE;
            }
        }

        wprintf(L"\n");
    }

    return 1;
}

static int lRunUsbTest(void)
{
    BOOL bQuit;
    INT nCount, nIndex, nDataIndex, nDataLen;
    LPWSTR pszData;

    wprintf(L"TestListPorts: Running the list usb port test.\n");

    if (!UtListPortsGetUsbPortCount(&nCount))
    {
        wprintf(L"TestListPorts: Could not retrieve the usb port count.\n");

        return 0;
    }

    wprintf(L"\n");

    for (nIndex = 0; nIndex < nCount; ++nIndex)
    {
        bQuit = FALSE;
        nDataIndex = 0;

        while (bQuit == FALSE)
        {
            pszData = NULL;

            if (!UtListPortsGetUsbPortData(nIndex, l_PortData[nDataIndex].nPortData,
                                           NULL, &nDataLen))
            {
                wprintf(L"TestListPorts: Could not retrieve the required buffer size.\n");

                return 0;
            }

            pszData = (LPWSTR)UtAllocMem(nDataLen);

            if (!UtListPortsGetUsbPortData(nIndex, l_PortData[nDataIndex].nPortData,
                                           pszData, &nDataLen))
            {
                wprintf(L"TestListPorts: Could not retrieve the data.\n");

                return 0;
            }

            wprintf(l_PortData[nDataIndex].pszPortDataString, pszData);

            UtFreeMem(pszData);

            ++nDataIndex;

            if (l_PortData[nDataIndex].pszPortDataString == NULL)
            {
                bQuit = TRUE;
            }
        }

        wprintf(L"\n");
    }

    return 1;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"TestListPorts [/list (all | lpt | usbprint | usb)]\n");
    wprintf(L"\n");
    wprintf(L"    /list\n");
    wprintf(L"        all      - List all lpt and usb printer ports\n");
    wprintf(L"        lpt      - List lpt printer ports\n");
    wprintf(L"        usbprint - List USB printer ports\n");
    wprintf(L"        usb      - List USB ports\n");
    wprintf(L"\n");

    return -1;
}

int _cdecl wmain(int argc, WCHAR* argv[])
{
    TTestFunc TestFunc;
    int nResult;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    if (argc == 3)
    {
        if (lstrcmpi(argv[1], L"/list") == 0 && lFindTestRun(argv[2], &TestFunc))
        {
            if (UtListPortsInitialize())
            {
                nResult = TestFunc();

                if (!UtListPortsUninitialize())
                {
                    wprintf(L"TestListPorts: Could not uninitialize the library.\n");
                }
            }
            else
            {
                wprintf(L"TestListPorts: Could not initialize the library.\n");

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
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/
