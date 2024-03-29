/***************************************************************************/
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UiPepCtrls_H)
#define UiPepCtrls_H

#define UIPEPCTRLSAPI __stdcall

#pragma region "Class Names"

#define CUiBufferViewerCtrlClass L"BufferViewerCtrl"
#define CUiCheckedListBoxCtrlClass L"CheckedListBoxCtrl"
#define CUiDeviceInfoCtrlClass L"DeviceInfoCtrl"

#pragma endregion

/* Data Organization flags */

#define CBufferViewerAsciiData            0x0001
#define CBufferViewerByteData             0x0002
#define CBufferViewerWordBigEndianData    0x0004
#define CBufferViewerWordLittleEndianData 0x0008

/* Check State flags */

#define CCheckedListBoxUncheckedState     0x0001
#define CCheckedListBoxCheckedState       0x0002
#define CCheckedListBoxIndeterminateState 0x0004

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

#define CLBM_ADDITEM            (WM_USER + 0x0100)
#define CLBM_INSERTITEM         (WM_USER + 0x0101)
#define CLBM_DELETEITEM         (WM_USER + 0x0102)
#define CLBM_DELETEALLITEMS     (WM_USER + 0x0103)

#define CLBM_SETNAME            (WM_USER + 0x0104)
#define CLBM_GETNAME            (WM_USER + 0x0105)

#define CLBM_SETCHECKSTATE      (WM_USER + 0x0106)
#define CLBM_GETCHECKSTATE      (WM_USER + 0x0107)

#define CLBM_GETMINWIDTH        (WM_USER + 0x0108)

#define DIM_SETNAME             (WM_USER + 0x0100)
#define DIM_GETNAME             (WM_USER + 0x0101)

#define DIM_SETADAPTER          (WM_USER + 0x0102)
#define DIM_GETADAPTER          (WM_USER + 0x0103)

#define DIM_SETDIPSWITCHES      (WM_USER + 0x0104)
#define DIM_GETDIPSWITCHES      (WM_USER + 0x0105)

#define DIM_SETSIZE             (WM_USER + 0x0106)
#define DIM_GETSIZE             (WM_USER + 0x0107)

#define DIM_SETVPP              (WM_USER + 0x0108)
#define DIM_GETVPP              (WM_USER + 0x0109)

#define DIM_SETBITS             (WM_USER + 0x010A)
#define DIM_GETBITS             (WM_USER + 0x010B)

#define DIM_SETCHIPENABLE       (WM_USER + 0x010C)
#define DIM_GETCHIPENABLE       (WM_USER + 0x010D)

#define DIM_SETOUTPUTENABLE     (WM_USER + 0x010E)
#define DIM_GETOUTPUTENABLE     (WM_USER + 0x010F)

#define DIM_GETMINRECT          (WM_USER + 0x0110)

/* Notification Messages */

#define CLBNM_STATECHANGE       (WMN_FIRST + 1)

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

/* Checked List Box Get Name */

typedef struct tagTUiCheckedListBoxGetName
{
	LPTSTR pszName; // buffer for the name (when allocate include extra character for the terminating null character)
	INT nNameLen; // length of the pszName field in characters minus the terminating null character
} TUiCheckedListBoxGetName;

/* Checked List Box Control State Change Notification */

typedef struct tagTUiCheckedListBoxCtrlNMStateChange
{
	NMHDR Hdr;
	INT nIndex;
	DWORD dwNewState;
} TUiCheckedListBoxCtrlNMStateChange;

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
/*  Copyright (C) 2010-2021 Kevin Eshbach                                  */
/***************************************************************************/
