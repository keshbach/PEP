/***************************************************************************/
/*  Copyright (C) 2009-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>
#include <assert.h>

#include <windows.h>

#include <Includes/UtMacros.h>

#include <Utils/UtConsole.h>

#pragma region "Structures"

#pragma pack(push, r1, 1)

typedef struct tagTHeader
{
    WORD wLength;
    WORD wValueLength;
    WORD wType; 
    WCHAR szKey[16];
} THeader;

typedef struct tagString
{
    WORD wLength;
    WORD wValueLength;
    WORD wType; 
    WCHAR szKey[1];
} String;

typedef struct tagStringTable
{
    WORD wLength;
    WORD wValueLength;
    WORD wType; 
    WCHAR szKey[9];
    String Children[1];
} StringTable;

typedef struct tagStringFileInfo
{
    WORD wLength;
    WORD wValueLength; 
    WORD wType;
    WCHAR szKey[14];
    WORD Padding[1];
    StringTable Children[1];
} StringFileInfo; 

typedef struct tagVar
{ 
  WORD wLength; 
  WORD wValueLength; 
  WORD wType; 
  WCHAR szKey[1]; 
  WORD Padding[1]; 
  DWORD Value[1]; 
} Var;

typedef struct tagVarFileInfo
{
    WORD wLength;
    WORD wValueLength;
    WORD wType; 
    WCHAR szKey[12];
    WORD Padding[1];
    Var Children[1];
} VarFileInfo; 

typedef struct tagVS_VERSIONINFO
{
    WORD wLength;
    WORD wValueLength;
    WORD wType; 
    WCHAR szKey[16];
    WORD Padding1[1];
    VS_FIXEDFILEINFO Value;
    WORD Children[1];
} VS_VERSIONINFO;

#pragma pack(pop, r1)

#pragma endregion

#pragma region "Local Functions"

static LPWSTR lGetValueAfterKey(
  LPWSTR pszKey)
{
    INT nKeyLen = lstrlenW(pszKey);
    LPWSTR pszValue = pszKey;

    pszValue += nKeyLen + 1;
  
    if (nKeyLen & 0x01)
    {
        ++pszValue;
    }

    return pszValue;
}

#pragma endregion

BOOL UTCONSOLEAPI UtConsolePrintDipSwitches(
  LPBOOL pbDipSwitches,
  ULONG ulTotalDipSwitches)
{
    ULONG ulIndex;

    for (ulIndex = 0; ulIndex < ulTotalDipSwitches; ++ulIndex)
    {
        wprintf(L"%d - ", ulIndex + 1);

        if (TRUE == pbDipSwitches[ulIndex])
        {
            wprintf(L"On");
        }
        else
        {
            wprintf(L"Off");
        }

        if (ulIndex + 1 < ulTotalDipSwitches)
        {
            wprintf(L", ");
        }
    }

    wprintf(L"\n");

    return TRUE;
}

BOOL UTCONSOLEAPI UtConsolePrintAppVersion(VOID)
{
    HRSRC hResource;
    HGLOBAL hGlobal;
    LPVOID pvData;
    VS_VERSIONINFO* pVersionInfo;
    LPBYTE pbyEndData, pbyTmpEndData, pbyCurData;
    THeader* pHeader;
    StringFileInfo* pStringFileInfo;
    StringTable* pStringTable;
    String* pString;
    LPWSTR pszKey, pszValue;
    VarFileInfo* pVarFileInfo;
    Var* pVar;
    LPCWSTR pszTitle, pszCopyright;

    hResource = FindResourceEx(GetModuleHandle(NULL), RT_VERSION,
                               MAKEINTRESOURCE(VS_VERSION_INFO),
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

    if (hResource == NULL)
    {
        return FALSE;
    }

    hGlobal = LoadResource(GetModuleHandle(NULL), hResource);

    if (hGlobal == NULL)
    {
        return FALSE;
    }

    pvData = LockResource(hGlobal);

    if (pvData == NULL)
    {
        return FALSE;
    }

    pVersionInfo = (VS_VERSIONINFO*)pvData;

    if (pVersionInfo->wValueLength == 0)
    {
        // No version data

        return FALSE;
    }

    if (pVersionInfo->Value.dwSignature != 0xFEEF04BD)
    {
        return FALSE;
    }

    pbyEndData = (LPBYTE)pVersionInfo + pVersionInfo->wLength;
    pbyCurData = (LPBYTE)pVersionInfo->Children;

    pszTitle = NULL;
    pszCopyright = NULL;

    while (pbyCurData < pbyEndData)
    {
        pHeader = (THeader*)pbyCurData;

        if (0 == lstrcmpiW(pHeader->szKey, L"StringFileInfo"))
        {
            pStringFileInfo = (StringFileInfo*)pbyCurData;

            pStringTable = (StringTable*)pStringFileInfo->Children;

            pbyTmpEndData = pbyCurData + pStringTable->wLength;
            pbyCurData = (LPBYTE)pStringTable->Children;

            while (pbyCurData < pbyTmpEndData)
            {
                pString = (String*)pbyCurData;

                pszKey = pString->szKey;
                pszValue = lGetValueAfterKey(pszKey);

                if (lstrcmpiW(pszKey, L"ProductName") == 0)
                {
                    pszTitle = pszValue;
                }
                else if (lstrcmpiW(pszKey, L"LegalCopyright") == 0)
                {
                    pszCopyright = pszValue;
                }

                pbyCurData = (LPBYTE)pString + pString->wLength;

                /* Data must be aligned on dword boundaries. */

                while (PtrToLong(pbyCurData) & 0x03)
                {
                    ++pbyCurData;
                }
            }
        }
        else if (0 == lstrcmpiW(pHeader->szKey, L"VarFileInfo"))
        {
            pVarFileInfo = (VarFileInfo*)pbyCurData;

            pbyTmpEndData = pbyCurData + pVarFileInfo->wLength;
            pbyCurData = (LPBYTE)pVarFileInfo->Children;

            while (pbyCurData < pbyTmpEndData)
            {
                pVar = (Var*)pbyCurData;

                pbyCurData += pVar->wLength;
            }
        }
        else
        {
            assert(0);

            pbyCurData = pbyEndData;
        }
    }

    if (pszTitle)
    {
        wprintf(L"%s\n", pszTitle);
    }

    if (pszCopyright)
    {
        wprintf(L"%s\n", pszCopyright);
    }

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2009-2019 Kevin Eshbach                                  */
/***************************************************************************/
