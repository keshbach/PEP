/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ImageManager.h"

#include "UtIcon.h"

#include "Resource.Designer.h"

#include <Utils/UtHeap.h>

#using "UtilNet.dll"

#pragma region "Constants"

#define CToolbarSmallImageWidth 16
#define CToolbarSmallImageHeight 16

#pragma endregion

#pragma region "Local Functions"

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

    s_FileSmallBorderImageListDataDict = gcnew System::Collections::Generic::Dictionary<System::Windows::Forms::ImageList^, TFileSmallBorderImageListData^>();

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
    delete s_FileSmallBorderImageListDataDict;

    s_FileSmallImageList = nullptr;
    s_ToolbarSmallImageList = nullptr;
    s_ToolbarSmallImageIconList = nullptr;
    s_FileSmallBorderImageListDataDict = nullptr;

    UtUninitHeap();

    return true;
}

System::Windows::Forms::ImageList^ Common::Forms::ImageManager::CreateFileSmallBorderImageList(
  System::UInt32 nLeftBorderWidth,
  System::UInt32 nTopBorderHeight,
  System::UInt32 nRightBorderWidth,
  System::UInt32 nBottomBorderHeight)
{
    TFileSmallBorderImageListData^ FileSmallBorderImageListData = gcnew TFileSmallBorderImageListData();
    System::Windows::Forms::ImageList^ ImageList;

    FileSmallBorderImageListData->nLeftBorderWidth = nLeftBorderWidth;
    FileSmallBorderImageListData->nTopBorderHeight = nTopBorderHeight;
    FileSmallBorderImageListData->nRightBorderWidth = nRightBorderWidth;
    FileSmallBorderImageListData->nBottomBorderHeight = nBottomBorderHeight;
    FileSmallBorderImageListData->IconList = gcnew System::Collections::Generic::List<System::IntPtr>();

    ImageList = gcnew System::Windows::Forms::ImageList();

    ImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
    ImageList->ImageSize = System::Drawing::Size(::GetSystemMetrics(SM_CXSMICON) + (nLeftBorderWidth + nRightBorderWidth),
                                                 ::GetSystemMetrics(SM_CYSMICON) + (nTopBorderHeight + nBottomBorderHeight));

    SyncFileSmallBorderImageList(ImageList, FileSmallBorderImageListData);

    s_FileSmallBorderImageListDataDict->Add(ImageList, FileSmallBorderImageListData);

    return ImageList;
}

System::Boolean Common::Forms::ImageManager::DestroyFileSmallBorderImageList(
  System::Windows::Forms::ImageList^ ImageList)
{
    TFileSmallBorderImageListData^ FileSmallBorderImageListData;

    if (false == s_FileSmallBorderImageListDataDict->TryGetValue(ImageList, FileSmallBorderImageListData))
    {
        return false;
    }

    for each (System::IntPtr Icon in FileSmallBorderImageListData->IconList)
    {
        if (FALSE == ::DestroyIcon((HICON)Icon.ToPointer()))
        {
#if !defined(NDEBUG)
            ::OutputDebugStringW(L"*** Warning failed to destroy icon ***");
#endif
        }
    }

    FileSmallBorderImageListData->IconList->Clear();

    delete ImageList;

    delete FileSmallBorderImageListData->IconList;
    delete FileSmallBorderImageListData;

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

    for each (System::Windows::Forms::ImageList ^ ImageList in s_FileSmallBorderImageListDataDict->Keys)
    {
        AddFileSmallBorderImage(hFileIcon, sImageName, ImageList,
                                s_FileSmallBorderImageListDataDict[ImageList]);
    }

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
    s_FileSmallImageList->ImageSize = System::Drawing::Size(::GetSystemMetrics(SM_CXSMICON),
                                                            ::GetSystemMetrics(SM_CYSMICON));
}

void Common::Forms::ImageManager::CreateToolbarSmallImageList()
{
    s_ToolbarSmallImageList = gcnew System::Windows::Forms::ImageList();

    s_ToolbarSmallImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
    s_ToolbarSmallImageList->ImageSize = System::Drawing::Size(CToolbarSmallImageWidth,
                                                               CToolbarSmallImageHeight);

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

void Common::Forms::ImageManager::SyncFileSmallBorderImageList(
  System::Windows::Forms::ImageList^ ImageList,
  Common::Forms::ImageManager::TFileSmallBorderImageListData^ FileSmallBorderImageListData)
{
    System::Drawing::Bitmap^ Bitmap;
    HICON hIcon;

    for each (System::String ^ sImageName in s_FileSmallImageList->Images->Keys)
    {
        if (false == ImageList->Images->ContainsKey(sImageName))
        {
            Bitmap = (System::Drawing::Bitmap^)s_FileSmallImageList->Images[sImageName];
            hIcon = (HICON)Bitmap->GetHicon().ToPointer();

            AddFileSmallBorderImage(hIcon, sImageName, ImageList, FileSmallBorderImageListData);

            if (FALSE == ::DestroyIcon(hIcon))
            {
#if !defined(NDEBUG)
                ::OutputDebugStringW(L"*** Warning failed to destroy icon ***");
#endif
            }
        }
    }
}

void Common::Forms::ImageManager::AddFileSmallBorderImage(
  HICON hIcon,
  System::String^ sImageName,
  System::Windows::Forms::ImageList^ ImageList,
  Common::Forms::ImageManager::TFileSmallBorderImageListData^ FileSmallBorderImageListData)
{
    System::Drawing::Icon^ Icon;
    HICON hBorderIcon;

    hBorderIcon = UtIconAddBorder(hIcon, FileSmallBorderImageListData->nLeftBorderWidth,
                                  FileSmallBorderImageListData->nTopBorderHeight,
                                  FileSmallBorderImageListData->nRightBorderWidth,
                                  FileSmallBorderImageListData->nBottomBorderHeight);

    if (hBorderIcon)
    {
        Icon = System::Drawing::Icon::FromHandle(System::IntPtr(hBorderIcon));

        ImageList->Images->Add(sImageName, Icon);

        FileSmallBorderImageListData->IconList->Add(System::IntPtr(hBorderIcon));
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
