/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
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

#if defined(_MSC_VER)
#define TUTPEPLOGICAPI __stdcall
#elif defined(__XC8) || defined(__18CXX)
#define TUTPEPLOGICAPI
#else
#error Need to define calling convention
#endif

typedef BOOLEAN (TUTPEPLOGICAPI *TUtPepLogicReadBitPortFunc)(
#if defined(ENABLE_DEVICE_CONTEXT)
    _In_ PVOID pvContext,
#endif
    _Out_ PBOOLEAN pbValue);
typedef BOOLEAN (TUTPEPLOGICAPI *TUtPepLogicWritePortFunc)(
#if defined(ENABLE_DEVICE_CONTEXT)
    _In_ PVOID pvContext,
#endif
    _In_ PUINT8 pnData,
#if defined(_MSC_VER)
    _In_ UINT32 nDataLen,
#elif defined(__XC8) || defined(__18CXX)
    _In_ UINT8 nDataLen,
#else
#error Need to define the size of ulDataLen
#endif
    _In_ UINT32 nWaitNanoSeconds);

#if defined(ENABLE_LOGGING) 
typedef VOID (__cdecl *TUtPepLogicLogFunc)(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);
#endif

#if defined(_MSC_VER)
#define MROM
#if defined(_X86_)
#pragma pack(push, 4)
#elif defined(_AMD64_)
#pragma pack(push, 8)
#else
#error Need to specify cpu architecture to configure structure padding
#endif
#elif defined(__XC8) || defined(__18CXX)
#define MROM rom
#else
#error Need to specify how to enable byte aligned structure padding
#endif

typedef struct tagTUtPepLogicData
{
#if defined(ENABLE_DEVICE_CONTEXT)
    PVOID pvDeviceContext;
#endif
    MROM TUtPepLogicReadBitPortFunc pReadBitPortFunc;
    MROM TUtPepLogicWritePortFunc pWritePortFunc;
#if defined(ENABLE_LOGGING) 
    TUtPepLogicLogFunc pLogFunc;
#endif
    PVOID pvLogicContext;
} TUtPepLogicData;

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__XC8) || defined(__18CXX)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* !defined(UtPepLogicDefs_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/
