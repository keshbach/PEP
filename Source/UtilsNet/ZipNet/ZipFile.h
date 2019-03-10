/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Zip
    {
        public ref class File sealed :
            System::Collections::Generic::IEnumerable<Common::Zip::Item^>
        {
        public:
            enum class EOpenMode
            {
                CreateNew, // fails if file already exists
                CreateAlways, // create and overwrite existing file
                Open, // open existing file and if file not found fail
                OpenAlways // open existing file and create if it does not exist
            };

            enum class ECompressionMethod
            {
                Stored,
                Deflated
            };

        public:
            /// <value>
            /// The comment associated with the zip file.
            /// </value>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            property System::String^ ZipComment
            {
                System::String^ get()
                {
                    return GetZipComment();
                }

                void set(System::String^ sValue)
                {
                    SetZipComment(sValue);
                }
            }

            /// <value>
            /// Flag indicating whether in the batch update mode.
            /// </value>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            property System::Boolean InUpdate
            {
                System::Boolean get()
                {
                    return GetZipInUpdate();
                }
            }

            /// <value>
            /// Flag indicating whether a zip file is open or not.
            /// </value>

            property System::Boolean Active
            {
                System::Boolean get()
                {
                    return (m_hZip != NULL) ? true : false;
                }
            }

        public:
            /// <summary>
            /// Open a zip file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            static Common::Zip::File^ Open(System::String^ sFile, EOpenMode OpenMode);

            /// <summary>
            /// Close and release any resources associated with the zip file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void Close();

            /// <summary>
            /// Validates an archive is not corrupt.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            System::Boolean IsValidArchive();

            /// <summary>
            /// Initiates a batch mode update.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void BeginUpdate();

            /// <summary>
            /// Ends a batch mode update and automatically updates the zip file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void EndUpdate();

            /// <summary>
            /// Extracts the contents of a file into a buffer.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void ExtractToBuffer(System::String^ sFile, array<System::Byte>^% Buffer);

            /// <summary>
            /// Extracts the contents of a file into a file that is automatically overwritten.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void ExtractToFile(System::String^ sFile, System::String^ sOutputFile);

            /// <summary>
            /// Modifies the comment associated with a file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void ChangeFileComment(System::String^ sFile, System::String^ sComment);

            /// <summary>
            /// Renames the name of a file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void ChangeFileName(System::String^ sFile, System::String^ sNewFile);

            /// <summary>
            /// Modifies the data associated with a file from a buffer.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>
            
            void ChangeFileData(System::String^ sFile, array<System::Byte>^ Buffer, ECompressionMethod CompressionMethod);

            /// <summary>
            /// Modifies the data associated with a file from an existing file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void ChangeFileData(System::String^ sFile, System::String^ sDataFile, ECompressionMethod CompressionMethod);

            /// <summary>
            /// Adds an existing file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void AddFile(System::String^ sFile, System::String^ sDataFile, System::String^ sComment, ECompressionMethod CompressionMethod);
            
            /// <summary>
            /// Adds the contents of a buffer.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void AddBuffer(System::String^ sFile, System::String^ sComment, array<System::Byte>^ Buffer, ECompressionMethod CompressionMethod);

            /// <summary>
            /// Adds the contents of a string.  (String is interpreted as ASCII only.)
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void AddString(System::String^ sFile, System::String^ sComment, System::String^ sBuffer, ECompressionMethod CompressionMethod);

            /// <summary>
            /// Deletes a file from a folder.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void DeleteFile(System::String^ sFile);

            /// <summary>
            /// Copies a file from one folder into a different folder.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void CopyFile(System::String^ sSrcFile, System::String^ sDestFile);

            /// <summary>
            /// Renames a folder and all of it's contents.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void RenameFolder(System::String^ sSrcPath, System::String^ sDestPath);

            /// <summary>
            /// Delete a folder and all of it's contents.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void DeleteFolder(System::String^ sPath);

            /// <summary>
            /// Copy a folder and all of it's contents to another folder.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            void CopyFolder(System::String^ sSrcPath, System::String^ sDestPath);

            /// <summary>
            /// Finds the zip item associated with a file.
            /// </summary>
            /// <exception cref="System::Exception">Thrown when an error occurs.</exception>

            Common::Zip::Item^ FindItem(System::String^ sFile);

            // System::Collections::Generic::IEnumerable<>
            virtual System::Collections::Generic::IEnumerator<Common::Zip::Item^>^ GetEnumerator() = System::Collections::Generic::IEnumerable<Common::Zip::Item^>::GetEnumerator;
            virtual System::Collections::IEnumerator^ GetEnumeratorBase() = System::Collections::IEnumerable::GetEnumerator;

        private:
            File();
            ~File();
            !File();

            void CheckIfZipClosed();

            System::Boolean GetZipInUpdate();

            System::String^ GetZipComment();
            void SetZipComment(System::String^ sComment);

            void EmptyBufferFileList();

        private:
            TZipHandle m_hZip;

            System::Collections::Generic::List<System::String^>^ m_TmpBufferFileList;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
