/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include <Utils/UtHeap.h>

static COMDLG_FILTERSPEC* lAllocFilterSpecs(
    System::Collections::Generic::List<Common::Forms::FileTypeItem^>^ FileTypesList)
{
    COMDLG_FILTERSPEC* pFilterSpecs;
    pin_ptr<const wchar_t> pszName;
    pin_ptr<const wchar_t> pszFilters;
    INT nNameLen, nFiltersLen, nFilterSpecsLen;
    Common::Forms::FileTypeItem^ FileTypeItem;

    if (FALSE == UtInitHeap())
    {
        return NULL;
    }

	nFilterSpecsLen = FileTypesList->Count * sizeof(COMDLG_FILTERSPEC);

    pFilterSpecs = (COMDLG_FILTERSPEC*)UtAllocMem(nFilterSpecsLen);

    if (!pFilterSpecs)
    {
        UtUninitHeap();

        return NULL;
    }

	::ZeroMemory(pFilterSpecs, nFilterSpecsLen);

    for (int nIndex = 0; nIndex < FileTypesList->Count; ++nIndex)
    {
        FileTypeItem = (Common::Forms::FileTypeItem^)FileTypesList[nIndex];

        pszName = PtrToStringChars(FileTypeItem->Name);
        pszFilters = PtrToStringChars(FileTypeItem->Filters);

        nNameLen = FileTypeItem->Name->Length + 1;
        nFiltersLen = FileTypeItem->Filters->Length + 1;

        pFilterSpecs[nIndex].pszName = (LPWSTR)UtAllocMem(nNameLen * sizeof(WCHAR));
        pFilterSpecs[nIndex].pszSpec = (LPWSTR)UtAllocMem(nFiltersLen * sizeof(WCHAR));

        if (pFilterSpecs[nIndex].pszName)
        {
            ::StringCchCopyW((LPWSTR)pFilterSpecs[nIndex].pszName, nNameLen, pszName);
        }

        if (pFilterSpecs[nIndex].pszSpec)
        {
            ::StringCchCopyW((LPWSTR)pFilterSpecs[nIndex].pszSpec, nFiltersLen, pszFilters);
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
        UtFreeMem((LPWSTR)pFilterSpecs[nIndex].pszName);
        UtFreeMem((LPWSTR)pFilterSpecs[nIndex].pszSpec);
    }

    UtFreeMem(pFilterSpecs);

    UtUninitHeap();
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

static void lConvertSystemGuidToGUID(System::Guid^ Guid, LPGUID pGuid)
{
	pin_ptr<const wchar_t> pszGuid = PtrToStringChars(Guid->ToString());

	::CLSIDFromString(pszGuid, pGuid);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
