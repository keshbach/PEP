/***************************************************************************/
/*  Copyright (C) 2007-2025 Kevin Eshbach                                  */
/***************************************************************************/

static BOOL lCreatePipeName(
  LPCWSTR pszPrefix,
  LPWSTR pszPipeName,
  INT nPipeNameLen)
{
    WCHAR cGuid[50];
    GUID guid;

    if (S_OK == CoCreateGuid(&guid) &&
        StringFromGUID2(&guid, cGuid, MArrayLen(cGuid)) &&
        S_OK == StringCchCopyW(pszPipeName, nPipeNameLen,
                               L"\\\\.\\pipe\\") &&
        S_OK == StringCchCatW(pszPipeName, nPipeNameLen, pszPrefix) &&
        S_OK == StringCchCatW(pszPipeName, nPipeNameLen, cGuid))
    {
        return TRUE;
    }

    return FALSE;
}

/***************************************************************************/
/*  Copyright (C) 2007-2025 Kevin Eshbach                                  */
/***************************************************************************/
