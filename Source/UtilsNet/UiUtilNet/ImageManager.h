/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ImageManager.
        /// </summary>

        public ref class ImageManager sealed
        {
        public:
            static System::String^ FolderImageName = L"Folder";
            static System::String^ OpenFolderImageName = L"OpenFolder";
            static System::String^ UnknownFileImageName = L"UnknownFile";

        public:
            /// <summary>
            /// An image list composed of small images of files.
            /// </summary>
            static property System::Windows::Forms::ImageList^ FileSmallImageList
            {
                System::Windows::Forms::ImageList^ get()
                {
                    return s_FileSmallImageList;
                }
            }

            static property System::Windows::Forms::ImageList^ ToolbarSmallImageList
            {
                System::Windows::Forms::ImageList^ get()
                {
                    return s_ToolbarSmallImageList;
                }
            }

        internal:
            static System::Boolean Initialize();
            static System::Boolean Uninitialize();

        public:
            static System::Windows::Forms::ImageList^ CreateFileSmallBorderImageList(System::UInt32 nLeftBorderWidth,
                                                                                     System::UInt32 nTopBorderHeight,
                                                                                     System::UInt32 nRightBorderWidth,
                                                                                     System::UInt32 nBottomBorderHeight);

            static System::Boolean DestroyFileSmallBorderImageList(System::Windows::Forms::ImageList^ ImageList);

            /// <summary>
            /// Adds a file's small image to the small image list.
            /// </summary>

            static System::Boolean AddFileSmallImage(System::String^ sFile, 
                                                     System::String^ sImageName);

            /// <summary>
            /// Adds a file extension's small image to the small image list.
            /// </summary>

            static System::Boolean AddFileExtensionSmallImage(System::String^ sFileExtension,
                                                              System::String^ sImageName);

            static System::Boolean AddToolbarSmallImages(System::Resources::ResourceManager^ ResourceManager);

            static System::String^ GenerateToolbarImageKey(System::Resources::ResourceManager^ ResourceManager,
                                                           System::String^ sImageName);

        private:
            static void CreateFileSmallImageList();
            static void CreateToolbarSmallImageList();

            static void InitializeFileImageLists();

        private:
            ref struct TFileSmallBorderImageListData
            {
                System::UInt32 nLeftBorderWidth;
                System::UInt32 nTopBorderHeight;
                System::UInt32 nRightBorderWidth;
                System::UInt32 nBottomBorderHeight;
                System::Collections::Generic::List<System::IntPtr>^ IconList;
            };

        private:
            static void SyncFileSmallBorderImageList(System::Windows::Forms::ImageList^ ImageList, Common::Forms::ImageManager::TFileSmallBorderImageListData^ FileSmallBorderImageListData);
            static void AddFileSmallBorderImage(HICON hIcon,
                                                System::String^ sImageName,
                                                System::Windows::Forms::ImageList^ ImageList,
                                                Common::Forms::ImageManager::TFileSmallBorderImageListData^ FileSmallBorderImageListData);

        private:
            static System::Windows::Forms::ImageList^ s_FileSmallImageList = nullptr;
            static System::Windows::Forms::ImageList^ s_ToolbarSmallImageList = nullptr;
            static System::Collections::Generic::List<System::IntPtr>^ s_ToolbarSmallImageIconList = nullptr;
            static System::Collections::Generic::Dictionary<System::Windows::Forms::ImageList^, TFileSmallBorderImageListData^>^ s_FileSmallBorderImageListDataDict = nullptr;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
