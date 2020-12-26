/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ImageManager.h"

#using "UtilNet.dll"

#define CToolbarSmallImageWidth 16
#define CToolbarSmallImageHeight 16

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
        // According to the documentation this handle is a copy and should be deleted,
        // but if deleted it causes memory corruption.

        return FileInfo.hIcon;
    }

    return NULL;
}

#pragma managed

static System::Drawing::Icon^ lGetFileSmallIcon(
  LPCWSTR pszFile,
  BOOL bUseOpen)
{
    System::Drawing::Icon^ Icon = nullptr;
    HICON hIcon = lCreateFileSmallIcon(pszFile, bUseOpen);

    if (hIcon)
    {
        Icon = System::Drawing::Icon::FromHandle(System::IntPtr(hIcon));
    }

    return Icon;
}

System::Boolean Common::Forms::ImageManager::Initialize()
{
    System::Drawing::Size SmallImageSize(::GetSystemMetrics(SM_CXSMICON),
                                         ::GetSystemMetrics(SM_CYSMICON));
    System::Drawing::Icon^ Icon;
    WCHAR cPath[MAX_PATH];

    if (s_FileSmallImageList != nullptr)
    {
        return false;
    }

    s_FileSmallImageList = gcnew System::Windows::Forms::ImageList();

    s_FileSmallImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
    s_FileSmallImageList->ImageSize = SmallImageSize;

    ::GetTempPathW(sizeof(cPath) / sizeof(cPath[0]), cPath);

    Icon = lGetFileSmallIcon(cPath, FALSE);

    s_FileSmallImageList->Images->Add(FolderImageName, Icon);

    // The HICON supposedly should be deleted but when it is a crash occurs later on.

    Icon = lGetFileSmallIcon(cPath, TRUE);

    s_FileSmallImageList->Images->Add(OpenFolderImageName, Icon);

    // The HICON supposedly should be deleted but when it is a crash occurs later on.

    AddFileExtensionSmallImage(L"", UnknownFileImageName);

    s_ToolbarSmallImageList = gcnew System::Windows::Forms::ImageList();

    s_ToolbarSmallImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
    s_ToolbarSmallImageList->ImageSize = System::Drawing::Size(CToolbarSmallImageWidth,
                                                               CToolbarSmallImageHeight);

    return true;
}

System::Boolean Common::Forms::ImageManager::Uninitialize()
{
    if (s_FileSmallImageList == nullptr)
    {
        return false;
    }

    delete s_FileSmallImageList;
    delete s_ToolbarSmallImageList;

    s_FileSmallImageList = nullptr;
    s_ToolbarSmallImageList = nullptr;

    return true;
}

System::Boolean Common::Forms::ImageManager::AddFileSmallImage(
  System::String^ sFile,
  System::String^ sImageName)
{
    System::Drawing::Icon^ Icon;
    pin_ptr<const wchar_t> pszFile;

    if (s_FileSmallImageList == nullptr || sImageName->Length == 0)
    {
        System::Diagnostics::Debug::Assert(false);

        return false;
    }

    if (-1 != s_FileSmallImageList->Images->IndexOfKey(sImageName))
    {
        return true;
    }

    pszFile = PtrToStringChars(sFile);

    Icon = lGetFileSmallIcon(pszFile, FALSE);

    if (Icon == nullptr)
    {
        return false;
    }

    s_FileSmallImageList->Images->Add(sImageName, Icon);

    return true;
}

System::Boolean Common::Forms::ImageManager::AddFileExtensionSmallImage(
  System::String^ sFileExtension,
  System::String^ sImageName)
{
    System::Boolean bResult = false;
    System::String^ sTempFile;
    System::IO::FileStream^ fs;

    if (s_FileSmallImageList == nullptr || sImageName->Length == 0)
    {
        System::Diagnostics::Debug::Assert(false);

        return false;
    }

    if (-1 != s_FileSmallImageList->Images->IndexOfKey(sImageName))
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

System::Boolean Common::Forms::ImageManager::AddToolbarSmallImages(
  System::Resources::ResourceManager^ ResourceManager)
{
    System::Resources::ResourceSet^ ResourceSet;
    System::Collections::IDictionaryEnumerator^ DictEnum;
    System::String^ sImageName;
    System::String^ sToolbarImageKey;
    System::Object^ ResourceObject;
    System::Drawing::Bitmap^ Bitmap;
    System::IntPtr IconPtr;
    System::Drawing::Icon^ Icon;

    ResourceSet = ResourceManager->GetResourceSet(System::Globalization::CultureInfo::CurrentUICulture,
                                                  true, true);

    if (ResourceSet == nullptr)
    {
        return false;
    }

    DictEnum = ResourceSet->GetEnumerator();

    while (DictEnum->MoveNext())
    {
        if (DictEnum->Value->GetType() == System::Drawing::Bitmap::typeid)
        {
            sImageName = (System::String^)DictEnum->Key;
            sToolbarImageKey = GenerateToolbarImageKey(ResourceManager, sImageName);

            if (!s_ToolbarSmallImageList->Images->ContainsKey(sToolbarImageKey))
            {
                ResourceObject = ResourceSet->GetObject(sImageName);
                Bitmap = (System::Drawing::Bitmap^)ResourceObject;

                if (Bitmap->Width == CToolbarSmallImageWidth &&
                    Bitmap->Height == CToolbarSmallImageHeight)
                {
                    IconPtr = Bitmap->GetHicon();
                    Icon = System::Drawing::Icon::FromHandle(IconPtr);

                    s_ToolbarSmallImageList->Images->Add(sToolbarImageKey, Icon);

                    ::DeleteObject((void*)IconPtr);

                    delete Icon;
                }

                delete Bitmap;
            }
        }
    }

    return true;
}

System::String^ Common::Forms::ImageManager::GenerateToolbarImageKey(
  System::Resources::ResourceManager^ ResourceManager,
  System::String^ sImageName)
{
    return System::String::Format("{0}.{1}", ResourceManager->BaseName, sImageName);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
