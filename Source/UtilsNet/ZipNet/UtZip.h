/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

//
// Zip Error Codes
//

#define CUtZipNoError 0
#define CUtZipInternalError 1
#define CUtZipFileCouldNotBeOpened 2
#define CUtZipFileError 3
#define CUtZipOutOfMemory 4
#define CUtZipInvalidZipFile 5
#define CUtZipEncryptedFilesNotSupported 6
#define CUtZipDiskSpanningNotSupported 7
#define CUtZipCompressionMethodNotSupported 8
#define CUtZipOperatingSystemNotSupported 9
#define CUtZipFileNotFound 10
#define CUtZipCreateFileError 11
#define CUtZipCRCMismatch 12
#define CUtZipErrorInCompressedData 13
#define CUtZipDictionaryMissing 14
#define CUtZipZipCommentTooLong 15
#define CUtZipFileCommentTooLong 16
#define CUtZipFileNameTooLong 17
#define CUtZipFileCompressionError 18
#define CUtZipFileAlreadyExists 19
#define CUtZipFileAlreadyAdded 20
#define CUtZipFileAlreadyDeleted 21
#define CUtZipNotInUpdate 22
#define CUtZipAlreadyInUpdate 23
#define CUtZipInvalidFileName 24
#define CUtZipFileAlreadyBeingChanged 25
#define CUtZipFileAlreadyBeingCopied 26
#define CUtZipDirectoryRenameFailed 27
#define CUtZipDirectoryDeleteFailed 28
#define CUtZipDirectoryCopyFailed 29
#define CUtZipDataFileReadError 30
#define CUtZipDataFileTooLarge 31
#define CUtZipInvalidDirectoryName 32

//
// Open Zip Modes
//

enum EZipOpenMode
{
    eZomCreateNew, // fails if file already exists
    eZomCreateAlways, // create and overwrite existing file
    eZomOpen, // open existing file and if file not found fail
    eZomOpenAlways // open existing file and create if it does not exist
};

//
// Compression Methods
//

enum EZipCompressionMethod
{
    eZcmStored,
    eZcmDeflated,
    eZcmUnsupportedCompressionMethod
};

//
// File Attributes
//

#define CZipFileAttributeHidden 0x0001
#define CZipFileAttributeReadOnly 0x0002
#define CZipFileAttributeSystem 0x0004
#define CZipFileAttributeArchive 0x0008

//
// Data on a File in a Zip File
//

#pragma pack(push, 1)

typedef struct tagTZipFileEntry
{
    EZipCompressionMethod ZipCompressionMethod;
    SYSTEMTIME FileModTime;
    UINT32 nFileAttributes;
    ULONG nCRC;
    ULONGLONG nCompressedSize;
    ULONGLONG nUncompressedSize;
    LPWSTR pszFileName;
    LPWSTR pszPath;
    LPWSTR pszSrcFileName;
    LPWSTR pszComment;
} TZipFileEntry;

#pragma pack(pop)

typedef std::vector<TZipFileEntry*> TZipFileEntryVec;

typedef LPVOID TZipHandle;

//
// Zip functions
//
// Note: All paths must use the / instead of the \ path separator character.
//       Any data files must not be deleted until the zip file has been written.
//

TZipHandle UtZipOpenFile(LPCWSTR pszFile, EZipOpenMode OpenMode, LPINT pnErrorCode);
BOOL UtZipClose(TZipHandle hZip, LPINT pnErrorCode);

BOOL UtZipIsValidArchive(TZipHandle hZip, LPBOOL pbIsValidArchive, LPINT pnErrorCode);

BOOL UtZipBeginUpdate(TZipHandle hZip, LPINT pnErrorCode);
BOOL UtZipEndUpdate(TZipHandle hZip, LPINT pnErrorCode);
BOOL UtZipGetUpdateFlag(TZipHandle hZip, LPBOOL pbUpdateFlag, LPINT pnErrorCode);

LPCWSTR UtZipGetZipComment(TZipHandle hZip, LPINT pnErrorCode);
BOOL UtZipSetZipComment(TZipHandle hZip, LPCWSTR pszComment, LPINT pnErrorCode);

BOOL UtZipChangeFileComment(TZipHandle hZip, LPCWSTR pszFile, LPCWSTR pszComment, LPINT pnErrorCode);
BOOL UtZipChangeFileName(TZipHandle hZip, LPCWSTR pszFile, LPCWSTR pszNewFile, LPINT pnErrorCode);
BOOL UtZipChangeFileData(TZipHandle hZip, LPCWSTR pszFile, LPCWSTR pszDataFile, EZipCompressionMethod CompressionMethod, LPINT pnErrorCode);

BOOL UtZipAllocFileEntries(TZipHandle hZip, TZipFileEntryVec& ZipFileEntryVec, LPINT pnErrorCode);
BOOL UtZipFreeFileEntries(TZipFileEntryVec& ZipFileEntryVec, LPINT pnErrorCode);

BOOL UtZipExtractFile(TZipHandle hZip, LPCWSTR pszFile, LPCWSTR pszOutputFile, LPINT pnErrorCode);

BOOL UtZipAddFile(TZipHandle hZip, LPCWSTR pszFile, LPCWSTR pszDataFile, LPCWSTR pszComment, EZipCompressionMethod CompressionMethod, LPINT pnErrorCode);

BOOL UtZipDeleteFile(TZipHandle hZip, LPCWSTR pszFile, LPINT pnErrorCode);

BOOL UtZipCopyFile(TZipHandle hZip, LPCWSTR pszSrcFile, LPCWSTR pszDestFile, LPINT pnErrorCode);

// Any existing files are automatically overwritten when using a directory function!

BOOL UtZipRenameDirectory(TZipHandle hZip, LPCWSTR pszSrcDirectory, LPCWSTR pszDestDirectory, LPINT pnErrorCode);

BOOL UtZipDeleteDirectory(TZipHandle hZip, LPCWSTR pszDirectory, LPINT pnErrorCode);

BOOL UtZipCopyDirectory(TZipHandle hZip, LPCWSTR pszSrcDirectory, LPCWSTR pszDestDirectory, LPINT pnErrorCode);

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
