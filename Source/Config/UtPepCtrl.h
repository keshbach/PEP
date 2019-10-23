/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
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

typedef enum tagEUtPepCtrlPortType
{
    eUtPepCtrlNonePortType,
    eUtPepCtrlParallelPortType,
    eUtPepCtrlUsbPrintPortType
} EUtPepCtrlPortType;

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
    eUtPepCtrlIgnoreOE,
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

typedef struct tagTUtPepCtrlProgramUserData
{
    UINT32 nAddress;
    BOOL bPerformProgram;
} TUtPepCtrlProgramUserData;

#pragma endregion

MExternC BOOL UTPEPCTRLAPI UtPepCtrlInitialize(_In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlUninitialize(VOID);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetPortSettings(_In_ EUtPepCtrlPortType PortType, _In_ LPCWSTR pszPortDeviceName);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlGetPortType(_Out_ EUtPepCtrlPortType* pPortType);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlGetPortDeviceName(_Out_ LPWSTR pszPortDeviceName, _Out_ LPINT pnPortDeviceNameLen);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlIsDevicePresent(_Out_ LPBOOL pbPresent);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReset(VOID);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetProgrammerMode(_In_ EUtPepCtrlProgrammerMode ProgrammerMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetVccMode(_In_ EUtPepCtrlVccMode VccMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetPinPulseMode(_In_ EUtPepCtrlPinPulseMode PinPulseMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetVppMode(_In_ EUtPepCtrlVppMode VppMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReadData(_In_ UINT32 nAddress, _Out_ LPBYTE pbyData, _Out_ UINT32 nDataLen);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReadUserData(_In_ const TUtPepCtrlReadUserData* pReadUserData, _In_ UINT32 nReadUserDataLen, _Out_ LPBYTE pbyData, _In_ UINT32 nDataLen);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlProgramData(_In_ UINT nAddress, _Out_ LPBYTE pbyData, _In_ UINT32 nDataLen);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlProgramUserData(_In_ const TUtPepCtrlProgramUserData* pProgramUserData, _In_ UINT32 nProgramUserDataLen, _Out_ LPBYTE pbyData, _In_ UINT32 nDataLen);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* UtPepCtrl_H */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
