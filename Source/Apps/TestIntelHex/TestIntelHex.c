/***************************************************************************/
/*  Copyright (C) 2019-2024 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include <UtilsDevice/UtIntelHex.h>

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

static VOID lDumpErrorCode(
  TUtIntelHexHandle hIntelHex)
{
	UINT16 nErrorCode;
	UINT32 nLineNumber;

	wprintf(L"Attempting to retrieve the error code.\n");

	if (UtIntelHexErrorCode(hIntelHex, &nErrorCode))
	{
		switch (nErrorCode)
		{
			case UtIntelHexErrorCodeSuccess:
				wprintf(L"Error Code: Success\n");
				break;
			case UtIntelHexErrorCodeFileError:
				wprintf(L"Error Code: File Error\n");
				break;
			case UtIntelHexErrorCodeOutOfMemory:
				wprintf(L"Error Code: Out of Memory\n");
				break;
			case UtIntelHexErrorCodeMinRecordLenNotSatisfied:
				wprintf(L"Error Code: Minimum Record Length not Satisfied\n");
				break;
			case UtIntelHexErrorCodeInvalidByteCount:
				wprintf(L"Error Code: Invalid Byte Count\n");
				break;
			case UtIntelHexErrorCodeInvalidAddress:
				wprintf(L"Error Code: Invalid Address\n");
				break;
			case UtIntelHexErrorCodeInvalidRecordType:
				wprintf(L"Error Code: Invalid Record Type\n");
				break;
			case UtIntelHexErrorCodeUnrecognizedRecordType:
				wprintf(L"Error Code: Unrecognized Record Type\n");
				break;
			case UtIntelHexErrorCodeInvalidDataLen:
				wprintf(L"Error Code: Invalid Data Length\n");
				break;
			case UtIntelHexErrorCodeInvalidData:
				wprintf(L"Error Code: Invalid Data\n");
				break;
			case UtIntelHexErrorCodeInvalidChecksum:
				wprintf(L"Error Code: Invalid Checksum\n");
				break;
			case UtIntelHexErrorCodeChecksumMismatch:
				wprintf(L"Error Code: Checksum Mismatch\n");
				break;
			case UtIntelHexErrorCodeInvalidFieldByteCount:
				wprintf(L"Error Code: Invalid Field Byte Count\n");
				break;
			case UtIntelHexErrorCodeUnsupportedRecordType:
				wprintf(L"Error Code: Unsupported Record Type\n");
				break;
			default:
				wprintf(L"Error Code: 0x%04X\n", nErrorCode);
				break;
		}

		if (nErrorCode != UtIntelHexErrorCodeSuccess)
		{
			wprintf(L"Attempting to retrieve the line number.\n");

			if (UtIntelHexErrorLineNumber(hIntelHex, &nLineNumber))
			{
				wprintf(L"Line Number: %d\n", nLineNumber);
			}
			else
			{
				wprintf(L"Failed to retrieve the line number.\n");
			}
		}
	}
	else
	{
		wprintf(L"Failed to retrieve the error code.\n");
	}
}

static int lTestDump(
  LPCWSTR pszFilename)
{
	TUtIntelHexHandle hIntelHex;
	UINT32 nTotalPages, nAddress, nDataLen;
    const UINT8* pData;

	wprintf(L"Attempting to initialize the library.\n");

	if (FALSE == UtIntelHexInitialize())
	{
		wprintf(L"Failed to initialize the library.\n");

		return -1;
	}

	wprintf(L"Successfully initialized the library.\n");

	wprintf(L"Attempting to load the hex file.\n");

	hIntelHex = UtIntelHexLoadFile(pszFilename);

	if (hIntelHex)
	{
		wprintf(L"Hex file loaded.\n");

		lDumpErrorCode(hIntelHex);

		wprintf(L"Attempting to retrieve the total pages.\n");

		if (UtIntelHexTotalPages(hIntelHex, &nTotalPages))
		{
			wprintf(L"Total Pages: %d\n", nTotalPages);

			for (UINT32 nPageNumber = 0; nPageNumber < nTotalPages; ++nPageNumber)
			{
				wprintf(L"Attempting to retrieve the address of Page Number: %d\n", nPageNumber);

				if (UtIntelHexGetPageAddress(hIntelHex, nPageNumber, &nAddress))
				{
					wprintf(L"Page Number: %d, Address: 0x%08X\n", nPageNumber, nAddress);

					wprintf(L"Attempting to retrieve the data of Page Number: %d\n", nPageNumber);

					if (UtIntelHexGetPageAddressData(hIntelHex, nPageNumber, &pData, &nDataLen))
					{
						wprintf(L"Page Number: %d, Data Length: 0x%08X\n", nPageNumber, nDataLen);

						for (UINT32 nIndex = 0; nIndex < nDataLen && !l_bQuit; ++nIndex)
						{
							wprintf(L"%02X ", pData[nIndex]);
						}

						wprintf(L"\n");

						if (l_bQuit)
						{
							wprintf(L"**** Cancelled ****\n");
						}
					}
					else
					{
						wprintf(L"Failed to retrieve the data of Page Number: %d.\n", nPageNumber);
					}
				}
				else
				{
					wprintf(L"Failed to retrieve the address of Page Number: %d.\n", nPageNumber);
				}
			}
		}
		else
		{
			wprintf(L"Failed to retrieve the total pages.\n");
		}

		UtIntelHexFreeData(hIntelHex);
	}
	else
	{
		wprintf(L"Failed to load the hex file.\n");
	}

	wprintf(L"Attempting to uninitialize the library.\n");

	if (FALSE == UtIntelHexUninitialize())
	{
		wprintf(L"Failed to uninitialize the library.\n");

		return -1;
	}

	wprintf(L"Successfully uninitialized the library.\n");

	return 0;
}

static int lDisplayHelp(void)
{
	wprintf(L"\n");

	UtConsolePrintAppVersion();

	wprintf(L"\n");
	wprintf(L"TestIntelHex [\"Input File\"]\n");
	wprintf(L"\n");
	wprintf(L"    \"Input File\" - Name of the file to dump\n");
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
		nResult = lTestDump(argv[1]);
	}
	else
	{
		nResult = lDisplayHelp();
	}

	SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

	return nResult;
}

/***************************************************************************/
/*  Copyright (C) 2019-2024 Kevin Eshbach                                  */
/***************************************************************************/
