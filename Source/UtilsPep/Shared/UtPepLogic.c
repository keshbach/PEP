/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if defined(BUILD_USER_LIB)
#include <windows.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtSleep.h>
#elif defined(BUILD_DRIVER_LIB)
#include <ntddk.h>

#include <ntstrsafe.h>

#include <Utils/UtHeapDriver.h>
#include <Utils/UtSleepDriver.h>
#else
#error Unsupported configuration
#endif

#include <Includes/UtMacros.h>

#include <UtilsPep/UtPepLogic.h>

#pragma region "Constants"

/*
   Unit to select
*/

#define CUnit0_DataBits0To3 0
#define CUnit1_DataBits4To7 1
#define CUnit2_AddressLines0To3 2
#define CUnit3_AddressLines4To7 3
#define CUnit4_AddressLines8To11 4
#define CUnit5_AddressLines12To19AndVppMode 5 /* Set Vpp voltage to either +12VDC, +21VDC or +25VDC */
#define CUnit6_LedAndVpp 6
#define CUnit7_Programmer 7 /* Set VCC voltage to either +5VDC or 6.25VDC */

#define CUnit_DontCare CUnit0_DataBits0To3

/*
    Constants to enable/disable a unit
*/

#define CUnitOn 1
#define CUnitOff 0

/* Unit 5 Control Bits */

#define CEnableAddressLines12To19Unit 2
#define CDisableAddressLines12To19Unit 0
#define CAddressLines12To19On 1
#define CAddressLines12To19Off 0
#define CEnable12Vpp 4
#define CEnable25Vpp 0
#define CEnable21Vpp 8

/* Unit 6 Control Bits */

#define CN0 1
#define CN1 2
#define CN2 4 /* Turn Socket Power LED On */
#define CN3 8 /* Turn Programming LED On */

/* Unit 7 Control Bits */

#define RDPGM 1 /* enable output enable pin (sets pin to low) */
#define PPCL 2 /* Clear 9602 */
#define VppON 4 /* enable Vpp */
#define SEL5 8 /* set Vcc to +5 VDC */
#define SEL6 0 /* set Vcc to +6.25 VDC */

#define PVAR 0
#define PFIX 1

#define CWaitNanoSeconds 10

#pragma endregion

#pragma region "Macros"

/*
    Byte Reversal Macro
*/

#define MSwapByte(data) \
    (UINT8)( ((data & 0x80) >> 7) | ((data & 0x40) >> 5) | ((data & 0x20) >> 3) | ((data & 0x10) >> 1) | \
             ((data & 0x08) << 1) | ((data & 0x04) << 3) | ((data & 0x02) << 5) | ((data & 0x01) << 7) )

/*
   Parallel Port Data Macro

   Bits   Definition
   ----   ----------
   0-3    Data
   4-6    Unit to select (Have 8 choices)
   7      Enable the unit (1 - enable unit, 0 - disable unit)
*/

#define MPortData(data, unit, enable) \
    (UINT8)((data & 0x0F) | ((unit & 0x07) << 4) | ((enable & 0x01) << 7))

/*
  Macro to select the Vcc mode
*/

#define MSelectVccMode(Mode) (Mode == CUtPepLogic5VDCMode ? SEL6 : SEL5)

/*
  Macro to select the VppON mode
*/

#define MEnableVpp(enable) (enable ? VppON : 0)

/*
  Macro to trigger a program pulse
*/

#define MEnableTriggerProgramPulse(enable) (enable ? RDPGM : 0)

/*
  Macro to reset the a program pulse duration
*/

#define MEnableResetProgramPulse(enable) (enable ? PPCL : 0)

#pragma endregion

#pragma region "Structures"

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

typedef struct tagTPepInternalLogicModes
{
    UINT32 nProgrammerMode;
    UINT32 nVccMode;
    UINT32 nPinPulseMode;
    UINT32 nVppMode;
} TPepInternalLogicModes;

typedef struct tagTPepInternalDelaySettings
{
	UINT32 nChipEnableNanoSeconds;   // number of nanoseconds before chip ready
	UINT32 nOutputEnableNanoSeconds; // number of nanoseconds before data available
} TPepInternalDelaySettings;

typedef struct tagTPepInternalLogicData
{
    UINT32 nLastAddress; /* Last address that was set */
	UINT32 nLastOutputEnable; /* Last state of the output enable pin */
	TPepInternalLogicModes Modes;
	TPepInternalDelaySettings DelaySettings;
} TPepInternalLogicData;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#pragma endregion

#pragma region "Local Function Declarations"

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWritePortData(_In_ TUtPepLogicData* pLogicData, _In_ UINT8 ucData, _In_ UINT8 ucUnit);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadByteFromProgrammer(_In_ TUtPepLogicData* pLogicData, _Out_ UINT8* pByte);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteByteToProgrammer(_In_ TUtPepLogicData* pLogicData, _In_ UINT8 ucByte);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lSetProgrammerAddress(_In_ TUtPepLogicData* pLogicData, _In_ UINT32 ulAddress);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lSetProgrammerVppMode(_In_ TUtPepLogicData* pLogicData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static UINT8 lPinPulseModeToData(_In_ TUtPepLogicData* pLogicData, _In_ UINT32 nPinPulseMode);

_IRQL_requires_max_(PASSIVE_LEVEL)
static UINT8 lVppModeToData(_In_ TUtPepLogicData* pLogicData, _In_ UINT32 nVppMode);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lResetProgrammerState(_In_ TUtPepLogicData* pLogicData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lEnableProgrammerReadMode(_In_ TUtPepLogicData* pLogicData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lEnableProgrammerWriteMode(_In_ TUtPepLogicData* pLogicData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWaitForProgramPulse(_In_ TUtPepLogicData* pLogicData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lInitModes(_In_ TPepInternalLogicData* pInternalData);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lInitDelaySettings(_In_ TPepInternalLogicData* pInternalData);

#pragma endregion

#if defined(BUILD_DRIVER_LIB)

#if defined(ALLOC_PRAGMA)

#pragma alloc_text (PAGE, lWritePortData)
#pragma alloc_text (PAGE, lReadByteFromProgrammer)
#pragma alloc_text (PAGE, lWriteByteToProgrammer)
#pragma alloc_text (PAGE, lSetProgrammerAddress)
#pragma alloc_text (PAGE, lSetProgrammerVppMode)
#pragma alloc_text (PAGE, lPinPulseModeToData)
#pragma alloc_text (PAGE, lVppModeToData)
#pragma alloc_text (PAGE, lResetProgrammerState)
#pragma alloc_text (PAGE, lEnableProgrammerReadMode)
#pragma alloc_text (PAGE, lEnableProgrammerWriteMode)
#pragma alloc_text (PAGE, lWaitForProgramPulse)
#pragma alloc_text (PAGE, lInitModes)
#pragma alloc_text (PAGE, lInitDelaySettings)

#pragma alloc_text (PAGE, UtPepLogicAllocLogicContext)
#pragma alloc_text (PAGE, UtPepLogicFreeLogicContext)
#pragma alloc_text (PAGE, UtPepLogicSetProgrammerMode)
#pragma alloc_text (PAGE, UtPepLogicSetVccMode)
#pragma alloc_text (PAGE, UtPepLogicSetPinPulseMode)
#pragma alloc_text (PAGE, UtPepLogicSetVppMode)
#pragma alloc_text (PAGE, UtPepLogicSetAddress)
#pragma alloc_text (PAGE, UtPepLogicSetAddressWithDelay)
#pragma alloc_text (PAGE, UtPepLogicGetData)
#pragma alloc_text (PAGE, UtPepLogicSetData)
#pragma alloc_text (PAGE, UtPepLogicTriggerProgram)
#pragma alloc_text (PAGE, UtPepLogicSetOutputEnable)
#pragma alloc_text (PAGE, UtPepLogicReset)
#pragma alloc_text (PAGE, UtPepLogicSetDelays)
#endif // #if defined(ALLOC_PRAGMA)

#endif // #if defined(BUILD_DRIVER_LIB)

/*
  Local Functions
*/

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWritePortData(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT8 nData,
  _In_ UINT8 nUnit)
{
    BOOLEAN bResult = FALSE;
    UINT8 nTmpData[3];

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

#if defined(PEPLOGIC_ALL_MESSAGES)
    pLogicData->pLogFunc("lWritePortData called.\n");
    pLogicData->pLogFunc("lWritePortData - Writing Data: 0x%X, Unit: 0x%X\n", (ULONG)nData, (ULONG)nUnit);
#endif

    nTmpData[0] = MPortData(nData, nUnit, CUnitOff);
    nTmpData[1] = MPortData(nData, nUnit, CUnitOn);
    nTmpData[2] = MPortData(nData, nUnit, CUnitOff);

    bResult = pLogicData->pWritePortFunc(pLogicData->pvDeviceContext, nTmpData,
                                         MArrayLen(nTmpData), CWaitNanoSeconds);

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadByteFromProgrammer(
  _In_ TUtPepLogicData* pLogicData,
  _Out_ UINT8* pnByte)
{
    UINT8 nBitPosition = 0;
    UINT8 nData = 0;
    UINT8 nTmpData, nPortOutput;
    BOOLEAN bValue;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lReadByteFromProgrammer called.\n");

    *pnByte = 0;

    while (nBitPosition < 8)
    {
        nTmpData = MPortData(nBitPosition, CUnit_DontCare, CUnitOff);

		if (!pLogicData->pWritePortFunc(pLogicData->pvDeviceContext, &nTmpData,
                                        sizeof(nTmpData), CWaitNanoSeconds))
		{
			return FALSE;
		}

		if (!pLogicData->pReadBitPortFunc(pLogicData->pvDeviceContext, &bValue))
        {
            return FALSE;
        }

        nPortOutput = (bValue == TRUE) ? 0x01 : 0x00;

        nPortOutput <<= nBitPosition;

        nData |= nPortOutput;

        ++nBitPosition;
    }

    nData = ~nData;

    pLogicData->pLogFunc("lReadByteFromProgrammer - Retrieved the byte 0x%X\n", (ULONG)nData);

    *pnByte = nData;

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteByteToProgrammer(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT8 nByte)
{
    UINT8 nDataLow = nByte & 0x0F;
    UINT8 nDataHigh = nByte >> 4;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lWriteByteToProgrammer called.\n");

    pLogicData->pLogFunc("lWriteByteToProgrammer - Writing the byte 0x%X\n", (ULONG)nByte);

    if (lWritePortData(pLogicData, nDataLow, CUnit0_DataBits0To3) &&
        lWritePortData(pLogicData, nDataHigh, CUnit1_DataBits4To7))
    {
        return TRUE;
    }

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lSetProgrammerAddress(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nAddress)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    UINT32 nLastAddress = pData->nLastAddress;
	UINT32 nIndex;
    UINT8 nEnable, nData;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lSetProgrammerAddress called.\n");

    pData->nLastAddress = nAddress;
    
    pLogicData->pLogFunc("lSetProgrammerAddress - Setting Programmer Address to 0x%X\n", nAddress);

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address lines 0 - 3\n");

        if (!lWritePortData(pLogicData, (UINT8)nAddress, CUnit2_AddressLines0To3))
        {
            pLogicData->pLogFunc("lSetProgrammerAddress failed.\n");

            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address lines 4 - 7\n");

        if (!lWritePortData(pLogicData, (UINT8)nAddress, CUnit3_AddressLines4To7))
        {
            pLogicData->pLogFunc("lSetProgrammerAddress failed.\n");

            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address lines 8 - 11\n");

        if (!lWritePortData(pLogicData, (UINT8)nAddress, CUnit4_AddressLines8To11))
        {
            pLogicData->pLogFunc("lSetProgrammerAddress failed.\n");

            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if (nLastAddress != nAddress)
    {
        nAddress = MSwapByte(nAddress);
        nData = lVppModeToData(pLogicData, pData->Modes.nVppMode);

	    for (nIndex = 0; nIndex < 8; ++nIndex)
	    {
		    nEnable = (nAddress & 1) ? CAddressLines12To19On : CAddressLines12To19Off;

            pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address line %d\n", nIndex + 12);

            if (!lWritePortData(pLogicData,
                                CDisableAddressLines12To19Unit | nEnable | nData,
                                CUnit5_AddressLines12To19AndVppMode) ||
                !lWritePortData(pLogicData,
                                CEnableAddressLines12To19Unit | nEnable | nData,
                                CUnit5_AddressLines12To19AndVppMode))
            {
                pLogicData->pLogFunc("lSetProgrammerAddress failed.\n");

                return FALSE;
            }

            nAddress >>= 1;
	    }
    }

    pLogicData->pLogFunc("lSetProgrammerAddress finished.\n");

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lSetProgrammerVppMode(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    UINT8 nData;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lSetProgrammerVppMode called.\n");

    nData = lVppModeToData(pLogicData, pData->Modes.nVppMode);

    return lWritePortData(pLogicData, CDisableAddressLines12To19Unit | nData,
                          CUnit5_AddressLines12To19AndVppMode);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static UINT8 lPinPulseModeToData(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nPinPulseMode)
{
#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lPinPulseModeToData called.\n");

    switch (nPinPulseMode)
    {
        case CUtPepLogicPinPulse1Mode:
            return 0;
        case CUtPepLogicPinPulse2Mode:
            return CN0;
        case CUtPepLogicPinPulse3Mode:
            return CN1;
        case CUtPepLogicPinPulse4Mode:
        default:
            return CN0 | CN1;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static UINT8 lVppModeToData(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nVppMode)
{
#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lVppModeToData called.\n");

    switch (nVppMode)
    {
        case CUtPepLogic12VDCVppMode:
            return CEnable12Vpp;
        case CUtPepLogic21VDCVppMode:
            return CEnable21Vpp;
        case CUtPepLogic25VDCVppMode:
            return CEnable25Vpp;
    }

    pLogicData->pLogFunc("lVppModeToData - Unknown Vpp mode - defaulting to +12VDC.\n");

    return CUtPepLogic12VDCVppMode;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lResetProgrammerState(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lResetProgrammerState called.\n");

    if (lWritePortData(pLogicData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(CUtPepLogic5VDCMode) |
                           MEnableVpp(FALSE),
                       CUnit7_Programmer) &&
        lWritePortData(pLogicData, lPinPulseModeToData(pLogicData, pData->Modes.nPinPulseMode),
                       CUnit6_LedAndVpp))
    {
        return TRUE;
    }

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lEnableProgrammerReadMode(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
	LARGE_INTEGER IntervalNanoseconds;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lEnableProgrammerReadMode called.\n");

    if (lWritePortData(pLogicData, lPinPulseModeToData(pLogicData, pData->Modes.nPinPulseMode) | CN2,
                       CUnit6_LedAndVpp) &&
        lWritePortData(pLogicData,
                       MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(pData->Modes.nVccMode) |
                           MEnableVpp(FALSE),
                       CUnit7_Programmer))
    {
		if (pData->DelaySettings.nChipEnableNanoSeconds > 0)
		{
			pLogicData->pLogFunc("lEnableProgrammerReadMode - Chip Enable delay detected.\n");

			IntervalNanoseconds.QuadPart = pData->DelaySettings.nChipEnableNanoSeconds;

			if (!UtSleep(&IntervalNanoseconds))
			{
				pLogicData->pLogFunc("lEnableProgrammerReadMode - Sleep failed.\n");
			}
		}

        return TRUE;
	}

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lEnableProgrammerWriteMode(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
	LARGE_INTEGER IntervalNanoseconds;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lEnableProgrammerWriteMode called.\n");

    if (lWritePortData(pLogicData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(pData->Modes.nVccMode) |
                           MEnableVpp(TRUE),
                       CUnit7_Programmer) &&
        lWritePortData(pLogicData,
                       lPinPulseModeToData(pLogicData, pData->Modes.nPinPulseMode) | CN2 | CN3,
                       CUnit6_LedAndVpp))
    {
		if (pData->DelaySettings.nChipEnableNanoSeconds > 0)
		{
			pLogicData->pLogFunc("lEnableProgrammerWriteMode - Chip Enable delay detected.\n");

			IntervalNanoseconds.QuadPart = pData->DelaySettings.nChipEnableNanoSeconds;

			if (!UtSleep(&IntervalNanoseconds))
			{
				pLogicData->pLogFunc("lEnableProgrammerWriteMode - Sleep failed.\n");
			}
		}
		
		return TRUE;
    }

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWaitForProgramPulse(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    LARGE_INTEGER IntervalNanoseconds;
    UINT8 nData;
    BOOLEAN bValue;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("lWaitForProgramPulse called.\n");

	switch (pData->Modes.nPinPulseMode)
	{
		case CUtPepLogicPinPulse1Mode:
			IntervalNanoseconds.QuadPart = (LONGLONG)MMilliToNanoseconds(1.1); /* 1.1 msec */
			break;
		case CUtPepLogicPinPulse2Mode:
		case CUtPepLogicPinPulse3Mode:
		case CUtPepLogicPinPulse4Mode:
			IntervalNanoseconds.QuadPart = (LONGLONG)MMicroToNanoseconds(250); /* 250 us */
			break;
		default:
			return FALSE;
	}

	pLogicData->pLogFunc("lWaitForProgramPulse - Putting thread to sleep briefly.\n");

	if (!UtSleep(&IntervalNanoseconds))
	{
		return FALSE;
	}

    switch (pData->Modes.nPinPulseMode)
    {
        case CUtPepLogicPinPulse1Mode:
            nData = MPortData(PFIX, CUnit_DontCare, CUnitOff);
            break;
        case CUtPepLogicPinPulse2Mode:
        case CUtPepLogicPinPulse3Mode:
        case CUtPepLogicPinPulse4Mode:
            nData = MPortData(PVAR, CUnit_DontCare, CUnitOff);
            break;
    }

    pLogicData->pLogFunc("lWaitForProgramPulse - Checking if program pulse finished.\n");

    if (pLogicData->pWritePortFunc(pLogicData->pvDeviceContext, &nData,
                                   sizeof(nData), CWaitNanoSeconds) &&
        pLogicData->pReadBitPortFunc(pLogicData->pvDeviceContext, &bValue))
    {
        if (bValue)
        {
            pLogicData->pLogFunc("lWaitForProgramPulse - Program pulse has finished.\n");

            return TRUE;
        }

        pLogicData->pLogFunc("lWaitForProgramPulse - Program pulse did not finish.\n");
    }
    else
    {
        pLogicData->pLogFunc("lWaitForProgramPulse - Unable to retrieve program pulse status.\n");
    }

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lInitModes(
  _In_ TPepInternalLogicData* pInternalData)
{
#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pInternalData->Modes.nProgrammerMode = CUtPepLogicProgrammerNoneMode;
    pInternalData->Modes.nVccMode = CUtPepLogic5VDCMode;
    pInternalData->Modes.nPinPulseMode = CUtPepLogicPinPulse4Mode;
    pInternalData->Modes.nVppMode = CUtPepLogic25VDCVppMode;

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lInitDelaySettings(
  _In_ TPepInternalLogicData* pInternalData)
{
#if defined(BUILD_DRIVER_LIB)
	PAGED_CODE()
#endif

	pInternalData->DelaySettings.nChipEnableNanoSeconds = 0;
	pInternalData->DelaySettings.nOutputEnableNanoSeconds = 0;

	return TRUE;
}

#pragma region "Public Functions"

_IRQL_requires_max_(PASSIVE_LEVEL)
PVOID TUTPEPLOGICAPI UtPepLogicAllocLogicContext()
{
    TPepInternalLogicData* pLogicData;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

#if defined(BUILD_USER_LIB)
    pLogicData = (TPepInternalLogicData*)UtAllocMem(sizeof(TPepInternalLogicData));
#elif defined(BUILD_DRIVER_LIB)
    pLogicData = (TPepInternalLogicData*)UtAllocPagedMem(sizeof(TPepInternalLogicData));
#endif

    pLogicData->nLastAddress = 0xFFFFFFFF;
	pLogicData->nLastOutputEnable = 0xFFFFFFFF;

    lInitModes(pLogicData);
	lInitDelaySettings(pLogicData);

    return pLogicData;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID TUTPEPLOGICAPI UtPepLogicFreeLogicContext(
  _In_ PVOID pvLogicContext)
{
#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

#if defined(BUILD_USER_LIB)
    UtFreeMem(pvLogicContext);
#elif defined(BUILD_DRIVER_LIB)
    UtFreePagedMem(pvLogicContext);
#endif
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetProgrammerMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nProgrammerMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicSetProgrammerMode called.\n");

    switch (nProgrammerMode)
	{
		case CUtPepLogicProgrammerReadMode:
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Setting the programmer to the read mode.\n");

            pData->Modes.nProgrammerMode = nProgrammerMode;

            bResult = lEnableProgrammerReadMode(pLogicData);
            break;
		case CUtPepLogicProgrammerWriteMode:
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Setting the programmer to the write mode.\n");

            pData->Modes.nProgrammerMode = nProgrammerMode;

            bResult = lEnableProgrammerWriteMode(pLogicData);
            break;
        case CUtPepLogicProgrammerNoneMode:
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Setting the programmer to the none mode.\n");

            bResult = lInitModes(pData) &&
				          lInitDelaySettings(pData) &&
                          lResetProgrammerState(pLogicData) &&
                          lSetProgrammerVppMode(pLogicData);
            break;
        default:
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Invalid programmer mode.\n");
            break;
	}

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVccMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nVccMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicSetVccMode called.\n");

    switch (nVccMode)
	{
		case CUtPepLogic5VDCMode:
            pLogicData->pLogFunc("UtPepLogicSetVccMode - Trying to set the programmer to the +5VDC mode.\n");

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
                pLogicData->pLogFunc("UtPepLogicSetVccMode - Programmer not in the none mode.\n");
            }
			break;
		case CUtPepLogic625VDCMode:
            pLogicData->pLogFunc("UtPepLogicSetVccMode - Trying to set the programmer to the +6.25VDC mode.\n");

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
                pLogicData->pLogFunc("UtPepLogicSetVccMode - Programmer not in the none mode.\n");
            }
			break;
        default:
            pLogicData->pLogFunc("UtPepLogicSetVccMode - Invalid VCC mode.\n");
            break;
	}

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetPinPulseMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nPinPulseMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicSetPinPulseMode called.\n");

    switch (nPinPulseMode)
	{
        case CUtPepLogicPinPulse1Mode:
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the VEN08 and WE08 pin mode.\n");

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        case CUtPepLogicPinPulse2Mode:
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the ~VP5 and Vpp16 pin mode.\n");

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        case CUtPepLogicPinPulse3Mode:
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the Vpp32 pin mode.\n");

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        case CUtPepLogicPinPulse4Mode:
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the Vpp64 pin mode.\n");

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        default:
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Invalid pin mode.\n");
            break;
	}

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVppMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nVppMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicSetVppMode called.\n");

    switch (nVppMode)
	{
        case CUtPepLogic12VDCVppMode:
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Setting the programmer to the +12VDC Vpp mode.\n");

            pData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CUtPepLogic21VDCVppMode:
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Setting the programmer to the +21VDC Vpp mode.\n");

            pData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CUtPepLogic25VDCVppMode:
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Setting the programmer to the +25VDC Vpp mode.\n");

            pData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        default:
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Invalid Vpp mode.\n");
            break;
	}

    if (bResult)
    {
        lSetProgrammerVppMode(pLogicData);
    }

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddress(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nAddress)
{
#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicSetAddress called.\n");

    return lSetProgrammerAddress(pLogicData, nAddress);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddressWithDelay(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nAddress,
  _In_ UINT32 nDelayNanoSeconds)
{
	BOOLEAN bResult;
	LARGE_INTEGER IntervalNanoseconds;

#if defined(BUILD_DRIVER_LIB)
	PAGED_CODE()
#endif

	pLogicData->pLogFunc("UtPepLogicSetAddressWithDelay called.\n");

	bResult = lSetProgrammerAddress(pLogicData, nAddress);

	if (bResult && nDelayNanoSeconds > 0)
	{
		pLogicData->pLogFunc("UtPepLogicSetAddressWithDelay - Delay detected.\n");

		IntervalNanoseconds.QuadPart = nDelayNanoSeconds;

		if (!UtSleep(&IntervalNanoseconds))
		{
			pLogicData->pLogFunc("UtPepLogicSetAddressWithDelay - Sleep failed.\n");
		}
	}

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicGetData(
  _In_ TUtPepLogicData* pLogicData,
  _Out_ UINT8* pnData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicGetData called.\n");

    if (pData->Modes.nProgrammerMode != CUtPepLogicProgrammerReadMode)
    {
        pLogicData->pLogFunc("UtPepLogicGetData - Error not in programmer read mode.\n");

        return FALSE;
    }

    return lReadByteFromProgrammer(pLogicData, pnData);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetData(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT8 nData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicSetData called.\n");

    if (pData->Modes.nProgrammerMode != CUtPepLogicProgrammerWriteMode)
    {
        pLogicData->pLogFunc("UtPepLogicSetData - Error not in programmer write mode.\n");

        return FALSE;
    }

    return lWriteByteToProgrammer(pLogicData, nData);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicTriggerProgram(
  _In_ TUtPepLogicData* pLogicData,
  _Out_ PBOOLEAN pbSuccess)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicTriggerProgram called.\n");

    *pbSuccess = FALSE;

    if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerWriteMode)
    {
        /* Disable reset program pulse */

        if (!lWritePortData(pLogicData,
                            MEnableTriggerProgramPulse(FALSE) |
                                MEnableResetProgramPulse(FALSE) |
                                MSelectVccMode(pData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        /* Enable trigger program pulse */

        if (!lWritePortData(pLogicData,
                            MEnableTriggerProgramPulse(TRUE) |
                                MEnableResetProgramPulse(FALSE) |
                                MSelectVccMode(pData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        /* Disable trigger program pulse */

        if (!lWritePortData(pLogicData,
                            MEnableTriggerProgramPulse(FALSE) |
                                MEnableResetProgramPulse(FALSE) |
                                MSelectVccMode(pData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        *pbSuccess = lWaitForProgramPulse(pLogicData);

        /* Disable reset program pulse */

        if (!lWritePortData(pLogicData,
                            MEnableTriggerProgramPulse(FALSE) |
                                MEnableResetProgramPulse(TRUE) |
                                MSelectVccMode(pData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        bResult = TRUE;
    }
    else
    {
        pLogicData->pLogFunc("UtPepLogicTriggerProgram - Programmer not in the write mode.\n");
    }

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetOutputEnable(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nOutputEnable)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
	BOOLEAN bResult, bOutputEnableChanged;
	LARGE_INTEGER IntervalNanoseconds;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicSetOutputEnable called.\n");

    if (pData->Modes.nProgrammerMode != CUtPepLogicProgrammerReadMode)
    {
        pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Error not in programmer read mode.\n");

        return FALSE;
    }

	bOutputEnableChanged = (pData->nLastOutputEnable != nOutputEnable);

	if (bOutputEnableChanged)
	{
		pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Output Enable state being changed.\n");
	}
	else
	{
		pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Output Enable state not being changed.\n");
	}

	bResult = lWritePortData(pLogicData,
                             MEnableTriggerProgramPulse(nOutputEnable) |
                                 MEnableResetProgramPulse(TRUE) |
                                 MSelectVccMode(pData->Modes.nVccMode) |
                                 MEnableVpp(FALSE),
                             CUnit7_Programmer);

	if (bResult)
	{
		pData->nLastOutputEnable = nOutputEnable;
	}

	if (bOutputEnableChanged && bResult && pData->DelaySettings.nOutputEnableNanoSeconds > 0)
	{
		pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Output Enable Delay detected.\n");

		IntervalNanoseconds.QuadPart = pData->DelaySettings.nOutputEnableNanoSeconds;

		if (!UtSleep(&IntervalNanoseconds))
		{
			pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Sleep failed.\n");
		}
	}

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicReset(
  _In_ TUtPepLogicData* pLogicData)
{
    BOOLEAN bResult = FALSE;
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pLogicData->pLogFunc("UtPepLogicReset called.\n");

    if (pData->Modes.nProgrammerMode != CUtPepLogicProgrammerNoneMode)
    {
        if (lInitModes(pData) &&
			lInitDelaySettings(pData) &&
            lResetProgrammerState(pLogicData) &&
            lSetProgrammerAddress(pLogicData, 0) &&
            lSetProgrammerVppMode(pLogicData))
        {
            bResult = TRUE;
        }
    }
    else
    {
        bResult = TRUE;
    }

    if (bResult)
    {
        pData->nLastAddress = 0xFFFFFFFF;
    }

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetDelays(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nChipEnableNanoSeconds,
  _In_ UINT32 nOutputEnableNanoSeconds)
{
	BOOLEAN bResult = FALSE;
	TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(BUILD_DRIVER_LIB)
	PAGED_CODE()
#endif

	pLogicData->pLogFunc("UtPepLogicSetDelays called.\n");

	if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
	{
		pData->DelaySettings.nChipEnableNanoSeconds = nChipEnableNanoSeconds;
		pData->DelaySettings.nOutputEnableNanoSeconds = nOutputEnableNanoSeconds;

		bResult = TRUE;

		pLogicData->pLogFunc("UtPepLogicSetDelays - Delay settings changed.\n");
	}
	else
	{
		pLogicData->pLogFunc("UtPepLogicSetDelays - Delay settings cannot be changed.\n");
	}

	return bResult;
}

#pragma endregion

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
