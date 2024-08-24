/***************************************************************************/
/*  Copyright (C) 2007-2024 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtIntelHex_H)
#define UtIntelHex_H

#include <Includes/UtExternC.h>

typedef PVOID TUtIntelHexHandle;

#define UTINTELHEXAPI __stdcall

#define UtIntelHexErrorCodeSuccess 0
#define UtIntelHexErrorCodeFileError 1
#define UtIntelHexErrorCodeOutOfMemory 2
#define UtIntelHexErrorCodeMinRecordLenNotSatisfied 3
#define UtIntelHexErrorCodeInvalidByteCount 4
#define UtIntelHexErrorCodeInvalidAddress 5
#define UtIntelHexErrorCodeInvalidRecordType 6
#define UtIntelHexErrorCodeUnrecognizedRecordType 7
#define UtIntelHexErrorCodeInvalidDataLen 8
#define UtIntelHexErrorCodeInvalidData 9
#define UtIntelHexErrorCodeInvalidChecksum 10
#define UtIntelHexErrorCodeChecksumMismatch 11
#define UtIntelHexErrorCodeInvalidFieldByteCount 12
#define UtIntelHexErrorCodeUnsupportedRecordType 13

MExternC BOOL UTINTELHEXAPI UtIntelHexInitialize(VOID);
MExternC BOOL UTINTELHEXAPI UtIntelHexUninitialize(VOID);

MExternC TUtIntelHexHandle UTINTELHEXAPI UtIntelHexLoadFile(LPCWSTR pszFile);
MExternC BOOL UTINTELHEXAPI UtIntelHexFreeData(TUtIntelHexHandle handle);

MExternC BOOL UTINTELHEXAPI UtIntelHexTotalPages(TUtIntelHexHandle handle, UINT32* pnTotalPages);

MExternC BOOL UTINTELHEXAPI UtIntelHexGetPageAddress(TUtIntelHexHandle handle, UINT32 nPageNumber, UINT32* pnAddress);

MExternC BOOL UTINTELHEXAPI UtIntelHexGetPageAddressData(TUtIntelHexHandle handle, UINT32 nPageNumber, const UINT8** ppData, UINT32* pnDataLen);

MExternC BOOL UTINTELHEXAPI UtIntelHexErrorLineNumber(TUtIntelHexHandle handle, UINT32* pnLineNumber);
MExternC BOOL UTINTELHEXAPI UtIntelHexErrorCode(TUtIntelHexHandle handle, UINT16* pnErrorCode);

#endif /* UtIntelHex_H */

/***************************************************************************/
/*  Copyright (C) 2007-2024 Kevin Eshbach                                  */
/***************************************************************************/
