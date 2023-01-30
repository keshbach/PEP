/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UtControlsHelper.h"

#include "Includes/UtMacros.h"

#include <VersionHelpers.h>

#pragma region Typedefs

typedef HRESULT(STDAPICALLTYPE* TSetWindowTheme)(HWND hWnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

#pragma endregion

#pragma unmanaged

BOOL UtControlsHelperIsEditControl(
  _In_ HWND hWnd)
{
    TCHAR cClassName[10];

    ::GetClassName(hWnd, cClassName, MArrayLen(cClassName));

    return (::lstrcmpi(cClassName, TEXT("edit")) == 0) ? TRUE : FALSE;
}

BOOL UtControlsHelperSetExplorerTheme(
  _In_ HWND hWnd)
{
    HRESULT hResult = E_FAIL;
    HMODULE hModule;
    TSetWindowTheme pSetWindowTheme;

    if (!IsWindowsVistaOrGreater())
    {
        return FALSE;
    }

    hModule = ::LoadLibraryW(L"uxtheme.dll");

    if (hModule)
    {
        pSetWindowTheme = (TSetWindowTheme)::GetProcAddress(hModule, "SetWindowTheme");

        if (pSetWindowTheme)
        {
            hResult = pSetWindowTheme(hWnd, L"Explorer", NULL);
        }

        ::FreeLibrary(hModule);
    }

    return (hResult == S_OK) ? TRUE : FALSE;
}

#pragma managed

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
