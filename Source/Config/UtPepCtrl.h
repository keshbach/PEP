/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepCtrl_H)
#define UtPepCtrl_H

#include <Includes/UtExternC.h>

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#pragma region "Constants"

#define UTPEPCTRLAPI __stdcall

#pragma endregion

#pragma region "Enumerations"

typedef enum tagEUtPepCtrlDeviceType
{
    eUtPepCtrlParallelPortDeviceType,
    eUtPepCtrlUsbPrintDeviceType,
    eUtPepCtrlUsbDeviceType
} EUtPepCtrlDeviceType;

typedef enum tagEUtPepCtrlProgrammerMode
{
    eUtPepCtrlProgrammerNoneMode,
    eUtPepCtrlProgrammerReadMode,
    eUtPepCtrlProgrammerWriteMode
} EUtPepCtrlProgrammerMode;

typedef enum taEUtPepCtrlVccMode
{
    eUtPepCtrl5VDCMode,
    eUtPepCtrl625VDCMode
} EUtPepCtrlVccMode;

typedef enum tagEUtPepCtrlPinPulseMode
{
    eUtPepCtrlPinPulse1Mode, /* Pin 23 as +12VDC, Pin 24 as +12VDC (Enable Vpp Mode), Pin 25 as -5VDC, 1 msec Vpp pulse */
    eUtPepCtrlPinPulse2Mode, /* Pin 25 as +5VDC/Vpp (Enable Vpp Mode), 250 us Vpp pulse */
    eUtPepCtrlPinPulse3Mode, /* Pin 24 as Vpp (Enable Vpp Mode), 250 us Chip Enable pulse */
    eUtPepCtrlPinPulse4Mode  /* Pin 1 and 3 as Vpp (Enable Vpp Mode), Chip Enable no pulse */
} EUtPepCtrlPinPulseMode;

typedef enum tagEUtPepCtrlVppMode
{
    eUtPepCtrl12VDCVppMode,
    eUtPepCtrl21VDCVppMode,
    eUtPepCtrl25VDCVppMode
} EUtPepCtrlVppMode;

typedef enum tagEUtPepCtrlOEMode
{
    eUtPepCtrlEnableOE,
    eUtPepCtrlDisableOE
} EUtPepCtrlOEMode;

typedef enum tagEUtPepCtrlDeviceChange
{
    eUtPepCtrlDeviceArrived,
    eUtPepCtrlDeviceRemoved
} EUtPepCtrlDeviceChange;

#pragma endregion

#pragma region "Type Definitions"

typedef VOID (UTPEPCTRLAPI *TUtPepCtrlDeviceChangeFunc)(_In_ EUtPepCtrlDeviceChange DeviceChange);

#pragma endregion

#pragma region "Structures"

typedef struct tagTUtPepCtrlReadUserData
{
    UINT32 nAddress;
    EUtPepCtrlOEMode OutputEnableMode;
    BOOL bPerformRead;
} TUtPepCtrlReadUserData;

typedef struct tagTUtPepCtrlReadUserDataWithDelay
{
	UINT32 nAddress;
	UINT32 nDelayNanoSeconds; // Delay execute after set address and before performing a read
	BOOL bPerformRead;
} TUtPepCtrlReadUserDataWithDelay;

typedef struct tagTUtPepCtrlProgramUserData
{
    UINT32 nAddress;
    BOOL bPerformProgram;
} TUtPepCtrlProgramUserData;

#pragma endregion

MExternC BOOL UTPEPCTRLAPI UtPepCtrlInitialize(_In_ EUtPepCtrlDeviceType DeviceType, _In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlUninitialize(VOID);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlGetDeviceName(_Out_ LPWSTR pszDeviceName, _Out_ LPINT pnDeviceNameLen);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetDeviceName(_Const_ _In_ LPCWSTR pszDeviceName);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetDelaySettings(_In_ UINT32 nChipEnableNanoSeconds, _In_ UINT32 nOutputEnableNanoSeconds);

_Success_(return)
MExternC BOOL UTPEPCTRLAPI UtPepCtrlIsDevicePresent(_Out_writes_(sizeof(BOOL)) LPBOOL pbPresent);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlReset(VOID);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetProgrammerMode(_In_ EUtPepCtrlProgrammerMode ProgrammerMode);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetVccMode(_In_ EUtPepCtrlVccMode VccMode);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetPinPulseMode(_In_ EUtPepCtrlPinPulseMode PinPulseMode);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetVppMode(_In_ EUtPepCtrlVppMode VppMode);

_Success_(return)
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReadData(_In_ UINT32 nAddress, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

_Success_(return)
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReadUserData(_Const_ _In_reads_(nReadUserDataLen) const TUtPepCtrlReadUserData* pReadUserData, _In_ UINT32 nReadUserDataLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

_Success_(return)
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReadUserDataWithDelay(_Const_ _In_reads_(nReadUserDataWithDelayLen) const TUtPepCtrlReadUserDataWithDelay* pReadUserDataWithDelay, _In_ UINT32 nReadUserDataWithDelayLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlProgramData(_In_ UINT nAddress, _Const_ _In_reads_(nDataLen) const LPBYTE pbyData, _In_ UINT32 nDataLen);

MExternC BOOL UTPEPCTRLAPI UtPepCtrlProgramUserData(_Const_ _In_reads_(nProgramUserDataLen) const TUtPepCtrlProgramUserData* pProgramUserData, _In_ UINT32 nProgramUserDataLen, _Const_ _In_reads_(nDataLen) const LPBYTE pbyData, _In_ UINT32 nDataLen);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* UtPepCtrl_H */

/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/
