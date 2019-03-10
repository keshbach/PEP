/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Path.h"

#include <shlwapi.h>

#pragma unmanaged

static BOOL lUnmanagedCanonicalizePath(
  LPCWSTR pszRootPath,
  LPCWSTR pszSourcePath,
  LPWSTR pszDestPath)
{
    WCHAR cParsePath[MAX_PATH];

    if (pszSourcePath[0] != L'\\')
    {
        ::PathCombineW(cParsePath, pszRootPath, pszSourcePath);
    }
    else
    {
        ::StringCchCopyW(cParsePath, sizeof(cParsePath) / sizeof(cParsePath[0]),
                         pszSourcePath);
    }

    return ::PathCanonicalizeW(pszDestPath, cParsePath);
}

#pragma managed

System::String^ Common::IO::Path::Canonicalize(
  System::String^ sRootPath,
  System::String^ sPath)
{
    pin_ptr<const wchar_t> pszRootPath = PtrToStringChars(sRootPath);
    pin_ptr<const wchar_t> pszPath = PtrToStringChars(sPath);
    WCHAR cDestPath[MAX_PATH];

    if (lUnmanagedCanonicalizePath(pszRootPath, pszPath, cDestPath))
    {
        return gcnew System::String(cDestPath);
    }

    throw gcnew System::Exception(L"The path could not be canonicalized.");
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
