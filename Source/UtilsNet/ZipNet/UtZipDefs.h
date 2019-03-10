/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define MMakeExtractVersion(hostSystem, majorVersion, minorVersion) \
    (UINT16)(((UINT32)hostSystem << 8) | ((UINT32)majorVersion * 10) | (UINT32)minorVersion)

#define CMSDOSCodePage 437

// General flags

#define CUTF8LanguageEncodingFlag 0x0800

// Zip file compression methods (only currently supported methods)

#define CFileIsStored 0
#define CFileIsDeflated 8

// Operating Systems

#define CMSDOS 0
#define CAmiga 1
#define COpenVMS 2
#define CUNIX 3
#define CVM_CMS 4
#define CAtariST 5
#define COS2 6
#define CMacintosh 7
#define CZ_System 8
#define CCPM 9
#define CWindowsNTFS 10
#define CMVS 11
#define CVSE 12
#define CAcornRisc 13
#define CVFAT 14
#define CAlternateMVS 15
#define CBeOS 16
#define CTandem 17
#define COS400 18
#define COSX 19

#pragma pack(push, 1)

typedef struct tagTDataDescriptor
{
    UINT32 nCRC;
    UINT32 nCompressedSize;
    UINT32 nUncompressedSize;
} TDataDescriptor;

typedef struct tagTLocalFileRecord
{
    UINT16 nExtractVersion;
    UINT16 nGeneralFlags;
    UINT16 nCompressionMethod;
    UINT16 nFileTime;
    UINT16 nFileDate;
    UINT32 nCRC;
    UINT32 nCompressedSize;
    UINT32 nUncompressedSize;
    UINT16 nFileNameLen;
    UINT16 nExtraFieldLen;
} TLocalFileRecord;

typedef struct tagTCentralDirectoryRecord
{
    UINT16 nVersionMadeBy;
    UINT16 nExtractVersion;
    UINT16 nGeneralFlags;
    UINT16 nCompressionMethod;
    UINT16 nFileTime;
    UINT16 nFileDate;
    UINT32 nCRC;
    UINT32 nCompressedSize;
    UINT32 nUncompressedSize;
    UINT16 nFileNameLen;
    UINT16 nExtraFieldLen;
    UINT16 nFileCommentLen;
    UINT16 nDiskNumberStart;
    UINT16 nInternalFileAttributes;
    UINT32 nExternalFileAttributes;
    UINT32 nLocalHeaderOffset;
} TCentralDirectoryRecord;

typedef struct tagTEndCentralDirectoryRecord
{
    UINT16 nDiskNumber;
    UINT16 nStartCentralDirectoryDiskNumber;
    UINT16 nTotalCentralDirectoryEntriesOnDisk;
    UINT16 nTotalCentralDirectoryEntries;
    UINT32 nCentralDirectorySize;
    UINT32 nStartCentralDirectoryOffset;
    UINT16 nZipFileCommentLen;
} TEndCentralDirectoryRecord;

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
