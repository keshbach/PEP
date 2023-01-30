/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ImageManager.h"

#include "Resource.Designer.h"

#include <Utils/UtHeap.h>

#using "UtilNet.dll"

#pragma region "Local Functions"

#pragma unmanaged

static BOOL lDoesFileSmallIconExist(
  LPCWSTR pszFileExtension)
{
    BOOL bExist = FALSE;
    HKEY hKey;

    if (ERROR_SUCCESS == ::RegOpenKeyExW(HKEY_CLASSES_ROOT, pszFileExtension, 0, KEY_READ, &hKey))
    {
        ::RegCloseKey(hKey);

        bExist = TRUE;
    }

    return bExist;
}

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

#pragma endregion

System::Boolean Common::Forms::ImageManager::Initialize()
{
    if (s_FileSmallImageList != nullptr)
    {
        return false;
    }

    if (FALSE == UtInitHeap())
    {
        return false;
    }

    // File Small

    CreateFileSmallImageList();

    InitializeFileImageLists();

    AddFileExtensionSmallImage(L"", UnknownFileImageName);

    // Toolbar

    CreateToolbarSmallImageList();

    AddToolbarSmallImages(UiUtilNet::Resources::Resource::ResourceManager);

    return true;
}

System::Boolean Common::Forms::ImageManager::Uninitialize()
{
    if (s_FileSmallImageList == nullptr)
    {
        return false;
    }

    for each (System::IntPtr Icon in s_ToolbarSmallImageIconList)
    {
        if (FALSE == ::DestroyIcon((HICON)Icon.ToPointer()))
        {
#if !defined(NDEBUG)
            ::OutputDebugStringW(L"*** Warning failed to destroy icon ***");
#endif
        }
    }

    s_ToolbarSmallImageIconList->Clear();

    delete s_FileSmallImageList;
    delete s_ToolbarSmallImageList;
    delete s_ToolbarSmallImageIconList;

    s_FileSmallImageList = nullptr;
    s_ToolbarSmallImageList = nullptr;
    s_ToolbarSmallImageIconList = nullptr;

    UtUninitHeap();

    return true;
}

System::Boolean Common::Forms::ImageManager::AddFileSmallImage(
  System::String^ sFile,
  System::String^ sImageName)
{
    pin_ptr<const wchar_t> pszFile;
    HICON hFileIcon;
    System::Drawing::Icon^ Icon;

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

    hFileIcon = lCreateFileSmallIcon(pszFile, FALSE);

    if (hFileIcon == NULL)
    {
        return false;
    }

    Icon = System::Drawing::Icon::FromHandle(System::IntPtr(hFileIcon));

    s_FileSmallImageList->Images->Add(sImageName, Icon);

    return true;
}

System::Boolean Common::Forms::ImageManager::AddFileExtensionSmallImage(
  System::String^ sFileExtension,
  System::String^ sImageName)
{
    System::Boolean bResult = false;
    pin_ptr<const wchar_t> pszFileExtension = PtrToStringChars(sFileExtension);
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

    if (sFileExtension->Length > 0 && FALSE == lDoesFileSmallIconExist(pszFileExtension))
    {
        // No icon associated with this file extension 

        return false;
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

                if (Bitmap->Width == ToolbarImageWidth &&
                    Bitmap->Height == ToolbarImageHeight)
                {
                    IconPtr = Bitmap->GetHicon();
                    Icon = System::Drawing::Icon::FromHandle(IconPtr);

                    s_ToolbarSmallImageList->Images->Add(sToolbarImageKey, Icon);

                    s_ToolbarSmallImageIconList->Add(IconPtr);
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

void Common::Forms::ImageManager::CreateFileSmallImageList()
{
    s_FileSmallImageList = gcnew System::Windows::Forms::ImageList();

    s_FileSmallImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
    s_FileSmallImageList->ImageSize = System::Drawing::Size(FileSmallImageWidth,
                                                            FileSmallImageHeight);
}

void Common::Forms::ImageManager::CreateToolbarSmallImageList()
{
    s_ToolbarSmallImageList = gcnew System::Windows::Forms::ImageList();

    s_ToolbarSmallImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
    s_ToolbarSmallImageList->ImageSize = System::Drawing::Size(ToolbarImageWidth,
                                                               ToolbarImageHeight);

    s_ToolbarSmallImageIconList = gcnew System::Collections::Generic::List<System::IntPtr>();
}

void Common::Forms::ImageManager::InitializeFileImageLists()
{
    WCHAR cPath[MAX_PATH];
    HICON hFileIcon;

    ::GetTempPathW(sizeof(cPath) / sizeof(cPath[0]), cPath);

    // Folder closed icon

    hFileIcon = lCreateFileSmallIcon(cPath, FALSE);

    s_FileSmallImageList->Images->Add(FolderImageName, 
                                      System::Drawing::Icon::FromHandle(System::IntPtr(hFileIcon)));

    // Folder opened icon

    hFileIcon = lCreateFileSmallIcon(cPath, TRUE);

    s_FileSmallImageList->Images->Add(OpenFolderImageName, 
                                      System::Drawing::Icon::FromHandle(System::IntPtr(hFileIcon)));
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
