/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepCtrl_H)
#define UtPepCtrl_H

#include <Includes/UtExternC.h>

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#define UTPEPCTRLAPI __stdcall

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

typedef VOID (UTPEPCTRLAPI *TUtPepCtrlDeviceChangeFunc)(EUtPepCtrlDeviceChange DeviceChange);

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

MExternC BOOL UTPEPCTRLAPI UtPepCtrlInitialize(TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlUninitialize(VOID);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlIsDevicePresent(LPBOOL pbPresent);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReset(VOID);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetProgrammerMode(EUtPepCtrlProgrammerMode ProgrammerMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetVccMode(EUtPepCtrlVccMode VccMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetPinPulseMode(EUtPepCtrlPinPulseMode PinPulseMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlSetVppMode(EUtPepCtrlVppMode VppMode);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReadData(UINT32 nAddress, LPBYTE pbyData, UINT32 nDataLen);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlReadUserData(const TUtPepCtrlReadUserData* pReadUserData, UINT32 nReadUserDataLen, LPBYTE pbyData, UINT32 nDataLen);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlProgramData(UINT nAddress, LPBYTE pbyData, UINT32 nDataLen);
MExternC BOOL UTPEPCTRLAPI UtPepCtrlProgramUserData(const TUtPepCtrlProgramUserData* pProgramUserData, UINT32 nProgramUserDataLen, LPBYTE pbyData, UINT32 nDataLen);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* UtPepCtrl_H */

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
