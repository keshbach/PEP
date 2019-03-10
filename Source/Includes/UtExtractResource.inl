/***************************************************************************/
/*  Copyright (C) 2014-2018 Kevin Eshbach                                  */
/***************************************************************************/

#if defined(_MANAGED)
#pragma managed(push, off)
#endif

#pragma region Functions

static BOOL UtExtractResource(
  _In_ HMODULE hModule,
  _In_ INT nResourceType,
  _In_ INT nResourceName,
  _In_z_ LPCTSTR pszFile)
{
    HRSRC hResource;
    HGLOBAL hResData;
    LPVOID pvResData;
    HANDLE hFile;
    DWORD dwBytesWrote;

    hResource = FindResource(hModule, MAKEINTRESOURCE(nResourceName),
                             MAKEINTRESOURCE(nResourceType));

    if (hResource == NULL)
    {
        return FALSE;
    }

    hResData = LoadResource(hModule, hResource);

    if (hResData == NULL)
    {
        return FALSE;
    }

    pvResData = LockResource(hResData);

    if (pvResData == NULL)
    {
        return FALSE;
    }

    hFile = CreateFile(pszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (FALSE == WriteFile(hFile, pvResData, ::SizeofResource(hModule, hResource),
                             &dwBytesWrote, NULL))
    {
        CloseHandle(hFile);

        DeleteFile(pszFile);

        return FALSE;
    }

    CloseHandle(hFile);

    return TRUE;
}

#pragma endregion

#if defined(_MANAGED)
#pragma managed(pop)
#endif

/***************************************************************************/
/*  Copyright (C) 2014-2018 Kevin Eshbach                                  */
/***************************************************************************/
