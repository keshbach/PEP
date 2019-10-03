/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiPepCtrls_H)
#define UiPepCtrls_H

#define UIPEPCTRLSAPI __stdcall

#define CUiBufferViewerCtrlClass L"BufferViewerCtrl"

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

#pragma region "Structures"

#if defined(_MSC_VER)
#pragma pack(push, 1)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

/* Buffer data */

typedef struct tagTUiBufferViewerMem
{
	LPCBYTE pbyBuffer;
	ULONG nBufferLen;
} TUiBufferViewerMem;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#pragma endregion

#if defined(__cplusplus)
extern "C"
{
#endif

VOID UIPEPCTRLSAPI UiPepCtrlsInitialize(VOID);
VOID UIPEPCTRLSAPI UiPepCtrlsUninitialize(VOID);

#if defined(__cplusplus)
};
#endif

#endif

/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/
