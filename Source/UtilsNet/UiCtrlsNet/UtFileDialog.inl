/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
#pragma unmanaged

static HANDLE l_hHeap = NULL;

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

#pragma managed

static COMDLG_FILTERSPEC* lAllocFilterSpecs(
    System::Collections::Generic::List<Common::Forms::FileTypeItem^>^ FileTypesList)
{
    COMDLG_FILTERSPEC* pFilterSpecs;
    pin_ptr<const wchar_t> pszName;
    pin_ptr<const wchar_t> pszFilters;
    INT nNameLen, nFiltersLen;
    Common::Forms::FileTypeItem^ FileTypeItem;

    if (FALSE == lInitHeap())
    {
        return NULL;
    }

    pFilterSpecs = (COMDLG_FILTERSPEC*)lAllocMem(FileTypesList->Count * sizeof(COMDLG_FILTERSPEC));

    if (!pFilterSpecs)
    {
        lUninitHeap();

        return NULL;
    }

    for (int nIndex = 0; nIndex < FileTypesList->Count; ++nIndex)
    {
        FileTypeItem = (Common::Forms::FileTypeItem^)FileTypesList[nIndex];

        pszName = PtrToStringChars(FileTypeItem->Name);
        pszFilters = PtrToStringChars(FileTypeItem->Filters);

        nNameLen = FileTypeItem->Name->Length + 1;
        nFiltersLen = FileTypeItem->Filters->Length + 1;

        pFilterSpecs[nIndex].pszName = (LPWSTR)lAllocMem(nNameLen * sizeof(WCHAR));
        pFilterSpecs[nIndex].pszSpec = (LPWSTR)lAllocMem(nFiltersLen * sizeof(WCHAR));

        if (pFilterSpecs[nIndex].pszName)
        {
            ::StringCchCopyW((LPWSTR)pFilterSpecs[nIndex].pszName, nNameLen,
                pszName);
        }

        if (pFilterSpecs[nIndex].pszSpec)
        {
            ::StringCchCopyW((LPWSTR)pFilterSpecs[nIndex].pszSpec, nFiltersLen,
                pszFilters);
        }
    }

    return pFilterSpecs;
}

static void lFreeFilterSpecs(
    COMDLG_FILTERSPEC* pFilterSpecs,
    INT nTotalFilterSpecs)
{
    for (INT nIndex = 0; nIndex < nTotalFilterSpecs; ++nIndex)
    {
        lFreeMem((LPWSTR)pFilterSpecs[nIndex].pszName);
        lFreeMem((LPWSTR)pFilterSpecs[nIndex].pszSpec);
    }

    lFreeMem(pFilterSpecs);

    lUninitHeap();
}

static System::String^ lGetFileSysDisplayName(
    IShellItem* pShellItem)
{
    System::String^ sResult(L"");
    LPWSTR pszName = NULL;

    if (S_OK == pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszName))
    {
        sResult = gcnew System::String(pszName);

        ::CoTaskMemFree(pszName);
    }

    return sResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
