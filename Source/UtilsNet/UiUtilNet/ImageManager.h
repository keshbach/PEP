/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
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

            static System::Int32 FileSmallImageWidth = 16;
            static System::Int32 FileSmallImageHeight = 16;

            static System::Int32 ToolbarImageWidth = 16;
            static System::Int32 ToolbarImageHeight = 16;

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
            static System::Windows::Forms::ImageList^ s_FileSmallImageList = nullptr;
            static System::Windows::Forms::ImageList^ s_ToolbarSmallImageList = nullptr;
            static System::Collections::Generic::List<System::IntPtr>^ s_ToolbarSmallImageIconList = nullptr;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
