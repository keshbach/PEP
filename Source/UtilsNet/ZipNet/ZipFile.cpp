/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UtZip.h"

#include "ZipItem.h"
#include "ZipEnumerator.h"

#include "ZipFile.h"

#using "UtilNet.dll"

static VOID lThrowZipError(
  INT nErrorCode)
{
    switch (nErrorCode)
    {
        case CUtZipInternalError:
            throw gcnew System::Exception(L"An internal error was experienced.");
            break;
        case CUtZipFileCouldNotBeOpened:
            throw gcnew System::Exception(L"The zip file could not be opened.");
            break;
        case CUtZipFileError:
            throw gcnew System::Exception(L"A file error occurred reading the zip file.");
            break;
        case CUtZipOutOfMemory:
            throw gcnew System::Exception(L"Not enough memory was available for a zip operation.");
            break;
        case CUtZipInvalidZipFile:
            throw gcnew System::Exception(L"The zip file is not a valid zip file.");
            break;
        case CUtZipEncryptedFilesNotSupported:
            throw gcnew System::Exception(L"The zip file contains encrypted file(s).");
            break;
        case CUtZipDiskSpanningNotSupported:
            throw gcnew System::Exception(L"Disk spanning is not supported.");
            break;
        case CUtZipCompressionMethodNotSupported:
            throw gcnew System::Exception(L"The zip file contains an unsupported compression method.");
            break;
        case CUtZipFileNotFound:
            throw gcnew System::Exception(L"The file could not be found.");
            break;
        case CUtZipCreateFileError:
            throw gcnew System::Exception(L"The file could not be created.");
            break;
        case CUtZipCRCMismatch:
            throw gcnew System::Exception(L"The zip file contains a CRC mismatch.");
            break;
        case CUtZipErrorInCompressedData:
            throw gcnew System::Exception(L"The zip file contains an error in the compressed data.");
            break;
        case CUtZipDictionaryMissing:
            throw gcnew System::Exception(L"The zip file is missing a dictionary.");
            break;
        case CUtZipZipCommentTooLong:
            throw gcnew System::Exception(L"The zip file's zip comment is too long.");
            break;
        case CUtZipFileCommentTooLong:
            throw gcnew System::Exception(L"The zip file's comment is too long.");
            break;
        case CUtZipFileNameTooLong:
            throw gcnew System::Exception(L"The zip file's name is too long.");
            break;
        case CUtZipFileCompressionError:
            throw gcnew System::Exception(L"An error occurred compressing a file.");
            break;
        case CUtZipFileAlreadyExists:
            throw gcnew System::Exception(L"The file already exists.");
            break;
        case CUtZipFileAlreadyAdded:
            throw gcnew System::Exception(L"The file is already being added.");
            break;
        case CUtZipFileAlreadyDeleted:
            throw gcnew System::Exception(L"The file is already being deleted.");
            break;
        case CUtZipNotInUpdate:
            throw gcnew System::Exception(L"The zip file is not in an update.");
            break;
        case CUtZipAlreadyInUpdate:
            throw gcnew System::Exception(L"The zip file is already in an update.");
            break;
        case CUtZipInvalidFileName:
            throw gcnew System::Exception(L"The file name is invalid.");
            break;
        case CUtZipFileAlreadyBeingChanged:
            throw gcnew System::Exception(L"The file is already being changed.");
            break;
        case CUtZipFileAlreadyBeingCopied:
            throw gcnew System::Exception(L"The file is already being copied.");
            break;
        case CUtZipDirectoryRenameFailed:
            throw gcnew System::Exception(L"A directory rename failed.");
            break;
        case CUtZipDirectoryDeleteFailed:
            throw gcnew System::Exception(L"A directory delete failed.");
            break;
        case CUtZipDirectoryCopyFailed:
            throw gcnew System::Exception(L"A directory copy failed.");
            break;
        case CUtZipDataFileReadError:
            throw gcnew System::Exception(L"Reading the data file has failed.");
            break;
        case CUtZipDataFileTooLarge:
            throw gcnew System::Exception(L"The data file is too large.");
            break;
        case CUtZipInvalidDirectoryName:
            throw gcnew System::Exception(L"The directory name is invalid.");
            break;
        default:
            System::Diagnostics::Debug::Assert(false, L"Unknown zip error.");
            break;
    }

    throw gcnew System::Exception(L"An unknown zip error occurred.");
}

static Common::Zip::Item::ECompressionMethod lTranslateZipCompressionMethod(
  EZipCompressionMethod ZipCompressionMethod)
{
    switch (ZipCompressionMethod)
    {
        case eZcmStored:
            return Common::Zip::Item::ECompressionMethod::Stored;
        case eZcmDeflated:
            return Common::Zip::Item::ECompressionMethod::Deflated;
        case eZcmUnsupportedCompressionMethod:
            return Common::Zip::Item::ECompressionMethod::Unknown;
    }

    System::Diagnostics::Debug::Assert(false, L"Unknown compression method.");

    return Common::Zip::Item::ECompressionMethod::Unknown;
}

static Common::Zip::Item::EFileAttributes lTranslateZipFileAttributes(
  UINT32 nZipFileAttributes)
{
    Common::Zip::Item::EFileAttributes FileAttributes(Common::Zip::Item::EFileAttributes::None);

    if (nZipFileAttributes & CZipFileAttributeHidden)
    {
        FileAttributes = FileAttributes | Common::Zip::Item::EFileAttributes::Hidden;
    }

    if (nZipFileAttributes & CZipFileAttributeReadOnly)
    {
        FileAttributes = FileAttributes | Common::Zip::Item::EFileAttributes::ReadOnly;
    }

    if (nZipFileAttributes & CZipFileAttributeSystem)
    {
        FileAttributes = FileAttributes | Common::Zip::Item::EFileAttributes::System;
    }

    if (nZipFileAttributes & CZipFileAttributeArchive)
    {
        FileAttributes = FileAttributes | Common::Zip::Item::EFileAttributes::Archive;
    }

    return FileAttributes;
}

static System::DateTime lTranslateSystemTime(
  LPSYSTEMTIME pSystemTime)
{
    return System::DateTime(pSystemTime->wYear,
                            pSystemTime->wMonth,
                            pSystemTime->wDay,
                            pSystemTime->wHour,
                            pSystemTime->wMinute,
                            pSystemTime->wSecond,
                            System::DateTimeKind::Utc);
}

static System::String^ lTranslateZipPathSeparatorToPathSeparator(
  LPCWSTR pszText)
{
    System::String^ sNewText = gcnew System::String(pszText);

    return sNewText->Replace(L'/', System::IO::Path::DirectorySeparatorChar);
}

static System::String^ lTranslatePathSeparatorToZipPathSeparator(
  System::String^ sText)
{
    return sText->Replace(System::IO::Path::DirectorySeparatorChar, L'/');
}

Common::Zip::File::File() :
  m_hZip(NULL)
{
    m_TmpBufferFileList = gcnew System::Collections::Generic::List<System::String^>();
}

Common::Zip::File::~File()
{
    this->!File();
}

Common::Zip::File::!File()
{
    if (m_hZip)
    {
        Close();
    }
}

Common::Zip::File^ Common::Zip::File::Open(
  System::String^ sFile,
  EOpenMode OpenMode)
{
    pin_ptr<const wchar_t> pszFile = PtrToStringChars(sFile);
    Common::Zip::File^ File = gcnew Common::Zip::File();
    EZipOpenMode ZipOpenMode;
    TZipHandle hZip;
    INT nErrorCode;

    switch (OpenMode)
    {
        case Common::Zip::File::EOpenMode::CreateNew:
            ZipOpenMode = eZomCreateNew;
            break;
        case Common::Zip::File::EOpenMode::CreateAlways:
            ZipOpenMode = eZomCreateAlways;
            break;
        case Common::Zip::File::EOpenMode::Open:
            ZipOpenMode = eZomOpen;
            break;
        case Common::Zip::File::EOpenMode::OpenAlways:
            ZipOpenMode = eZomOpenAlways;
            break;
        default:
            throw gcnew System::Exception(L"Unknown zip open mode.");
            break;
    }

    hZip = UtZipOpenFile(pszFile, ZipOpenMode, &nErrorCode);

    if (hZip == NULL)
    {
        lThrowZipError(nErrorCode);
    }

    File->m_hZip = hZip;

    return File;
}

void Common::Zip::File::Close()
{
    INT nErrorCode;

    CheckIfZipClosed();

    UtZipClose(m_hZip, &nErrorCode);

    EmptyBufferFileList();

    m_TmpBufferFileList = nullptr;
    m_hZip = NULL;

    if (nErrorCode != CUtZipNoError)
    {
        lThrowZipError(nErrorCode);
    }
}

System::Boolean Common::Zip::File::IsValidArchive()
{
    INT nErrorCode;
    BOOL bIsValidArchive;

    CheckIfZipClosed();

    if (UtZipIsValidArchive(m_hZip, &bIsValidArchive, &nErrorCode))
    {
        return bIsValidArchive ? true : false;
    }

    lThrowZipError(nErrorCode);

    return false;
}

void Common::Zip::File::BeginUpdate()
{
    INT nErrorCode;

    CheckIfZipClosed();

    if (!UtZipBeginUpdate(m_hZip, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::EndUpdate()
{
    INT nErrorCode;

    CheckIfZipClosed();

    UtZipEndUpdate(m_hZip, &nErrorCode);

    EmptyBufferFileList();

    if (nErrorCode != CUtZipNoError)
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::ExtractToBuffer(
  System::String^ sFile,
  array<System::Byte>^% Buffer)
{
    pin_ptr<const wchar_t> pszNewFile, pszTmpFile;
    System::String^ sNewFile;
    System::String^ sTmpFile;
    INT nErrorCode;

    Buffer = nullptr;

    CheckIfZipClosed();

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    sTmpFile = Common::IO::File::GenerateTempFileName(L".tmp");

    pszTmpFile = PtrToStringChars(sTmpFile);

    if (!UtZipExtractFile(m_hZip, pszNewFile, pszTmpFile, &nErrorCode))
    {
        ::DeleteFileW(pszTmpFile);

        lThrowZipError(nErrorCode);
    }

    try
    {
        Buffer = System::IO::File::ReadAllBytes(sTmpFile);
    }
    catch (System::Exception^)
    {
    }
    finally
    {
        ::DeleteFileW(pszTmpFile);
    }

    if (Buffer == nullptr)
    {
        lThrowZipError(CUtZipInternalError);
    }
}

void Common::Zip::File::ExtractToFile(
  System::String^ sFile,
  System::String^ sOutputFile)
{
    pin_ptr<const wchar_t> pszOutputFile = PtrToStringChars(sOutputFile);
    pin_ptr<const wchar_t> pszNewFile;
    System::String^ sNewFile;
    INT nErrorCode;

    CheckIfZipClosed();

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    if (!UtZipExtractFile(m_hZip, pszNewFile, pszOutputFile, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::ChangeFileComment(
  System::String^ sFile,
  System::String^ sComment)
{
    pin_ptr<const wchar_t> pszComment = PtrToStringChars(sComment);
    pin_ptr<const wchar_t> pszNewFile;
    System::String^ sNewFile;
    INT nErrorCode;

    CheckIfZipClosed();

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    if (!UtZipChangeFileComment(m_hZip, pszNewFile, pszComment, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::ChangeFileName(
  System::String^ sFile,
  System::String^ sNewFile)
{
    pin_ptr<const wchar_t> pszTmpFile;
    pin_ptr<const wchar_t> pszTmpNewFile;
    System::String^ sTmpFile;
    System::String^ sTmpNewFile;
    INT nErrorCode;

    CheckIfZipClosed();

    sTmpFile = lTranslatePathSeparatorToZipPathSeparator(sFile);
    sTmpNewFile = lTranslatePathSeparatorToZipPathSeparator(sNewFile);

    pszTmpFile = PtrToStringChars(sTmpFile);
    pszTmpNewFile = PtrToStringChars(sTmpNewFile);

    if (!UtZipChangeFileName(m_hZip, pszTmpFile, pszTmpNewFile, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::ChangeFileData(
  System::String^ sFile,
  array<System::Byte>^ Buffer,
  ECompressionMethod CompressionMethod)
{
    pin_ptr<const wchar_t> pszNewFile, pszDataFile;
    System::String^ sNewFile;
    System::String^ sDataFile;
    INT nErrorCode;
    EZipCompressionMethod ZipCompressionMethod;
    BOOL bUpdateFlag;

    CheckIfZipClosed();

    if (!UtZipGetUpdateFlag(m_hZip, &bUpdateFlag, &nErrorCode))
    {
        lThrowZipError(CUtZipInternalError);
    }

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    sDataFile = Common::IO::File::GenerateTempFileName(L".tmp");

    pszDataFile = PtrToStringChars(sDataFile);

    try
    {
        System::IO::File::WriteAllBytes(sDataFile, Buffer);
    }

    catch (System::Exception^)
    {
        ::DeleteFile(pszDataFile);

        lThrowZipError(CUtZipInternalError);
    }

    if (bUpdateFlag)
    {
        m_TmpBufferFileList->Add(sDataFile);
    }

    switch (CompressionMethod)
    {
        case ECompressionMethod::Stored:
            ZipCompressionMethod = eZcmStored;
            break;
        case ECompressionMethod::Deflated:
            ZipCompressionMethod = eZcmDeflated;
            break;
        default:
            ZipCompressionMethod = eZcmStored;

            lThrowZipError(CUtZipCompressionMethodNotSupported);
            break;
    }

    UtZipChangeFileData(m_hZip, pszNewFile, pszDataFile, ZipCompressionMethod, &nErrorCode);

    if (!bUpdateFlag)
    {
        ::DeleteFileW(pszDataFile);
    }

    if (nErrorCode != CUtZipNoError)
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::ChangeFileData(
  System::String^ sFile,
  System::String^ sDataFile,
  ECompressionMethod CompressionMethod)
{
    pin_ptr<const wchar_t> pszDataFile = PtrToStringChars(sDataFile);
    pin_ptr<const wchar_t> pszNewFile;
    System::String^ sNewFile;
    INT nErrorCode;
    EZipCompressionMethod ZipCompressionMethod;

    CheckIfZipClosed();

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    switch (CompressionMethod)
    {
        case ECompressionMethod::Stored:
            ZipCompressionMethod = eZcmStored;
            break;
        case ECompressionMethod::Deflated:
            ZipCompressionMethod = eZcmDeflated;
            break;
        default:
            ZipCompressionMethod = eZcmStored;

            lThrowZipError(CUtZipCompressionMethodNotSupported);
            break;
    }

    if (!UtZipChangeFileData(m_hZip, pszNewFile, pszDataFile, ZipCompressionMethod, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::AddFile(
  System::String^ sFile,
  System::String^ sDataFile,
  System::String^ sComment,
  ECompressionMethod CompressionMethod)
{
    pin_ptr<const wchar_t> pszDataFile = PtrToStringChars(sDataFile);
    pin_ptr<const wchar_t> pszComment = PtrToStringChars(sComment);
    pin_ptr<const wchar_t> pszNewFile;
    System::String^ sNewFile;
    INT nErrorCode;
    EZipCompressionMethod ZipCompressionMethod;

    CheckIfZipClosed();

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    switch (CompressionMethod)
    {
        case ECompressionMethod::Stored:
            ZipCompressionMethod = eZcmStored;
            break;
        case ECompressionMethod::Deflated:
            ZipCompressionMethod = eZcmDeflated;
            break;
        default:
            ZipCompressionMethod = eZcmStored;

            lThrowZipError(CUtZipCompressionMethodNotSupported);
            break;
    }

    if (!UtZipAddFile(m_hZip, pszNewFile, pszDataFile, pszComment,
                      ZipCompressionMethod, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::AddBuffer(
  System::String^ sFile,
  System::String^ sComment,
  array<System::Byte>^ Buffer,
  ECompressionMethod CompressionMethod)
{
    pin_ptr<const wchar_t> pszComment = PtrToStringChars(sComment);
    pin_ptr<const wchar_t> pszNewFile, pszDataFile;
    System::String^ sNewFile;
    System::String^ sDataFile;
    INT nErrorCode;
    EZipCompressionMethod ZipCompressionMethod;
    BOOL bUpdateFlag;

    CheckIfZipClosed();

    if (!UtZipGetUpdateFlag(m_hZip, &bUpdateFlag, &nErrorCode))
    {
        lThrowZipError(CUtZipInternalError);
    }

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    sDataFile = Common::IO::File::GenerateTempFileName(L".tmp");

    pszDataFile = PtrToStringChars(sDataFile);

    try
    {
        System::IO::File::WriteAllBytes(sDataFile, Buffer);
    }

    catch (System::Exception^)
    {
        ::DeleteFileW(pszDataFile);

        lThrowZipError(CUtZipInternalError);
    }

    if (bUpdateFlag)
    {
        m_TmpBufferFileList->Add(sDataFile);
    }

    switch (CompressionMethod)
    {
        case ECompressionMethod::Stored:
            ZipCompressionMethod = eZcmStored;
            break;
        case ECompressionMethod::Deflated:
            ZipCompressionMethod = eZcmDeflated;
            break;
        default:
            ZipCompressionMethod = eZcmStored;

            lThrowZipError(CUtZipCompressionMethodNotSupported);
            break;
    }

    UtZipAddFile(m_hZip, pszNewFile, pszDataFile, pszComment, ZipCompressionMethod,
                 &nErrorCode);

    if (!bUpdateFlag)
    {
        ::DeleteFileW(pszDataFile);
    }

    if (nErrorCode != CUtZipNoError)
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::AddString(
  System::String^ sFile,
  System::String^ sComment,
  System::String^ sBuffer,
  ECompressionMethod CompressionMethod)
{
    System::Text::ASCIIEncoding^ Encoding = gcnew System::Text::ASCIIEncoding();
    array<System::Byte>^ NewBuffer = Encoding->GetBytes(sBuffer);

    AddBuffer(sFile, sComment, NewBuffer, CompressionMethod);
}

void Common::Zip::File::DeleteFile(
  System::String^ sFile)
{
    pin_ptr<const wchar_t> pszNewFile;
    System::String^ sNewFile;
    INT nErrorCode;

    CheckIfZipClosed();

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    if (!UtZipDeleteFile(m_hZip, pszNewFile, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::CopyFile(
  System::String^ sSrcFile,
  System::String^ sDestFile)
{
    pin_ptr<const wchar_t> pszNewSrcFile, pszNewDestFile;
    System::String^ sNewSrcFile;
    System::String^ sNewDestFile;
    INT nErrorCode;

    CheckIfZipClosed();

    sNewSrcFile = lTranslatePathSeparatorToZipPathSeparator(sSrcFile);
    sNewDestFile = lTranslatePathSeparatorToZipPathSeparator(sDestFile);

    pszNewSrcFile = PtrToStringChars(sNewSrcFile);
    pszNewDestFile = PtrToStringChars(sNewDestFile);

    if (!UtZipCopyFile(m_hZip, pszNewSrcFile, pszNewDestFile, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::RenameFolder(
  System::String^ sSrcPath,
  System::String^ sDestPath)
{
    pin_ptr<const wchar_t> pszNewSrcPath, pszNewDestPath;
    System::String^ sNewSrcPath;
    System::String^ sNewDestPath;
    INT nErrorCode;

    CheckIfZipClosed();

    sNewSrcPath = lTranslatePathSeparatorToZipPathSeparator(sSrcPath);
    sNewDestPath = lTranslatePathSeparatorToZipPathSeparator(sDestPath);

    pszNewSrcPath = PtrToStringChars(sNewSrcPath);
    pszNewDestPath = PtrToStringChars(sNewDestPath);

    if (!UtZipRenameDirectory(m_hZip, pszNewSrcPath, pszNewDestPath, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::DeleteFolder(
  System::String^ sPath)
{
    pin_ptr<const wchar_t> pszNewPath;
    System::String^ sNewPath;
    INT nErrorCode;

    CheckIfZipClosed();

    sNewPath = lTranslatePathSeparatorToZipPathSeparator(sPath);

    pszNewPath = PtrToStringChars(sNewPath);

    if (!UtZipDeleteDirectory(m_hZip, pszNewPath, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::CopyFolder(
  System::String^ sSrcPath,
  System::String^ sDestPath)
{
    pin_ptr<const wchar_t> pszNewSrcPath, pszNewDestPath;
    System::String^ sNewSrcPath;
    System::String^ sNewDestPath;
    INT nErrorCode;

    CheckIfZipClosed();

    sNewSrcPath = lTranslatePathSeparatorToZipPathSeparator(sSrcPath);
    sNewDestPath = lTranslatePathSeparatorToZipPathSeparator(sDestPath);

    pszNewSrcPath = PtrToStringChars(sNewSrcPath);
    pszNewDestPath = PtrToStringChars(sNewDestPath);

    if (!UtZipCopyDirectory(m_hZip, pszNewSrcPath, pszNewDestPath, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

Common::Zip::Item^ Common::Zip::File::FindItem(
  System::String^ sFile)
{
    pin_ptr<const wchar_t> pszNewFile;
    System::String^ sNewFile;
    TZipFileEntryVec ZipFileEntryVec;
    INT nErrorCode;
    Common::Zip::Item^ ZipItem;

    CheckIfZipClosed();

    sNewFile = lTranslatePathSeparatorToZipPathSeparator(sFile);

    pszNewFile = PtrToStringChars(sNewFile);

    ZipItem = nullptr;

    if (!UtZipAllocFileEntries(m_hZip, ZipFileEntryVec, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }

    for (TZipFileEntryVec::iterator it = ZipFileEntryVec.begin();
         ZipItem == nullptr && it != ZipFileEntryVec.end(); ++it)
    {
        if (::lstrcmpW(pszNewFile, (*it)->pszSrcFileName) == 0)
        {
            ZipItem = gcnew Common::Zip::Item(
                            lTranslateSystemTime(&(*it)->FileModTime),
                            lTranslateZipFileAttributes((*it)->nFileAttributes),
                            (*it)->nCRC,
                            lTranslateZipCompressionMethod((*it)->ZipCompressionMethod),
                            (*it)->nCompressedSize,
                            (*it)->nUncompressedSize,
                            gcnew System::String((*it)->pszFileName),
                            lTranslateZipPathSeparatorToPathSeparator((*it)->pszPath),
                            lTranslateZipPathSeparatorToPathSeparator((*it)->pszSrcFileName),
                            gcnew System::String((*it)->pszComment));
        }
    }

    if (!UtZipFreeFileEntries(ZipFileEntryVec, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }

    return ZipItem;
}

System::Collections::Generic::IEnumerator<Common::Zip::Item^>^ Common::Zip::File::GetEnumerator()
{
    TZipFileEntryVec ZipFileEntryVec;
    System::Collections::Generic::List<Common::Zip::Item^>^ ZipItemList;
    Common::Zip::Item^ ZipItem;
    INT nErrorCode;

    CheckIfZipClosed();

    ZipItemList = gcnew System::Collections::Generic::List<Common::Zip::Item^>();

    if (UtZipAllocFileEntries(m_hZip, ZipFileEntryVec, &nErrorCode))
    {
        for (TZipFileEntryVec::iterator it = ZipFileEntryVec.begin();
             it != ZipFileEntryVec.end(); ++it)
        { 
            if (*((*it)->pszFileName) != 0)
            {
                ZipItem = gcnew Common::Zip::Item(
                              lTranslateSystemTime(&(*it)->FileModTime),
                              lTranslateZipFileAttributes((*it)->nFileAttributes),
                              (*it)->nCRC,
                              lTranslateZipCompressionMethod((*it)->ZipCompressionMethod),
                              (*it)->nCompressedSize,
                              (*it)->nUncompressedSize,
                              gcnew System::String((*it)->pszFileName),
                              lTranslateZipPathSeparatorToPathSeparator((*it)->pszPath),
                              lTranslateZipPathSeparatorToPathSeparator((*it)->pszSrcFileName),
                              gcnew System::String((*it)->pszComment));

                ZipItemList->Add(ZipItem);
            }
        }

        UtZipFreeFileEntries(ZipFileEntryVec, &nErrorCode);
    }
    else
    {
        lThrowZipError(nErrorCode);
    }

    return gcnew Common::Zip::Enumerator(ZipItemList);
}

System::Collections::IEnumerator^ Common::Zip::File::GetEnumeratorBase()
{
    return GetEnumerator();
}

void Common::Zip::File::CheckIfZipClosed()
{
    if (m_hZip == NULL)
    {
        throw gcnew System::Exception(L"The zip file has been closed.");
    }
}

System::Boolean Common::Zip::File::GetZipInUpdate()
{
    BOOL bUpdateFlag;
    INT nErrorCode;

    CheckIfZipClosed();

    if (!UtZipGetUpdateFlag(m_hZip, &bUpdateFlag, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }

    return (bUpdateFlag == TRUE) ? true : false;
}

System::String^ Common::Zip::File::GetZipComment()
{
    INT nErrorCode;
    LPCWSTR pszComment;

    CheckIfZipClosed();

    pszComment = UtZipGetZipComment(m_hZip, &nErrorCode);

    if (!pszComment)
    {
        lThrowZipError(nErrorCode);
    }

    return gcnew System::String(pszComment);
}

void Common::Zip::File::SetZipComment(
  System::String^ sComment)
{
    pin_ptr<const wchar_t> pszComment = PtrToStringChars(sComment);
    INT nErrorCode;

    CheckIfZipClosed();

    if (!UtZipSetZipComment(m_hZip, pszComment, &nErrorCode))
    {
        lThrowZipError(nErrorCode);
    }
}

void Common::Zip::File::EmptyBufferFileList()
{
    for each (System::String^ sFile in m_TmpBufferFileList)
    {
        try
        {
            System::IO::File::Delete(sFile);
        }
        catch (System::Exception^)
        {
        }
    }

    m_TmpBufferFileList->Clear();
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
