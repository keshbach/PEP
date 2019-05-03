/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepLogicDefs_H)
#define UtPepLogicDefs_H

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

typedef BOOLEAN (TUTPEPLOGICAPI *TUtPepLogicReadBitPortFunc)(_In_ PVOID pvContext,
                                                             OUT PBOOLEAN pbValue);
typedef BOOLEAN (TUTPEPLOGICAPI *TUtPepLogicWritePortFunc)(_In_ PVOID pvContext,
                                                           _In_ PUCHAR pucData,
                                                           _In_ ULONG ulDataLen);

typedef VOID (__cdecl *TUtPepLogicLogFunc)(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);

#if defined(_MSC_VER)
#if defined(_X86_)
#pragma pack(push, 4)
#elif defined(_AMD64_)
#pragma pack(push, 8)
#else
#error Need to specify cpu architecture to configure structure padding
#endif
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

#endif /* !defined(UtPepLogicDefs_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
