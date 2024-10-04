/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include "IntelHexData.h"
#include "IntelHex.h"

#include <UtilsDevice/UtIntelHex.h>

#include <Utils/UtHeap.h>

#define CIntelHexLibrary L"UtIntelHex.dll"

typedef BOOL (UTINTELHEXAPI* TUtIntelHexInitializeFunc)(VOID);
typedef BOOL (UTINTELHEXAPI* TUtIntelHexUninitializeFunc)(VOID);
typedef TUtIntelHexHandle (UTINTELHEXAPI* TUtIntelHexLoadFileFunc)(LPCWSTR pszFile);
typedef TUtIntelHexHandle (UTINTELHEXAPI* TUtIntelHexFromMemoryFunc)(LPVOID pvMemory, UINT32 nMemoryLen);
typedef BOOL (UTINTELHEXAPI* TUtIntelHexFreeDataFunc)(TUtIntelHexHandle handle);
typedef BOOL (UTINTELHEXAPI* TUtIntelHexTotalPagesFunc)(TUtIntelHexHandle handle, UINT32* pnTotalPages);
typedef BOOL (UTINTELHEXAPI* TUtIntelHexGetPageAddressFunc)(TUtIntelHexHandle handle, UINT32 nPageNumber, UINT32* pnAddress);
typedef BOOL (UTINTELHEXAPI* TUtIntelHexGetPageAddressDataFunc)(TUtIntelHexHandle handle, UINT32 nPageNumber, const UINT8** ppData, UINT32* pnDataLen);
typedef BOOL (UTINTELHEXAPI* TUtIntelHexErrorLineNumberFunc)(TUtIntelHexHandle handle, UINT32* pnLineNumber);
typedef BOOL (UTINTELHEXAPI* TUtIntelHexErrorCodeFunc)(TUtIntelHexHandle handle, UINT16* pnErrorCode);

static System::String^ lFormatErrorMessage(
  _In_ UINT16 nErrorCode,
  _In_ UINT32 nLineNumber)
{
	switch (nErrorCode)
	{
		case UtIntelHexErrorCodeOutOfMemory:
			return gcnew System::String(L"Out of memory loading the intel hex file");
		case UtIntelHexErrorCodeMinRecordLenNotSatisfied:
			return gcnew System::String(System::String::Format(L"Minimum record length not satisfied (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeInvalidByteCount:
			return gcnew System::String(System::String::Format(L"Invalid byte count (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeInvalidAddress:
			return gcnew System::String(System::String::Format(L"Invalid address (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeInvalidRecordType:
			return gcnew System::String(System::String::Format(L"Invalid record type (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeUnrecognizedRecordType:
			return gcnew System::String(System::String::Format(L"Unrecognized record type (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeInvalidDataLen:
			return gcnew System::String(System::String::Format(L"Invalid data length (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeInvalidData:
			return gcnew System::String(System::String::Format(L"Invalid data (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeInvalidChecksum:
			return gcnew System::String(System::String::Format(L"Invalid checksum (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeChecksumMismatch:
			return gcnew System::String(System::String::Format(L"Checksum mismatch (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeInvalidFieldByteCount:
			return gcnew System::String(System::String::Format(L"Invalid field byte count (Line #{0})", nLineNumber));
		case UtIntelHexErrorCodeUnsupportedRecordType:
			return gcnew System::String(System::String::Format(L"Unsupported record type (Line #{0})", nLineNumber));
	}

	return gcnew System::String(System::String::Format(L"Unknown error (Line #{0})", nLineNumber));
}

static Pep::Programmer::IntelHexData^ lCreateIntelHexData(
  TUtIntelHexHandle IntelHexHandle,
  TUtIntelHexTotalPagesFunc pIntelHexTotalPagesFunc,
  TUtIntelHexGetPageAddressFunc pIntelHexGetPageAddressFunc,
  TUtIntelHexGetPageAddressDataFunc pIntelHexGetPageAddressDataFunc)
{
	UINT32 nTotalPages = 0;
	UINT32* pnAddress = nullptr;
	UINT8** ppData = nullptr;
	UINT32* pnDataLen = nullptr;
	Pep::Programmer::IntelHexData^ IntelHexData;

	if (!UtInitHeap())
	{
		return nullptr;
	}

	pIntelHexTotalPagesFunc(IntelHexHandle, &nTotalPages);

	pnAddress = (UINT32*)UtAllocMem(nTotalPages * sizeof(UINT32));
	ppData = (UINT8**)UtAllocMem(nTotalPages * sizeof(UINT8*));
	pnDataLen = (UINT32*)UtAllocMem(nTotalPages * sizeof(UINT32));

	if (pnAddress == nullptr || ppData == nullptr || pnDataLen == nullptr)
	{
		if (pnAddress)
		{
			UtFreeMem(pnAddress);
		}

		if (ppData)
		{
			UtFreeMem(ppData);
		}

		if (pnDataLen)
		{
			UtFreeMem(pnDataLen);
		}

		UtUninitHeap();

		return nullptr;
	}

    for (UINT32 nPageNumber = 0; nPageNumber < nTotalPages; ++nPageNumber)
    {
        pIntelHexGetPageAddressFunc(IntelHexHandle,
                                    nPageNumber,
                                    &pnAddress[nPageNumber]);

        pIntelHexGetPageAddressDataFunc(IntelHexHandle,
                                        nPageNumber,
                                        (const UINT8**)&ppData[nPageNumber],
                                        &pnDataLen[nPageNumber]);
    }

	IntelHexData = gcnew Pep::Programmer::IntelHexData(nTotalPages, pnAddress, (const UINT8**)ppData, pnDataLen);

	UtFreeMem(pnAddress);
	UtFreeMem(ppData);
	UtFreeMem(pnDataLen);

	UtUninitHeap();

	return IntelHexData;
}

System::Boolean Pep::Programmer::IntelHex::LoadIntelHexFile(
	System::String^ sFile,
	IntelHexData^% IntelHexData,
	System::String^% sErrorMessage)
{
	pin_ptr<const wchar_t> pszName = PtrToStringChars(sFile);
	HMODULE hModule = ::LoadLibrary(CIntelHexLibrary);
	TUtIntelHexInitializeFunc pIntelHexInitializeFunc;
	TUtIntelHexUninitializeFunc pIntelHexUninitializeFunc;
	TUtIntelHexLoadFileFunc pIntelHexLoadFileFunc;
	TUtIntelHexFreeDataFunc pIntelHexFreeDataFunc;
	TUtIntelHexTotalPagesFunc pIntelHexTotalPagesFunc;
	TUtIntelHexGetPageAddressFunc pIntelHexGetPageAddressFunc;
	TUtIntelHexGetPageAddressDataFunc pIntelHexGetPageAddressDataFunc;
	TUtIntelHexErrorLineNumberFunc pIntelHexErrorLineNumberFunc;
	TUtIntelHexErrorCodeFunc pIntelHexErrorCodeFunc;
	TUtIntelHexHandle IntelHexHandle;
	UINT32 nIntelHexLineNumber;
	UINT16 nIntelHexErrorCode;

	IntelHexData = nullptr;
	sErrorMessage = nullptr;

	if (hModule == NULL)
	{
		sErrorMessage = gcnew System::String(L"Could not find the IntelHex.dll file.");

		return false;
	}

	pIntelHexInitializeFunc = (TUtIntelHexInitializeFunc)::GetProcAddress(hModule, "UtIntelHexInitialize");
	pIntelHexUninitializeFunc = (TUtIntelHexUninitializeFunc)::GetProcAddress(hModule, "UtIntelHexUninitialize");
	pIntelHexLoadFileFunc = (TUtIntelHexLoadFileFunc)::GetProcAddress(hModule, "UtIntelHexLoadFile");
	pIntelHexFreeDataFunc = (TUtIntelHexFreeDataFunc)::GetProcAddress(hModule, "UtIntelHexFreeData");
	pIntelHexTotalPagesFunc = (TUtIntelHexTotalPagesFunc)::GetProcAddress(hModule, "UtIntelHexTotalPages");
	pIntelHexGetPageAddressFunc = (TUtIntelHexGetPageAddressFunc)::GetProcAddress(hModule, "UtIntelHexGetPageAddress");
	pIntelHexGetPageAddressDataFunc = (TUtIntelHexGetPageAddressDataFunc)::GetProcAddress(hModule, "UtIntelHexGetPageAddressData");
	pIntelHexErrorLineNumberFunc = (TUtIntelHexErrorLineNumberFunc)::GetProcAddress(hModule, "UtIntelHexErrorLineNumber");
	pIntelHexErrorCodeFunc = (TUtIntelHexErrorCodeFunc)::GetProcAddress(hModule, "UtIntelHexErrorCode");

	if (pIntelHexInitializeFunc == NULL ||
		pIntelHexUninitializeFunc == NULL ||
		pIntelHexLoadFileFunc == NULL ||
		pIntelHexFreeDataFunc == NULL ||
		pIntelHexTotalPagesFunc == NULL ||
		pIntelHexGetPageAddressFunc == NULL ||
		pIntelHexGetPageAddressDataFunc == NULL ||
		pIntelHexErrorLineNumberFunc == NULL ||
		pIntelHexErrorCodeFunc == NULL)
	{
		::FreeLibrary(hModule);

		sErrorMessage = gcnew System::String(L"Could not load the IntelHex.dll file.");

		return false;
	}

	if (pIntelHexInitializeFunc() == FALSE)
	{
		::FreeLibrary(hModule);

		sErrorMessage = gcnew System::String(L"Could not initialize the IntelHex.dll file.");

		return false;
	}

	IntelHexHandle = pIntelHexLoadFileFunc(pszName);

	if (IntelHexHandle == NULL)
	{
		pIntelHexUninitializeFunc();

		::FreeLibrary(hModule);

		sErrorMessage = gcnew System::String(L"Out of memory error.");

		return true;
	}

	pIntelHexErrorCodeFunc(IntelHexHandle, &nIntelHexErrorCode);
	pIntelHexErrorLineNumberFunc(IntelHexHandle, &nIntelHexLineNumber);

	if (nIntelHexErrorCode != UtIntelHexErrorCodeSuccess)
	{
		pIntelHexFreeDataFunc(IntelHexHandle);

		pIntelHexUninitializeFunc();

		::FreeLibrary(hModule);

		sErrorMessage = lFormatErrorMessage(nIntelHexErrorCode, nIntelHexLineNumber);

		return false;
	}

	IntelHexData = lCreateIntelHexData(IntelHexHandle,
                                       pIntelHexTotalPagesFunc,
                                       pIntelHexGetPageAddressFunc,
                                       pIntelHexGetPageAddressDataFunc);

	pIntelHexFreeDataFunc(IntelHexHandle);

	pIntelHexUninitializeFunc();

	::FreeLibrary(hModule);

	if (IntelHexData == nullptr)
	{
		sErrorMessage = gcnew System::String(L"Could not initialize heap memory.");

		return false;
	}

	return true;
}

System::Boolean Pep::Programmer::IntelHex::IntelHexFromMemory(
	array<System::Byte>^ byData,
	IntelHexData^% IntelHexData,
	System::String^% sErrorMessage)
{
	pin_ptr<System::Byte> pbyData = &byData[0];
	HMODULE hModule = ::LoadLibrary(CIntelHexLibrary);
	TUtIntelHexInitializeFunc pIntelHexInitializeFunc;
	TUtIntelHexUninitializeFunc pIntelHexUninitializeFunc;
	TUtIntelHexFromMemoryFunc pIntelHexFromMemoryFunc;
	TUtIntelHexFreeDataFunc pIntelHexFreeDataFunc;
	TUtIntelHexTotalPagesFunc pIntelHexTotalPagesFunc;
	TUtIntelHexGetPageAddressFunc pIntelHexGetPageAddressFunc;
	TUtIntelHexGetPageAddressDataFunc pIntelHexGetPageAddressDataFunc;
	TUtIntelHexErrorLineNumberFunc pIntelHexErrorLineNumberFunc;
	TUtIntelHexErrorCodeFunc pIntelHexErrorCodeFunc;
	TUtIntelHexHandle IntelHexHandle;
	UINT32 nIntelHexLineNumber;
	UINT16 nIntelHexErrorCode;

	IntelHexData = nullptr;
	sErrorMessage = nullptr;

	if (hModule == NULL)
	{
		sErrorMessage = gcnew System::String(L"Could not find the IntelHex.dll file.");

		return false;
	}

	pIntelHexInitializeFunc = (TUtIntelHexInitializeFunc)::GetProcAddress(hModule, "UtIntelHexInitialize");
	pIntelHexUninitializeFunc = (TUtIntelHexUninitializeFunc)::GetProcAddress(hModule, "UtIntelHexUninitialize");
	pIntelHexFromMemoryFunc = (TUtIntelHexFromMemoryFunc)::GetProcAddress(hModule, "UtIntelHexFromMemory");
	pIntelHexFreeDataFunc = (TUtIntelHexFreeDataFunc)::GetProcAddress(hModule, "UtIntelHexFreeData");
	pIntelHexTotalPagesFunc = (TUtIntelHexTotalPagesFunc)::GetProcAddress(hModule, "UtIntelHexTotalPages");
	pIntelHexGetPageAddressFunc = (TUtIntelHexGetPageAddressFunc)::GetProcAddress(hModule, "UtIntelHexGetPageAddress");
	pIntelHexGetPageAddressDataFunc = (TUtIntelHexGetPageAddressDataFunc)::GetProcAddress(hModule, "UtIntelHexGetPageAddressData");
	pIntelHexErrorLineNumberFunc = (TUtIntelHexErrorLineNumberFunc)::GetProcAddress(hModule, "UtIntelHexErrorLineNumber");
	pIntelHexErrorCodeFunc = (TUtIntelHexErrorCodeFunc)::GetProcAddress(hModule, "UtIntelHexErrorCode");

	if (pIntelHexInitializeFunc == NULL ||
		pIntelHexUninitializeFunc == NULL ||
		pIntelHexFromMemoryFunc == NULL ||
		pIntelHexFreeDataFunc == NULL ||
		pIntelHexTotalPagesFunc == NULL ||
		pIntelHexGetPageAddressFunc == NULL ||
		pIntelHexGetPageAddressDataFunc == NULL ||
		pIntelHexErrorLineNumberFunc == NULL ||
		pIntelHexErrorCodeFunc == NULL)
	{
		::FreeLibrary(hModule);

		sErrorMessage = gcnew System::String(L"Could not load the IntelHex.dll file.");

		return false;
	}

	if (pIntelHexInitializeFunc() == FALSE)
	{
		::FreeLibrary(hModule);

		sErrorMessage = gcnew System::String(L"Could not initialize the IntelHex.dll file.");

		return false;
	}

	IntelHexHandle = pIntelHexFromMemoryFunc(pbyData, byData->Length);

	if (IntelHexHandle == NULL)
	{
		pIntelHexUninitializeFunc();

		::FreeLibrary(hModule);

		sErrorMessage = gcnew System::String(L"Out of memory error.");

		return true;
	}

	pIntelHexErrorCodeFunc(IntelHexHandle, &nIntelHexErrorCode);
	pIntelHexErrorLineNumberFunc(IntelHexHandle, &nIntelHexLineNumber);

	if (nIntelHexErrorCode != UtIntelHexErrorCodeSuccess)
	{
		pIntelHexFreeDataFunc(IntelHexHandle);

		pIntelHexUninitializeFunc();

		::FreeLibrary(hModule);

		sErrorMessage = lFormatErrorMessage(nIntelHexErrorCode, nIntelHexLineNumber);

		return false;
	}

    IntelHexData = lCreateIntelHexData(IntelHexHandle,
                                       pIntelHexTotalPagesFunc,
                                       pIntelHexGetPageAddressFunc,
                                       pIntelHexGetPageAddressDataFunc);

	pIntelHexFreeDataFunc(IntelHexHandle);

	pIntelHexUninitializeFunc();

	::FreeLibrary(hModule);

	if (IntelHexData == nullptr)
	{
		sErrorMessage = gcnew System::String(L"Could not initialize heap memory.");

		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
