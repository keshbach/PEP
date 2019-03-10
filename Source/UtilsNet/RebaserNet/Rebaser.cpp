/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Rebaser.h"

static HANDLE l_hHeap = NULL;

#pragma unmanaged

static BOOL lInitHeap(VOID)
{
    if (l_hHeap == NULL)
    {
        l_hHeap = ::HeapCreate(0, 0, 0);

        if (l_hHeap)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL lUninitHeap(VOID)
{
    BOOL bResult = FALSE;

    if (l_hHeap)
    {
        bResult = ::HeapDestroy(l_hHeap);

        l_hHeap = NULL;
    }

    return bResult;
}

static LPVOID lAllocMem(
  ULONG ulMemLen)
{
    if (l_hHeap)
    {
        return ::HeapAlloc(l_hHeap, HEAP_ZERO_MEMORY, ulMemLen);
    }

    return NULL;
}

static BOOL lFreeMem(
  LPVOID pvMem)
{
    if (l_hHeap)
    {
        return ::HeapFree(l_hHeap, 0, pvMem);
    }

    return FALSE;
}

static LPSTR lAllocString(
  LPCWSTR pszString)
{
    INT nStringLen;
    LPSTR pszNewString;

    nStringLen = ::WideCharToMultiByte(CP_ACP, 0, pszString, -1, NULL, 0,
                                       NULL, NULL) + 1;

    if (nStringLen == 0)
    {
        return NULL;
    }

    pszNewString = (LPSTR)lAllocMem(nStringLen);

    if (pszNewString == NULL)
    {
        return NULL;
    }

    nStringLen = ::WideCharToMultiByte(CP_ACP, 0, pszString, -1, pszNewString,
                                       nStringLen,  NULL, NULL);

    if (nStringLen == 0)
    {
        lFreeMem(pszNewString);

        return NULL;
    }

    return pszNewString;
}

static VOID lFreeString(
  LPSTR pszString)
{
    lFreeMem(pszString);
}

#pragma managed

System::Boolean Common::Rebaser::GetFileImageBase(
  System::String^ sFile,
  System::UInt64% nImageBase,
  System::String^% sErrorMsg)
{
    pin_ptr<const wchar_t> pszFile = PtrToStringChars(sFile);
    System::Boolean bResult = false;
    HANDLE hFile = NULL;
    HANDLE hFileMapping = NULL;
    LPVOID pvData = NULL;
    PIMAGE_NT_HEADERS pImageHeaders = NULL;

    nImageBase = 0;
    sErrorMsg = L"";

    hFile = ::CreateFileW(pszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        sErrorMsg = L"Could not open the file.";

        goto End;
    }

    hFileMapping = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (hFileMapping == NULL)
    {
        sErrorMsg = L"Could not open a file mapping to the file.";

        goto End;
    }

    pvData = ::MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

    if (pvData == NULL)
    {
        sErrorMsg = L"Could not map a view of the file.";

        goto End;
    }

    pImageHeaders = ::ImageNtHeader(pvData);

    if (pImageHeaders == NULL)
    {
        sErrorMsg = L"Could not get the image header of the file.";

        goto End;
    }

    nImageBase = pImageHeaders->OptionalHeader.ImageBase;

    bResult = true;

End:
    if (pvData)
    {
        ::UnmapViewOfFile(pvData);
    }

    if (hFileMapping)
    {
        ::CloseHandle(hFileMapping);
    }

    if (hFile)
    {
        ::CloseHandle(hFile);
    }

    return bResult;
}

System::Boolean Common::Rebaser::ReBaseFile(
  System::String^ sFile,
  System::String^ sSymbolPath,
  System::UInt64 nNewImageBase,
  System::String^% sErrorMsg)
{
    pin_ptr<const wchar_t> pszFile = PtrToStringChars(sFile);
    pin_ptr<const wchar_t> pszSymbolPath = PtrToStringChars(sSymbolPath);
    System::Boolean bResult = false;
    LPSTR pszTmpFile, pszTmpSymbolPath;
    ULONG64 ulOldImageBase, ulNewImageBase;
    ULONG ulOldImageSize, ulNewImageSize;

    sErrorMsg = L"";

    if (FALSE == lInitHeap())
    {
        sErrorMsg = L"Could not initialize a local heap.";

        return false;
    }

    pszTmpFile = lAllocString(pszFile);
    pszTmpSymbolPath = lAllocString(pszSymbolPath);

    if (pszTmpFile == NULL || pszTmpSymbolPath == NULL)
    {
        sErrorMsg = L"Could not allocate temporary memory.";

        bResult = false;

        goto End;
    }

    ulOldImageBase = 0;
    ulNewImageBase = nNewImageBase;
    ulOldImageSize = 0;
    ulNewImageSize = 0;

    ::ReBaseImage64(pszTmpFile, pszTmpSymbolPath, TRUE, FALSE, FALSE, 0,
                    &ulOldImageSize, &ulOldImageBase,
                    &ulNewImageSize, &ulNewImageBase, 0);

    if (::GetLastError() == NOERROR)
    {
        bResult = true;
    }
    else
    {
        sErrorMsg = L"File could not be rebased.";
    }

End:
    if (pszTmpFile)
    {
        lFreeString(pszTmpFile);
    }

    if (pszTmpSymbolPath)
    {
        lFreeString(pszTmpSymbolPath);
    }

    if (FALSE == lUninitHeap())
    {
        if (bResult)
        {
            sErrorMsg = L"Could not uninitialize a local heap.";

            bResult = false;
        }
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
