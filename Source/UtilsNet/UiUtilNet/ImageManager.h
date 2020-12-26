/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
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

            static System::Boolean AddFileSmallImage(System::String^ sFile, System::String^ sImageName);

            /// <summary>
            /// Adds a file extension's small image to the small image list.
            /// </summary>

            static System::Boolean AddFileExtensionSmallImage(System::String^ sFileExtension, System::String^ sImageName);

            static System::Boolean AddToolbarSmallImages(System::Resources::ResourceManager^ ResourceManager);

            static System::String^ GenerateToolbarImageKey(System::Resources::ResourceManager^ ResourceManager, System::String^ sImageName);

        private:
            static System::Windows::Forms::ImageList^ s_FileSmallImageList = nullptr;
            static System::Windows::Forms::ImageList^ s_ToolbarSmallImageList = nullptr;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
