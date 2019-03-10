/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiBufferViewerCtrl_H)
#define UiBufferViewerCtrl_H

#include <windows.h>

#if defined(_MSC_VER)
#pragma pack(push, 1)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#define CUiBufferViewerCtrlClass L"BufferViewerCtrl"

/* Buffer data */

typedef struct tagTUiBufferViewerMem
{
    LPBYTE pbyBuffer;
    ULONG nBufferLen;
} TUiBufferViewerMem;

/* Data Organization flags */

#define CBufferViewerAsciiData            0x0001
#define CBufferViewerByteData             0x0002
#define CBufferViewerWordBigEndianData    0x0004
#define CBufferViewerWordLittleEndianData 0x0008

/* Min/Max Font Point Sizes */

#define CBufferViewerMinPtSize 8
#define CBufferViewwerMaxPtSize 32

#define CBufferViewerDefPtSize 10

/* Messages */

#define BVM_GETBUFFER           (WM_USER + 0x0100)
#define BVM_SETBUFFER           (WM_USER + 0x0101)

#define BVM_GETDATAORGANIZATION (WM_USER + 0x0102)
#define BVM_SETDATAORGANIZATION (WM_USER + 0x0104)

#define BVM_GETFONTPTSIZE       (WM_USER + 0x0105)
#define BVM_SETFONTPTSIZE       (WM_USER + 0x0106)

VOID UiBufferViewerCtrlRegister(VOID);
VOID UiBufferViewerCtrlUnregister(VOID);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* end of UiBufferViewerCtrl_H */

/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/
