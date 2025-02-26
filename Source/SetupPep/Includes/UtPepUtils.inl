/***************************************************************************/
/*  Copyright (C) 2013-2023 Kevin Eshbach                                  */
/***************************************************************************/

static BOOL UtGenerateTempDirectoryName(
  _Out_writes_z_(nPathLen) LPWSTR pszPath,
  _In_ INT nPathLen)
{
    WCHAR cTempPath[MAX_PATH], cGuid[50];
    GUID guid;

    *pszPath = 0;

    if (::GetTempPath(sizeof(cTempPath) / sizeof(cTempPath[0]), cTempPath) &&
        S_OK == ::CoCreateGuid(&guid) &&
        ::StringFromGUID2(guid, cGuid, sizeof(cGuid) / sizeof(cGuid[0])) &&
        ::PathAppend(cTempPath, cGuid) &&
        S_OK == ::StringCchCopyW(pszPath, nPathLen, cTempPath))
    {
        return TRUE;
    }

    return FALSE;
}

/***************************************************************************/
/*  Copyright (C) 2013-2023 Kevin Eshbach                                  */
/***************************************************************************/
