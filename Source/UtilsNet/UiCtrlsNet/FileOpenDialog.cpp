/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "FileTypeItem.h"
#include "FileOpenDialog.h"

#include "UtFileDialog.inl"

#pragma unmanaged

static BOOL lCreateFileOpenDialog(
  IFileOpenDialog** ppFileOpenDialog)
{
    BOOL bResult = FALSE;
    IFileDialog* pFileDialog = NULL;

    *ppFileOpenDialog = NULL;

    if (S_OK != ::CoCreateInstance(CLSID_FileOpenDialog, NULL,
                                   CLSCTX_INPROC_SERVER, IID_IFileDialog,
                                   (LPVOID*)&pFileDialog))
    {
        return FALSE;
    }

    if (SUCCEEDED(pFileDialog->QueryInterface(IID_IFileOpenDialog,
                                              (LPVOID*)ppFileOpenDialog)))
    {
        bResult = TRUE;
    }

    pFileDialog->Release();

    return bResult;
}

#pragma managed

static System::String^ lGetFileSysName(
  IFileOpenDialog* pFileOpenDialog)
{
    System::String^ sResult(L"");
    IShellItem* pShellItem = NULL;

    if (S_OK == pFileOpenDialog->GetResult(&pShellItem))
    {
        sResult = lGetFileSysDisplayName(pShellItem);

        pShellItem->Release();
    }

    return sResult;
}

static System::Collections::Specialized::StringCollection^ lGetMultipleFileSysNames(
  IFileOpenDialog* pFileOpenDialog)
{
    System::Collections::Specialized::StringCollection^ Result;
    IShellItemArray* pShellItemArray = NULL;
    IShellItem* pShellItem;
    DWORD dwNumItems;

    Result = gcnew System::Collections::Specialized::StringCollection();

    if (S_OK == pFileOpenDialog->GetResults(&pShellItemArray))
    {
        if (S_OK != pShellItemArray->GetCount(&dwNumItems))
        {
            dwNumItems = 0;
        }

        for (DWORD dwIndex = 0; dwIndex < dwNumItems; ++dwIndex)
        {
            if (S_OK == pShellItemArray->GetItemAt(dwIndex, &pShellItem))
            {
                Result->Add(lGetFileSysDisplayName(pShellItem));

                pShellItem->Release();
            }
        }

        pShellItemArray->Release();
    }

    return Result;
}

Common::Forms::FileOpenDialog::FileOpenDialog() :
  m_bPickFolders(false),
  m_bSelectMultipleItems(false),
  m_bAllowReadOnly(true),
  m_bAddToRecentList(false),
  m_bShowHidden(false),
  m_sFileName(nullptr),
  m_FileNameCollection(nullptr),
  m_sFolder(nullptr),
  m_FolderCollection(nullptr),
  m_FileTypesList(nullptr),
  m_nSelectedFileType(0)
{
}

Common::Forms::FileOpenDialog::~FileOpenDialog()
{
}

System::Windows::Forms::DialogResult Common::Forms::FileOpenDialog::ShowDialog(
  System::Windows::Forms::IWin32Window^ Owner)
{
    System::Windows::Forms::DialogResult Result(System::Windows::Forms::DialogResult::Cancel);
    IFileOpenDialog* pFileOpenDialog;
    IShellItem* pShellItem;
    FILEOPENDIALOGOPTIONS FileOpenDialogOptions;
    COMDLG_FILTERSPEC* pFilterSpecs;
    pin_ptr<const wchar_t> pszFileName;
    pin_ptr<const wchar_t> pszFolder;

    if (!lCreateFileOpenDialog(&pFileOpenDialog))
    {
        throw gcnew System::Exception(L"File Open Dialog COM object could not be created.");
    }

    if (S_OK == pFileOpenDialog->GetOptions(&FileOpenDialogOptions))
    {
        if (m_bPickFolders)
        {
            FileOpenDialogOptions |= FOS_PICKFOLDERS;
        }
        else
        {
            FileOpenDialogOptions &= ~FOS_PICKFOLDERS;
        }

        if (m_bSelectMultipleItems)
        {
            FileOpenDialogOptions |= FOS_ALLOWMULTISELECT;
        }
        else
        {
            FileOpenDialogOptions &= ~FOS_ALLOWMULTISELECT;
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

        FileOpenDialogOptions &= ~FOS_FORCEPREVIEWPANEON;

        pFileOpenDialog->SetOptions(FileOpenDialogOptions);
    }

    if (m_sFileName != nullptr)
    {
        pszFileName = PtrToStringChars(m_sFileName);

        pFileOpenDialog->SetFileName(pszFileName);
    }

    if (m_sFolder != nullptr)
    {
        pszFolder = PtrToStringChars(m_sFolder);
        pShellItem = NULL;

        if (S_OK == ::SHCreateItemFromParsingName(pszFolder, NULL,
                                                  IID_IShellItem,
                                                  (LPVOID*)&pShellItem))
        {
            pFileOpenDialog->SetFolder(pShellItem);

            pShellItem->Release();
        }
    }

    if (m_FileTypesList != nullptr &&
        m_FileTypesList->Count > 0)
    {
        pFilterSpecs = lAllocFilterSpecs(m_FileTypesList);

        if (pFilterSpecs)
        {
            pFileOpenDialog->SetFileTypes(m_FileTypesList->Count,
                                          pFilterSpecs);

            pFileOpenDialog->SetFileTypeIndex(m_nSelectedFileType);

            lFreeFilterSpecs(pFilterSpecs, m_FileTypesList->Count);
        }
    }

    switch (pFileOpenDialog->Show((HWND)Owner->Handle.ToPointer()))
    {
        case S_OK:
            if (!m_bPickFolders)
            {
                if (!m_bSelectMultipleItems)
                {
                    m_sFileName = lGetFileSysName(pFileOpenDialog);
                }
                else
                {
                    m_FileNameCollection = lGetMultipleFileSysNames(pFileOpenDialog);
                }
            }
            else
            {
                if (!m_bSelectMultipleItems)
                {
                    m_sFolder = lGetFileSysName(pFileOpenDialog);
                }
                else
                {
                    m_FolderCollection = lGetMultipleFileSysNames(pFileOpenDialog);
                }
            }

            Result = System::Windows::Forms::DialogResult::OK;
            break;
        case __HRESULT_FROM_WIN32(ERROR_CANCELLED):
            break;
        default:
            System::Diagnostics::Debug::Assert(false);
            break;
    }

    pFileOpenDialog->Release();

    return Result;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
