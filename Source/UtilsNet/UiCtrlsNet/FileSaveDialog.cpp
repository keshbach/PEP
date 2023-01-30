/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "FileTypeItem.h"
#include "FileSaveDialog.h"

#include "UtFileDialog.inl"

#pragma region "Local Functions"

#pragma unmanaged

static BOOL lCreateFileSaveDialog(
  IFileSaveDialog** ppFileSaveDialog)
{
    BOOL bResult = FALSE;
    IFileDialog* pFileDialog = NULL;

    *ppFileSaveDialog = NULL;

    if (S_OK != ::CoCreateInstance(CLSID_FileSaveDialog, NULL,
                                   CLSCTX_INPROC_SERVER, IID_IFileDialog,
                                   (LPVOID*)&pFileDialog))
    {
        return FALSE;
    }

    if (SUCCEEDED(pFileDialog->QueryInterface(IID_IFileSaveDialog,
                                              (LPVOID*)ppFileSaveDialog)))
    {
        bResult = TRUE;
    }

    pFileDialog->Release();

    return bResult;
}

#pragma managed

static System::String^ lGetFileSysName(
  IFileSaveDialog* pFileSaveDialog)
{
    System::String^ sResult(L"");
    IShellItem* pShellItem = NULL;

    if (S_OK == pFileSaveDialog->GetResult(&pShellItem))
    {
        sResult = lGetFileSysDisplayName(pShellItem);

        pShellItem->Release();
    }

    return sResult;
}

#pragma endregion

Common::Forms::FileSaveDialog::FileSaveDialog() :
  m_sTitle(nullptr),
  m_bOverwritePrompt(false),
  m_bStrictFileType(false),
  m_bCreatePrompt(false),
  m_bAllowReadOnly(false),
  m_bAddToRecentList(false),
  m_bShowHidden(false),
  m_sFileName(nullptr),
  m_sDefaultExtension(nullptr),
  m_FileTypesList(nullptr),
  m_nSelectedFileType(0),
  m_ClientGuid(nullptr)
{
}

Common::Forms::FileSaveDialog::~FileSaveDialog()
{
}

System::Windows::Forms::DialogResult Common::Forms::FileSaveDialog::ShowDialog(
  System::Windows::Forms::IWin32Window^ Owner)
{
    if (IsWindowsVistaOrGreater())
    {
        return ShowDialogVista(Owner);
    }
    else
    {
        return ShowDialogWindowsXP(Owner);
    }
}

System::Windows::Forms::DialogResult Common::Forms::FileSaveDialog::ShowDialogVista(
  System::Windows::Forms::IWin32Window^ Owner)
{
    System::Windows::Forms::DialogResult Result(System::Windows::Forms::DialogResult::Cancel);
    IFileSaveDialog* pFileSaveDialog;
    FILEOPENDIALOGOPTIONS FileOpenDialogOptions;
    COMDLG_FILTERSPEC* pFilterSpecs;
    pin_ptr<const wchar_t> pszTitle, pszFileName, pszDefaultExtension;
	GUID Guid;

    if (!lCreateFileSaveDialog(&pFileSaveDialog))
    {
        throw gcnew System::Exception(L"File Save Dialog COM object could not be created.");
    }

    if (S_OK == pFileSaveDialog->GetOptions(&FileOpenDialogOptions))
    {
        if (m_bOverwritePrompt)
        {
            FileOpenDialogOptions |= FOS_OVERWRITEPROMPT;
        }
        else
        {
            FileOpenDialogOptions &= ~FOS_OVERWRITEPROMPT;
        }

        if (m_bStrictFileType)
        {
            FileOpenDialogOptions |= FOS_STRICTFILETYPES;
        }
        else
        {
            FileOpenDialogOptions &= ~FOS_STRICTFILETYPES;
        }

        if (m_bCreatePrompt)
        {
            FileOpenDialogOptions |= FOS_CREATEPROMPT;
        }
        else
        {
            FileOpenDialogOptions &= ~FOS_CREATEPROMPT;
        }

        if (m_bAllowReadOnly)
        {
            FileOpenDialogOptions &= ~FOS_NOREADONLYRETURN;
        }
        else
        {
            FileOpenDialogOptions |= FOS_NOREADONLYRETURN;
        }

        if (m_bAddToRecentList)
        {
            FileOpenDialogOptions &= ~FOS_DONTADDTORECENT;
        }
        else
        {
            FileOpenDialogOptions |= FOS_DONTADDTORECENT;
        }

        if (m_bShowHidden)
        {
            FileOpenDialogOptions |= FOS_FORCESHOWHIDDEN;
        }
        else
        {
            FileOpenDialogOptions &= ~FOS_FORCESHOWHIDDEN;
        }

        // Force file system items
        FileOpenDialogOptions &= ~FOS_ALLNONSTORAGEITEMS;
        FileOpenDialogOptions |= FOS_FORCEFILESYSTEM;

        FileOpenDialogOptions |= FOS_DEFAULTNOMINIMODE;
        FileOpenDialogOptions |= FOS_NOTESTFILECREATE;

        pFileSaveDialog->SetOptions(FileOpenDialogOptions);
    }

    if (m_sTitle != nullptr)
    {
        pszTitle = PtrToStringChars(m_sTitle);

        pFileSaveDialog->SetTitle(pszTitle);
    }

    if (m_sFileName != nullptr)
    {
        pszFileName = PtrToStringChars(m_sFileName);

        pFileSaveDialog->SetFileName(pszFileName);
    }

    if (m_sDefaultExtension != nullptr)
    {
        pszDefaultExtension = PtrToStringChars(m_sDefaultExtension);

        pFileSaveDialog->SetDefaultExtension(pszDefaultExtension);
    }

    if (m_FileTypesList != nullptr &&
        m_FileTypesList->Count > 0)
    {
        pFilterSpecs = lAllocFilterSpecs(m_FileTypesList);

        if (pFilterSpecs)
        {
            pFileSaveDialog->SetFileTypes(m_FileTypesList->Count,
                                          pFilterSpecs);

            pFileSaveDialog->SetFileTypeIndex(m_nSelectedFileType);

            lFreeFilterSpecs(pFilterSpecs, m_FileTypesList->Count);
        }
    }

	if (m_ClientGuid != nullptr)
	{
		lConvertSystemGuidToGUID(m_ClientGuid, &Guid);

		pFileSaveDialog->SetClientGuid(Guid);
	}

    switch (pFileSaveDialog->Show((HWND)Owner->Handle.ToPointer()))
    {
        case S_OK:
            m_sFileName = lGetFileSysName(pFileSaveDialog);

            Result = System::Windows::Forms::DialogResult::OK;
            break;
        case __HRESULT_FROM_WIN32(ERROR_CANCELLED):
            break;
        default:
            System::Diagnostics::Debug::Assert(false);
            break;
    }

    pFileSaveDialog->Release();

    return Result;
}

System::Windows::Forms::DialogResult Common::Forms::FileSaveDialog::ShowDialogWindowsXP(
  System::Windows::Forms::IWin32Window^ Owner)
{
    Owner;

    System::Diagnostics::Debug::Assert(false);

    return System::Windows::Forms::DialogResult::Cancel;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
