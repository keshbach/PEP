/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UtPepApp.h"

#include <Utils/UtHeapProcess.h>

#pragma region Local Variables

static HINSTANCE l_hInstance = NULL;

#pragma endregion

#pragma region Public Functions

VOID UtPepAppSetInstance(
  _In_ HINSTANCE hInstance)
{
    l_hInstance = hInstance;
}

_Ret_maybenull_z_ LPCWSTR UtPepAppAllocString(
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

VOID UtPepAppFreeString(
  _In_opt_z_ LPCWSTR pszData)
{
    UtFreeMem((LPVOID)pszData);
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
