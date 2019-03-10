
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "File.h"

#include <shlwapi.h>

#pragma unmanaged

static BOOL lUnmanagedGenerateTempFile(
    LPCWSTR pszFileExt,
    LPWSTR pszTempFile,
    INT nTempFileLen)
{
    WCHAR cTempPath[MAX_PATH], cTempFile[MAX_PATH], cGuid[50];
    GUID guid;

    if (::GetTempPathW(sizeof(cTempPath) / sizeof(cTempPath[0]), cTempPath) &&
        S_OK == ::CoCreateGuid(&guid) &&
        ::StringFromGUID2(guid, cGuid, sizeof(cGuid) / sizeof(cGuid[0])))
    {
        ::PathCombineW(cTempFile, cTempPath, cGuid);
        ::PathAddExtensionW(cTempFile, pszFileExt);

        if (S_OK == ::StringCchCopyW(pszTempFile, nTempFileLen, cTempFile))
        {
            return TRUE;
        }
    }

    return FALSE;
}

#pragma managed

System::String^ Common::IO::File::GenerateTempFileName(
    System::String^ sFileExtension)
{
    pin_ptr<const wchar_t> pszFileExtension = PtrToStringChars(sFileExtension);
    WCHAR cFile[MAX_PATH];

    if (lUnmanagedGenerateTempFile(pszFileExtension, cFile,
        sizeof(cFile) / sizeof(cFile[0])))
    {
        return gcnew System::String(cFile);
    }

    throw gcnew System::Exception(L"A temporary file name could not be generated.");
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
