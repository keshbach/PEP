/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
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
            static property System::Windows::Forms::ImageList^ SmallImageList
            {
                System::Windows::Forms::ImageList^ get()
                {
                    return s_SmallImageList;
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

        private:
            static System::Windows::Forms::ImageList^ s_SmallImageList = nullptr;
            static System::Collections::Generic::List<System::IntPtr>^ s_IconHandleArrayList = nullptr;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
