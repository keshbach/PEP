/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ImageManager.h"

#using "UtilNet.dll"

#pragma unmanaged

static HICON lCreateFileSmallIcon(
  LPCWSTR pszFile,
  BOOL bUseOpen)
{
    SHFILEINFO FileInfo;

    ::ZeroMemory(&FileInfo, sizeof(FileInfo));

    if (::SHGetFileInfoW(pszFile, 0, &FileInfo, sizeof(FileInfo),
                         SHGFI_ICON | SHGFI_SMALLICON |
                         (bUseOpen ? SHGFI_OPENICON : 0)))
    {
        return FileInfo.hIcon;
    }

    return NULL;
}

#pragma managed

static System::Drawing::Icon^ lGetFileSmallIcon(
  LPCWSTR pszFile,
  BOOL bUseOpen,
  System::Collections::Generic::List<System::IntPtr>^% IconHandleArrayList)
{
    System::Drawing::Icon^ Icon = nullptr;
    HICON hIcon = lCreateFileSmallIcon(pszFile, bUseOpen);

    if (hIcon)
    {
        IconHandleArrayList->Add(System::IntPtr(hIcon));

        Icon = System::Drawing::Icon::FromHandle(System::IntPtr(hIcon));
    }

    return Icon;
}

System::Boolean Common::Forms::ImageManager::Initialize()
{
    System::Drawing::Size SmallImageSize(::GetSystemMetrics(SM_CXSMICON),
                                         ::GetSystemMetrics(SM_CYSMICON));
    WCHAR cPath[MAX_PATH];

    if (s_SmallImageList != nullptr)
    {
        return false;
    }

    s_SmallImageList = gcnew System::Windows::Forms::ImageList();
    s_IconHandleArrayList = gcnew System::Collections::Generic::List<System::IntPtr>();

    s_SmallImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
    s_SmallImageList->ImageSize = SmallImageSize;

    ::GetTempPathW(sizeof(cPath) / sizeof(cPath[0]), cPath);

    s_SmallImageList->Images->Add(FolderImageName,
                                  lGetFileSmallIcon(cPath, FALSE, s_IconHandleArrayList));
    s_SmallImageList->Images->Add(OpenFolderImageName,
                                  lGetFileSmallIcon(cPath, TRUE, s_IconHandleArrayList));

    AddFileExtensionSmallImage(L"", UnknownFileImageName);

    return true;
}

System::Boolean Common::Forms::ImageManager::Uninitialize()
{
    if (s_SmallImageList == nullptr)
    {
        return false;
    }

    delete s_SmallImageList;

    s_SmallImageList = nullptr;

    for each (System::IntPtr IconHandle in s_IconHandleArrayList)
    {
        ::DestroyIcon((HICON)IconHandle.ToPointer());
    }

    delete s_IconHandleArrayList;

    s_IconHandleArrayList = nullptr;

    return true;
}

System::Boolean Common::Forms::ImageManager::AddFileSmallImage(
  System::String^ sFile,
  System::String^ sImageName)
{
    System::Drawing::Icon^ Icon;
    pin_ptr<const wchar_t> pszFile;

    if (s_SmallImageList == nullptr || sImageName->Length == 0)
    {
        System::Diagnostics::Debug::Assert(false);

        return false;
    }

    if (-1 != s_SmallImageList->Images->IndexOfKey(sImageName))
    {
        return true;
    }

    pszFile = PtrToStringChars(sFile);

    Icon = lGetFileSmallIcon(pszFile, FALSE, s_IconHandleArrayList);

    if (Icon == nullptr)
    {
        return false;
    }

    s_SmallImageList->Images->Add(sImageName, Icon);

    return true;
}

System::Boolean Common::Forms::ImageManager::AddFileExtensionSmallImage(
  System::String^ sFileExtension,
  System::String^ sImageName)
{
    System::Boolean bResult = false;
    System::String^ sTempFile;
    System::IO::FileStream^ fs;

    if (s_SmallImageList == nullptr || sImageName->Length == 0)
    {
        System::Diagnostics::Debug::Assert(false);

        return false;
    }

    if (-1 != s_SmallImageList->Images->IndexOfKey(sImageName))
    {
        return true;
    }

    try
    {
        sTempFile = Common::IO::File::GenerateTempFileName(sFileExtension);

        fs = gcnew System::IO::FileStream(sTempFile, System::IO::FileMode::OpenOrCreate,
                                          System::IO::FileAccess::Read,
                                          System::IO::FileShare::ReadWrite);

        fs->Close();

        delete fs;

        fs = nullptr;

        bResult = AddFileSmallImage(sTempFile, sImageName);

        System::IO::File::Delete(sTempFile);

        delete sTempFile;

        sTempFile = nullptr;
    }
    catch (System::Exception^)
    {
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
