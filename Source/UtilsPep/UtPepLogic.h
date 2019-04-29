/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepLogic_H)
#define UtPepLogic_H

#include <Includes/UtExternC.h>

/*
  Programmer Modes
*/

#define CUtPepLogicProgrammerNoneMode  0x0001
#define CUtPepLogicProgrammerReadMode  0x0002
#define CUtPepLogicProgrammerWriteMode 0x0004 /* OE pin always going to be high */

/*
  Switched VCC Modes (Power voltage)
*/

#define CUtPepLogic5VDCMode   0x0001 /* +5 VDC */
#define CUtPepLogic625VDCMode 0x0002 /* +6.25 VDC */

/*
  Pin/Pulse Modes
*/

#define CUtPepLogicPinPulse1Mode 0x0001 /* Pin 23 as +12VDC, Pin 24 as +12VDC (Enable Vpp Mode), Pin 25 as -5VDC, 1 msec Vpp pulse */
#define CUtPepLogicPinPulse2Mode 0x0002 /* Pin 25 as +5VDC/Vpp (Enable Vpp Mode), 250 us Vpp pulse */
#define CUtPepLogicPinPulse3Mode 0x0003 /* Pin 24 as Vpp (Enable Vpp Mode), 250 us Chip Enable pulse */
#define CUtPepLogicPinPulse4Mode 0x0004 /* Pin 1 and 3 as Vpp (Enable Vpp Mode), User-defined Chip Enable pulse */

/*
  Vpp Modes (Program voltage)
*/

#define CUtPepLogic12VDCVppMode 0x0001 /* +12 VDC */
#define CUtPepLogic21VDCVppMode 0x0002 /* +21 VDC */
#define CUtPepLogic25VDCVppMode 0x0004 /* +25 VDC */

#define TUTPEPLOGICAPI __stdcall

typedef BOOLEAN (TUTPEPLOGICAPI *TUtPepLogicReadBitPortFunc)(IN PVOID pvContext,
                                                             OUT PBOOLEAN pbValue);
typedef BOOLEAN (TUTPEPLOGICAPI *TUtPepLogicWritePortFunc)(IN PVOID pvContext,
                                                           IN PUCHAR pucData,
                                                           IN ULONG ulDataLen);

typedef VOID (__cdecl *TUtPepLogicLogFunc)(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

typedef struct tagTUtPepLogicData
{
    PVOID pvDeviceContext;
    TUtPepLogicReadBitPortFunc pReadBitPortFunc;
    TUtPepLogicWritePortFunc pWritePortFunc;
    TUtPepLogicLogFunc pLogFunc;
    PVOID pvLogicContext;
} TUtPepLogicData;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

MExternC PVOID TUTPEPLOGICAPI UtPepLogicAllocLogicContext();
MExternC VOID TUTPEPLOGICAPI UtPepLogicFreeLogicContext(IN PVOID pvLogicContext);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetProgrammerMode(IN TUtPepLogicData* pLogicData,
                                                            IN UINT32 nProgrammerMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVccMode(IN TUtPepLogicData* pLogicData,
                                                     IN UINT32 nVccMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetPinPulseMode(IN TUtPepLogicData* pLogicData,
                                                          IN UINT32 nPinMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVppMode(IN TUtPepLogicData* pLogicData,
                                                     IN UINT32 nVppMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddress(IN TUtPepLogicData* pLogicData,
                                                     IN UINT32 nAddress);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicGetData(IN TUtPepLogicData* pLogicData,
                                                  OUT UINT8* pnData);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetData(IN TUtPepLogicData* pLogicData,
                                                  IN UINT8 nData);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicTriggerProgram(IN TUtPepLogicData* pLogicData,
                                                         OUT PBOOLEAN pbSuccess);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetOutputEnable(IN TUtPepLogicData* pLogicData,
                                                          IN UINT32 nOutputEnable);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicReset(IN TUtPepLogicData* pLogicData);

#endif /* !defined(UtPepLogic_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
