/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for FileOpenDialog.
        /// </summary>

        public ref class FileOpenDialog
        {
        public:
            /// <summary>
            /// Pick folders instead of files.
            /// </summary>

            property System::Boolean PickFolders
            {
                System::Boolean get()
                {
                    return m_bPickFolders;
                }

                void set(System::Boolean value)
                {
                    m_bPickFolders = value;
                }
            }

            /// <summary>
            /// Select multiple items.
            /// </summary>

            property System::Boolean SelectMultipleItems
            {
                System::Boolean get()
                {
                    return m_bSelectMultipleItems;
                }

                void set(System::Boolean value)
                {
                    m_bSelectMultipleItems = value;
                }
            }

            /// <summary>
            /// Allow read-only files to be selected.
            /// </summary>

            property System::Boolean AllowReadOnly
            {
                System::Boolean get()
                {
                    return m_bAllowReadOnly;
                }

                void set(System::Boolean value)
                {
                    m_bAllowReadOnly = value;
                }
            }

            /// <summary>
            /// Add to recently used document list.
            /// </summary>

            property System::Boolean AddToRecentList
            {
                System::Boolean get()
                {
                    return m_bAddToRecentList;
                }

                void set(System::Boolean value)
                {
                    m_bAddToRecentList = value;
                }
            }

            /// <summary>
            /// Show hidden and system files.
            /// </summary>

            property System::Boolean ShowHidden
            {
                System::Boolean get()
                {
                    return m_bShowHidden;
                }

                void set(System::Boolean value)
                {
                    m_bShowHidden = value;
                }
            }

            /// <summary>
            /// Initial file name or selected file name if in single selection mode
            /// </summary>

            property System::String^ FileName
            {
                System::String^ get()
                {
                    return m_sFileName;
                }

                void set(System::String^ value)
                {
                    m_sFileName = value;
                }
            }

            /// <summary>
            /// File names selected when in multiple selection mode
            /// </summary>

            property System::Collections::Specialized::StringCollection^ FileNames
            {
                System::Collections::Specialized::StringCollection^ get()
                {
                    return m_FileNameCollection;
                }
            }

            /// <summary>
            /// Initial folder or selected folder if in single selection mode
            /// </summary>

            property System::String^ Folder
            {
                System::String^ get()
                {
                    return m_sFolder;
                }

                void set(System::String^ value)
                {
                    m_sFolder = value;
                }
            }

            /// <summary>
            /// Folder selected when in multiple selection mode
            /// </summary>

            property System::Collections::Specialized::StringCollection^ Folders
            {
                System::Collections::Specialized::StringCollection^ get()
                {
                    return m_FolderCollection;
                }
            }

            /// <summary>
            /// File Types that are selectable
            /// </summary>

            property System::Collections::Generic::List<Common::Forms::FileTypeItem^>^ FileTypes
            {
                System::Collections::Generic::List<Common::Forms::FileTypeItem^>^ get()
                {
                    return m_FileTypesList;
                }

                void set(System::Collections::Generic::List<Common::Forms::FileTypeItem^>^ value)
                {
                    m_FileTypesList = value;
                }
            }

            /// <summary>
            /// Selected File Type
            /// </summary>

            property System::UInt32 SelectedFileType
            {
                System::UInt32 get()
                {
                    return m_nSelectedFileType;
                }

                void set(System::UInt32 value)
                {
                    m_nSelectedFileType = value;
                }
            }

        public:
            FileOpenDialog();

            /// <summary>
            /// Shows the File Open dialog box
            /// </summary>

            System::Windows::Forms::DialogResult ShowDialog(System::Windows::Forms::IWin32Window^ Owner);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~FileOpenDialog();

        protected:
            System::Boolean m_bPickFolders;
            System::Boolean m_bSelectMultipleItems;
            System::Boolean m_bAllowReadOnly;
            System::Boolean m_bAddToRecentList;
            System::Boolean m_bShowHidden;
            System::String^ m_sFileName;
            System::Collections::Specialized::StringCollection^ m_FileNameCollection;
            System::String^ m_sFolder;
            System::Collections::Specialized::StringCollection^ m_FolderCollection;
            System::Collections::Generic::List<Common::Forms::FileTypeItem^>^ m_FileTypesList;
            System::UInt32 m_nSelectedFileType;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
