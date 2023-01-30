/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for FileSaveDialog.
        /// </summary>

        public ref class FileSaveDialog
        {
        public:
            /// <summary>
            /// Title for the save dialog.
            /// </summary>
            property System::String^ Title
            {
                System::String^ get()
                {
                    return m_sTitle;
                }

                void set(System::String^ value)
                {
                    m_sTitle = value;
                }
            }

            /// <summary>
            /// Prompt before overwriting an existing file.
            /// </summary>

            property System::Boolean OverwritePrompt
            {
                System::Boolean get()
                {
                    return m_bOverwritePrompt;
                }

                void set(System::Boolean value)
                {
                    m_bOverwritePrompt = value;
                }
            }

            /// <summary>
            /// Only files that match a file type are allowed.
            /// </summary>

            property System::Boolean StrictFileType
            {
                System::Boolean get()
                {
                    return m_bStrictFileType;
                }

                void set(System::Boolean value)
                {
                    m_bStrictFileType = value;
                }
            }

            /// <summary>
            /// Prompt for creation if the file does not exist.  (The file is not actually created.)
            /// </summary>

            property System::Boolean CreatePrompt
            {
                System::Boolean get()
                {
                    return m_bCreatePrompt;
                }

                void set(System::Boolean value)
                {
                    m_bCreatePrompt = value;
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
            /// Initial file name or selected file name
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
            /// Default file extension to add to a file name.  (Do not include the leading period.)
            /// </summary>

            property System::String^ DefaultExtension
            {
                System::String^ get()
                {
                    return m_sDefaultExtension;
                }

                void set(System::String^ value)
                {
                    m_sDefaultExtension = value;
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

			/// <summary>
			/// Client GUID used to store the dialog's last position and size.
			/// </summary>
			property System::Guid^ ClientGuid
			{
				System::Guid^ get()
				{
					return m_ClientGuid;
				}

				void set(System::Guid^ value)
				{
					m_ClientGuid = value;
				}
			}

        public:
            FileSaveDialog();

            /// <summary>
            /// Shows the File Save dialog box
            /// </summary>

            System::Windows::Forms::DialogResult ShowDialog(System::Windows::Forms::IWin32Window^ Owner);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~FileSaveDialog();

        private:
            System::Windows::Forms::DialogResult ShowDialogVista(System::Windows::Forms::IWin32Window^ Owner);
            System::Windows::Forms::DialogResult ShowDialogWindowsXP(System::Windows::Forms::IWin32Window^ Owner);

        protected:
            System::String^ m_sTitle;
            System::Boolean m_bOverwritePrompt;
            System::Boolean m_bStrictFileType;
            System::Boolean m_bCreatePrompt;
            System::Boolean m_bAllowReadOnly;
            System::Boolean m_bAddToRecentList;
            System::Boolean m_bShowHidden;
            System::String^ m_sFileName;
            System::String^ m_sDefaultExtension;
            System::Collections::Generic::List<Common::Forms::FileTypeItem^>^ m_FileTypesList;
            System::UInt32 m_nSelectedFileType;
			System::Guid^ m_ClientGuid;
		};
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
