/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UtPepSetup.h"

#include <Includes/UtPepUtils.inl>

#include <Utils/UtHeapProcess.h>

#pragma region Local Variables

static HINSTANCE l_hInstance = NULL;

#pragma endregion

#pragma region Public Functions

VOID UtPepSetupSetInstance(
  _In_ HINSTANCE hInstance)
{
    l_hInstance = hInstance;
}

_Ret_maybenull_z_ LPCWSTR UtPepSetupAllocString(
  _In_ INT nResourceId)
{
    INT nStringPos = nResourceId & 0x0F;
    HRSRC hResource;
    HGLOBAL hMem;
    LPBYTE pbyData;
    WORD nStringLen;
    LPWSTR pszData;

    hResource = ::FindResourceExW(l_hInstance, RT_STRING,
                                  MAKEINTRESOURCE((nResourceId / 16) + 1),
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

    if (hResource == NULL)
    {
        return NULL;
    }

    hMem = ::LoadResource(l_hInstance, hResource);

    if (hMem == NULL)
    {
        return NULL;
    }

    pbyData = (LPBYTE)::LockResource(hMem);

    if (pbyData == NULL)
    {
        return NULL;
    }

    for (INT nCurStringPos = 0; nCurStringPos < nStringPos; ++nCurStringPos)
    {
        nStringLen = *((LPWORD)pbyData);

        pbyData += ((nStringLen * sizeof(WCHAR)) + sizeof(WORD));
    }

    nStringLen = *((LPWORD)pbyData);

    pbyData += sizeof(WORD);

    pszData = (LPWSTR)UtAllocMem((nStringLen + 1) * sizeof(WCHAR));

    if (pszData == NULL)
    {
        return NULL;
    }

    ::StringCchCopyW(pszData, nStringLen + 1, (LPWSTR)pbyData);

    return pszData;
}

VOID UtPepSetupFreeString(
  _In_opt_z_ LPCWSTR pszData)
{
    UtFreeMem((LPVOID)pszData);
}

BOOL UtPepSetupAppendLine(
  _In_z_ LPCWSTR pszLogFile,
  _In_z_ LPCWSTR pszMessage)
{
    BOOL bResult(FALSE);
    HANDLE hFile;
    LARGE_INTEGER FileSize;
    DWORD dwBytesWrote;

    // if have no file then add unicode indicator to beginnning

    hFile = ::CreateFile(pszLogFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (::GetFileSizeEx(hFile, &FileSize) &&
        ::SetFilePointerEx(hFile, FileSize, NULL, FILE_BEGIN) &&
        ::WriteFile(hFile, pszMessage, ::lstrlenW(pszMessage), &dwBytesWrote, NULL))
    {
        bResult = TRUE;
    }

    ::CloseHandle(hFile);

    return bResult;
}

BOOL UtPepSetupConvertAnsiLogFileToUnicodeLogFile(
  _In_z_ LPCWSTR pszAnsiLogFile,
  _In_z_ LPCWSTR pszUnicodeLogFile)
{
    BOOL bResult(FALSE);
    HANDLE hFile;
    LARGE_INTEGER LargeInteger;
    LPSTR pszAnsiBuffer;
    LPWSTR pszUnicodeBuffer;
    DWORD dwBytesRead;
    INT nResult;

    hFile = ::CreateFile(pszAnsiLogFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!::GetFileSizeEx(hFile, &LargeInteger))
    {
        ::CloseHandle(hFile);

        return FALSE;
    }

    pszAnsiBuffer = (LPSTR)UtAllocMem(LargeInteger.LowPart * sizeof(CHAR));

    if (pszAnsiBuffer == NULL)
    {
        ::CloseHandle(hFile);

        return FALSE;
    }

    if (!::ReadFile(hFile, pszAnsiBuffer, LargeInteger.LowPart, &dwBytesRead, NULL))
    {
        UtFreeMem(pszAnsiBuffer);

        ::CloseHandle(hFile);

        return FALSE;
    }

    nResult = ::MultiByteToWideChar(CP_UTF8, 0, pszAnsiBuffer, dwBytesRead, NULL, 0);

    if (nResult <= 0)
    {
        UtFreeMem(pszAnsiBuffer);

        ::CloseHandle(hFile);

        return FALSE;
    }

    pszUnicodeBuffer = (LPWSTR)UtAllocMem((nResult + 1) * sizeof(WCHAR));

    if (pszUnicodeBuffer == NULL)
    {
        UtFreeMem(pszAnsiBuffer);

        ::CloseHandle(hFile);

        return FALSE;
    }

    if (!::MultiByteToWideChar(CP_UTF8, 0, pszAnsiBuffer, dwBytesRead, pszUnicodeBuffer, nResult))
    {
        UtFreeMem(pszAnsiBuffer);
        UtFreeMem(pszUnicodeBuffer);

        ::CloseHandle(hFile);

        return FALSE;
    }

    UtFreeMem(pszAnsiBuffer);

    ::CloseHandle(hFile);

    pszUnicodeBuffer[nResult] = 0;

    bResult = UtPepSetupAppendLine(pszUnicodeLogFile, pszUnicodeBuffer);

    UtFreeMem(pszUnicodeBuffer);

    return bResult;
}

BOOL UtPepSetupGenerateTempDirectoryName(
  _Out_writes_z_(nPathLen) LPWSTR pszPath,
  _In_ INT nPathLen)
{
    return UtGenerateTempDirectoryName(pszPath, nPathLen);
}

BOOL UtPepSetupIsWindows64Present()
{
    return UtIsWindows64Present();
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
