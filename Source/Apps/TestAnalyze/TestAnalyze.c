/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <UtilsPep/UtPepAnalyze.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

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

static int lTestBlank(
  LPCWSTR pszFilename)
{
	LPBYTE pbyData;
	HANDLE hFile;
	DWORD dwBytesRead;
	LARGE_INTEGER FileSize;
	BOOL bIsEmpty;

	wprintf(L"Attempting to open the file \"%s\".\n", pszFilename);

	hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Could not open the file.  (Error Code: 0x%08x).\n",
                GetLastError());

		return -1;
	}

	wprintf(L"Attempting to get the file size.\n");

	if (FALSE == GetFileSizeEx(hFile, &FileSize))
	{
        wprintf(L"Could not retrieve the file's size.  (Error Code: 0x%08x).\n",
                GetLastError());

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Attempting to allocate memory for the file.\n");

	pbyData = (LPBYTE)UtAllocMem(FileSize.LowPart);

	if (pbyData == NULL)
	{
		wprintf(L"Could not allocate memory for the file.\n");

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Attempting to read the file.\n");

	if (FALSE == ReadFile(hFile, pbyData, FileSize.LowPart, &dwBytesRead, NULL))
	{
		wprintf(L"Could not read the file.  (Error Code: 0x%08x).\n",
                GetLastError());

		UtFreeMem(pbyData);

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Analyzing the file.\n");

	if (UtPepAnalyzeIsEmpty(pbyData, FileSize.LowPart, &bIsEmpty))
	{
		wprintf(bIsEmpty ? L"File is blank\n" : L"File is not blank\n");
	}
	else
	{
		wprintf(L"Analyze has failed.\n");
	}

	UtFreeMem(pbyData);

	CloseHandle(hFile);

	return 0;
}

static int lTestStuckBits(
  LPCWSTR pszFilename)
{
	LPBYTE pbyData;
	HANDLE hFile;
	DWORD dwBytesRead;
	LARGE_INTEGER FileSize;
	UINT32 nBitStucks;

	wprintf(L"Attempting to open the file \"%s\".\n", pszFilename);

	hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Could not open the file.  (Error Code: 0x%08x).\n",
                GetLastError());

		return -1;
	}

	wprintf(L"Attempting to get the file size.\n");

	if (FALSE == GetFileSizeEx(hFile, &FileSize))
	{
		wprintf(L"Could not retrieve the file's size.  (Error Code: 0x%08x).\n",
                GetLastError());

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Attempting to allocate memory for the file.\n");

	pbyData = (LPBYTE)UtAllocMem(FileSize.LowPart);

	if (pbyData == NULL)
	{
		wprintf(L"Could not allocate memory for the file.\n");

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Attempting to read the file.\n");

	if (FALSE == ReadFile(hFile, pbyData, FileSize.LowPart, &dwBytesRead, NULL))
	{
		wprintf(L"Could not read the file.  (Error Code: 0x%08x).\n",
                GetLastError());

		UtFreeMem(pbyData);

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Analyzing the file.\n");

	if (UtPepAnalyzeCheckStuckBits(pbyData, FileSize.LowPart, &nBitStucks))
	{
		if (nBitStucks == 0)
		{
			wprintf(L"No stuck bits detected.\n");
		}
		else
		{
            if (nBitStucks & CPepAnalyzeBitStuckHigh0)
			{
				wprintf(L"Bit 0 is stuck high.\n");
            }

			if (nBitStucks & CPepAnalyzeBitStuckLow0)
			{
				wprintf(L"Bit 0 is stuck low.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckHigh1)
			{
				wprintf(L"Bit 1 is stuck high.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckLow1)
			{
				wprintf(L"Bit 1 is stuck low.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckHigh2)
			{
				wprintf(L"Bit 2 is stuck high.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckLow2)
			{
				wprintf(L"Bit 2 is stuck low.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckHigh3)
			{
				wprintf(L"Bit 3 is stuck high.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckLow3)
			{
				wprintf(L"Bit 3 is stuck low.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckHigh4)
			{
				wprintf(L"Bit 4 is stuck high.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckLow4)
			{
				wprintf(L"Bit 4 is stuck low.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckHigh5)
			{
				wprintf(L"Bit 5 is stuck high.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckLow5)
			{
				wprintf(L"Bit 5 is stuck low.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckHigh6)
			{
				wprintf(L"Bit 6 is stuck high.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckLow6)
			{
				wprintf(L"Bit 6 is stuck low.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckHigh7)
			{
				wprintf(L"Bit 7 is stuck high.\n");
			}

			if (nBitStucks & CPepAnalyzeBitStuckLow7)
			{
				wprintf(L"Bit 7 is stuck low.\n");
			}
		}
	}
	else
	{
		wprintf(L"Analyze has failed.\n");
	}

	UtFreeMem(pbyData);

	CloseHandle(hFile);

	return 0;
}

static int lTestOverdump(
  LPCWSTR pszFilename)
{
	LPBYTE pbyData;
	HANDLE hFile;
	DWORD dwBytesRead;
	LARGE_INTEGER FileSize;
	BOOL bIsOverdump;

	wprintf(L"Attempting to open the file \"%s\".\n", pszFilename);

	hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Could not open the file.  (Error Code: 0x%08x).\n",
			GetLastError());

		return -1;
	}

	wprintf(L"Attempting to get the file size.\n");

	if (FALSE == GetFileSizeEx(hFile, &FileSize))
	{
		wprintf(L"Could not retrieve the file's size.  (Error Code: 0x%08x).\n",
                GetLastError());

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Attempting to allocate memory for the file.\n");

	pbyData = (LPBYTE)UtAllocMem(FileSize.LowPart);

	if (pbyData == NULL)
	{
		wprintf(L"Could not allocate memory for the file.\n");

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Attempting to read the file.\n");

	if (FALSE == ReadFile(hFile, pbyData, FileSize.LowPart, &dwBytesRead, NULL))
	{
		wprintf(L"Could not read the file.  (Error Code: 0x%08x).\n",
                GetLastError());

		UtFreeMem(pbyData);

		CloseHandle(hFile);

		return -1;
	}

	wprintf(L"Analyzing the file.\n");

	if (UtPepAnalyzeIsOverdump(pbyData, FileSize.LowPart, &bIsOverdump))
	{
		wprintf(bIsOverdump ? L"File is overdump (first half matches second half)\n" : L"File is not an overdump\n");
	}
	else
	{
		wprintf(L"Analyze has failed.\n");
	}

	UtFreeMem(pbyData);

	CloseHandle(hFile);

	return 0;
}

static int lDisplayHelp(void)
{
	wprintf(L"\n");

	UtConsolePrintAppVersion();

	wprintf(L"\n");
	wprintf(L"TestAnalyze [/blank \"Input File\"]\n");
	wprintf(L"            [/stuckbits \"Input File\"]\n");
	wprintf(L"            [/overdump \"Input File\"]\n");
	wprintf(L"\n");
	wprintf(L"    /blank\n");
	wprintf(L"        \"Input File\" - Name of the file to check if blank\n");
	wprintf(L"    /stuckbits\n");
	wprintf(L"        \"Input File\" - Name of the file to check for stuck bits\n");
	wprintf(L"    /overdump\n");
	wprintf(L"        \"Input File\" - Name of the file to check if an overdump\n");
	wprintf(L"\n");

	return -1;
}

int _cdecl wmain(int argc, WCHAR* argv[])
{
	int nResult;

	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

	if (argc == 3)
	{
		if (lstrcmpi(argv[1], L"/blank") == 0)
		{
			nResult = lTestBlank(argv[2]);
		}
		else if (lstrcmpi(argv[1], L"/stuckbits") == 0)
		{
			nResult = lTestStuckBits(argv[2]);
		}
		else if (lstrcmpi(argv[1], L"/overdump") == 0)
		{
			nResult = lTestOverdump(argv[2]);
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
