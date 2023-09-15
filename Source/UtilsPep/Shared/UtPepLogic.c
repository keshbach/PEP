/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
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
#elif defined(__XC8) || defined(__18CXX)
#include "UtPortTypeDefs.h"
#include "UtSleepFirmware.h"
#else
#error Unsupported configuration
#endif

#include <Includes/UtMacros.h>

#include <UtilsPep/UtPepLogic.h>

#pragma region "Programmer Protocol"

/*
	Only the data and status register are used by the programmer.


	A command is written to the programmer through the data register and there
	are eight units that can be addressed and each units can accept a nibble of data.

	Bits   Definition
	----   ---------------------------------
	0-3    Data
	4-6    Unit to address
	7      Trigger the unit to load the data

	Unit   Definition
	----   -----------------------------------------------
	0      Sets lower nibble of data to program (bits 0-3)
	1      Sets upper nibble of data to program (bits 4-7)
	2      Sets address lines A0-A3
	3      Sets address lines A4-A7
	4      Sets address lines A8-A11
	5      Shift in address lines A12-A19 and set the Vpp voltage to either +12VDC, +21VDC or +25VDC
	6
	7


	Data is read from the programmer one bit through the busy bit of the status register.
*/

#pragma endregion

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

#if defined(ENABLE_LOGGING) 
#if defined(BUILD_USER_LIB)
#define MCurrentThreadId() (UINT_PTR)GetCurrentThreadId()
#elif defined(BUILD_DRIVER_LIB)
#define MCurrentThreadId() PsGetCurrentThread()
#elif defined(__XC8)
#else
#error Unsupported configuration
#endif
#endif

/*
    Byte Reversal Macro
*/

#define MSwapByte(data) \
    (UINT8)( ((data & 0x80) >> 7) | ((data & 0x40) >> 5) | ((data & 0x20) >> 3) | ((data & 0x10) >> 1) | \
             ((data & 0x08) << 1) | ((data & 0x04) << 3) | ((data & 0x02) << 5) | ((data & 0x01) << 7) )

/*
   Parallel Port Data Macro

   Bits   Definition
   ----   ---------------------------------
   0-3    Data
   4-6    Unit to address
   7      Trigger the unit to load the data
*/

#define MPortData(data, unit, trigger) \
    (UINT8)((data & 0x0F) | ((unit & 0x07) << 4) | ((trigger & 0x01) << 7))

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
#elif defined(__XC8) || defined(__18CXX)
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
#elif defined(__XC8) || defined(__18CXX)
#else
#error Need to specify how to restore original structure padding
#endif

#pragma endregion

#if defined(__XC8) || defined(__18CXX)
#pragma udata PepLogicDataSectionName
static TPepInternalLogicData l_InternalLogicData;
#endif

#pragma region "Local Function Declarations"

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWritePortData(_In_ TUtPepLogicData* pLogicData, _In_ UINT8 nData, _In_ UINT8 nUnit);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lReadByteFromProgrammer(_In_ TUtPepLogicData* pLogicData, _Out_ UINT8* pnByte);

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteByteToProgrammer(_In_ TUtPepLogicData* pLogicData, _In_ UINT8 nByte);

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

#if defined(__XC8) || defined(__18CXX)
#pragma code
#endif

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

#if defined(ENABLE_LOGGING) 
#if defined(PEPLOGIC_ALL_MESSAGES)
	pLogicData->pLogFunc("lWritePortData entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

#if defined(ENABLE_LOGGING) 
#if defined(PEPLOGIC_ALL_MESSAGES)
    pLogicData->pLogFunc("lWritePortData entering.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif
#endif

    nTmpData[0] = MPortData(nData, nUnit, CUnitOff);
    nTmpData[1] = MPortData(nData, nUnit, CUnitOn);
    nTmpData[2] = MPortData(nData, nUnit, CUnitOff);

    bResult = pLogicData->pWritePortFunc(
#if defined(ENABLE_DEVICE_CONTEXT)
                  pLogicData->pvDeviceContext,
#endif
                  nTmpData, MArrayLen(nTmpData), CWaitNanoSeconds);

#if defined(ENABLE_LOGGING) 
#if defined(PEPLOGIC_ALL_MESSAGES)
	pLogicData->pLogFunc("lWritePortData leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif
#endif

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

#if defined(ENABLE_LOGGING) 
    pLogicData->pLogFunc("lReadByteFromProgrammer entering.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
	PAGED_CODE()
#endif

    *pnByte = 0;

    while (nBitPosition < 8)
    {
        nTmpData = MPortData(nBitPosition, CUnit_DontCare, CUnitOff);

		if (!pLogicData->pWritePortFunc(
#if defined(ENABLE_DEVICE_CONTEXT)
                pLogicData->pvDeviceContext,
#endif
                &nTmpData, sizeof(nTmpData), CWaitNanoSeconds))
		{
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lReadByteFromProgrammer leaving.  (Write port failed)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

			return FALSE;
		}

		if (!pLogicData->pReadBitPortFunc(
#if defined(ENABLE_DEVICE_CONTEXT)
                pLogicData->pvDeviceContext,
#endif
                &bValue))
        {
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lReadByteFromProgrammer leaving.  (Read bit from port failed)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif
			
			return FALSE;
        }

        nPortOutput = (bValue == TRUE) ? 0x01 : 0x00;

        nPortOutput <<= nBitPosition;

        nData |= nPortOutput;

        ++nBitPosition;
    }

    nData = ~nData;

#if defined(ENABLE_LOGGING) 
    pLogicData->pLogFunc("lReadByteFromProgrammer - Retrieved the byte 0x%X.  (Thread: 0x%p)\n",
		                 (ULONG)nData, MCurrentThreadId());
#endif

    *pnByte = nData;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lReadByteFromProgrammer leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWriteByteToProgrammer(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT8 nByte)
{
    UINT8 nDataLow = nByte & 0x0F;
    UINT8 nDataHigh = nByte >> 4;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lWriteByteToProgrammer entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

#if defined(ENABLE_LOGGING) 
    pLogicData->pLogFunc("lWriteByteToProgrammer - Writing the byte 0x%X.  (Thread: 0x%p)\n",
		                 (ULONG)nByte, MCurrentThreadId());
#endif

    if (lWritePortData(pLogicData, nDataLow, CUnit0_DataBits0To3) &&
        lWritePortData(pLogicData, nDataHigh, CUnit1_DataBits4To7))
    {
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("lWriteByteToProgrammer leaving.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
		
		return TRUE;
    }

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lWriteByteToProgrammer leaving.  (Write port data failed)  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

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

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lSetProgrammerAddress entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    pData->nLastAddress = nAddress;
    
#if defined(ENABLE_LOGGING) 
    pLogicData->pLogFunc("lSetProgrammerAddress - Setting Programmer Address to 0x%X.  (Thread: 0x%p)\n",
		                 nAddress, MCurrentThreadId());
#endif

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address lines 0 - 3.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

        if (!lWritePortData(pLogicData, (UINT8)nAddress, CUnit2_AddressLines0To3))
        {
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lSetProgrammerAddress leaving.  (Write port data failed)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address lines 4 - 7.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

        if (!lWritePortData(pLogicData, (UINT8)nAddress, CUnit3_AddressLines4To7))
        {
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lSetProgrammerAddress leaving.  (Write port data failed)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address lines 8 - 11.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

        if (!lWritePortData(pLogicData, (UINT8)nAddress, CUnit4_AddressLines8To11))
        {
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lSetProgrammerAddress leaving.  (Write port data failed)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

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

#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("lSetProgrammerAddress - Setting Address line %d.  (Thread: 0x%p)\n",
				                 nIndex + 12, MCurrentThreadId());
#endif

            if (!lWritePortData(pLogicData,
                                CDisableAddressLines12To19Unit | nEnable | nData,
                                CUnit5_AddressLines12To19AndVppMode) ||
                !lWritePortData(pLogicData,
                                CEnableAddressLines12To19Unit | nEnable | nData,
                                CUnit5_AddressLines12To19AndVppMode))
            {
#if defined(ENABLE_LOGGING) 
				pLogicData->pLogFunc("lSetProgrammerAddress leaving.  (Write port data failed)  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif

                return FALSE;
            }

            nAddress >>= 1;
	    }
    }

#if defined(ENABLE_LOGGING) 
    pLogicData->pLogFunc("lSetProgrammerAddress leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return TRUE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lSetProgrammerVppMode(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    UINT8 nData;
	BOOLEAN bResult;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lSetProgrammerVppMode entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    nData = lVppModeToData(pLogicData, pData->Modes.nVppMode);

	bResult = lWritePortData(pLogicData, CDisableAddressLines12To19Unit | nData,
                             CUnit5_AddressLines12To19AndVppMode);

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lSetProgrammerVppMode leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static UINT8 lPinPulseModeToData(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nPinPulseMode)
{
	UINT8 nData;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lPinPulseModeToData entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    switch (nPinPulseMode)
    {
        case CUtPepLogicPinPulse1Mode:
            nData = 0;
			break;
        case CUtPepLogicPinPulse2Mode:
            nData = CN0;
			break;
        case CUtPepLogicPinPulse3Mode:
            nData = CN1;
			break;
        case CUtPepLogicPinPulse4Mode:
			nData = CN0 | CN1;
			break;
        default:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lPinPulseModeToData - Unknown pin pulse mode: 0x%X.  (Thread: 0x%p)\n",
				                 nPinPulseMode, MCurrentThreadId());
#endif

			nData = CN0 | CN1;
			break;
    }

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lPinPulseModeToData leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return nData;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static UINT8 lVppModeToData(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nVppMode)
{
	UINT8 nData;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lVppModeToData entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    switch (nVppMode)
    {
        case CUtPepLogic12VDCVppMode:
            nData = CEnable12Vpp;
			break;
        case CUtPepLogic21VDCVppMode:
            nData = CEnable21Vpp;
			break;
        case CUtPepLogic25VDCVppMode:
            nData = CEnable25Vpp;
			break;
		default:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lVppModeToData - Unknown Vpp mode: 0x%X.  (Thread: 0x%p)\n",
				                 nVppMode, MCurrentThreadId());
#endif

			nData = CUtPepLogic12VDCVppMode;
			break;
    }

#if defined(ENABLE_LOGGING) 
    pLogicData->pLogFunc("lVppModeToData leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return nData;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lResetProgrammerState(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lResetProgrammerState entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    if (lWritePortData(pLogicData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(CUtPepLogic5VDCMode) |
                           MEnableVpp(FALSE),
                       CUnit7_Programmer) &&
        lWritePortData(pLogicData, lPinPulseModeToData(pLogicData, pData->Modes.nPinPulseMode),
                       CUnit6_LedAndVpp))
    {
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("lResetProgrammerState leaving.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

		return TRUE;
    }

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lResetProgrammerState leaving.  (Write failed)  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lEnableProgrammerReadMode(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lEnableProgrammerReadMode entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

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
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lEnableProgrammerReadMode - Chip Enable delay detected.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

			if (!UtSleep(pData->DelaySettings.nChipEnableNanoSeconds))
			{
#if defined(ENABLE_LOGGING) 
				pLogicData->pLogFunc("lEnableProgrammerReadMode - Sleep failed.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
			}
		}

#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("lEnableProgrammerReadMode leaving.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

        return TRUE;
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lEnableProgrammerReadMode leaving.  (Write port data failed)  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lEnableProgrammerWriteMode(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lEnableProgrammerWriteMode entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

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
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lEnableProgrammerWriteMode - Chip Enable delay detected.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

			if (!UtSleep(pData->DelaySettings.nChipEnableNanoSeconds))
			{
#if defined(ENABLE_LOGGING) 
				pLogicData->pLogFunc("lEnableProgrammerWriteMode - Sleep failed.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
			}
		}

#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("lEnableProgrammerWriteMode leaving.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

		return TRUE;
    }

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lEnableProgrammerWriteMode leaving.  (Write port data failed)  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return FALSE;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lWaitForProgramPulse(
  _In_ TUtPepLogicData* pLogicData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    UINT32 nIntervalNanoseconds;
    UINT8 nData;
    BOOLEAN bValue;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lWaitForProgramPulse entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

	switch (pData->Modes.nPinPulseMode)
	{
		case CUtPepLogicPinPulse1Mode:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lWaitForProgramPulse - Using a 1.1 msecs pulse.  (Thread: 0x%p)\n",
                                 MCurrentThreadId());
#endif

			nIntervalNanoseconds = (UINT32)MMilliToNanoseconds(1.1); /* 1.1 msec */
			break;
		case CUtPepLogicPinPulse2Mode:
		case CUtPepLogicPinPulse3Mode:
		case CUtPepLogicPinPulse4Mode:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lWaitForProgramPulse - Using a 250 us pulse.  (Thread: 0x%p)\n",
                                 MCurrentThreadId());
#endif

			nIntervalNanoseconds = (UINT32)MMicroToNanoseconds(250); /* 250 us */
			break;
		default:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lWaitForProgramPulse leaving.  (Cannot determine sleep delay because of unknown pin pulse mode: 0x%X)  (Thread: 0x%p)\n",
				                 pData->Modes.nPinPulseMode, MCurrentThreadId());
#endif

			return FALSE;
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lWaitForProgramPulse - Putting thread to sleep briefly.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	if (!UtSleep(nIntervalNanoseconds))
	{
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("lWaitForProgramPulse leaving.  (Sleep failed)  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

		return FALSE;
	}

    switch (pData->Modes.nPinPulseMode)
    {
        case CUtPepLogicPinPulse1Mode:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lWaitForProgramPulse - Preparing to read the PFIX value.  (Thread: 0x%p)\n",
                                 MCurrentThreadId());
#endif

            nData = MPortData(PFIX, CUnit_DontCare, CUnitOff);
            break;
        case CUtPepLogicPinPulse2Mode:
        case CUtPepLogicPinPulse3Mode:
        case CUtPepLogicPinPulse4Mode:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lWaitForProgramPulse - Preparing to read the PVAR value.  (Thread: 0x%p)\n",
                                 MCurrentThreadId());
#endif
			
			nData = MPortData(PVAR, CUnit_DontCare, CUnitOff);
            break;
		default:
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("lWaitForProgramPulse leaving.  (Cannot determine data to write because of unknown pin pulse mode: 0x%X)  (Thread: 0x%p)\n",
				                 pData->Modes.nPinPulseMode, MCurrentThreadId());
#endif

			return FALSE;
	}

#if defined(ENABLE_LOGGING) 
    pLogicData->pLogFunc("lWaitForProgramPulse - Checking if program pulse finished.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    if (pLogicData->pWritePortFunc(
#if defined(ENABLE_DEVICE_CONTEXT)
            pLogicData->pvDeviceContext,
#endif
            &nData, sizeof(nData), CWaitNanoSeconds) &&
        pLogicData->pReadBitPortFunc(
#if defined(ENABLE_DEVICE_CONTEXT)
            pLogicData->pvDeviceContext,
#endif
            &bValue))
    {
        if (!bValue)
        {
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("lWaitForProgramPulse - Program pulse has finished.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
			pLogicData->pLogFunc("lWaitForProgramPulse leaving.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            return TRUE;
        }

#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("lWaitForProgramPulse - Program pulse did not finish.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
    }
    else
    {
#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("lWaitForProgramPulse - Unable to retrieve program pulse status.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
    }

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("lWaitForProgramPulse leaving.  (Failure occurred)  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

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
PVOID TUTPEPLOGICAPI UtPepLogicAllocLogicContext(VOID)
{
    TPepInternalLogicData* pLogicData;

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

#if defined(BUILD_USER_LIB)
    pLogicData = (TPepInternalLogicData*)UtAllocMem(sizeof(TPepInternalLogicData));
#elif defined(BUILD_DRIVER_LIB)
    pLogicData = (TPepInternalLogicData*)UtAllocPagedMem(sizeof(TPepInternalLogicData));
#elif defined(__XC8) || defined(__18CXX)
    pLogicData = &l_InternalLogicData;
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
#elif defined(__XC8)
#endif
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetProgrammerMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nProgrammerMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetProgrammerMode entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    switch (nProgrammerMode)
	{
		case CUtPepLogicProgrammerReadMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Setting the programmer to the read mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            pData->Modes.nProgrammerMode = nProgrammerMode;

            bResult = lEnableProgrammerReadMode(pLogicData);
            break;
		case CUtPepLogicProgrammerWriteMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Setting the programmer to the write mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            pData->Modes.nProgrammerMode = nProgrammerMode;

            bResult = lEnableProgrammerWriteMode(pLogicData);
            break;
        case CUtPepLogicProgrammerNoneMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Setting the programmer to the none mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            bResult = lInitModes(pData) &&
				          lInitDelaySettings(pData) &&
                          lResetProgrammerState(pLogicData) &&
                          lSetProgrammerVppMode(pLogicData);
            break;
        default:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetProgrammerMode - Invalid programmer mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif
            break;
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetProgrammerMode leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVccMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nVccMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetVccMode entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    switch (nVccMode)
	{
		case CUtPepLogic5VDCMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetVccMode - Trying to set the programmer to the +5VDC mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
#if defined(ENABLE_LOGGING) 
                pLogicData->pLogFunc("UtPepLogicSetVccMode - Programmer not in the none mode.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
            }
			break;
		case CUtPepLogic625VDCMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetVccMode - Trying to set the programmer to the +6.25VDC mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
#if defined(ENABLE_LOGGING) 
                pLogicData->pLogFunc("UtPepLogicSetVccMode - Programmer not in the none mode.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
            }
			break;
        default:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetVccMode - Invalid VCC mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif
            break;
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetVccMode leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetPinPulseMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nPinPulseMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetPinPulseMode entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    switch (nPinPulseMode)
	{
        case CUtPepLogicPinPulse1Mode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the VEN08 and WE08 pin mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
#if defined(ENABLE_LOGGING) 
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
            }
            break;
        case CUtPepLogicPinPulse2Mode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the ~VP5 and Vpp16 pin mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
#if defined(ENABLE_LOGGING) 
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
            }
            break;
        case CUtPepLogicPinPulse3Mode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the Vpp32 pin mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
#if defined(ENABLE_LOGGING) 
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
            }
            break;
        case CUtPepLogicPinPulse4Mode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Trying to set the programmer to the Vpp64 pin mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
            {
                pData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
#if defined(ENABLE_LOGGING) 
                pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Programmer not in the none mode.  (Thread: 0x%p)\n",
					                 MCurrentThreadId());
#endif
            }
            break;
        default:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetPinPulseMode - Invalid pin mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif
            break;
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetPinPulseMode leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVppMode(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nVppMode)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetVppMode entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    switch (nVppMode)
	{
        case CUtPepLogic12VDCVppMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Setting the programmer to the +12VDC Vpp mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            pData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CUtPepLogic21VDCVppMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Setting the programmer to the +21VDC Vpp mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            pData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CUtPepLogic25VDCVppMode:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Setting the programmer to the +25VDC Vpp mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            pData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        default:
#if defined(ENABLE_LOGGING) 
            pLogicData->pLogFunc("UtPepLogicSetVppMode - Invalid Vpp mode.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif
            break;
	}

    if (bResult)
    {
        lSetProgrammerVppMode(pLogicData);
    }

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetVppMode leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddress(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nAddress)
{
	BOOLEAN bResult;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetAddress entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

	bResult = lSetProgrammerAddress(pLogicData, nAddress);

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetAddress leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddressWithDelay(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nAddress,
  _In_ UINT32 nDelayNanoSeconds)
{
	BOOLEAN bResult;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetAddressWithDelay entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
	PAGED_CODE()
#endif

	bResult = lSetProgrammerAddress(pLogicData, nAddress);

	if (bResult && nDelayNanoSeconds > 0)
	{
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("UtPepLogicSetAddressWithDelay - Delay detected.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

		if (!UtSleep(nDelayNanoSeconds))
		{
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("UtPepLogicSetAddressWithDelay - Sleep failed.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif
		}
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetAddressWithDelay leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicGetData(
  _In_ TUtPepLogicData* pLogicData,
  _Out_ UINT8* pnData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
	BOOLEAN bResult;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicGetData entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    if (pData->Modes.nProgrammerMode != CUtPepLogicProgrammerReadMode)
    {
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("UtPepLogicGetData leaving.  (Not in programmer read mode)  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

        return FALSE;
    }

    bResult = lReadByteFromProgrammer(pLogicData, pnData);

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicGetData leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetData(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT8 nData)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
	BOOLEAN bResult;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetData entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    if (pData->Modes.nProgrammerMode != CUtPepLogicProgrammerWriteMode)
    {
#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("UtPepLogicSetData leaving. (Not in programmer write mode)  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

        return FALSE;
    }

	bResult = lWriteByteToProgrammer(pLogicData, nData);

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetData leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicTriggerProgram(
  _In_ TUtPepLogicData* pLogicData,
  _Out_ PBOOLEAN pbSuccess)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
    BOOLEAN bResult = FALSE;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicTriggerProgram entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

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
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("UtPepLogicTriggerProgram leaving.  (Failed to disable reset program pulse)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

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
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("UtPepLogicTriggerProgram leaving.  (Failed to enable trigger program pulse)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

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
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("UtPepLogicTriggerProgram leaving.  (Failed to disable trigger program pulse)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

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
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("UtPepLogicTriggerProgram leaving.  (Failed to disable reset program pulse)  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif

            return FALSE;
        }

        bResult = TRUE;
    }
    else
    {
#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("UtPepLogicTriggerProgram - Programmer not in the write mode.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
    }

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicTriggerProgram leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicSetOutputEnable(
  _In_ TUtPepLogicData* pLogicData,
  _In_ UINT32 nOutputEnable)
{
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;
	BOOLEAN bResult, bOutputEnableChanged;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetOutputEnable entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

    if (pData->Modes.nProgrammerMode != CUtPepLogicProgrammerReadMode)
    {
#if defined(ENABLE_LOGGING) 
        pLogicData->pLogFunc("UtPepLogicSetOutputEnable leaving.  (Not in programmer read mode)  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

        return FALSE;
    }

	bOutputEnableChanged = (pData->nLastOutputEnable != nOutputEnable);

	if (bOutputEnableChanged)
	{
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Output Enable state being changed.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
	}
	else
	{
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Output Enable state not being changed.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
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
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Output Enable Delay detected.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif

		if (!UtSleep(pData->DelaySettings.nOutputEnableNanoSeconds))
		{
#if defined(ENABLE_LOGGING) 
			pLogicData->pLogFunc("UtPepLogicSetOutputEnable - Sleep failed.  (Thread: 0x%p)\n",
				                 MCurrentThreadId());
#endif
		}
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetOutputEnable leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN TUTPEPLOGICAPI UtPepLogicReset(
  _In_ TUtPepLogicData* pLogicData)
{
    BOOLEAN bResult = FALSE;
    TPepInternalLogicData* pData = (TPepInternalLogicData*)pLogicData->pvLogicContext;

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicReset entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
    PAGED_CODE()
#endif

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

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicReset leaving.  (Thread: 0x%p)\n",
		                 MCurrentThreadId());
#endif

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

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetDelays entering.  (Thread: 0x%p)\n",
                         MCurrentThreadId());
#endif

#if defined(BUILD_DRIVER_LIB)
	PAGED_CODE()
#endif

	if (pData->Modes.nProgrammerMode == CUtPepLogicProgrammerNoneMode)
	{
		pData->DelaySettings.nChipEnableNanoSeconds = nChipEnableNanoSeconds;
		pData->DelaySettings.nOutputEnableNanoSeconds = nOutputEnableNanoSeconds;

		bResult = TRUE;

#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("UtPepLogicSetDelays - Delay settings changed.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
	}
	else
	{
#if defined(ENABLE_LOGGING) 
		pLogicData->pLogFunc("UtPepLogicSetDelays - Delay settings cannot be changed.  (Thread: 0x%p)\n",
			                 MCurrentThreadId());
#endif
	}

#if defined(ENABLE_LOGGING) 
	pLogicData->pLogFunc("UtPepLogicSetDelays leaving  (Thread: 0x%p).\n",
		                 MCurrentThreadId());
#endif

	return bResult;
}

#pragma endregion

/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/
