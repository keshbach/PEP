/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>
#include <shlwapi.h>

#include <UtilsDevice/UtPepDevices.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

#include <UtilsDevice/UtPAL.h>

#include <assert.h>

static BOOL l_bQuit = FALSE;

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

    l_bQuit = TRUE;

    return TRUE;
}

static void lPrintROMData(
  TROMData* pROMData)
{
    wprintf(L"Size (Bytes):   %d\n", pROMData->nSize);
    wprintf(L"Bits per Data:  %d\n", pROMData->nBitsPerValue);
    wprintf(L"Device Vpp:     %s\n",
		    UtPepDevicesGetDeviceVppName(pROMData->DeviceVpp));

    wprintf(L"Operations:     ");

    if (pROMData->pReadDeviceFunc)
    {
        wprintf(L"Read ");
    }

    if (pROMData->pProgramDeviceFunc)
    {
        wprintf(L"Program ");
    }

    if (pROMData->pVerifyDeviceFunc)
    {
        wprintf(L"Verify ");
    }

    wprintf(L"\n");
}

static void lPrintRAMData(
  TRAMData* pRAMData)
{
    wprintf(L"Size (Bytes):   %d\n", pRAMData->nSize);
    wprintf(L"Bits per Data:  %d\n", pRAMData->nBitsPerValue);

    wprintf(L"Operations:     ");

    if (pRAMData->pWriteDeviceFunc)
    {
        wprintf(L"Write ");
    }

    if (pRAMData->pVerifyDeviceFunc)
    {
        wprintf(L"Verify ");
    }

    wprintf(L"\n");
}

static void lPrintPALData(
  TPALData* pPALData)
{
    TDevicePinConfigValues* pDevicePinConfigValues;
    TDevicePinFuseRows* pDevicePinFuseRows;
    TDevicePinFuseColumns* pDevicePinFuseColumns;
    UINT nPinIndex, nPinTypeIndex, nPinPolarityIndex;
    UINT nFuseNumber;
    LPWSTR pszText, pszTextPos;
    ULONG ulTextLen;

    wprintf(L"Fuse Map:\n");

    nFuseNumber = 0;

    if (UtPALFuseMapSampleText(pPALData, NULL, &ulTextLen))
    {
        pszText = (LPWSTR)UtAllocMem(ulTextLen * sizeof(WCHAR));

        if (UtPALFuseMapSampleText(pPALData, pszText, &ulTextLen))
        {
            wprintf(L"    ");

            pszTextPos = pszText;

            while (*pszTextPos)
            {
                wprintf(L"%c", *pszTextPos);

                ++pszTextPos;

                if (*pszTextPos == 0x0d)
                {
                    wprintf(L"\n");

                    pszTextPos += 2;

                    if (*pszTextPos != 0)
                    {
                        wprintf(L"    ");
                    }
                }
            }
        }
        else
        {
            wprintf(L"Could not generate a sample fuse map.\n");
        }

        UtFreeMem(pszText);
    }
    else
    {
        wprintf(L"Could not get the required size for a sample fuse map.\n");
    }

    wprintf(L"Pin Config Values:\n");

    for (nPinIndex = 0; nPinIndex < pPALData->nDevicePinConfigValuesCount; ++nPinIndex)
    {
        pDevicePinConfigValues = &pPALData->pDevicePinConfigValues[nPinIndex];

        wprintf(L"    Pin Number:     %d\n", pDevicePinConfigValues->nPin);
        wprintf(L"    Pin Types:      ");

        for (nPinTypeIndex = 0; nPinTypeIndex < pDevicePinConfigValues->nTypesCount;
             ++nPinTypeIndex)
        {
            wprintf(L"\"%s\"", pDevicePinConfigValues->ppszTypes[nPinTypeIndex]);

            if (nPinTypeIndex < pDevicePinConfigValues->nTypesCount - 1)
            {
                wprintf(L", ");
            }
        }

        wprintf(L"\n");

        if (pDevicePinConfigValues->nPolaritiesCount > 0)
        {
            wprintf(L"    Pin Polarities: ");

            for (nPinPolarityIndex = 0; nPinPolarityIndex < pDevicePinConfigValues->nPolaritiesCount;
                 ++nPinPolarityIndex)
            {
                wprintf(L"\"%s\"", pDevicePinConfigValues->ppszPolarities[nPinPolarityIndex]);

                if (nPinPolarityIndex < pDevicePinConfigValues->nPolaritiesCount - 1)
                {
                    wprintf(L", ");
                }
            }

            wprintf(L"\n");
        }

        wprintf(L"\n");
    }

    wprintf(L"Pin Fuse Rows:\n");

    for (nPinIndex = 0; nPinIndex < pPALData->nDevicePinFuseRowsCount; ++nPinIndex)
    {
        pDevicePinFuseRows = &pPALData->pDevicePinFuseRows[nPinIndex];

        wprintf(L"    Pin Number:        %d\n", pDevicePinFuseRows->nPin);

        wprintf(L"    Output Enable Row: ");

        if (pDevicePinFuseRows->nFuseRowOutputEnable != CNoOutputEnableFuseRow)
        {
            wprintf(L"%d\n", pDevicePinFuseRows->nFuseRowOutputEnable);
        }
        else
        {
            wprintf(L"Not Supported\n");
        }

        wprintf(L"    Start Row:         %d\n", pDevicePinFuseRows->nFuseRowTermStart);
        wprintf(L"    End Row:           %d\n", pDevicePinFuseRows->nFuseRowTermEnd);
        wprintf(L"\n");
    }

    wprintf(L"Pin Fuse Columns:\n");

    for (nPinIndex = 0; nPinIndex < pPALData->nDevicePinFuseColumnsCount; ++nPinIndex)
    {
        pDevicePinFuseColumns = &pPALData->pDevicePinFuseColumns[nPinIndex];

        wprintf(L"    Pin Number:         %d\n", pDevicePinFuseColumns->nPin);
        wprintf(L"    Low Output Column:  %d\n", pDevicePinFuseColumns->nLowFuseColumn);
        wprintf(L"    High Output Column: %d\n", pDevicePinFuseColumns->nHighFuseColumn);
        wprintf(L"\n");
    }

    wprintf(L"Operations:                 ");

    if (pPALData->pGetFuseMapSizeFunc)
    {
        wprintf(L"\"Get Fuse Map Size\" ");
    }

    if (pPALData->pConfigFuseMapFunc)
    {
        wprintf(L"\"Config Fuse Map\" ");
    }

    if (pPALData->pSetDevicePinConfigsFunc)
    {
        wprintf(L"\"Set Device Pin Configs\" ");
    }

    if (pPALData->pReadDeviceFunc)
    {
        wprintf(L"Read ");
    }

    wprintf(L"\n");

    wprintf(L"\n");
}

static void lPrintDevice(
  TDevice* pDevice)
{
    ULONG ulIndex;
    LPCTSTR pszPinDiagram;

    wprintf(L"Name:           %s\n", pDevice->pszName);
    wprintf(L"Device Type:    %s\n",
            UtPepDevicesGetDeviceTypeName(pDevice->DeviceType));
	wprintf(L"Device Package: %s\n",
			UtPepDevicesGetDevicePackageName(pDevice->DevicePackage));
    wprintf(L"Pin Count:      %d\n", pDevice->nPinCount);
    wprintf(L"Adapter:        ");

    if (pDevice->pszAdapter)
    {
        wprintf(pDevice->pszAdapter);
    }

    wprintf(L"\n");
	wprintf(L"Message:        ");

	if (pDevice->pszMessage)
    {
        wprintf(pDevice->pszMessage);
    }

    wprintf(L"\n");

    wprintf(L"Dip Switches:   ");

    UtConsolePrintDipSwitches(pDevice->bDipSwitches,
                              MArrayLen(pDevice->bDipSwitches));

	wprintf(L"Pin Names:      ");

	for (ulIndex = 0; ulIndex < pDevice->nPinCount; ++ulIndex)
	{
		wprintf(L"Pin %d - %s", ulIndex + 1, pDevice->ppszPinNames[ulIndex]);

		if (ulIndex + 1 < pDevice->nPinCount)
        {
            printf(", ");
        }
	}

    wprintf(L"\n");
	wprintf(L"Pin Diagram:\n");

    pszPinDiagram = UtPepDevicesAllocPinDiagram(pDevice->DevicePackage,
                                                pDevice->ppszPinNames,
                                                pDevice->nPinCount);
    if (pszPinDiagram)
    {
        wprintf(pszPinDiagram);

        UtPepDevicesFreePinDiagram(pszPinDiagram);
    }
    else
    {
        wprintf(L"    Could not allocate memory for the pin diagram.\n");
    }

    switch (pDevice->DeviceType)
    {
        case edtEPROM:
        case edtEPROMWithIO:
        case edtPROM:
            lPrintROMData(&pDevice->DeviceData.ROMData);
            break;
        case edtSRAM:
            lPrintRAMData(&pDevice->DeviceData.RAMData);
            break;
        case edtPAL:
            lPrintPALData(&pDevice->DeviceData.PALData);
            break;
        default:
            assert(0);
            wprintf(L"Unknown device type");
            break;
    }

    wprintf(L"\n");
}

static int lListDevices(void)
{
    int nResult = 0;
    WCHAR cPath[MAX_PATH];
    ULONG ulDeviceCount, ulIndex;
    TDevice Device;

    GetModuleFileName(GetModuleHandleW(NULL), cPath, MArrayLen(cPath));

    PathRemoveFileSpec(cPath);

    wprintf(L"Calling UtPepDevicesInitialize with the plugin path of \"%s\".\n", cPath);

    if (FALSE == UtPepDevicesInitialize(cPath))
    {
        wprintf(L"UtPepDevicesInitialize call has failed.\n");

        return -1;
    }

    wprintf(L"Calling UtPepDevicesGetDeviceCount.\n");

    if (FALSE == UtPepDevicesGetDeviceCount(&ulDeviceCount))
    {
        wprintf(L"UtPepDevicesGetDeviceCount call has failed.\n");

        nResult = -1;

        goto End;
    }

    for (ulIndex = 0; ulIndex < ulDeviceCount && l_bQuit == FALSE; ++ulIndex)
    {
        wprintf(L"Calling UtPepDevicesGetDevice for index %d.\n", ulIndex);

        if (FALSE == UtPepDevicesGetDevice(ulIndex, &Device))
        {
            wprintf(L"UtPepDevicesGetDevice call has failed.\n");

            nResult = -1;

            goto End;
        }

        lPrintDevice(&Device);
    }

End:
    wprintf(L"Calling UtPepDevicesUninitialize.\n");

    if (FALSE == UtPepDevicesUninitialize())
    {
        wprintf(L"UtPepDevicesUninitialize call has failed.\n");

        nResult = -1;
    }

    return nResult;
}

static int lListModules(void)
{
    int nResult = 0;
    WCHAR cPath[MAX_PATH];
    ULONG ulPluginCount, ulIndex;
    LPCWSTR pszName;
    WORD wProductMajorVersion, wProductMinorVersion;
    WORD wProductBuildVersion, wProductPrivateVersion;

    GetModuleFileName(GetModuleHandleW(NULL), cPath, MArrayLen(cPath));

    PathRemoveFileSpec(cPath);

    wprintf(L"Calling UtPepDevicesInitialize with the plugin path of \"%s\".\n", cPath);

    if (FALSE == UtPepDevicesInitialize(cPath))
    {
        wprintf(L"UtPepDevicesInitialize call has failed.\n");

        return -1;
    }

    wprintf(L"Calling UtPepDevicesGetPluginCount.\n");

    if (FALSE == UtPepDevicesGetPluginCount(&ulPluginCount))
    {
        wprintf(L"UtPepDevicesGetPluginCount call has failed.\n");

        nResult = -1;

        goto End;
    }

    for (ulIndex = 0; ulIndex < ulPluginCount && l_bQuit == FALSE; ++ulIndex)
    {
        wprintf(L"Calling UtPepDevicesGetDevice for index %d.\n", ulIndex);

        if (TRUE == UtPepDevicesGetPluginName(ulIndex, &pszName))
        {
            wprintf(L"Name:                    %s\n", pszName);
        }
        else
        {
            wprintf(L"UtPepDevicesGetPluginName call has failed.\n");

            nResult = -1;

            goto End;
        }

        if (TRUE == UtPepDevicesGetPluginVersion(ulIndex, &wProductMajorVersion,
                                                 &wProductMinorVersion,
                                                 &wProductBuildVersion,
                                                 &wProductPrivateVersion))
        {
            wprintf(L"Product Major Version:   %d\n", (ULONG)wProductMajorVersion);
            wprintf(L"Product Minor Version:   %d\n", (ULONG)wProductMinorVersion);
            wprintf(L"Product Build Version:   %d\n", (ULONG)wProductBuildVersion);
            wprintf(L"Product Private Version: %d\n", (ULONG)wProductPrivateVersion);
        }
        else
        {
            wprintf(L"UtPepDevicesGetPluginVersion call has failed.\n");

            nResult = -1;

            goto End;
        }

        wprintf(L"\n");
    }

End:
    wprintf(L"Calling UtPepDevicesUninitialize.\n");

    if (FALSE == UtPepDevicesUninitialize())
    {
        wprintf(L"UtPepDevicesUninitialize call has failed.\n");

        nResult = -1;
    }

    return nResult;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"TestListDevices [/listplugins] [/listdevices]\n");
    wprintf(L"\n");
    wprintf(L"    /listplugins - Lists all of the plugins available\n");
    wprintf(L"    /listdevices - List all of the devices available\n");
    wprintf(L"\n");

    return -1;
}

int _cdecl wmain(int argc, WCHAR* argv[])
{
    int nResult;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    if (argc == 2)
    {
        if (lstrcmpi(argv[1], L"/listplugins") == 0)
        {
            nResult = lListModules();
        }
        else if (lstrcmpi(argv[1], L"/listdevices") == 0)
        {
            nResult = lListDevices();
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
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/
