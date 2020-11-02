/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

//
// Future enhancements:
//   - Lock data file until finished with it
//
//

#include "stdafx.h"

#include "UtZip.h"
#include "UtZipDefs.h"
#include "UtZipString.h"

#include <cassert>
#include <memory>

#include "zlib.h"

// Change File Record flags

#define CChangeFileName 0x0001
#define CChangeComment 0x0002
#define CChangeData 0x0004

enum EHeaderType
{
    ehtUnknownHeader,
    ehtLocalFileHeader, // 0x04034b50
    ehtCentralFileHeader, // 0x02014b50
    ehtEndCentralDirectoryRecord, // 0x06054b50
    ehtDataDescriptor, // 0x08074b50
    ehtArchiveExtraDataRecord, // 0x08064b50
    ehtDigitalSignature, // 0x05054b50
    ehtZip64EndCentralDirectoryRecord, // 0x06064b50
    ehtZip64EndCentralDirectoryLocator // 0x07064b50
};

enum EDataType
{
    edtBinary,
    edtText
};

typedef struct tagTFileRecord
{
    UINT16 nGeneralFlags;
    UINT16 nCompressionMethod;
    UINT16 nFileTime;
    UINT16 nFileDate;
    UINT32 nCRC;
    UINT32 nCompressedSize;
    UINT32 nUncompressedSize;
    BYTE byOperatingSystem;
    UINT32 nFileAttributes;
    UINT32 nLocalHeaderOffset;
    UINT32 nCentralDirectoryOffset;
    CUtZipString* pFileName;
    CUtZipString* pComment;
    EDataType DataType;
    UINT16 nExtraFieldLen;
} TFileRecord;

typedef std::vector<TFileRecord*> TFileRecordVec;

typedef struct tagTAddFileRecord
{
    UINT16 nCompressionMethod;
    UINT16 nFileTime;
    UINT16 nFileDate;
    UINT32 nFileAttributes;
    CUtZipString* pFileName;
    CUtZipString* pComment;
    EDataType DataType;
    LPWSTR pszDataFile;
} TAddFileRecord;

typedef std::vector<TAddFileRecord*> TAddFileRecordVec;

typedef struct tagTChangeFileRecord
{
    UINT32 nChangeFlags;
    UINT16 nCompressionMethod;
    CUtZipString* pFileName;
    CUtZipString* pComment;
    LPWSTR pszDataFile;
    TFileRecord* pFileRecord;
} TChangeFileRecord;

typedef std::vector<TChangeFileRecord*> TChangeFileRecordVec;

typedef struct tagTCopyFileRecord
{
    LPWSTR pszSrcFile;
    LPWSTR pszDestFile;
    TFileRecord* pFileRecord;
} TCopyFileRecord;

typedef std::vector<TCopyFileRecord*> TCopyFileRecordVec;

typedef struct tagTZip
{
    LPWSTR pszFile;
    HANDLE hFile;
    UINT16 nTotalCentralDirectoryEntries;
    UINT32 nStartCentralDirectoryOffset;
    LPWSTR pszZipComment;
    BOOL bInitFileRecordVec;
    TFileRecordVec* pFileRecordVec;
    TAddFileRecordVec* pAddFileRecordVec;
    TChangeFileRecordVec* pChangeFileRecordVec;
    TFileRecordVec* pDeleteFileRecordVec;
    TCopyFileRecordVec* pCopyFileRecordVec;
    BOOL bInUpdate;
} TZip;

typedef struct tagTInflateReadData
{
    HANDLE hFile;
    unsigned char* pBuffer;
    UINT16 nBufferLen;
} TInflateReadData;

typedef struct tagTInflateWriteData
{
    HANDLE hFile;
    uLong nCRC;
} TInflateWriteData;

typedef std::vector<LONGLONG> TLongLongVec;

typedef BOOL (*TUncompressFileFunc)(HANDLE hSrcFile, HANDLE hDestFile, const TFileRecord* pFileRecord, LPINT pnErrorCode);
typedef BOOL(*TCompressFileFunc)(HANDLE hSrcFile, HANDLE hDestFile, TFileRecord* pFileRecord, LPINT pnErrorCode);

#if defined(__cplusplus_cli)
#pragma unmanaged
#endif

static BOOL lDosDateTimeToSystemTime(
  UINT16 nDate,
  UINT16 nTime,
  LPSYSTEMTIME pSystemTime)
{
    FILETIME FileTime;

    if (::DosDateTimeToFileTime(nDate, nTime, &FileTime) &&
        ::FileTimeToSystemTime(&FileTime, pSystemTime))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL lGetFileDosDateTime(
  LPCWSTR pszFile,
  UINT16* pnDate,
  UINT16* pnTime)
{
    BOOL bResult(FALSE);
    HANDLE hFile;
    FILETIME CreationFileTime, LastAccessFileTime, LastWriteFileTime, LocalFileTime;

    *pnDate = 0;
    *pnTime = 0;

    hFile = ::CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (::GetFileTime(hFile, &CreationFileTime, &LastAccessFileTime,
                      &LastWriteFileTime) &&
        ::FileTimeToLocalFileTime(&CreationFileTime, &LocalFileTime) &&
        ::FileTimeToDosDateTime(&LocalFileTime, pnDate, pnTime))
    {
        bResult = TRUE;
    }

    ::CloseHandle(hFile);

    return bResult;
}

static UINT32 lTranslateZipFileAttributes(
  BYTE byOperatingSystem,
  UINT32 nFileAttributes)
{
    UINT32 nZipFileAttributes(0);

    switch (byOperatingSystem)
    {
        case CMSDOS:
            if (nFileAttributes & 0x0001)
            {
                nZipFileAttributes |= CZipFileAttributeReadOnly;
            }

            if (nFileAttributes & 0x0002)
            {
                nZipFileAttributes |= CZipFileAttributeHidden;
            }

            if (nFileAttributes & 0x0004)
            {
                nZipFileAttributes |= CZipFileAttributeSystem;
            }

            if (nFileAttributes & 0x0020)
            {
                nZipFileAttributes |= CZipFileAttributeArchive;
            }
            break;
    }

    return nZipFileAttributes;
}

static UINT32 lGetFileAttributes(
  LPCWSTR pszFile)
{
    DWORD dwAttributes;

    dwAttributes = ::GetFileAttributesW(pszFile);

    if (dwAttributes == INVALID_FILE_ATTRIBUTES)
    {
        return 0;
    }

    return dwAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN |
                              FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE);
}

static BOOL lIsNameValid(
  LPCWSTR pszName)
{
    INT nNameLen;

    if (pszName == NULL || *pszName == 0 || *pszName == L'/')
    {
        return FALSE;
    }

    // Check if have actual file name

    nNameLen = ::lstrlenW(pszName);

    if (*(pszName + (nNameLen - 1)) == L'/')
    {
        return FALSE;
    }

    // Check for wrong slash character

    for (LPCWSTR pszNamePos = pszName; *pszNamePos != 0; ++pszNamePos)
    {
        if (*pszNamePos == L'\\')
        {
            return FALSE;
        }
    }

    // Check for a drive letter

    if (::lstrlenW(pszName) >= 2)
    {
        if (*(pszName + 1) == ':' &&
                ((*pszName >= L'a' && *pszName <= L'z') ||
                 (*pszName >= L'A' && *pszName <= L'Z')))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL lIsDataFileLarge(
  LPCWSTR pszFile,
  LPBOOL pbLargeFile)
{
    HANDLE hFile;
    LARGE_INTEGER FileSize;

    *pbLargeFile = FALSE;

    hFile = ::CreateFileW(pszFile, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!::GetFileSizeEx(hFile, &FileSize))
    {
        ::CloseHandle(hFile);

        return FALSE;
    }
    
    ::CloseHandle(hFile);

    *pbLargeFile = (FileSize.QuadPart > UINT32_MAX) ? TRUE : FALSE;

    return TRUE;
}

static BOOL lReadFile(
  HANDLE hFile,
  LPBYTE byBuffer,
  UINT32 nBufferLen)
{
    DWORD dwBytesRead;

    if (::ReadFile(hFile, byBuffer, nBufferLen, &dwBytesRead, NULL) &&
        dwBytesRead == nBufferLen)
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL lWriteFile(
  HANDLE hFile,
  LPCBYTE byBuffer,
  UINT32 nBufferLen)
{
    DWORD dwBytesWrote;

    if (::WriteFile(hFile, byBuffer, nBufferLen, &dwBytesWrote, NULL) &&
        dwBytesWrote == nBufferLen)
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL lReadHeaderType(
  HANDLE hFile,
  EHeaderType* pHeaderType)
{
    UINT32 nSignature;

    *pHeaderType = ehtUnknownHeader;

    if (!lReadFile(hFile, (LPBYTE)&nSignature, sizeof(nSignature)))
    {
        return FALSE;
    }

    switch (nSignature)
    {
        case 0x04034b50:
            *pHeaderType = ehtLocalFileHeader;
            break;
        case 0x02014b50:
            *pHeaderType = ehtCentralFileHeader;
            break;
        case 0x06054b50:
            *pHeaderType = ehtEndCentralDirectoryRecord;
            break;
        case 0x08074b50:
            *pHeaderType = ehtDataDescriptor;
            break;
        case 0x08064b50:
            *pHeaderType = ehtArchiveExtraDataRecord;
            break;
        case 0x05054b50:
            *pHeaderType = ehtDigitalSignature;
            break;
        case 0x06064b50:
            *pHeaderType = ehtZip64EndCentralDirectoryRecord;
            break;
        case 0x07064b50:
            *pHeaderType = ehtZip64EndCentralDirectoryLocator;
            break;
    }

    return TRUE;
}

static BOOL lWriteHeaderType(
  HANDLE hFile,
  EHeaderType HeaderType)
{
    UINT32 nSignature;

    switch (HeaderType)
    {
        case ehtLocalFileHeader:
            nSignature = 0x04034b50;
            break;
        case ehtCentralFileHeader:
            nSignature = 0x02014b50;
            break;
        case ehtEndCentralDirectoryRecord:
            nSignature = 0x06054b50;
            break;
        case ehtDataDescriptor:
            nSignature = 0x08074b50;
            break;
        case ehtArchiveExtraDataRecord:
            nSignature = 0x08064b50;
            break;
        case ehtDigitalSignature:
            nSignature = 0x05054b50;
            break;
        case ehtZip64EndCentralDirectoryRecord:
            nSignature = 0x06064b50;
            break;
        case ehtZip64EndCentralDirectoryLocator:
            nSignature = 0x07064b50;
            break;
        default:
            assert(0);
            return FALSE;
    }

    return lWriteFile(hFile, (LPCBYTE)&nSignature, sizeof(nSignature));
}

static BOOL lGetFilePointer(
  HANDLE hFile,
  PLARGE_INTEGER pLargeInteger)
{
    LARGE_INTEGER SeekLargeInteger;

    SeekLargeInteger.QuadPart = 0;

    return ::SetFilePointerEx(hFile, SeekLargeInteger, pLargeInteger, FILE_CURRENT);
}

static BOOL lInitFileEntryFile(
  TZipFileEntry* pZipFileEntry,
  CUtZipString* pFile)
{
    LPCWSTR pszNewFilePos;

    if (pFile->UnicodeLength() == 0)
    {
        pZipFileEntry->pszFileName = new (std::nothrow) WCHAR[1];
        pZipFileEntry->pszPath = new (std::nothrow) WCHAR[1];
        pZipFileEntry->pszSrcFileName = new (std::nothrow) WCHAR[1];

        if (pZipFileEntry->pszFileName == NULL || pZipFileEntry->pszPath == NULL ||
            pZipFileEntry->pszSrcFileName == NULL)
        {
            delete[] pZipFileEntry->pszFileName;
            delete[] pZipFileEntry->pszPath;
            delete[] pZipFileEntry->pszSrcFileName;

            pZipFileEntry->pszFileName = NULL;
            pZipFileEntry->pszPath = NULL;
            pZipFileEntry->pszSrcFileName = NULL;

            return FALSE;
        }

        *pZipFileEntry->pszFileName = 0;
        *pZipFileEntry->pszPath = 0;
        *pZipFileEntry->pszSrcFileName = 0;

        return TRUE;
    }

    pZipFileEntry->pszSrcFileName = new (std::nothrow) WCHAR[pFile->UnicodeLength() + 1];

    if (pZipFileEntry->pszSrcFileName == NULL)
    {
        return FALSE;
    }

    ::StringCchCopyW(pZipFileEntry->pszSrcFileName, pFile->UnicodeLength() + 1,
                     pFile->AsUnicode());

    pszNewFilePos = pFile->AsUnicode() + pFile->UnicodeLength();

    if (*pszNewFilePos != L'/')
    {
        // Have file name (and potential path)

        while (pszNewFilePos >= pFile->AsUnicode() && *pszNewFilePos != L'/')
        {
            --pszNewFilePos;
        }

        pZipFileEntry->pszFileName = new (std::nothrow) WCHAR[pFile->UnicodeLength() + 1];
        pZipFileEntry->pszPath = new (std::nothrow) WCHAR[pFile->UnicodeLength() + 1];

        if (pZipFileEntry->pszFileName == NULL || pZipFileEntry->pszPath == NULL)
        {
            delete[] pZipFileEntry->pszFileName;
            delete[] pZipFileEntry->pszPath;
            delete[] pZipFileEntry->pszSrcFileName;

            pZipFileEntry->pszFileName = NULL;
            pZipFileEntry->pszPath = NULL;
            pZipFileEntry->pszSrcFileName = NULL;

            return FALSE;
        }

        if (pszNewFilePos < pFile->AsUnicode())
        {
            // Have no path

            ::StringCchCopyW(pZipFileEntry->pszFileName, pFile->UnicodeLength() + 1,
                             pFile->AsUnicode());

            *pZipFileEntry->pszPath = 0;
        }
        else
        {
            // Have path

            ::StringCchCopyNW(pZipFileEntry->pszFileName, pFile->UnicodeLength() + 1,
                              pszNewFilePos + 1,
                              (pFile->UnicodeLength() + 1) - ((pszNewFilePos - pFile->AsUnicode()) + 1));

            ::StringCchCopyNW(pZipFileEntry->pszPath, pFile->UnicodeLength() + 1,
                              pFile->AsUnicode(), pszNewFilePos - pFile->AsUnicode());
        }
    }
    else
    {
        // Have directory name only

        pZipFileEntry->pszFileName = new (std::nothrow) WCHAR[1];

        if (pZipFileEntry->pszFileName == NULL)
        { 
            delete[] pZipFileEntry->pszSrcFileName;

            pZipFileEntry->pszSrcFileName = NULL;

            return FALSE;
        }

        *pZipFileEntry->pszFileName = 0;

        pZipFileEntry->pszPath = new (std::nothrow) WCHAR[pFile->UnicodeLength() + 1];

        if (pZipFileEntry->pszPath == NULL)
        {
            delete[] pZipFileEntry->pszSrcFileName;
            delete[] pZipFileEntry->pszFileName;

            pZipFileEntry->pszSrcFileName = NULL;
            pZipFileEntry->pszFileName = NULL;

            return FALSE;
        }

        ::StringCchCopyNW(pZipFileEntry->pszPath, pFile->UnicodeLength() + 1,
                          pFile->AsUnicode(), pFile->UnicodeLength());
    }

    return TRUE;
}

static BOOL lReadEndCentralDirectoryRecord(
  HANDLE hFile,
  TEndCentralDirectoryRecord* pEndCentralDirectoryRecord,
  LPWSTR* ppszZipComment,
  LPINT pnErrorCode)
{
    LARGE_INTEGER FileSize, FilePos;
    EHeaderType HeaderType;
    LPSTR pszTmpZipComment;
    INT nNewZipCommentLen;

    *pnErrorCode = CUtZipNoError;

    if (!::GetFileSizeEx(hFile, &FileSize))
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    if (FileSize.QuadPart < sizeof(UINT32) + sizeof(TEndCentralDirectoryRecord))
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    FilePos.QuadPart = FileSize.QuadPart - (sizeof(UINT32) + sizeof(TEndCentralDirectoryRecord));

    while (FilePos.QuadPart >= 0)
    {
        if (!::SetFilePointerEx(hFile, FilePos, NULL, FILE_BEGIN) ||
            !lReadHeaderType(hFile, &HeaderType))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        if (HeaderType == ehtEndCentralDirectoryRecord)
        {
            if (!lReadFile(hFile, (LPBYTE)pEndCentralDirectoryRecord, sizeof(TEndCentralDirectoryRecord)))
            {
                *pnErrorCode = CUtZipFileError;

                return FALSE;
            }

            pszTmpZipComment = new (std::nothrow) CHAR[pEndCentralDirectoryRecord->nZipFileCommentLen];

            if (pszTmpZipComment == NULL)
            {
                *pnErrorCode = CUtZipOutOfMemory;

                return FALSE;
            }

            if (!lReadFile(hFile, (LPBYTE)pszTmpZipComment, pEndCentralDirectoryRecord->nZipFileCommentLen))
            {
                delete[] pszTmpZipComment;

                *pnErrorCode = CUtZipFileError;

                return FALSE;
            }

            nNewZipCommentLen = ::MultiByteToWideChar(CMSDOSCodePage, 0, pszTmpZipComment,
                                                      pEndCentralDirectoryRecord->nZipFileCommentLen,
                                                      NULL, 0);

            *ppszZipComment = new (std::nothrow) WCHAR[nNewZipCommentLen + 1];

            if (*ppszZipComment == NULL)
            {
                delete[] pszTmpZipComment;

                *pnErrorCode = CUtZipOutOfMemory;

                return FALSE;
            }

            *(*ppszZipComment + pEndCentralDirectoryRecord->nZipFileCommentLen) = 0;

            ::MultiByteToWideChar(CMSDOSCodePage, 0, pszTmpZipComment,
                                  pEndCentralDirectoryRecord->nZipFileCommentLen,
                                  *ppszZipComment, nNewZipCommentLen + 1);

            delete[] pszTmpZipComment;

            return TRUE;
        }

        --FilePos.QuadPart;
    }

    *pnErrorCode = CUtZipInvalidZipFile;

    return FALSE;
}

static VOID lEmptyFileRecordVec(
  TFileRecordVec* pFileRecordVec)
{
    try
    {
        for (TFileRecordVec::iterator it = pFileRecordVec->begin();
            it != pFileRecordVec->end(); ++it)
        {
            delete (*it)->pFileName;
            delete (*it)->pComment;

            delete *it;
        }

        pFileRecordVec->clear();
    }

    catch (std::exception&)
    {
    }
}

static VOID lEmptyAddFileRecordVec(
  TAddFileRecordVec* pAddFileRecordVec)
{
    try
    {
        for (TAddFileRecordVec::iterator it = pAddFileRecordVec->begin();
             it != pAddFileRecordVec->end(); ++it)
        {
            delete (*it)->pFileName;
            delete (*it)->pComment;
            delete[] (*it)->pszDataFile;

            delete *it;
        }

        pAddFileRecordVec->clear();
    }

    catch (std::exception&)
    {
    }
}

static VOID lEmptyChangeFileRecordVec(
  TChangeFileRecordVec* pChangeFileRecordVec)
{
    try
    {
        for (TChangeFileRecordVec::iterator it = pChangeFileRecordVec->begin();
            it != pChangeFileRecordVec->end(); ++it)
        {
            delete (*it)->pFileName;
            delete (*it)->pComment;
            delete[] (*it)->pszDataFile;

            delete *it;
        }

        pChangeFileRecordVec->clear();
    }

    catch (std::exception&)
    {
    }
}

static VOID lEmptyCopyFileRecordVec(
  TCopyFileRecordVec* pCopyFileRecordVec)
{
    try
    {
        for (TCopyFileRecordVec::iterator it = pCopyFileRecordVec->begin();
             it != pCopyFileRecordVec->end(); ++it)
        {
            delete[] (*it)->pszSrcFile;
            delete[] (*it)->pszDestFile;

            delete *it;
        }

        pCopyFileRecordVec->clear();
    }

    catch (std::exception&)
    {
    }
}

static TChangeFileRecord* lCloneChangeFileRecord(
  const TChangeFileRecord* pChangeFileRecord)
{
    TChangeFileRecord* pNewChangeFileRecord = new (std::nothrow) TChangeFileRecord;
    INT nStringLen;

    if (pNewChangeFileRecord == NULL)
    {
        return NULL;
    }

    pNewChangeFileRecord->nChangeFlags = pChangeFileRecord->nChangeFlags;
    pNewChangeFileRecord->nCompressionMethod = pChangeFileRecord->nCompressionMethod;
    pNewChangeFileRecord->pFileRecord = pChangeFileRecord->pFileRecord;
    pNewChangeFileRecord->pFileName = NULL;
    pNewChangeFileRecord->pComment = NULL;
    pNewChangeFileRecord->pszDataFile = NULL;

    // Copy the file name

    if (pChangeFileRecord->pFileName)
    {
        pNewChangeFileRecord->pFileName = new (std::nothrow) CUtZipString(*pChangeFileRecord->pFileName);

        if (pNewChangeFileRecord->pFileName == NULL ||
            !pNewChangeFileRecord->pFileName->IsValid())
        {
            delete pNewChangeFileRecord->pFileName;
            delete pNewChangeFileRecord;

            return NULL;
        }
    }

    // Copy the comment

    if (pChangeFileRecord->pComment)
    {
        pNewChangeFileRecord->pComment = new (std::nothrow) CUtZipString(*pChangeFileRecord->pComment);

        if (pNewChangeFileRecord->pComment == NULL ||
            !pNewChangeFileRecord->pComment->IsValid())
        {
            delete pNewChangeFileRecord->pFileName;
            delete pNewChangeFileRecord->pComment;
            delete pNewChangeFileRecord;

            return NULL;
        }
    }

    // Copy the data file

    if (pChangeFileRecord->pszDataFile)
    {
        nStringLen = ::lstrlenW(pChangeFileRecord->pszDataFile) + 1;

        pNewChangeFileRecord->pszDataFile = new (std::nothrow) WCHAR[nStringLen];

        if (pNewChangeFileRecord->pszDataFile == NULL)
        {
            delete pNewChangeFileRecord->pFileName;
            delete pNewChangeFileRecord->pComment;
            delete pNewChangeFileRecord;

            return NULL;
        }

        ::StringCchCopyW(pNewChangeFileRecord->pszDataFile, nStringLen,
                         pChangeFileRecord->pszDataFile);
    }

    return pNewChangeFileRecord;
}

static TChangeFileRecordVec* lCloneChangeFileRecordVec(
  const TChangeFileRecordVec* pChangeFileRecordVec)
{
    TChangeFileRecordVec* pNewChangeFileRecordVec = new (std::nothrow) TChangeFileRecordVec;
    TChangeFileRecord* pNewChangeFileRecord;

    if (pNewChangeFileRecordVec == NULL)
    {
        return NULL;
    }

    for (TChangeFileRecordVec::const_iterator it = pChangeFileRecordVec->begin();
         it != pChangeFileRecordVec->end(); ++it)
    {
        pNewChangeFileRecord = lCloneChangeFileRecord(*it);

        if (pNewChangeFileRecord == NULL)
        {
            lEmptyChangeFileRecordVec(pNewChangeFileRecordVec);

            delete pNewChangeFileRecordVec;

            return NULL;
        }
    }

    return pNewChangeFileRecordVec;
}

static TFileRecordVec* lCloneDeleteFileRecordVec(
  const TFileRecordVec* pFileRecordVec)
{
    TFileRecordVec* pNewFileRecordVec = new (std::nothrow) TFileRecordVec;

    if (pNewFileRecordVec == NULL)
    {
        return NULL;
    }

    for (TFileRecordVec::const_iterator it = pFileRecordVec->begin();
         it != pFileRecordVec->end(); ++it)
    {
        pNewFileRecordVec->push_back(*it);
    }
        
    return pNewFileRecordVec;
}

static TCopyFileRecord* lCloneCopyFileRecord(
  const TCopyFileRecord* pCopyFileRecord)
{
    TCopyFileRecord* pNewCopyFileRecord = new (std::nothrow) TCopyFileRecord;
    INT nStringLen;

    if (pNewCopyFileRecord == NULL)
    {
        return NULL;
    }

    pNewCopyFileRecord->pFileRecord = pCopyFileRecord->pFileRecord;

    // Copy the source file

    nStringLen = ::lstrlenW(pCopyFileRecord->pszSrcFile) + 1;

    pNewCopyFileRecord->pszSrcFile = new (std::nothrow) WCHAR[nStringLen];

    if (pNewCopyFileRecord->pszSrcFile == NULL)
    {
        delete pNewCopyFileRecord;

        return NULL;
    }

    ::StringCchCopyW(pNewCopyFileRecord->pszSrcFile, nStringLen,
                     pCopyFileRecord->pszSrcFile);

    // Copy the destination file

    nStringLen = ::lstrlenW(pCopyFileRecord->pszDestFile) + 1;

    pNewCopyFileRecord->pszDestFile = new (std::nothrow) WCHAR[nStringLen];

    if (pNewCopyFileRecord->pszDestFile == NULL)
    {
        delete[] pNewCopyFileRecord->pszSrcFile;
        delete pNewCopyFileRecord;

        return NULL;
    }

    ::StringCchCopyW(pNewCopyFileRecord->pszDestFile, nStringLen,
                     pCopyFileRecord->pszDestFile);

    return pNewCopyFileRecord;
}

static TCopyFileRecordVec* lCloneCopyFileRecordVec(
  const TCopyFileRecordVec* pCopyFileRecordVec)
{
    TCopyFileRecordVec* pNewCopyFileRecordVec = new (std::nothrow) TCopyFileRecordVec;
    TCopyFileRecord* pNewCopyFileRecord;

    if (pNewCopyFileRecordVec == NULL)
    {
        return NULL;
    }

    for (TCopyFileRecordVec::const_iterator it = pCopyFileRecordVec->begin();
         it != pCopyFileRecordVec->end(); ++it)
    {
        pNewCopyFileRecord = lCloneCopyFileRecord(*it);

        if (pNewCopyFileRecord == NULL)
        {
            lEmptyCopyFileRecordVec(pNewCopyFileRecordVec);

            delete pNewCopyFileRecordVec;

            return NULL;
        }

        pNewCopyFileRecordVec->push_back(pNewCopyFileRecord);
    }

    return pNewCopyFileRecordVec;
}

static TZip* lAllocZip(VOID)
{
    TZip* pZip = new (std::nothrow) TZip;

    if (pZip == NULL)
    {
        return NULL;
    }

    pZip->pszFile = NULL;
    pZip->hFile = NULL;
    pZip->nTotalCentralDirectoryEntries = 0;
    pZip->nStartCentralDirectoryOffset = 0;
    pZip->pszZipComment = NULL;
    pZip->bInitFileRecordVec = TRUE;
    pZip->pFileRecordVec = new (std::nothrow) TFileRecordVec;
    pZip->pAddFileRecordVec = new (std::nothrow) TAddFileRecordVec;
    pZip->pChangeFileRecordVec = new (std::nothrow) TChangeFileRecordVec;
    pZip->pDeleteFileRecordVec = new (std::nothrow) TFileRecordVec;
    pZip->pCopyFileRecordVec = new (std::nothrow) TCopyFileRecordVec;
    pZip->bInUpdate = FALSE;

    if (pZip->pFileRecordVec == NULL || pZip->pAddFileRecordVec == NULL ||
        pZip->pChangeFileRecordVec == NULL || pZip->pDeleteFileRecordVec == NULL ||
        pZip->pCopyFileRecordVec == NULL)
    {
        delete pZip->pFileRecordVec;
        delete pZip->pAddFileRecordVec;
        delete pZip->pChangeFileRecordVec;
        delete pZip->pDeleteFileRecordVec;
        delete pZip->pCopyFileRecordVec;
        delete pZip;

        pZip = NULL;
    }

    return pZip;
}

static VOID lFreeZip(
  TZip* pZip)
{
    if (pZip->hFile)
    {
        ::CloseHandle(pZip->hFile);
    }

    if (pZip->pFileRecordVec)
    {
        lEmptyFileRecordVec(pZip->pFileRecordVec);

        delete pZip->pFileRecordVec;
    }

    if (pZip->pAddFileRecordVec)
    {
        lEmptyAddFileRecordVec(pZip->pAddFileRecordVec);

        delete pZip->pAddFileRecordVec;
    }

    if (pZip->pChangeFileRecordVec)
    {
        lEmptyChangeFileRecordVec(pZip->pChangeFileRecordVec);

        delete pZip->pChangeFileRecordVec;
    }

    if (pZip->pCopyFileRecordVec)
    {
        lEmptyCopyFileRecordVec(pZip->pCopyFileRecordVec);

        delete pZip->pCopyFileRecordVec;
    }

    if (pZip->pDeleteFileRecordVec)
    {
        pZip->pDeleteFileRecordVec->clear();

        delete pZip->pDeleteFileRecordVec;
    }

    delete[] pZip->pszFile;
    delete[] pZip->pszZipComment;
    delete pZip;
}

static BOOL lInitZip(
  TZip* pZip,
  LPINT pnErrorCode)
{
    TCentralDirectoryRecord CentralDirectoryRecord;
    TFileRecord* pFileRecord;
    LPSTR pszFileName, pszComment;
    EHeaderType HeaderType;
    LARGE_INTEGER LargeInteger;
    UINT32 nCentralDirectoryOffset;

    *pnErrorCode = CUtZipNoError;

    if (pZip->bInitFileRecordVec == TRUE)
    {
        pZip->bInitFileRecordVec = FALSE;

        LargeInteger.QuadPart = pZip->nStartCentralDirectoryOffset;

        if (!::SetFilePointerEx(pZip->hFile, LargeInteger, NULL, FILE_BEGIN))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        for (UINT16 nDirectoryIndex = 0;
             nDirectoryIndex < pZip->nTotalCentralDirectoryEntries;
             ++nDirectoryIndex)
        {
            if (!lGetFilePointer(pZip->hFile, &LargeInteger) ||
                !lReadHeaderType(pZip->hFile, &HeaderType))
            {
                *pnErrorCode = CUtZipFileError;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                return FALSE;
            }

            nCentralDirectoryOffset = (UINT32)LargeInteger.QuadPart;

            if (HeaderType != ehtCentralFileHeader)
            {
                *pnErrorCode = CUtZipInvalidZipFile;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                return FALSE;
            }

            if (!lReadFile(pZip->hFile, (LPBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)))
            {
                *pnErrorCode = CUtZipFileError;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                return FALSE;
            }

            if (CentralDirectoryRecord.nGeneralFlags & 0x0001)
            {
                *pnErrorCode = CUtZipEncryptedFilesNotSupported;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                return FALSE;
            }

            if (CentralDirectoryRecord.nGeneralFlags & 0x0008)
            {
                // File contains file data descriptor

                *pnErrorCode = CUtZipInternalError;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                return FALSE;
            }

            switch (CentralDirectoryRecord.nVersionMadeBy >> 8)
            {
                case CMSDOS:
					break;
				// Disable version made by check for the time being since some zip files have been
				// found to contain other values
                /*default:
                    *pnErrorCode = CUtZipOperatingSystemNotSupported;

                    lEmptyFileRecordVec(pZip->pFileRecordVec);

                    return FALSE;*/
            }

            pszFileName = new (std::nothrow) CHAR[CentralDirectoryRecord.nFileNameLen];

            if (pszFileName == NULL)
            {
                *pnErrorCode = CUtZipOutOfMemory;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                return FALSE;
            }

            if (!lReadFile(pZip->hFile, (LPBYTE)pszFileName, CentralDirectoryRecord.nFileNameLen) ||
                INVALID_SET_FILE_POINTER == ::SetFilePointer(pZip->hFile, CentralDirectoryRecord.nExtraFieldLen,
                                                             NULL, FILE_CURRENT))
            {
                *pnErrorCode = CUtZipFileError;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                delete[] pszFileName;

                return FALSE;
            }

            pszComment = new (std::nothrow) CHAR[CentralDirectoryRecord.nFileCommentLen];

            if (pszComment == NULL)
            {
                *pnErrorCode = CUtZipOutOfMemory;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                delete[] pszFileName;

                return FALSE;
            }

            if (!lReadFile(pZip->hFile, (LPBYTE)pszComment, CentralDirectoryRecord.nFileCommentLen))
            {
                *pnErrorCode = CUtZipFileError;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                delete[] pszComment;
                delete[] pszFileName;

                return FALSE;
            }

            pFileRecord = new (std::nothrow) TFileRecord;

            if (pFileRecord == NULL)
            {
                *pnErrorCode = CUtZipOutOfMemory;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                delete[] pszFileName;
                delete[] pszComment;

                return FALSE;
            }

            pFileRecord->nGeneralFlags = CentralDirectoryRecord.nGeneralFlags;
            pFileRecord->nCompressionMethod = CentralDirectoryRecord.nCompressionMethod;
            pFileRecord->nFileTime = CentralDirectoryRecord.nFileTime;
            pFileRecord->nFileDate = CentralDirectoryRecord.nFileDate;
            pFileRecord->nCRC = CentralDirectoryRecord.nCRC;
            pFileRecord->nCompressedSize = CentralDirectoryRecord.nCompressedSize;
            pFileRecord->nUncompressedSize = CentralDirectoryRecord.nUncompressedSize;
            pFileRecord->byOperatingSystem = (BYTE)(CentralDirectoryRecord.nVersionMadeBy >> 8);
            pFileRecord->nFileAttributes = CentralDirectoryRecord.nExternalFileAttributes;
            pFileRecord->nLocalHeaderOffset = CentralDirectoryRecord.nLocalHeaderOffset;
            pFileRecord->nCentralDirectoryOffset = nCentralDirectoryOffset;
            pFileRecord->pFileName = NULL;
            pFileRecord->pComment = NULL;
            pFileRecord->DataType = (CentralDirectoryRecord.nInternalFileAttributes & 0x0001) ? edtText : edtBinary;
            pFileRecord->nExtraFieldLen = CentralDirectoryRecord.nExtraFieldLen;

            pFileRecord->pFileName = new (std::nothrow) CUtZipString(pszFileName,
                                         CentralDirectoryRecord.nFileNameLen,
                                         (CentralDirectoryRecord.nGeneralFlags & CUTF8LanguageEncodingFlag) ? CP_UTF8 : CMSDOSCodePage);

            pFileRecord->pComment = new (std::nothrow) CUtZipString(pszComment,
                                        CentralDirectoryRecord.nFileCommentLen,
                                        (CentralDirectoryRecord.nGeneralFlags & CUTF8LanguageEncodingFlag) ? CP_UTF8 : CMSDOSCodePage);

            if ((CentralDirectoryRecord.nGeneralFlags & CUTF8LanguageEncodingFlag) == 0)
            {
                pFileRecord->nGeneralFlags |= CUTF8LanguageEncodingFlag;
            }

            delete[] pszFileName;
            delete[] pszComment;

            if (!pFileRecord->pFileName->IsValid() || !pFileRecord->pComment->IsValid())
            {
                *pnErrorCode = CUtZipOutOfMemory;

                lEmptyFileRecordVec(pZip->pFileRecordVec);

                delete pFileRecord->pFileName;
                delete pFileRecord->pComment;
                delete pFileRecord;

                return FALSE;
            }

            pZip->pFileRecordVec->push_back(pFileRecord);
        }
    }

    return TRUE;
}

static TFileRecord* lFindFileRecord(
  LPCWSTR pszFile,
  TFileRecordVec* pFileRecordVec,
  LPINT pnErrorCode)
{
    *pnErrorCode = CUtZipNoError;

    for (TFileRecordVec::iterator it = pFileRecordVec->begin();
         it != pFileRecordVec->end(); ++it)
    {
        if (*(*it)->pFileName == pszFile)
        {
            return *it;
        }
    }

    *pnErrorCode = CUtZipFileNotFound;

    return NULL;
}

static TAddFileRecord* lFindAddFileRecord(
  LPCWSTR pszFile,
  TAddFileRecordVec* pAddFileRecordVec,
  LPINT pnErrorCode)
{
    *pnErrorCode = CUtZipNoError;

    for (TAddFileRecordVec::iterator it = pAddFileRecordVec->begin();
         it != pAddFileRecordVec->end(); ++it)
    {
        if (*(*it)->pFileName == pszFile)
        {
            return *it;
        }
    }

    *pnErrorCode = CUtZipFileNotFound;

    return NULL;
}

static TChangeFileRecord* lFindChangeFileRecord(
  LPCWSTR pszFile,
  TChangeFileRecordVec* pChangeFileRecordVec,
  LPINT pnErrorCode)
{
    *pnErrorCode = CUtZipNoError;

    for (TChangeFileRecordVec::iterator it = pChangeFileRecordVec->begin();
         it != pChangeFileRecordVec->end(); ++it)
    {
        if (*(*it)->pFileName == pszFile)
        {
            return *it;
        }
    }

    *pnErrorCode = CUtZipFileNotFound;

    return NULL;
}

static TChangeFileRecord* lFindChangeFileRecord(
  TFileRecord* pFileRecord,
  TChangeFileRecordVec* pChangeFileRecordVec)
{
    for (TChangeFileRecordVec::iterator it = pChangeFileRecordVec->begin();
         it != pChangeFileRecordVec->end(); ++it)
    {
        if (pFileRecord == (*it)->pFileRecord)
        {
            return *it;
        }
    }

    return NULL;
}

static BOOL lDoesFileRecordExist(
  TFileRecord* pFileRecord,
  const TFileRecordVec* pFileRecordVec)
{
    for (TFileRecordVec::const_iterator it = pFileRecordVec->begin();
         it != pFileRecordVec->end(); ++it)
    {
        if (*it == pFileRecord)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL lDoesCopyFileRecordExist(
  LPCWSTR pszFile,
  const TCopyFileRecordVec* pCopyFileRecordVec)
{
    for (TCopyFileRecordVec::const_iterator it = pCopyFileRecordVec->begin();
         it != pCopyFileRecordVec->end(); ++it)
    {
        if (0 == ::lstrcmpiW(pszFile, (*it)->pszSrcFile) ||
            0 == ::lstrcmpiW(pszFile, (*it)->pszDestFile))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL lDoesCopyFileRecordExist(
  TFileRecord* pFileRecord,
  const TCopyFileRecordVec* pCopyFileRecordVec)
{
    for (TCopyFileRecordVec::const_iterator it = pCopyFileRecordVec->begin();
         it != pCopyFileRecordVec->end(); ++it)
    {
        if ((*it)->pFileRecord == pFileRecord)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL lFindFileRecordsMatchingPath(
  LPCWSTR pszPath,
  const TFileRecordVec* pFileRecordVec,
  TFileRecordVec* pMatchingFileRecordVec,
  LPINT pnErrorCode)
{
    INT nPathLen;

    *pnErrorCode = CUtZipNoError;

    nPathLen = ::lstrlenW(pszPath);

    for (TFileRecordVec::const_iterator it = pFileRecordVec->begin();
         it != pFileRecordVec->end(); ++it)
    {
        if (nPathLen < (*it)->pFileName->UnicodeLength())
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                               pszPath, nPathLen,
                                               (*it)->pFileName->AsUnicode(), nPathLen) &&
                *((*it)->pFileName->AsUnicode() + nPathLen) == L'/')
            {
                pMatchingFileRecordVec->push_back(*it);
            }
        }
    }

    return TRUE;
}

static BOOL lAddChangeFileRecordFileName(
  TChangeFileRecordVec* pChangeFileRecordVec,
  LPCWSTR pszNewName,
  TFileRecord* pFileRecord,
  LPINT pnErrorCode)
{
    TChangeFileRecord* pChangeFileRecord;

    *pnErrorCode = CUtZipNoError;

    pChangeFileRecord = lFindChangeFileRecord(pFileRecord, pChangeFileRecordVec);

    if (pChangeFileRecord == NULL)
    {
        pChangeFileRecord = new (std::nothrow) TChangeFileRecord;

        if (pChangeFileRecord == NULL)
        {
            *pnErrorCode = CUtZipOutOfMemory;

            return FALSE;
        }

        pChangeFileRecord->nChangeFlags = 0;
        pChangeFileRecord->pszDataFile = NULL;
        pChangeFileRecord->pComment = NULL;
        pChangeFileRecord->pFileRecord = pFileRecord;

        pChangeFileRecordVec->push_back(pChangeFileRecord);
    }
    else
    {
        delete pChangeFileRecord->pFileName;
    }

    pChangeFileRecord->pFileName = new (std::nothrow) CUtZipString(pszNewName);

    if (pChangeFileRecord->pFileName == NULL ||
        !pChangeFileRecord->pFileName->IsValid())
    {
        delete pChangeFileRecord->pFileName;

        pChangeFileRecord->pFileName = NULL;

        pChangeFileRecord->nChangeFlags &= ~CChangeFileName;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    pChangeFileRecord->nChangeFlags |= CChangeFileName;

    return TRUE;
}

static BOOL lAddCopyFileRecordFileName(
  TCopyFileRecordVec* pCopyFileRecordVec,
  LPCWSTR pszSrcFile,
  LPCWSTR pszDestFile,
  TFileRecord* pFileRecord,
  LPINT pnErrorCode)
{
    TCopyFileRecord* pCopyFileRecord;
    INT nSrcFileLen, nDestFileLen;

    *pnErrorCode = CUtZipNoError;

    nSrcFileLen = ::lstrlenW(pszSrcFile) + 1;
    nDestFileLen = ::lstrlenW(pszDestFile) + 1;

    pCopyFileRecord = new (std::nothrow) TCopyFileRecord;

    if (pCopyFileRecord == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    pCopyFileRecord->pszSrcFile = new (std::nothrow) WCHAR[nSrcFileLen];
    pCopyFileRecord->pszDestFile = new (std::nothrow) WCHAR[nDestFileLen];

    if (pCopyFileRecord->pszSrcFile == NULL || pCopyFileRecord->pszDestFile == NULL)
    {
        delete[] pCopyFileRecord->pszSrcFile;
        delete[] pCopyFileRecord->pszDestFile;
        delete pCopyFileRecord;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    ::StringCchCopyW(pCopyFileRecord->pszSrcFile, nSrcFileLen, pszSrcFile);
    ::StringCchCopyW(pCopyFileRecord->pszDestFile, nDestFileLen, pszDestFile);

    pCopyFileRecord->pFileRecord = pFileRecord;

    pCopyFileRecordVec->push_back(pCopyFileRecord);

    return TRUE;
}

static LPWSTR lAllocRenamedFileName(
  LPCWSTR pszFileName,
  LPCWSTR pszSrcDirectory,
  LPCWSTR pszDestDirectory,
  LPINT pnErrorCode)
{
    LPWSTR pszNewFileName;
    INT nNewFileNameLen;

    *pnErrorCode = CUtZipNoError;

    nNewFileNameLen = (::lstrlenW(pszFileName) - ::lstrlenW(pszSrcDirectory)) +
                          ::lstrlenW(pszDestDirectory) + 1;

    pszNewFileName = new (std::nothrow) WCHAR[nNewFileNameLen];

    if (pszNewFileName == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return NULL;
    }

    ::StringCchCopyW(pszNewFileName, nNewFileNameLen, pszDestDirectory);
    ::StringCchCatW(pszNewFileName, nNewFileNameLen, pszFileName + ::lstrlenW(pszSrcDirectory));

    return pszNewFileName;
}

static BOOL lSeekCompressedDataStart(
  HANDLE hFile,
  TFileRecord* pFileRecord,
  LPINT pnErrorCode)
{
    EHeaderType HeaderType;
    TLocalFileRecord LocalFileRecord;

    *pnErrorCode = CUtZipNoError;

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(hFile, pFileRecord->nLocalHeaderOffset,
                                                     NULL, FILE_BEGIN) ||
        !lReadHeaderType(hFile, &HeaderType))
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    if (HeaderType != ehtLocalFileHeader)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lReadFile(hFile, (LPBYTE)&LocalFileRecord, sizeof(LocalFileRecord)))
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(hFile, LocalFileRecord.nFileNameLen,
                                                     NULL, FILE_CURRENT) ||
        INVALID_SET_FILE_POINTER == ::SetFilePointer(hFile, LocalFileRecord.nExtraFieldLen,
                                                     NULL, FILE_CURRENT))
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    return TRUE;
}

static BOOL lCopyFileData(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  UINT32 nTotalBytes)
{
    LPBYTE pbyBuffer = new (std::nothrow) BYTE[UINT16_MAX];
    UINT32 nBytesLeft = nTotalBytes;
    UINT32 nBytesRead;

    if (pbyBuffer == NULL)
    {
        return FALSE;
    }

    while (nBytesLeft > 0)
    {
        if (nBytesLeft >= UINT16_MAX)
        {
            nBytesRead = UINT16_MAX;
        }
        else
        {
            nBytesRead = nBytesLeft;
        }

        if (!lReadFile(hSrcFile, pbyBuffer, nBytesRead) ||
            !lWriteFile(hDestFile, pbyBuffer, nBytesRead))
        {
            delete[] pbyBuffer;

            return FALSE;
        }

        nBytesLeft -= nBytesRead;
    }

    delete[] pbyBuffer;

    return TRUE;
}

static unsigned lReadDataForZlib(
  void* in_desc,
  z_const unsigned char** ppBuf)
{
    TInflateReadData* pInflateReadData = (TInflateReadData*)in_desc;
    DWORD dwActualBytesRead;

    *ppBuf = pInflateReadData->pBuffer;

    ::ReadFile(pInflateReadData->hFile, pInflateReadData->pBuffer,
               pInflateReadData->nBufferLen, &dwActualBytesRead, NULL);

    return dwActualBytesRead;
}

static int lWriteDataForZlib(
  void* out_desc,
  unsigned char* pBuf,
  unsigned nLen)
{
    TInflateWriteData* pInflateWriteData = (TInflateWriteData*)out_desc;
    DWORD dwBytesWrote;

    pInflateWriteData->nCRC = crc32(pInflateWriteData->nCRC, pBuf, nLen);

    if (::WriteFile(pInflateWriteData->hFile, pBuf, nLen, &dwBytesWrote, NULL))
    {
        return 0;
    }

    return -1;
}

static BOOL lUncompressStoredFile(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  const TFileRecord* pFileRecord,
  LPINT pnErrorCode)
{
    BYTE byBuffer[512];
    DWORD dwBytesRead, dwBytesWrite, dwActualBytesRead, dwActualBytesWrote;
    UINT32 nBytesLeft;
    uLong nCRC;

    *pnErrorCode = CUtZipNoError;

    if (pFileRecord->nCompressedSize != pFileRecord->nUncompressedSize)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    nCRC = crc32(0, Z_NULL, 0);
    nBytesLeft = pFileRecord->nCompressedSize;

    while (nBytesLeft > 0)
    {
        if (nBytesLeft >= sizeof(byBuffer))
        {
            dwBytesRead = sizeof(byBuffer);
        }
        else
        {
            dwBytesRead = nBytesLeft;
        }

        if (!::ReadFile(hSrcFile, byBuffer, dwBytesRead, &dwActualBytesRead, NULL) ||
            dwBytesRead != dwActualBytesRead)
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        dwBytesWrite = dwBytesRead;

        if (!::WriteFile(hDestFile, byBuffer, dwBytesWrite, &dwActualBytesWrote, NULL) ||
            dwBytesWrite != dwActualBytesWrote)
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        nCRC = crc32(nCRC, byBuffer, dwBytesRead);

        nBytesLeft -= dwBytesRead;
    }

    if (pFileRecord->nCRC != nCRC)
    {
        *pnErrorCode = CUtZipCRCMismatch;

        return FALSE;
    }

    return TRUE;
}

static BOOL lUncompressDeflatedFile(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  const TFileRecord* pFileRecord,
  LPINT pnErrorCode)
{
    BOOL bResult(TRUE);
    z_stream ZipStream;
    unsigned char* pWindow;
    TInflateReadData InflateReadData;
    TInflateWriteData InflateWriteData;

    *pnErrorCode = CUtZipNoError;

    pWindow = new (std::nothrow) unsigned char[UINT16_MAX];

    if (pWindow == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    InflateReadData.hFile = hSrcFile;
    InflateReadData.nBufferLen = UINT16_MAX;
    InflateReadData.pBuffer = new (std::nothrow) unsigned char[InflateReadData.nBufferLen * sizeof(unsigned char)];

    if (InflateReadData.pBuffer == NULL)
    {
        delete[] pWindow;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    InflateWriteData.hFile = hDestFile;
    InflateWriteData.nCRC = crc32(0, Z_NULL, 0);

    ZipStream.next_in = Z_NULL;
    ZipStream.avail_in = 0;
    ZipStream.next_out = Z_NULL;
    ZipStream.avail_out = 0;
    ZipStream.zalloc = Z_NULL;
    ZipStream.zfree = Z_NULL;
    ZipStream.opaque = NULL;
    ZipStream.data_type = (pFileRecord->DataType == edtBinary) ? Z_BINARY : Z_TEXT;

    if (Z_OK != inflateBackInit(&ZipStream, 15, pWindow))
    {
        delete[] InflateReadData.pBuffer;
        delete[] pWindow;

        *pnErrorCode = CUtZipInternalError;

        return FALSE;
    }

    switch (inflateBack(&ZipStream, lReadDataForZlib, &InflateReadData,
                        lWriteDataForZlib, &InflateWriteData))
    {
        case Z_OK:
        case Z_STREAM_END:
            if (pFileRecord->nCRC != InflateWriteData.nCRC)
            {
                *pnErrorCode = CUtZipCRCMismatch;

                bResult = FALSE;
            }
            break;
        case Z_NEED_DICT:
            *pnErrorCode = CUtZipDictionaryMissing;

            bResult = FALSE;
            break;
        case Z_DATA_ERROR:
            *pnErrorCode = CUtZipErrorInCompressedData;

            bResult = FALSE;
            break;
        case Z_MEM_ERROR:
            *pnErrorCode = CUtZipOutOfMemory;

            bResult = FALSE;
            break;
        case Z_BUF_ERROR:
            *pnErrorCode = CUtZipFileError;

            bResult = FALSE;
            break;
        case Z_ERRNO:
        case Z_STREAM_ERROR:
        case Z_VERSION_ERROR:
            *pnErrorCode = CUtZipInternalError;

            bResult = FALSE;
            break;
        default:
            assert(0);

            *pnErrorCode = CUtZipInternalError;

            bResult = FALSE;
            break;
    }

    inflateBackEnd(&ZipStream);

    delete[] InflateReadData.pBuffer;
    delete[] pWindow;

    return bResult;
}

static BOOL lCompressStoredFile(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  TFileRecord* pFileRecord,
  LPINT pnErrorCode)
{
    LARGE_INTEGER LargeInteger;
    BYTE byBuffer[512];
    DWORD dwBytesRead, dwBytesWrite, dwActualBytesRead, dwActualBytesWrote;
    UINT32 nBytesLeft;
    uLong nCRC;

    *pnErrorCode = CUtZipNoError;

    if (!::GetFileSizeEx(hSrcFile, &LargeInteger))
    { 
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    if (LargeInteger.QuadPart > UINT32_MAX)
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    nCRC = crc32(0, Z_NULL, 0);
    nBytesLeft = (UINT32)LargeInteger.QuadPart;

    while (nBytesLeft > 0)
    {
        if (nBytesLeft >= sizeof(byBuffer))
        {
            dwBytesRead = sizeof(byBuffer);
        }
        else
        {
            dwBytesRead = nBytesLeft;
        }

        if (!::ReadFile(hSrcFile, byBuffer, dwBytesRead, &dwActualBytesRead, NULL) ||
            dwBytesRead != dwActualBytesRead)
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        dwBytesWrite = dwBytesRead;

        if (!::WriteFile(hDestFile, byBuffer, dwBytesWrite, &dwActualBytesWrote, NULL) ||
            dwBytesWrite != dwActualBytesWrote)
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        nCRC = crc32(nCRC, byBuffer, dwBytesRead);

        nBytesLeft -= dwBytesRead;
    }

    pFileRecord->nCompressedSize = (UINT32)LargeInteger.QuadPart;
    pFileRecord->nUncompressedSize = (UINT32)LargeInteger.QuadPart;
    pFileRecord->nCRC = nCRC;

    return TRUE;
}

static BOOL lCompressDeflatedFile(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  TFileRecord* pFileRecord,
  LPINT pnErrorCode)
{
    LARGE_INTEGER LargeInteger;
    DWORD dwBytesRead, dwBytesWrite, dwActualBytesRead, dwActualBytesWrote;
    UINT32 nBytesLeft;
    uLong nCRC;
    z_stream ZipStream;
    Bytef* pBufferIn;
    Bytef* pBufferOut;
    uInt nBufferInLen, nBufferOutLen;
    int nFlush, nReturn;

    *pnErrorCode = CUtZipNoError;

    nBufferInLen = UINT16_MAX;
    nBufferOutLen = UINT16_MAX;

    if (!::GetFileSizeEx(hSrcFile, &LargeInteger))
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    if (LargeInteger.QuadPart > UINT32_MAX)
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    pFileRecord->nCompressedSize = 0;
    pFileRecord->nUncompressedSize = (UINT32)LargeInteger.QuadPart;

    nCRC = crc32(0, Z_NULL, 0);
    nBytesLeft = (UINT32)LargeInteger.QuadPart;

    pBufferIn = new (std::nothrow) Bytef[nBufferInLen];
    pBufferOut = new (std::nothrow) Bytef[nBufferOutLen];

    if (pBufferIn == NULL || pBufferOut == NULL)
    {
        delete[] pBufferIn;
        delete[] pBufferOut;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    ZipStream.zalloc = Z_NULL;
    ZipStream.zfree = Z_NULL;
    ZipStream.opaque = NULL;
    ZipStream.data_type = (pFileRecord->DataType == edtBinary) ? Z_BINARY : Z_TEXT;

    if (Z_OK != deflateInit2(&ZipStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY))
    {
        delete[] pBufferIn;
        delete[] pBufferOut;

        *pnErrorCode = CUtZipInternalError;

        return FALSE;
    }

    do
    {
        nFlush = Z_NO_FLUSH;

        if (nBytesLeft >= nBufferInLen)
        {
            dwBytesRead = nBufferInLen;
        }
        else
        {
            dwBytesRead = nBytesLeft;
        }

        nBytesLeft -= dwBytesRead;

        if (nBytesLeft == 0)
        {
            nFlush = Z_FINISH;
        }

        ZipStream.next_in = pBufferIn;
        ZipStream.avail_in = dwBytesRead;

        if (!::ReadFile(hSrcFile, pBufferIn, dwBytesRead, &dwActualBytesRead, NULL) ||
            dwBytesRead != dwActualBytesRead)
        {
            deflateEnd(&ZipStream);

            delete[] pBufferIn;
            delete[] pBufferOut;

            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        nCRC = crc32(nCRC, pBufferIn, dwBytesRead);

        do
        {
            ZipStream.next_out = pBufferOut;
            ZipStream.avail_out = nBufferOutLen;

            nReturn = deflate(&ZipStream, nFlush);

            if (nReturn == Z_OK ||
                nReturn == Z_STREAM_END)
            {
            }
            else
            {
                assert(nReturn != Z_STREAM_ERROR);

                deflateEnd(&ZipStream);

                delete[] pBufferIn;
                delete[] pBufferOut;

                *pnErrorCode = CUtZipFileCompressionError;

                return FALSE;
            }

            dwBytesWrite = nBufferOutLen - ZipStream.avail_out;

            pFileRecord->nCompressedSize += dwBytesWrite;

            if (!::WriteFile(hDestFile, pBufferOut, dwBytesWrite, &dwActualBytesWrote, NULL) ||
                dwBytesWrite != dwActualBytesWrote)
            {
                deflateEnd(&ZipStream);

                delete[] pBufferIn;
                delete[] pBufferOut;

                *pnErrorCode = CUtZipFileError;

                return FALSE;
            }
        } while (ZipStream.avail_out == 0);
    } while (nFlush != Z_FINISH);

    pFileRecord->nCRC = nCRC;

    deflateEnd(&ZipStream);

    delete[] pBufferIn;
    delete[] pBufferOut;

    return TRUE;
}

static BOOL lCopyUnchangedLocalFileRecord(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  const TFileRecord* pFileRecord)
{
    LPBYTE pbyBuffer = new (std::nothrow) BYTE[UINT16_MAX];
    TLocalFileRecord LocalFileRecord;
    EHeaderType HeaderType;
    LARGE_INTEGER LocalFileHeaderFilePos;

    if (pbyBuffer == NULL)
    {
        return FALSE;
    }

    if (!lReadHeaderType(hSrcFile, &HeaderType) ||
        !lReadFile(hSrcFile, (LPBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        INVALID_SET_FILE_POINTER == ::SetFilePointer(hSrcFile, LocalFileRecord.nFileNameLen,
                                                     NULL, FILE_CURRENT))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    if (!lWriteHeaderType(hDestFile, HeaderType) ||
        !lGetFilePointer(hDestFile, &LocalFileHeaderFilePos) ||
        !lWriteFile(hDestFile, (LPCBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !lWriteFile(hDestFile, (LPCBYTE)pFileRecord->pFileName->AsAnsi(),
                    LocalFileRecord.nFileNameLen) ||
        !lReadFile(hSrcFile, pbyBuffer, LocalFileRecord.nExtraFieldLen) ||
        !lWriteFile(hDestFile, pbyBuffer, LocalFileRecord.nExtraFieldLen))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    if (!lCopyFileData(hSrcFile, hDestFile, LocalFileRecord.nCompressedSize))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    delete[] pbyBuffer;

    return TRUE;
}

static BOOL lCopyChangedLocalFileRecord(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  const TChangeFileRecord* pChangeFileRecord)
{
    LPBYTE pbyBuffer = new (std::nothrow) BYTE[UINT16_MAX];
    TLocalFileRecord LocalFileRecord;
    TFileRecord FileRecord;
    EHeaderType HeaderType;
    LARGE_INTEGER DestLocalFileHeaderFilePos, DestEndFilePos;
    HANDLE hDataFile;
    INT nErrorCode;
    TCompressFileFunc pCompressFileFunc;

    if (!lReadHeaderType(hSrcFile, &HeaderType) ||
        !lReadFile(hSrcFile, (LPBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !lWriteHeaderType(hDestFile, HeaderType) ||
        !lGetFilePointer(hDestFile, &DestLocalFileHeaderFilePos) ||
        !lWriteFile(hDestFile, (LPCBYTE)&LocalFileRecord, sizeof(LocalFileRecord)))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    if (pChangeFileRecord->nChangeFlags & CChangeFileName)
    {
        if (INVALID_SET_FILE_POINTER == ::SetFilePointer(hSrcFile, LocalFileRecord.nFileNameLen,
                                                         NULL, FILE_CURRENT) ||
            !lWriteFile(hDestFile, (LPCBYTE)pChangeFileRecord->pFileName->AsAnsi(),
                        pChangeFileRecord->pFileName->AnsiLength()))
        {
            delete[] pbyBuffer;

            return FALSE;
        }

        LocalFileRecord.nGeneralFlags |= CUTF8LanguageEncodingFlag;
        LocalFileRecord.nFileNameLen = (UINT16)pChangeFileRecord->pFileName->AnsiLength();
    }
    else
    {
        if (!lReadFile(hSrcFile, pbyBuffer, LocalFileRecord.nFileNameLen) ||
            !lWriteFile(hDestFile, pbyBuffer, LocalFileRecord.nFileNameLen))
        {
            delete[] pbyBuffer;

            return FALSE;
        }
    }

    if (!lReadFile(hSrcFile, pbyBuffer, LocalFileRecord.nExtraFieldLen) ||
        !lWriteFile(hDestFile, pbyBuffer, LocalFileRecord.nExtraFieldLen))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    if (pChangeFileRecord->nChangeFlags & CChangeComment)
    {
        LocalFileRecord.nGeneralFlags |= CUTF8LanguageEncodingFlag;
    }

    if (pChangeFileRecord->nChangeFlags & CChangeData)
    {
        hDataFile = ::CreateFileW(pChangeFileRecord->pszDataFile, GENERIC_READ,
                                  FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, NULL);

        if (hDataFile == INVALID_HANDLE_VALUE)
        {
            delete[] pbyBuffer;

            return FALSE;
        }

        switch (pChangeFileRecord->nCompressionMethod)
        {
            case CFileIsStored:
                pCompressFileFunc = lCompressStoredFile;
                break;
            case CFileIsDeflated:
                pCompressFileFunc = lCompressDeflatedFile;
                break;
            default:
                assert(0);

                pCompressFileFunc = NULL;
                break;
        }

        if (pCompressFileFunc)
        {
            pCompressFileFunc(hDataFile, hDestFile, &FileRecord, &nErrorCode);
        }
        else
        {
            ::ZeroMemory(&FileRecord, sizeof(FileRecord));

            nErrorCode = CUtZipInternalError;
        }

        ::CloseHandle(hDataFile);

        if (nErrorCode != CUtZipNoError)
        {
            delete[] pbyBuffer;

            return FALSE;
        }

        LocalFileRecord.nCRC = FileRecord.nCRC;
        LocalFileRecord.nUncompressedSize = FileRecord.nUncompressedSize;
        LocalFileRecord.nCompressedSize = FileRecord.nCompressedSize;
        LocalFileRecord.nCompressionMethod = pChangeFileRecord->nCompressionMethod;
    }
    else
    {
        if (!lCopyFileData(hSrcFile, hDestFile, LocalFileRecord.nCompressedSize))
        {
            delete[] pbyBuffer;

            return FALSE;
        }
    }

    if (!lGetFilePointer(hDestFile, &DestEndFilePos) ||
        !::SetFilePointerEx(hDestFile, DestLocalFileHeaderFilePos, NULL, FILE_BEGIN) ||
        !lWriteFile(hDestFile, (LPCBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !::SetFilePointerEx(hDestFile, DestEndFilePos, NULL, FILE_BEGIN))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    delete[] pbyBuffer;

    return TRUE;
}

static BOOL lAddLocalFileRecord(
  HANDLE hDestFile,
  const TAddFileRecord* pAddFileRecord)
{
    TLocalFileRecord LocalFileRecord;
    TFileRecord FileRecord;
    LARGE_INTEGER DestLocalFileHeaderFilePos, DestEndFilePos;
    HANDLE hDataFile;
    INT nErrorCode;
    TCompressFileFunc pCompressFileFunc;

    if (!lWriteHeaderType(hDestFile, ehtLocalFileHeader) ||
        !lGetFilePointer(hDestFile, &DestLocalFileHeaderFilePos) ||
        !lWriteFile(hDestFile, (LPCBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !lWriteFile(hDestFile, (LPCBYTE)pAddFileRecord->pFileName->AsAnsi(),
                    pAddFileRecord->pFileName->AnsiLength()))
    {
        return FALSE;
    }

    hDataFile = ::CreateFileW(pAddFileRecord->pszDataFile, GENERIC_READ,
                              FILE_SHARE_READ, NULL, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDataFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    switch (pAddFileRecord->nCompressionMethod)
    {
        case CFileIsStored:
            pCompressFileFunc = lCompressStoredFile;
            break;
        case CFileIsDeflated:
            pCompressFileFunc = lCompressDeflatedFile;
            break;
        default:
            assert(0);

            pCompressFileFunc = NULL;
            break;
    }

    if (pCompressFileFunc)
    {
        FileRecord.DataType = pAddFileRecord->DataType;

        pCompressFileFunc(hDataFile, hDestFile, &FileRecord, &nErrorCode);
    }
    else
    {
        ::ZeroMemory(&FileRecord, sizeof(FileRecord));

        nErrorCode = CUtZipInternalError;
    }

    ::CloseHandle(hDataFile);

    if (nErrorCode != CUtZipNoError)
    {
        return FALSE;
    }

    LocalFileRecord.nExtractVersion = MMakeExtractVersion(CMSDOS, 2, 0);
    LocalFileRecord.nGeneralFlags = CUTF8LanguageEncodingFlag;
    LocalFileRecord.nCompressionMethod = pAddFileRecord->nCompressionMethod;
    LocalFileRecord.nFileTime = pAddFileRecord->nFileTime;
    LocalFileRecord.nFileDate = pAddFileRecord->nFileDate;
    LocalFileRecord.nCRC = FileRecord.nCRC;
    LocalFileRecord.nCompressedSize = FileRecord.nCompressedSize;
    LocalFileRecord.nUncompressedSize = FileRecord.nUncompressedSize;
    LocalFileRecord.nFileNameLen = (UINT16)pAddFileRecord->pFileName->AnsiLength();
    LocalFileRecord.nExtraFieldLen = 0;

    switch (pAddFileRecord->nCompressionMethod)
    {
        case CFileIsDeflated:
            LocalFileRecord.nGeneralFlags |= 0x0002;
            break;
    }

    if (!lGetFilePointer(hDestFile, &DestEndFilePos) ||
        !::SetFilePointerEx(hDestFile, DestLocalFileHeaderFilePos, NULL, FILE_BEGIN) ||
        !lWriteFile(hDestFile, (LPCBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !::SetFilePointerEx(hDestFile, DestEndFilePos, NULL, FILE_BEGIN))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL lCopyLocalFileRecord(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  const TCopyFileRecord* pCopyFileRecord)
{
    LPBYTE pbyBuffer = new (std::nothrow) BYTE[UINT16_MAX];
    LARGE_INTEGER SrcCurrentFilePos, SrcLocalFileRecordPos;
    EHeaderType HeaderType;
    TLocalFileRecord LocalFileRecord;
    LPSTR pszTmpFileName;
    INT nTmpFileNameLen;

    if (pbyBuffer == NULL)
    {
        return FALSE;
    }

    SrcLocalFileRecordPos.QuadPart = pCopyFileRecord->pFileRecord->nLocalHeaderOffset;

    if (!lGetFilePointer(hSrcFile, &SrcCurrentFilePos) ||
        !::SetFilePointerEx(hSrcFile, SrcLocalFileRecordPos, NULL, FILE_BEGIN) ||
        !lReadHeaderType(hSrcFile, &HeaderType) ||
        !lReadFile(hSrcFile, (LPBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !lReadFile(hSrcFile, pbyBuffer, LocalFileRecord.nFileNameLen) ||
        !lReadFile(hSrcFile, pbyBuffer, LocalFileRecord.nExtraFieldLen))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    nTmpFileNameLen = ::WideCharToMultiByte(CP_UTF8, 0, pCopyFileRecord->pszDestFile, -1,
                                            NULL, 0, NULL, NULL);

    pszTmpFileName = new (std::nothrow) CHAR[nTmpFileNameLen];

    if (pszTmpFileName == NULL)
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    ::WideCharToMultiByte(CP_UTF8, 0, pCopyFileRecord->pszDestFile, -1,
                          pszTmpFileName, nTmpFileNameLen, NULL, NULL);

    LocalFileRecord.nGeneralFlags |= CUTF8LanguageEncodingFlag;
    LocalFileRecord.nFileNameLen = (UINT16)::lstrlenA(pszTmpFileName);

    if (!lWriteHeaderType(hDestFile, HeaderType) ||
        !lWriteFile(hDestFile, (LPCBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !lWriteFile(hDestFile, (LPCBYTE)pszTmpFileName, LocalFileRecord.nFileNameLen) ||
        !lWriteFile(hDestFile, pbyBuffer, LocalFileRecord.nExtraFieldLen) ||
        !lCopyFileData(hSrcFile, hDestFile, LocalFileRecord.nCompressedSize) ||
        !::SetFilePointerEx(hSrcFile, SrcCurrentFilePos, NULL, FILE_BEGIN))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    delete[] pbyBuffer;

    return TRUE;
}

static BOOL lCopyUnchangedCentralDirectoryRecord(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  UINT32 nLocalHeaderOffset,
  UINT32* pnCentralDirectoryRecordLen)
{
    LPBYTE pbyBuffer = new (std::nothrow) BYTE[UINT16_MAX];
    TCentralDirectoryRecord CentralDirectoryRecord;
    EHeaderType HeaderType;

    *pnCentralDirectoryRecordLen = 0;

    if (pbyBuffer == NULL)
    {
        return FALSE;
    }

    if (!lReadHeaderType(hSrcFile, &HeaderType) ||
        !lReadFile(hSrcFile, (LPBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    CentralDirectoryRecord.nLocalHeaderOffset = nLocalHeaderOffset;

    if (!lWriteHeaderType(hDestFile, HeaderType) ||
        !lWriteFile(hDestFile, (LPCBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    if (!lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nFileNameLen) ||
        !lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nFileNameLen) ||
        !lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nExtraFieldLen) ||
        !lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nExtraFieldLen) ||
        !lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nFileCommentLen) ||
        !lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nFileCommentLen))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    *pnCentralDirectoryRecordLen = sizeof(UINT32) + sizeof(CentralDirectoryRecord) +
                                       CentralDirectoryRecord.nFileNameLen +
                                       CentralDirectoryRecord.nExtraFieldLen +
                                       CentralDirectoryRecord.nFileCommentLen;

    delete[] pbyBuffer;

    return TRUE;
}

static BOOL lCopyChangedCentralDirectoryRecord(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  UINT32 nLocalHeaderOffset,
  const TChangeFileRecord* pChangeFileRecord,
  UINT32* pnCentralDirectoryRecordLen)
{
    LPBYTE pbyBuffer = new (std::nothrow) BYTE[UINT16_MAX];
    TCentralDirectoryRecord CentralDirectoryRecord;
    TLocalFileRecord LocalFileRecord;
    EHeaderType HeaderType;
    LARGE_INTEGER DestLocalFileHeaderFilePos, DestCentralDirectoryRecordFilePos;
    LARGE_INTEGER DestEndFilePos;

    *pnCentralDirectoryRecordLen = 0;

    if (pbyBuffer == NULL)
    {
        return FALSE;
    }

    DestLocalFileHeaderFilePos.QuadPart = nLocalHeaderOffset + sizeof(UINT32);

    if (!lReadHeaderType(hSrcFile, &HeaderType) ||
        !lReadFile(hSrcFile, (LPBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)) ||
        !lWriteHeaderType(hDestFile, HeaderType) ||
        !lGetFilePointer(hDestFile, &DestCentralDirectoryRecordFilePos) ||
        !::SetFilePointerEx(hDestFile, DestLocalFileHeaderFilePos, NULL, FILE_BEGIN) ||
        !lReadFile(hDestFile, (LPBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !::SetFilePointerEx(hDestFile, DestCentralDirectoryRecordFilePos, NULL, FILE_BEGIN) ||
        !lWriteFile(hDestFile, (LPCBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    if (pChangeFileRecord->nChangeFlags & CChangeFileName)
    {
        if (INVALID_SET_FILE_POINTER == ::SetFilePointer(hSrcFile,
                                                         pChangeFileRecord->pFileRecord->nLocalHeaderOffset +
                                                             sizeof(UINT32) + sizeof(TLocalFileRecord) +
                                                             CentralDirectoryRecord.nFileNameLen,
                                                         NULL, FILE_BEGIN) ||
            !lWriteFile(hDestFile, (LPCBYTE)pChangeFileRecord->pFileName->AsAnsi(),
                        LocalFileRecord.nFileNameLen))
        {
            delete[] pbyBuffer;

            return FALSE;
        }

        CentralDirectoryRecord.nFileNameLen = LocalFileRecord.nFileNameLen;
    }
    else
    {
        if (INVALID_SET_FILE_POINTER == ::SetFilePointer(hSrcFile,
                                                         pChangeFileRecord->pFileRecord->nLocalHeaderOffset +
                                                             sizeof(UINT32) + sizeof(TLocalFileRecord),
                                                         NULL, FILE_BEGIN) ||
            !lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nFileNameLen) ||
            !lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nFileNameLen))
        {
            delete[] pbyBuffer;

            return FALSE;
        }
    }

    if (!lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nExtraFieldLen) ||
        !lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nExtraFieldLen))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    if (pChangeFileRecord->nChangeFlags & CChangeComment)
    {
        CentralDirectoryRecord.nFileCommentLen = (UINT16)pChangeFileRecord->pComment->AnsiLength();

        if (!lWriteFile(hDestFile, (LPCBYTE)pChangeFileRecord->pComment->AsAnsi(),
                        CentralDirectoryRecord.nFileCommentLen))
        {
            delete[] pbyBuffer;

            return FALSE;
        }
    }
    else
    {
        if (!lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nFileCommentLen) ||
            !lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nFileCommentLen))
        {
            delete[] pbyBuffer;

            return FALSE;
        }
    }

    CentralDirectoryRecord.nLocalHeaderOffset = nLocalHeaderOffset;
    CentralDirectoryRecord.nGeneralFlags = LocalFileRecord.nGeneralFlags;
    CentralDirectoryRecord.nFileNameLen = LocalFileRecord.nFileNameLen;
    CentralDirectoryRecord.nCompressedSize = LocalFileRecord.nCompressedSize;
    CentralDirectoryRecord.nUncompressedSize = LocalFileRecord.nUncompressedSize;
    CentralDirectoryRecord.nCRC = LocalFileRecord.nCRC;
    CentralDirectoryRecord.nCompressionMethod = LocalFileRecord.nCompressionMethod;

    if (!lGetFilePointer(hDestFile, &DestEndFilePos) ||
        !::SetFilePointerEx(hDestFile, DestCentralDirectoryRecordFilePos, NULL, FILE_BEGIN) ||
        !lWriteFile(hDestFile, (LPCBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)) ||
        !::SetFilePointerEx(hDestFile, DestEndFilePos, NULL, FILE_BEGIN))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    *pnCentralDirectoryRecordLen = sizeof(UINT32) + sizeof(CentralDirectoryRecord) +
                                       CentralDirectoryRecord.nFileNameLen +
                                       CentralDirectoryRecord.nExtraFieldLen +
                                       CentralDirectoryRecord.nFileCommentLen;

    delete[] pbyBuffer;

    return TRUE;
}

static BOOL lAddCentralDirectoryRecord(
  HANDLE hDestFile,
  UINT32 nLocalHeaderOffset,
  const TAddFileRecord* pAddFileRecord,
  UINT32* pnCentralDirectoryRecordLen)
{
    TCentralDirectoryRecord CentralDirectoryRecord;
    TLocalFileRecord LocalFileRecord;
    LARGE_INTEGER DestLocalFileHeaderFilePos, DestCentralDirectoryRecordFilePos;
    LARGE_INTEGER DestEndFilePos;
    EHeaderType HeaderType;

    *pnCentralDirectoryRecordLen = 0;

    DestLocalFileHeaderFilePos.QuadPart = nLocalHeaderOffset;

    if (!lWriteHeaderType(hDestFile, ehtCentralFileHeader) ||
        !lGetFilePointer(hDestFile, &DestCentralDirectoryRecordFilePos) ||
        !::SetFilePointerEx(hDestFile, DestLocalFileHeaderFilePos, NULL, FILE_BEGIN) ||
        !lReadHeaderType(hDestFile, &HeaderType) ||
        !lReadFile(hDestFile, (LPBYTE)&LocalFileRecord, sizeof(LocalFileRecord)) ||
        !::SetFilePointerEx(hDestFile, DestCentralDirectoryRecordFilePos, NULL, FILE_BEGIN) ||
        !lWriteFile(hDestFile, (LPCBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)) ||
        !lWriteFile(hDestFile, (LPCBYTE)pAddFileRecord->pFileName->AsAnsi(),
                    LocalFileRecord.nFileNameLen) ||
        !lWriteFile(hDestFile, (LPCBYTE)pAddFileRecord->pComment->AsAnsi(),
                    pAddFileRecord->pComment->AnsiLength()) ||
        !lGetFilePointer(hDestFile, &DestEndFilePos) ||
        !::SetFilePointerEx(hDestFile, DestCentralDirectoryRecordFilePos, NULL, FILE_BEGIN))
    {
        return FALSE;
    }

    CentralDirectoryRecord.nVersionMadeBy = LocalFileRecord.nExtractVersion;
    CentralDirectoryRecord.nExtractVersion = 20;
    CentralDirectoryRecord.nGeneralFlags = LocalFileRecord.nGeneralFlags;
    CentralDirectoryRecord.nCompressionMethod = LocalFileRecord.nCompressionMethod;
    CentralDirectoryRecord.nFileTime = LocalFileRecord.nFileTime;
    CentralDirectoryRecord.nFileDate = LocalFileRecord.nFileDate;
    CentralDirectoryRecord.nCRC = LocalFileRecord.nCRC;
    CentralDirectoryRecord.nCompressedSize = LocalFileRecord.nCompressedSize;
    CentralDirectoryRecord.nUncompressedSize = LocalFileRecord.nUncompressedSize;
    CentralDirectoryRecord.nFileNameLen = LocalFileRecord.nFileNameLen;
    CentralDirectoryRecord.nExtraFieldLen = LocalFileRecord.nExtraFieldLen;
    CentralDirectoryRecord.nFileCommentLen = (UINT16)pAddFileRecord->pComment->AnsiLength();
    CentralDirectoryRecord.nDiskNumberStart = 0;
    CentralDirectoryRecord.nInternalFileAttributes = 0;
    CentralDirectoryRecord.nExternalFileAttributes = 0;
    CentralDirectoryRecord.nLocalHeaderOffset = nLocalHeaderOffset;

    if (!lWriteFile(hDestFile, (LPCBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)) ||
        !::SetFilePointerEx(hDestFile, DestEndFilePos, NULL, FILE_BEGIN))
    {
        return FALSE;
    }

    *pnCentralDirectoryRecordLen = sizeof(UINT32) + sizeof(CentralDirectoryRecord) +
                                       CentralDirectoryRecord.nFileNameLen +
                                       CentralDirectoryRecord.nExtraFieldLen +
                                       CentralDirectoryRecord.nFileCommentLen;

    return TRUE;
}

static BOOL lCopyCentralDirectoryRecord(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  UINT32 nLocalHeaderOffset,
  const TCopyFileRecord* pCopyFileRecord,
  UINT32* pnCentralDirectoryRecordLen)
{
    LPBYTE pbyBuffer = new (std::nothrow) BYTE[UINT16_MAX];
    TCentralDirectoryRecord CentralDirectoryRecord;
    EHeaderType HeaderType;
    LPSTR pszTmpFileName;
    INT nTmpFileNameLen;
    BOOL bConvertFileComment;
    LARGE_INTEGER CentralDirectoryRecordFilePos, DestCurrentFilePos;

    *pnCentralDirectoryRecordLen = 0;

    if (pbyBuffer == NULL)
    {
        return FALSE;
    }

    if (!lReadHeaderType(hSrcFile, &HeaderType) ||
        !lReadFile(hSrcFile, (LPBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)) ||
        !lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nFileNameLen))
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    nTmpFileNameLen = ::WideCharToMultiByte(CP_UTF8, 0, pCopyFileRecord->pszDestFile, -1,
                                            NULL, 0, NULL, NULL);

    pszTmpFileName = new (std::nothrow) CHAR[nTmpFileNameLen];

    if (pszTmpFileName == NULL)
    {
        delete[] pbyBuffer;

        return FALSE;
    }

    ::WideCharToMultiByte(CP_UTF8, 0, pCopyFileRecord->pszDestFile, -1,
                          pszTmpFileName, nTmpFileNameLen, NULL, NULL);

    bConvertFileComment = (CentralDirectoryRecord.nGeneralFlags & CUTF8LanguageEncodingFlag) ? FALSE : TRUE;

    CentralDirectoryRecord.nGeneralFlags |= CUTF8LanguageEncodingFlag;
    CentralDirectoryRecord.nFileNameLen = (UINT16)::lstrlenA(pszTmpFileName);
    CentralDirectoryRecord.nLocalHeaderOffset = nLocalHeaderOffset;

    if (!lWriteHeaderType(hDestFile, HeaderType) ||
        !lGetFilePointer(hDestFile, &CentralDirectoryRecordFilePos) ||
        !lWriteFile(hDestFile, (LPCBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)) ||
        !lWriteFile(hDestFile, (LPCBYTE)pszTmpFileName, CentralDirectoryRecord.nFileNameLen) ||
        !lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nExtraFieldLen) ||
        !lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nExtraFieldLen) ||
        !lReadFile(hSrcFile, pbyBuffer, CentralDirectoryRecord.nFileCommentLen))
    {
        delete[] pszTmpFileName;
        delete[] pbyBuffer;

        return FALSE;
    }

    if (bConvertFileComment)
    {
        CUtZipString FileComment((LPSTR)pbyBuffer,
                                 CentralDirectoryRecord.nFileCommentLen,
                                 CMSDOSCodePage);

        if (!FileComment.IsValid())
        {
            delete[] pszTmpFileName;
            delete[] pbyBuffer;

            return FALSE;
        }

        CentralDirectoryRecord.nFileCommentLen = (UINT16)FileComment.AnsiLength();

        if (!lWriteFile(hDestFile, (LPCBYTE)FileComment.AsAnsi(),
                        CentralDirectoryRecord.nFileCommentLen) ||
            !lGetFilePointer(hDestFile, &DestCurrentFilePos) ||
            !::SetFilePointerEx(hDestFile, CentralDirectoryRecordFilePos, NULL, FILE_BEGIN) ||
            !lWriteFile(hDestFile, (LPCBYTE)&CentralDirectoryRecord, sizeof(CentralDirectoryRecord)) ||
            !::SetFilePointerEx(hDestFile, DestCurrentFilePos, NULL, FILE_BEGIN))
        {
            delete[] pszTmpFileName;
            delete[] pbyBuffer;

            return FALSE;
        }
    }
    else
    {
        if (!lWriteFile(hDestFile, pbyBuffer, CentralDirectoryRecord.nFileCommentLen))
        {
            delete[] pszTmpFileName;
            delete[] pbyBuffer;

            return FALSE;
        }
    }

    delete[] pszTmpFileName;
    delete[] pbyBuffer;

    return TRUE;
}

static VOID lGenerateUnchangedFileRecordVec(
  const TFileRecordVec* pFileRecordVec,
  const TChangeFileRecordVec* pChangeFileRecordVec,
  const TAddFileRecordVec* pAddFileRecordVec,
  const TFileRecordVec* pDeleteFileRecordVec,
  TFileRecordVec* pUnchangedFileRecordVec)
{
    BOOL bFound;

    for (TFileRecordVec::const_iterator it = pFileRecordVec->begin();
         it != pFileRecordVec->end(); ++it)
    {
        bFound = FALSE;

        for (TChangeFileRecordVec::const_iterator itChange = pChangeFileRecordVec->begin();
             bFound == FALSE && itChange != pChangeFileRecordVec->end(); ++itChange)
        {
            if (*it == (*itChange)->pFileRecord)
            {
                bFound = TRUE;
            }
        }

        for (TAddFileRecordVec::const_iterator itAdd = pAddFileRecordVec->begin();
             bFound == FALSE && itAdd != pAddFileRecordVec->end(); ++itAdd)
        {
            if ((*it)->pFileName == (*itAdd)->pFileName)
            {
                bFound = TRUE;
            }
        }

        for (TFileRecordVec::const_iterator itDelete = pDeleteFileRecordVec->begin();
             bFound == FALSE && itDelete != pDeleteFileRecordVec->end(); ++itDelete)
        {
            if (*it == *itDelete)
            {
                bFound = TRUE;
            }
        }

        if (bFound == FALSE)
        {
            pUnchangedFileRecordVec->push_back(*it);
        }
    }
}

static BOOL lWriteZipFile(
  HANDLE hSrcFile,
  HANDLE hDestFile,
  TFileRecordVec* pUnchangedFileRecordVec,
  TChangeFileRecordVec* pChangeFileRecordVec,
  TAddFileRecordVec* pAddFileRecordVec,
  TCopyFileRecordVec* pCopyFileRecordVec,
  LPCWSTR pszComment,
  UINT16* pnTotalCentralDirectoryEntries,
  UINT32* pnStartCentralDirectoryOffset,
  LPINT pnErrorCode)
{
    LARGE_INTEGER SeekFilePosLargeInteger, CurFilePosLargeInteger;
    TLongLongVec UnchangedLocalFileRecordFilePosVec, ChangedLocalFileRecordFilePosVec;
    TLongLongVec AddLocalFileRecordFilePosVec, CopyLocalFileRecordFilePosVec;
    TLongLongVec CentralDirectoryRecordFilePosVec;
    TEndCentralDirectoryRecord EndCentralDirectoryRecord;
    TFileRecord* pFileRecord;
    LPSTR pszNewComment;
    INT nNewCommentLen;
    UINT32 nCentralDirectoryRecordLen;

    *pnTotalCentralDirectoryEntries = 0;
    *pnStartCentralDirectoryOffset = 0;
    *pnErrorCode = CUtZipNoError;

    EndCentralDirectoryRecord.nDiskNumber = 0;
    EndCentralDirectoryRecord.nStartCentralDirectoryDiskNumber = 0;
    EndCentralDirectoryRecord.nCentralDirectorySize = 0;
    EndCentralDirectoryRecord.nStartCentralDirectoryOffset = 0;

    // Copy unchanged file records

    for (TFileRecordVec::iterator it = pUnchangedFileRecordVec->begin();
        it != pUnchangedFileRecordVec->end(); ++it)
    {
        SeekFilePosLargeInteger.QuadPart = (*it)->nLocalHeaderOffset;

        if (!::SetFilePointerEx(hSrcFile, SeekFilePosLargeInteger, NULL, FILE_BEGIN) ||
            !lGetFilePointer(hDestFile, &CurFilePosLargeInteger) ||
            !lCopyUnchangedLocalFileRecord(hSrcFile, hDestFile, *it))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        UnchangedLocalFileRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    // Copy changed file records

    for (TChangeFileRecordVec::iterator it = pChangeFileRecordVec->begin();
         it != pChangeFileRecordVec->end(); ++it)
    {
        SeekFilePosLargeInteger.QuadPart = (*it)->pFileRecord->nLocalHeaderOffset;

        if (!::SetFilePointerEx(hSrcFile, SeekFilePosLargeInteger, NULL, FILE_BEGIN) ||
            !lGetFilePointer(hDestFile, &CurFilePosLargeInteger) ||
            !lCopyChangedLocalFileRecord(hSrcFile, hDestFile, *it))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        ChangedLocalFileRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    // Add new file records

    for (TAddFileRecordVec::iterator it = pAddFileRecordVec->begin();
         it != pAddFileRecordVec->end(); ++it)
    {
        if (!lGetFilePointer(hDestFile, &CurFilePosLargeInteger) ||
            !lAddLocalFileRecord(hDestFile, *it))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        AddLocalFileRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    // Make copies of file records to copy

    for (TCopyFileRecordVec::iterator it = pCopyFileRecordVec->begin();
         it != pCopyFileRecordVec->end(); ++it)
    {
        if (!lGetFilePointer(hDestFile, &CurFilePosLargeInteger) ||
            !lCopyLocalFileRecord(hSrcFile, hDestFile, *it))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        CopyLocalFileRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    // Copy unchanged central directory records

    for (TFileRecordVec::size_type Index = 0; Index < pUnchangedFileRecordVec->size(); ++Index)
    {
        pFileRecord = pUnchangedFileRecordVec->at(Index);

        if (!lGetFilePointer(hDestFile, &CurFilePosLargeInteger))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        SeekFilePosLargeInteger.QuadPart = pFileRecord->nCentralDirectoryOffset;

        if (!::SetFilePointerEx(hSrcFile, SeekFilePosLargeInteger, NULL, FILE_BEGIN) ||
            !lCopyUnchangedCentralDirectoryRecord(hSrcFile, hDestFile,
                                                  (UINT32)UnchangedLocalFileRecordFilePosVec[Index],
                                                  &nCentralDirectoryRecordLen))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        EndCentralDirectoryRecord.nCentralDirectorySize += nCentralDirectoryRecordLen;

        CentralDirectoryRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    // Copy changed central directory records

    for (TChangeFileRecordVec::size_type Index = 0; Index < pChangeFileRecordVec->size(); ++Index)
    {
        pFileRecord = pChangeFileRecordVec->at(Index)->pFileRecord;

        if (!lGetFilePointer(hDestFile, &CurFilePosLargeInteger))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        SeekFilePosLargeInteger.QuadPart = pFileRecord->nCentralDirectoryOffset;

        if (!::SetFilePointerEx(hSrcFile, SeekFilePosLargeInteger, NULL, FILE_BEGIN) ||
            !lCopyChangedCentralDirectoryRecord(hSrcFile, hDestFile,
                                                (UINT32)ChangedLocalFileRecordFilePosVec[Index],
                                                pChangeFileRecordVec->at(Index),
                                                &nCentralDirectoryRecordLen))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        EndCentralDirectoryRecord.nCentralDirectorySize += nCentralDirectoryRecordLen;

        CentralDirectoryRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    // Add new central directory records

    for (TAddFileRecordVec::size_type Index = 0; Index < pAddFileRecordVec->size(); ++Index)
    {
        if (!lGetFilePointer(hDestFile, &CurFilePosLargeInteger) ||
            !lAddCentralDirectoryRecord(hDestFile,
                                        (UINT32)AddLocalFileRecordFilePosVec[Index],
                                        pAddFileRecordVec->at(Index),
                                        &nCentralDirectoryRecordLen))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        EndCentralDirectoryRecord.nCentralDirectorySize += nCentralDirectoryRecordLen;

        CentralDirectoryRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    // Make copies of the central directory records to copy

    for (TCopyFileRecordVec::size_type Index = 0; Index < pCopyFileRecordVec->size(); ++Index)
    {
        pFileRecord = pCopyFileRecordVec->at(Index)->pFileRecord;

        if (!lGetFilePointer(hSrcFile, &CurFilePosLargeInteger))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        SeekFilePosLargeInteger.QuadPart = pFileRecord->nCentralDirectoryOffset;

        if (!::SetFilePointerEx(hSrcFile, SeekFilePosLargeInteger, NULL, FILE_BEGIN) ||
            !lCopyCentralDirectoryRecord(hSrcFile, hDestFile,
                                         (UINT32)CopyLocalFileRecordFilePosVec[Index],
                                         pCopyFileRecordVec->at(Index),
                                         &nCentralDirectoryRecordLen))
        {
            *pnErrorCode = CUtZipFileError;

            return FALSE;
        }

        EndCentralDirectoryRecord.nCentralDirectorySize += nCentralDirectoryRecordLen;

        CentralDirectoryRecordFilePosVec.push_back(CurFilePosLargeInteger.QuadPart);
    }

    nNewCommentLen = ::WideCharToMultiByte(CMSDOSCodePage, 0, pszComment, -1,
                                           NULL, 0, NULL, NULL);

    pszNewComment = new (std::nothrow) CHAR[nNewCommentLen];

    if (pszNewComment == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    ::WideCharToMultiByte(CMSDOSCodePage, 0, pszComment, -1,
                          pszNewComment, nNewCommentLen, NULL, NULL);

    EndCentralDirectoryRecord.nTotalCentralDirectoryEntriesOnDisk = (UINT16)CentralDirectoryRecordFilePosVec.size();
    EndCentralDirectoryRecord.nTotalCentralDirectoryEntries = (UINT16)CentralDirectoryRecordFilePosVec.size();
    EndCentralDirectoryRecord.nZipFileCommentLen = (UINT16)(nNewCommentLen - 1);

    if (CentralDirectoryRecordFilePosVec.size() > 0)
    {
        EndCentralDirectoryRecord.nStartCentralDirectoryOffset = (UINT32)CentralDirectoryRecordFilePosVec[0];
    }

    *pnTotalCentralDirectoryEntries = EndCentralDirectoryRecord.nTotalCentralDirectoryEntries;
    *pnStartCentralDirectoryOffset = EndCentralDirectoryRecord.nStartCentralDirectoryOffset;

    if (!lWriteHeaderType(hDestFile, ehtEndCentralDirectoryRecord) ||
        !lWriteFile(hDestFile, (LPCBYTE)&EndCentralDirectoryRecord, sizeof(EndCentralDirectoryRecord)) ||
        !lWriteFile(hDestFile, (LPCBYTE)pszNewComment, EndCentralDirectoryRecord.nZipFileCommentLen))
    {
        delete[] pszNewComment;

        return FALSE;
    }

    delete[] pszNewComment;

    return TRUE;
}

static BOOL lWriteNewZipFile(
  TZip* pZip,
  LPINT pnErrorCode)
{
    BOOL bResult(FALSE);
    WCHAR cTempPath[MAX_PATH], cTempFile[MAX_PATH];
    HANDLE hFile;
    TFileRecordVec UnchangedFileRecordVec;

    *pnErrorCode = CUtZipNoError;

    if (!::GetTempPathW(sizeof(cTempPath) / sizeof(cTempPath[0]), cTempPath) ||
        !::GetTempFileNameW(cTempPath, L"zip", 0, cTempFile))
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    hFile = ::CreateFile(cTempFile, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    lGenerateUnchangedFileRecordVec(pZip->pFileRecordVec, pZip->pChangeFileRecordVec,
                                    pZip->pAddFileRecordVec, pZip->pDeleteFileRecordVec,
                                    &UnchangedFileRecordVec);

    bResult = lWriteZipFile(pZip->hFile, hFile, &UnchangedFileRecordVec,
                            pZip->pChangeFileRecordVec, pZip->pAddFileRecordVec,
                            pZip->pCopyFileRecordVec, pZip->pszZipComment,
                            &pZip->nTotalCentralDirectoryEntries,
                            &pZip->nStartCentralDirectoryOffset,
                            pnErrorCode);

    ::CloseHandle(hFile);

    if (!bResult)
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    ::CloseHandle(pZip->hFile);

    pZip->hFile = NULL;

    lEmptyFileRecordVec(pZip->pFileRecordVec);
    lEmptyAddFileRecordVec(pZip->pAddFileRecordVec);
    lEmptyChangeFileRecordVec(pZip->pChangeFileRecordVec);
    lEmptyCopyFileRecordVec(pZip->pCopyFileRecordVec);

    pZip->pDeleteFileRecordVec->clear();

    pZip->bInitFileRecordVec = FALSE;

    if (!::MoveFileEx(cTempFile, pZip->pszFile,
                      MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING |
                          MOVEFILE_WRITE_THROUGH))
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    hFile = ::CreateFileW(pZip->pszFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                          NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        *pnErrorCode = CUtZipFileError;

        return FALSE;
    }

    pZip->hFile = hFile;
    pZip->bInitFileRecordVec = TRUE;

    return TRUE;
}

TZipHandle UtZipOpenFile(
  LPCWSTR pszFile,
  EZipOpenMode OpenMode,
  LPINT pnErrorCode)
{
    TZip* pZip;
    HANDLE hFile;
    DWORD dwCreateDisposition;
    TEndCentralDirectoryRecord EndCentralDirectoryRecord;
    LARGE_INTEGER FileSize;
    BOOL bReadZip;
    INT nFileLen, nTmpErrorCode;

    *pnErrorCode = CUtZipNoError;

    switch (OpenMode)
    {
        case eZomCreateNew:
            dwCreateDisposition = CREATE_NEW;
            break;
        case eZomCreateAlways:
            dwCreateDisposition = CREATE_ALWAYS;
            break;
        case eZomOpen:
            dwCreateDisposition = OPEN_EXISTING;
            break;
        case eZomOpenAlways:
            dwCreateDisposition = OPEN_ALWAYS;
            break;
        default:
            *pnErrorCode = CUtZipFileCouldNotBeOpened;

            return NULL;
    }

    hFile = ::CreateFileW(pszFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                          NULL, dwCreateDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        *pnErrorCode = CUtZipFileCouldNotBeOpened;

        return NULL;
    }

    pZip = lAllocZip();

    if (pZip == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        ::CloseHandle(hFile);

        return NULL;
    }

    nFileLen = ::lstrlenW(pszFile) + 1;

    pZip->pszFile = new (std::nothrow) WCHAR[nFileLen];
    pZip->hFile = hFile;

    if (pZip->pszFile == NULL)
    {
        lFreeZip(pZip);

        *pnErrorCode = CUtZipOutOfMemory;

        return NULL;
    }

    ::StringCchCopyW(pZip->pszFile, nFileLen, pszFile);

    switch (OpenMode)
    {
        case eZomCreateNew:
        case eZomCreateAlways:
            bReadZip = FALSE;
            break;
        case eZomOpen:
            bReadZip = TRUE;
            break;
        case eZomOpenAlways:
            if (FALSE == ::GetFileSizeEx(hFile, &FileSize) || FileSize.QuadPart == 0)
            {
                bReadZip = FALSE;
            }
            else
            {
                bReadZip = TRUE;
            }
            break;
        default: // This condition should never occur because this is enforced above, but still here as an extra sanity check.
            bReadZip = FALSE;

            assert(0);
            break;
    }

    if (bReadZip)
    {
        if (!lReadEndCentralDirectoryRecord(hFile, &EndCentralDirectoryRecord,
                                            &pZip->pszZipComment,
                                            &nTmpErrorCode))
        {
            lFreeZip(pZip);

            *pnErrorCode = CUtZipInvalidZipFile;

            return NULL;
        }

        if (EndCentralDirectoryRecord.nTotalCentralDirectoryEntriesOnDisk != EndCentralDirectoryRecord.nTotalCentralDirectoryEntries)
        {
            lFreeZip(pZip);

            *pnErrorCode = CUtZipDiskSpanningNotSupported;

            return NULL;
        }

        pZip->nTotalCentralDirectoryEntries = EndCentralDirectoryRecord.nTotalCentralDirectoryEntries;
        pZip->nStartCentralDirectoryOffset = EndCentralDirectoryRecord.nStartCentralDirectoryOffset;
        pZip->bInitFileRecordVec = TRUE;
    }
    else
    {
        pZip->nTotalCentralDirectoryEntries = 0;
        pZip->nStartCentralDirectoryOffset = 0;
        pZip->bInitFileRecordVec = FALSE;

        pZip->pszZipComment = new (std::nothrow) WCHAR[1];

        if (pZip->pszZipComment == NULL)
        {
            lFreeZip(pZip);

            *pnErrorCode = CUtZipOutOfMemory;

            return NULL;
        }

        *pZip->pszZipComment = 0;

        if (!lWriteNewZipFile(pZip, &nTmpErrorCode))
        {
            lFreeZip(pZip);

            *pnErrorCode = CUtZipFileCouldNotBeOpened;

            return NULL;
        }
    }

    return pZip;
}

BOOL UtZipClose(
  TZipHandle hZip,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;

    *pnErrorCode = CUtZipNoError;

    lFreeZip(pZip);

    return TRUE;
}

BOOL UtZipIsValidArchive(
  TZipHandle hZip,
  LPBOOL pbIsValidArchive,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;

    *pbIsValidArchive = FALSE;
    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    *pbIsValidArchive = TRUE;

    return TRUE;
}

BOOL UtZipBeginUpdate(
  TZipHandle hZip,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!pZip->bInUpdate)
    {
        pZip->bInUpdate = TRUE;

        return TRUE;
    }

    *pnErrorCode = CUtZipAlreadyInUpdate;

    return FALSE;
}

BOOL UtZipEndUpdate(
  TZipHandle hZip,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (pZip->bInUpdate)
    {
        pZip->bInUpdate = FALSE;

        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    *pnErrorCode = CUtZipNotInUpdate;

    return FALSE;
}

BOOL UtZipGetUpdateFlag(
  TZipHandle hZip,
  LPBOOL pbUpdateFlag,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    *pbUpdateFlag = pZip->bInUpdate;
    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    return TRUE;
}

LPCWSTR UtZipGetZipComment(
  TZipHandle hZip,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    return pZip->pszZipComment;
}

BOOL UtZipSetZipComment(
  TZipHandle hZip,
  LPCWSTR pszComment,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    LPWSTR pszNewComment;
    INT nNewCommentLen;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (UINT16_MAX < ::WideCharToMultiByte(CMSDOSCodePage, 0, pszComment, -1,
                                           NULL, 0, NULL, NULL) - 1)
    {
        *pnErrorCode = CUtZipZipCommentTooLong;

        return FALSE;
    }

    nNewCommentLen = ::lstrlenW(pszComment) + 1;
    pszNewComment = new (std::nothrow) WCHAR[nNewCommentLen];

    if (pszNewComment == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    ::StringCchCopyW(pszNewComment, nNewCommentLen, pszComment);

    delete[] pZip->pszZipComment;

    pZip->pszZipComment = pszNewComment;

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipChangeFileComment(
  TZipHandle hZip,
  LPCWSTR pszFile,
  LPCWSTR pszComment,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TFileRecord* pFileRecord;
    TChangeFileRecord* pChangeFileRecord;
    INT nTmpErrorCode, nNewCommentLen;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    pFileRecord = lFindFileRecord(pszFile, pZip->pFileRecordVec, pnErrorCode);

    if (pFileRecord == NULL)
    {
        return FALSE;
    }

    if (lDoesCopyFileRecordExist(pszFile, pZip->pCopyFileRecordVec))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingCopied;

        return FALSE;
    }

    nNewCommentLen = ::WideCharToMultiByte(CP_UTF8, 0, pszComment, -1, NULL, 0,
                                           NULL, NULL);

    if (UINT16_MAX < nNewCommentLen - 1)
    {
        *pnErrorCode = CUtZipFileCommentTooLong;

        return FALSE;
    }

    pChangeFileRecord = lFindChangeFileRecord(pszFile, pZip->pChangeFileRecordVec,
                                              &nTmpErrorCode);

    if (pChangeFileRecord == NULL)
    {
        pChangeFileRecord = new (std::nothrow) TChangeFileRecord;

        if (pChangeFileRecord == NULL)
        {
            *pnErrorCode = CUtZipOutOfMemory;

            return FALSE;
        }

        pChangeFileRecord->nChangeFlags = 0;
        pChangeFileRecord->pszDataFile = NULL;
        pChangeFileRecord->pFileName = NULL;
        pChangeFileRecord->pComment = NULL;
        pChangeFileRecord->pFileRecord = pFileRecord;

        pZip->pChangeFileRecordVec->push_back(pChangeFileRecord);
    }
    else
    {
        delete pChangeFileRecord->pComment;
    }

    pChangeFileRecord->pComment = new (std::nothrow) CUtZipString(pszComment);

    if (pChangeFileRecord->pComment == NULL ||
        !pChangeFileRecord->pComment->IsValid())
    {
        delete pChangeFileRecord->pComment;

        pChangeFileRecord->pComment = NULL;

        pChangeFileRecord->nChangeFlags &= ~CChangeComment;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    pChangeFileRecord->nChangeFlags |= CChangeComment;

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipChangeFileName(
  TZipHandle hZip,
  LPCWSTR pszFile,
  LPCWSTR pszNewFile,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TFileRecord* pFileRecord;
    INT nNewFileLen;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    if (!lIsNameValid(pszNewFile))
    {
        *pnErrorCode = CUtZipInvalidFileName;

        return FALSE;
    }

    pFileRecord = lFindFileRecord(pszFile, pZip->pFileRecordVec, pnErrorCode);

    if (pFileRecord == NULL)
    {
        return FALSE;
    }

    if (lDoesCopyFileRecordExist(pszFile, pZip->pCopyFileRecordVec) ||
        lDoesCopyFileRecordExist(pszNewFile, pZip->pCopyFileRecordVec))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingCopied;

        return FALSE;
    }

    nNewFileLen = ::WideCharToMultiByte(CP_UTF8, 0, pszFile, -1, NULL, 0,
                                        NULL, NULL);

    if (UINT16_MAX < nNewFileLen - 1)
    {
        *pnErrorCode = CUtZipFileCommentTooLong;

        return FALSE;
    }

    if (!lAddChangeFileRecordFileName(pZip->pChangeFileRecordVec, pszNewFile,
                                      pFileRecord, pnErrorCode))
    {
        return FALSE;
    }

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipChangeFileData(
  TZipHandle hZip,
  LPCWSTR pszFile,
  LPCWSTR pszDataFile,
  EZipCompressionMethod CompressionMethod,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TFileRecord* pFileRecord;
    TChangeFileRecord* pChangeFileRecord;
    INT nDataFileLen, nTmpErrorCode;
    BOOL bLargeFile;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    pFileRecord = lFindFileRecord(pszFile, pZip->pFileRecordVec, pnErrorCode);

    if (pFileRecord == NULL)
    {
        return FALSE;
    }

    if (lDoesCopyFileRecordExist(pszFile, pZip->pCopyFileRecordVec))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingCopied;

        return FALSE;
    }

    switch (CompressionMethod)
    {
        case eZcmStored:
        case eZcmDeflated:
            break;
        default:
            *pnErrorCode = CUtZipCompressionMethodNotSupported;

            return FALSE;
    }

    if (!lIsDataFileLarge(pszDataFile, &bLargeFile))
    {
        *pnErrorCode = CUtZipDataFileReadError;

        return FALSE;
    }

    if (bLargeFile)
    {
        *pnErrorCode = CUtZipDataFileTooLarge;

        return FALSE;
    }

    pChangeFileRecord = lFindChangeFileRecord(pszFile, pZip->pChangeFileRecordVec,
                                              &nTmpErrorCode);

    if (pChangeFileRecord == NULL)
    {
        pChangeFileRecord = new (std::nothrow) TChangeFileRecord;

        if (pChangeFileRecord == NULL)
        {
            *pnErrorCode = CUtZipOutOfMemory;

            return FALSE;
        }

        pChangeFileRecord->nChangeFlags = 0;
        pChangeFileRecord->pComment = NULL;
        pChangeFileRecord->pFileName = NULL;
        pChangeFileRecord->pszDataFile = NULL;
        pChangeFileRecord->pFileRecord = pFileRecord;

        pZip->pChangeFileRecordVec->push_back(pChangeFileRecord);
    }
    else
    {
        delete[] pChangeFileRecord->pszDataFile;
    }

    nDataFileLen = ::lstrlenW(pszDataFile) + 1;

    pChangeFileRecord->pszDataFile = new (std::nothrow) WCHAR[nDataFileLen];

    if (pChangeFileRecord->pszDataFile == NULL)
    {
        pChangeFileRecord->nChangeFlags &= ~CChangeData;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    ::StringCchCopyW(pChangeFileRecord->pszDataFile, nDataFileLen, pszDataFile);

    pChangeFileRecord->nChangeFlags |= CChangeData;

    switch (CompressionMethod)
    {
        case eZcmStored:
            pChangeFileRecord->nCompressionMethod = CFileIsStored;
            break;
        case eZcmDeflated:
            pChangeFileRecord->nCompressionMethod = CFileIsDeflated;
            break;
        default:
            assert(0);

            *pnErrorCode = CUtZipCompressionMethodNotSupported;

            return FALSE;
    }

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipAllocFileEntries(
  TZipHandle hZip,
  TZipFileEntryVec& ZipFileEntryVec,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TZipFileEntry* pZipFileEntry;
    INT nTmpErrorCode;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    UtZipFreeFileEntries(ZipFileEntryVec, &nTmpErrorCode);

    if (!lInitZip(pZip, &nTmpErrorCode))
    {
        *pnErrorCode = nTmpErrorCode;

        return FALSE;
    }

    for (TFileRecordVec::iterator it = pZip->pFileRecordVec->begin();
         it != pZip->pFileRecordVec->end(); ++it)
    {
        pZipFileEntry = new (std::nothrow) TZipFileEntry;

        if (pZipFileEntry == NULL)
        {
            *pnErrorCode = CUtZipOutOfMemory;

            UtZipFreeFileEntries(ZipFileEntryVec, &nTmpErrorCode);

            return FALSE;
        }

        if (!lInitFileEntryFile(pZipFileEntry, (*it)->pFileName))
        {
            *pnErrorCode = CUtZipOutOfMemory;

            delete pZipFileEntry;

            UtZipFreeFileEntries(ZipFileEntryVec, &nTmpErrorCode);

            return FALSE;
        }

        pZipFileEntry->pszComment = new (std::nothrow) WCHAR[(*it)->pComment->UnicodeLength() + 1];

        if (pZipFileEntry->pszComment == NULL)
        {
            *pnErrorCode = CUtZipOutOfMemory;

            delete[] pZipFileEntry->pszFileName;
            delete[] pZipFileEntry->pszPath;
            delete[] pZipFileEntry->pszSrcFileName;

            delete pZipFileEntry;

            UtZipFreeFileEntries(ZipFileEntryVec, &nTmpErrorCode);

            return FALSE;
        }

        ::StringCchCopyW(pZipFileEntry->pszComment, (*it)->pComment->UnicodeLength() + 1,
                         (*it)->pComment->AsUnicode());

        switch ((*it)->nCompressionMethod)
        {
            case CFileIsStored:
                pZipFileEntry->ZipCompressionMethod = eZcmStored;
                break;
            case CFileIsDeflated:
                pZipFileEntry->ZipCompressionMethod = eZcmDeflated;
                break;
            default:
                pZipFileEntry->ZipCompressionMethod = eZcmUnsupportedCompressionMethod;
                break;
        }

        if (!lDosDateTimeToSystemTime((*it)->nFileDate,
                                      (*it)->nFileTime,
                                      &pZipFileEntry->FileModTime))
        {
            ::ZeroMemory(&pZipFileEntry->FileModTime, sizeof(pZipFileEntry->FileModTime));
        }

        pZipFileEntry->nFileAttributes = lTranslateZipFileAttributes((*it)->byOperatingSystem,
                                                                     (*it)->nFileAttributes);
        pZipFileEntry->nCRC = (*it)->nCRC;
        pZipFileEntry->nCompressedSize = (*it)->nCompressedSize;
        pZipFileEntry->nUncompressedSize = (*it)->nUncompressedSize;

        try
        {
            ZipFileEntryVec.push_back(pZipFileEntry);
        }
        catch (std::exception&)
        {
            *pnErrorCode = CUtZipInternalError;

            delete[] pZipFileEntry->pszFileName;
            delete[] pZipFileEntry->pszPath;
            delete[] pZipFileEntry->pszSrcFileName;
            delete[] pZipFileEntry->pszComment;

            delete pZipFileEntry;

            UtZipFreeFileEntries(ZipFileEntryVec, &nTmpErrorCode);

            return FALSE;
        }
    }

    return TRUE;
}

BOOL UtZipFreeFileEntries(
  TZipFileEntryVec& ZipFileEntryVec,
  LPINT pnErrorCode)
{
    TZipFileEntry* pZipFileEntry;

    *pnErrorCode = CUtZipNoError;

    try
    {
        for (TZipFileEntryVec::iterator it = ZipFileEntryVec.begin();
             it != ZipFileEntryVec.end(); ++it)
        {
            pZipFileEntry = *it;

            delete[] pZipFileEntry->pszFileName;
            delete[] pZipFileEntry->pszSrcFileName;
            delete[] pZipFileEntry->pszPath;
            delete pZipFileEntry;
        }

        ZipFileEntryVec.clear();
    }

    catch (std::exception&)
    {
        *pnErrorCode = CUtZipInternalError;

        return FALSE;
    }

    return TRUE;
}

BOOL UtZipExtractFile(
  TZipHandle hZip,
  LPCWSTR pszFile,
  LPCWSTR pszOutputFile,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TFileRecord* pFileRecord;
    HANDLE hDestFile;
    TUncompressFileFunc pUncompressFileFunc;
    LARGE_INTEGER LargeInteger;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    pFileRecord = lFindFileRecord(pszFile, pZip->pFileRecordVec, pnErrorCode);

    if (pFileRecord == NULL)
    {
        return FALSE;
    }

    if (!lSeekCompressedDataStart(pZip->hFile, pFileRecord, pnErrorCode))
    {
        return FALSE;
    }

    hDestFile = ::CreateFileW(pszOutputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDestFile == INVALID_HANDLE_VALUE)
    {
        *pnErrorCode = CUtZipCreateFileError;

        return FALSE;
    }

    LargeInteger.QuadPart = pFileRecord->nUncompressedSize;

    if (!::SetFilePointerEx(hDestFile, LargeInteger, NULL, FILE_BEGIN) ||
        !::SetEndOfFile(hDestFile) ||
        INVALID_SET_FILE_POINTER == ::SetFilePointer(hDestFile, 0, NULL, FILE_BEGIN))
    {
        *pnErrorCode = CUtZipFileError;

        ::CloseHandle(hDestFile);

        ::DeleteFileW(pszOutputFile);

        return FALSE;
    }

    switch (pFileRecord->nCompressionMethod)
    {
        case CFileIsStored:
            pUncompressFileFunc = lUncompressStoredFile;
            break;
        case CFileIsDeflated:
            pUncompressFileFunc = lUncompressDeflatedFile;
            break;
        default:
            *pnErrorCode = CUtZipCompressionMethodNotSupported;

            ::CloseHandle(hDestFile);

            ::DeleteFileW(pszOutputFile);

            return FALSE;
    }

    if (!pUncompressFileFunc(pZip->hFile, hDestFile, pFileRecord, pnErrorCode))
    {
        ::CloseHandle(hDestFile);

        ::DeleteFileW(pszOutputFile);

        return FALSE;
    }

    ::CloseHandle(hDestFile);

    return TRUE;
}

BOOL UtZipAddFile(
  TZipHandle hZip,
  LPCWSTR pszFile,
  LPCWSTR pszDataFile,
  LPCWSTR pszComment,
  EZipCompressionMethod CompressionMethod,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TFileRecord* pFileRecord;
    TAddFileRecord* pAddFileRecord;
    INT nTmpTextLen;
    BOOL bLargeFile;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lIsNameValid(pszFile))
    {
        *pnErrorCode = CUtZipInvalidFileName;

        return FALSE;
    }

    switch (CompressionMethod)
    {
        case eZcmStored:
        case eZcmDeflated:
            break;
        default:
            *pnErrorCode = CUtZipCompressionMethodNotSupported;

            return FALSE;
    }

    if (!lIsDataFileLarge(pszDataFile, &bLargeFile))
    {
        *pnErrorCode = CUtZipDataFileReadError;

        return FALSE;
    }

    if (bLargeFile)
    {
        *pnErrorCode = CUtZipDataFileTooLarge;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    pFileRecord = lFindFileRecord(pszFile, pZip->pFileRecordVec, pnErrorCode);

    if (pFileRecord != NULL)
    {
        *pnErrorCode = CUtZipFileAlreadyExists;

        return FALSE;
    }

    pAddFileRecord = lFindAddFileRecord(pszFile, pZip->pAddFileRecordVec, pnErrorCode);

    if (pAddFileRecord != NULL)
    {
        *pnErrorCode = CUtZipFileAlreadyAdded;

        return FALSE;
    }

    if (lDoesCopyFileRecordExist(pszFile, pZip->pCopyFileRecordVec))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingCopied;

        return FALSE;
    }

    pAddFileRecord = new (std::nothrow) TAddFileRecord;

    if (pAddFileRecord == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    pAddFileRecord->pFileName = new (std::nothrow) CUtZipString(pszFile);

    if (pAddFileRecord->pFileName == NULL)
    {
        delete pAddFileRecord;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    if (pszComment)
    {
        pAddFileRecord->pComment = new (std::nothrow) CUtZipString(pszComment);
    }
    else
    {
        pAddFileRecord->pComment = new (std::nothrow) CUtZipString(L"");
    }

    if (pAddFileRecord->pComment == NULL)
    {
        delete pAddFileRecord->pFileName;
        delete pAddFileRecord;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    nTmpTextLen = ::lstrlenW(pszDataFile) + 1;

    pAddFileRecord->pszDataFile = new (std::nothrow) WCHAR[nTmpTextLen];

    if (pAddFileRecord->pszDataFile == NULL)
    {
        delete pAddFileRecord->pComment;
        delete pAddFileRecord->pFileName;
        delete pAddFileRecord;

        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    ::StringCchCopyW(pAddFileRecord->pszDataFile, nTmpTextLen, pszDataFile);

    pAddFileRecord->nFileAttributes = lGetFileAttributes(pszDataFile);
    pAddFileRecord->DataType = edtBinary;

    lGetFileDosDateTime(pszDataFile, &pAddFileRecord->nFileDate,
                        &pAddFileRecord->nFileTime);

    switch (CompressionMethod)
    {
        case eZcmStored:
            pAddFileRecord->nCompressionMethod = CFileIsStored;
            break;
        case eZcmDeflated:
            pAddFileRecord->nCompressionMethod = CFileIsDeflated;
            break;
        default:
            assert(0);
            break;
    }

    pZip->pAddFileRecordVec->push_back(pAddFileRecord);

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipDeleteFile(
  TZipHandle hZip,
  LPCWSTR pszFile,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TFileRecord* pFileRecord;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    pFileRecord = lFindFileRecord(pszFile, pZip->pFileRecordVec, pnErrorCode);

    if (pFileRecord == NULL)
    {
        return FALSE;
    }

    if (NULL != lFindChangeFileRecord(pszFile, pZip->pChangeFileRecordVec, pnErrorCode))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingChanged;

        return FALSE;
    }

    if (NULL != lFindFileRecord(pszFile, pZip->pDeleteFileRecordVec, pnErrorCode))
    {
        *pnErrorCode = CUtZipFileAlreadyDeleted;

        return FALSE;
    }

    if (lDoesCopyFileRecordExist(pszFile, pZip->pCopyFileRecordVec))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingCopied;

        return FALSE;
    }

    pZip->pDeleteFileRecordVec->push_back(pFileRecord);

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipCopyFile(
  TZipHandle hZip,
  LPCWSTR pszSrcFile,
  LPCWSTR pszDestFile,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    TFileRecord* pFileRecord;
    INT nSrcFileLen, nDestFileLen;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    pFileRecord = lFindFileRecord(pszSrcFile, pZip->pFileRecordVec, pnErrorCode);

    if (pFileRecord == NULL)
    {
        return FALSE;
    }

    if (NULL != lFindFileRecord(pszDestFile, pZip->pFileRecordVec, pnErrorCode))
    {
        *pnErrorCode = CUtZipFileAlreadyExists;

        return FALSE;
    }

    if (NULL != lFindAddFileRecord(pszDestFile, pZip->pAddFileRecordVec, pnErrorCode))
    {
        *pnErrorCode = CUtZipFileAlreadyAdded;

        return FALSE;
    }

    if (NULL != lFindChangeFileRecord(pszSrcFile, pZip->pChangeFileRecordVec, pnErrorCode))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingChanged;

        return FALSE;
    }

    if (NULL != lFindFileRecord(pszSrcFile, pZip->pDeleteFileRecordVec, pnErrorCode))
    {
        *pnErrorCode = CUtZipFileAlreadyDeleted;

        return FALSE;
    }

    if (lDoesCopyFileRecordExist(pszSrcFile, pZip->pCopyFileRecordVec) ||
        lDoesCopyFileRecordExist(pszDestFile, pZip->pCopyFileRecordVec))
    {
        *pnErrorCode = CUtZipFileAlreadyBeingCopied;

        return FALSE;
    }

    if (!lIsNameValid(pszDestFile))
    {
        *pnErrorCode = CUtZipInvalidFileName;

        return FALSE;
    }

    nSrcFileLen = ::lstrlenW(pszSrcFile);
    nDestFileLen = ::lstrlenW(pszDestFile);

    if (nSrcFileLen > UINT16_MAX || nDestFileLen > UINT16_MAX)
    {
        *pnErrorCode = CUtZipFileNameTooLong;

        return FALSE;
    }

    if (!lAddCopyFileRecordFileName(pZip->pCopyFileRecordVec, pszSrcFile,
                                    pszDestFile, pFileRecord, pnErrorCode))
    {
        return FALSE;
    }

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipRenameDirectory(
  TZipHandle hZip,
  LPCWSTR pszSrcDirectory,
  LPCWSTR pszDestDirectory,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    BOOL bHadRenameError = FALSE;
    TChangeFileRecordVec* pNewChangeFileRecordVec;
    TFileRecordVec FileRecordVec;
    LPWSTR pszRenamedFileName;
    INT nTmpErrorCode;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lIsNameValid(pszSrcDirectory) || !lIsNameValid(pszDestDirectory))
    {
        *pnErrorCode = CUtZipInvalidDirectoryName;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    if (!lFindFileRecordsMatchingPath(pszSrcDirectory, pZip->pFileRecordVec,
                                      &FileRecordVec, pnErrorCode))
    {
        return FALSE;
    }

    pNewChangeFileRecordVec = lCloneChangeFileRecordVec(pZip->pChangeFileRecordVec);

    if (pNewChangeFileRecordVec == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    for (TFileRecordVec::iterator it = FileRecordVec.begin();
         bHadRenameError == FALSE && it != FileRecordVec.end(); ++it)
    {
        if (lDoesFileRecordExist(*it, pZip->pDeleteFileRecordVec) ||
            lDoesCopyFileRecordExist(*it, pZip->pCopyFileRecordVec))
        {
            bHadRenameError = TRUE;

            continue;
        }

        pszRenamedFileName = lAllocRenamedFileName((*it)->pFileName->AsUnicode(),
                                                   pszSrcDirectory,
                                                   pszDestDirectory,
                                                   &nTmpErrorCode);

        if (pszRenamedFileName == NULL)
        {
            bHadRenameError = TRUE;

            continue;
        }

        if (lDoesCopyFileRecordExist(pszRenamedFileName, pZip->pCopyFileRecordVec) ||
            lFindAddFileRecord(pszRenamedFileName, pZip->pAddFileRecordVec,
                               &nTmpErrorCode))
        {
            delete[] pszRenamedFileName;

            bHadRenameError = TRUE;

            continue;
        }

        if (!lAddChangeFileRecordFileName(pNewChangeFileRecordVec,
                                          pszRenamedFileName, *it, &nTmpErrorCode))
        {
            bHadRenameError = TRUE;
        }

        delete[] pszRenamedFileName;
    }

    if (bHadRenameError)
    {
        lEmptyChangeFileRecordVec(pNewChangeFileRecordVec);

        delete pNewChangeFileRecordVec;

        *pnErrorCode = CUtZipDirectoryRenameFailed;

        return FALSE;
    }

    lEmptyChangeFileRecordVec(pZip->pChangeFileRecordVec);

    delete pZip->pChangeFileRecordVec;

    pZip->pChangeFileRecordVec = pNewChangeFileRecordVec;

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipDeleteDirectory(
  TZipHandle hZip,
  LPCWSTR pszDirectory,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    BOOL bHadDeleteError = FALSE;
    TFileRecordVec FileRecordVec;
    TFileRecordVec* pNewDeleteFileRecordVec;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lIsNameValid(pszDirectory))
    {
        *pnErrorCode = CUtZipInvalidDirectoryName;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    if (!lFindFileRecordsMatchingPath(pszDirectory, pZip->pFileRecordVec,
                                      &FileRecordVec, pnErrorCode))
    {
        return FALSE;
    }

    pNewDeleteFileRecordVec = lCloneDeleteFileRecordVec(pZip->pDeleteFileRecordVec);

    if (pNewDeleteFileRecordVec == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    for (TFileRecordVec::iterator it = FileRecordVec.begin();
         bHadDeleteError == FALSE && it != FileRecordVec.end(); ++it)
    {
        if (lFindChangeFileRecord(*it, pZip->pChangeFileRecordVec) ||
            lDoesFileRecordExist(*it, pNewDeleteFileRecordVec) ||
            lDoesCopyFileRecordExist(*it, pZip->pCopyFileRecordVec))
        {
            bHadDeleteError = TRUE;

            continue;
        }

        pNewDeleteFileRecordVec->push_back(*it);
    }

    if (bHadDeleteError)
    {
        delete pNewDeleteFileRecordVec;

        *pnErrorCode = CUtZipDirectoryDeleteFailed;

        return FALSE;
    }

    delete pZip->pDeleteFileRecordVec;

    pZip->pDeleteFileRecordVec = pNewDeleteFileRecordVec;

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

BOOL UtZipCopyDirectory(
  TZipHandle hZip,
  LPCWSTR pszSrcDirectory,
  LPCWSTR pszDestDirectory,
  LPINT pnErrorCode)
{
    TZip* pZip = (TZip*)hZip;
    BOOL bHadCopyError = FALSE;
    TFileRecordVec FileRecordVec;
    TCopyFileRecordVec* pNewCopyFileRecordVec;
    LPWSTR pszRenamedFileName;
    INT nTmpErrorCode;

    *pnErrorCode = CUtZipNoError;

    if (pZip->hFile == NULL)
    {
        *pnErrorCode = CUtZipInvalidZipFile;

        return FALSE;
    }

    if (!lIsNameValid(pszSrcDirectory) || !lIsNameValid(pszDestDirectory))
    {
        *pnErrorCode = CUtZipInvalidDirectoryName;

        return FALSE;
    }

    if (!lInitZip(pZip, pnErrorCode))
    {
        return FALSE;
    }

    if (!lFindFileRecordsMatchingPath(pszSrcDirectory, pZip->pFileRecordVec,
                                      &FileRecordVec, pnErrorCode))
    {
        return FALSE;
    }

    pNewCopyFileRecordVec = lCloneCopyFileRecordVec(pZip->pCopyFileRecordVec);

    if (pNewCopyFileRecordVec == NULL)
    {
        *pnErrorCode = CUtZipOutOfMemory;

        return FALSE;
    }

    for (TFileRecordVec::iterator it = FileRecordVec.begin();
         bHadCopyError == FALSE && it != FileRecordVec.end(); ++it)
    {
        if (lFindChangeFileRecord(*it, pZip->pChangeFileRecordVec) ||
            lDoesFileRecordExist(*it, pZip->pDeleteFileRecordVec) ||
            lDoesCopyFileRecordExist(*it, pNewCopyFileRecordVec))
        {
            bHadCopyError = TRUE;

            continue;
        }

        pszRenamedFileName = lAllocRenamedFileName((*it)->pFileName->AsUnicode(), pszSrcDirectory,
                                                   pszDestDirectory, &nTmpErrorCode);

        if (pszRenamedFileName == NULL)
        {
            bHadCopyError = TRUE;

            continue;
        }

        if (lDoesCopyFileRecordExist(pszRenamedFileName, pNewCopyFileRecordVec) ||
            lFindAddFileRecord(pszRenamedFileName, pZip->pAddFileRecordVec,
                               &nTmpErrorCode))
        {
            delete[] pszRenamedFileName;

            bHadCopyError = TRUE;

            continue;
        }

        if (!lAddCopyFileRecordFileName(pNewCopyFileRecordVec, (*it)->pFileName->AsUnicode(),
                                        pszRenamedFileName, *it, &nTmpErrorCode))
        {
            bHadCopyError = TRUE;
        }

        delete[] pszRenamedFileName;
    }

    if (bHadCopyError)
    {
        lEmptyCopyFileRecordVec(pNewCopyFileRecordVec);

        delete pNewCopyFileRecordVec;

        *pnErrorCode = CUtZipDirectoryCopyFailed;

        return FALSE;
    }

    lEmptyCopyFileRecordVec(pZip->pCopyFileRecordVec);

    delete pZip->pCopyFileRecordVec;

    pZip->pCopyFileRecordVec = pNewCopyFileRecordVec;    

    if (!pZip->bInUpdate)
    {
        return lWriteNewZipFile(pZip, pnErrorCode);
    }

    return TRUE;
}

#if defined(__cplusplus_cli)
#pragma managed
#endif

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
