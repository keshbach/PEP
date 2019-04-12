/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include <Drivers/PepCtrlIOCTL.h>

#include <Includes/UtMacros.h>

#include "PepCtrlPortData.h"
#include "PepCtrlLogic.h"
#include "PepCtrlLog.h"

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
   Unit to select
*/

#define CUnit0_DataBits0To3 0
#define CUnit1_DataBits4To7 1
#define CUnit2_AddressLines0To3 2
#define CUnit3_AddressLines4To7 3
#define CUnit4_AddressLines8To11 4
#define CUnit5_AddressLines12To19AndVppMode 5
#define CUnit6_LedAndVpp 6
#define CUnit7_Programmer 7

#define CUnit_DontCare CUnit0_DataBits0To3

/*
    Constants to enable/disable a unit
*/

#define CUnitOn 1
#define CUnitOff 0

#define CEnableAddressLines12To19Unit 2
#define CDisableAddressLines12To19Unit 0
#define CAddressLines12To19On 1
#define CAddressLines12To19Off 0
#define CEnable12Vpp 4
#define CEnable25Vpp 0
#define CEnable21Vpp 8

#define CN0 1
#define CN1 2
#define CN2 4 /* Turn Socket Power LED On */
#define CN3 8 /* Turn Programming LED On */

#define RDPGM 1 /* enable output enable pin (sets pin to low) */
#define PPCL 2 /* Clear 9602 */
#define VppON 4 /* enable Vpp */
#define SEL5 8 /* set Vcc to +5 VDC */
#define SEL6 0 /* set Vcc to +6.25 VDC */

#define PVAR 0
#define PFIX 1

/*
  Macro to select the Vcc mode
*/

#define MSelectVccMode(Mode) (Mode == CPepCtrl5VDCMode ? SEL6 : SEL5)

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

/*
  Local Function Prototypes
*/

static BOOLEAN lWritePortData(IN TPepCtrlPortData* pPortData, IN UINT8 ucData, IN UINT8 ucUnit);
static BOOLEAN lReadByteFromProgrammer(IN TPepCtrlPortData* pPortData, UINT8* pByte);
static BOOLEAN lWriteByteToProgrammer(IN TPepCtrlPortData* pPortData, IN UINT8 ucByte);
static BOOLEAN lSetProgrammerAddress(IN TPepCtrlPortData* pPortData, IN UINT32 ulAddress);
static BOOLEAN lSetProgrammerVppMode(IN TPepCtrlPortData* pPortData);
static UINT8 lPinPulseModeToData(IN UINT32 nPinPulseMode);
static UINT8 lVppModeToData(IN UINT32 nVppMode);
static BOOLEAN lResetProgrammerState(IN TPepCtrlPortData* pPortData);
static BOOLEAN lEnableProgrammerReadMode(IN TPepCtrlPortData* pPortData);
static BOOLEAN lEnableProgrammerWriteMode(IN TPepCtrlPortData* pPortData);
static BOOLEAN lWaitForProgramPulse(IN TPepCtrlPortData* pPortData);

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

#pragma alloc_text (PAGE, PepCtrlSetProgrammerMode)
#pragma alloc_text (PAGE, PepCtrlSetVccMode)
#pragma alloc_text (PAGE, PepCtrlSetPinPulseMode)
#pragma alloc_text (PAGE, PepCtrlSetVppMode)
#pragma alloc_text (PAGE, PepCtrlSetAddress)
#pragma alloc_text (PAGE, PepCtrlGetData)
#pragma alloc_text (PAGE, PepCtrlSetData)
#pragma alloc_text (PAGE, PepCtrlTriggerProgram)
#pragma alloc_text (PAGE, PepCtrlSetOutputEnable)
#pragma alloc_text (PAGE, PepCtrlReset)
#pragma alloc_text (PAGE, PepCtrlInitModes)
#endif

/*
  Local Functions
*/

static BOOLEAN lWritePortData(
  IN TPepCtrlPortData* pPortData,
  IN UINT8 nData,
  IN UINT8 nUnit)
{
    BOOLEAN bResult = FALSE;
    UINT8 nTmpData[3];

    PAGED_CODE()

#if defined(PEPCTRL_ALL_MESSAGES)
    PepCtrlLog("lWritePortData called.\n");
    PepCtrlLog("lWritePortData - Writing Data: 0x%X, Unit: 0x%X\n", (ULONG)nData, (ULONG)nUnit);
#endif

    if (!pPortData->bPortEjected)
    {
        nTmpData[0] = MPortData(nData, nUnit, CUnitOff);
        nTmpData[1] = MPortData(nData, nUnit, CUnitOn);
        nTmpData[2] = MPortData(nData, nUnit, CUnitOff);

        bResult = pPortData->Funcs.pWritePortFunc(&pPortData->Object, nTmpData,
                                                  MArrayLen(nTmpData));
    }
    else
    {
#if defined(PEPCTRL_ALL_MESSAGES)
        PepCtrlLog("lWritePortData - No data written because the port was ejected.\n");
#endif
    }

    return bResult;
}

static BOOLEAN lReadByteFromProgrammer(
  IN TPepCtrlPortData* pPortData,
  UINT8* pnByte)
{
    UINT8 nBitPosition = 0;
    UINT8 nData = 0;
    UINT8 nTmpData, nPortOutput;
    BOOLEAN bValue;

    PAGED_CODE()

    PepCtrlLog("lReadByteFromProgrammer called.\n");

    *pnByte = 0;

    if (pPortData->bPortEjected)
    {
        PepCtrlLog("lReadByteFromProgrammer - No data read because port was ejected.\n");

        return FALSE;
    }

    while (nBitPosition < 8)
    {
        nTmpData = MPortData(nBitPosition, CUnit_DontCare, CUnitOff);

        if (!pPortData->Funcs.pWritePortFunc(&pPortData->Object, &nTmpData,
                                            sizeof(nTmpData)) ||
            !pPortData->Funcs.pReadBitPortFunc(&pPortData->Object, &bValue))
        {
            return FALSE;
        }

        nPortOutput = (bValue == TRUE) ? 0x01 : 0x00;

        nPortOutput <<= nBitPosition;

        nData |= nPortOutput;

        ++nBitPosition;
    }

    nData = ~nData;

    PepCtrlLog("lReadByteFromProgrammer - Retrieved the byte 0x%X\n", (ULONG)nData);

    *pnByte = nData;

    return TRUE;
}

static BOOLEAN lWriteByteToProgrammer(
  IN TPepCtrlPortData* pPortData,
  IN UINT8 nByte)
{
    UINT8 nDataLow = nByte & 0x0F;
    UINT8 nDataHigh = nByte >> 4;

    PAGED_CODE()

    PepCtrlLog("lWriteByteToProgrammer called.\n");

    PepCtrlLog("lWriteByteToProgrammer - Writing the byte 0x%X\n", (ULONG)nByte);

    if (lWritePortData(pPortData, nDataLow, CUnit0_DataBits0To3) &&
        lWritePortData(pPortData, nDataHigh, CUnit1_DataBits4To7))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOLEAN lSetProgrammerAddress(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nAddress)
{
    UINT32 nLastAddress = pPortData->nLastAddress;
	UINT32 nIndex;
    UINT8 nEnable, nData;

    PAGED_CODE()

    PepCtrlLog("lSetProgrammerAddress called.\n");

    pPortData->nLastAddress = nAddress;
    
    PepCtrlLog("lSetProgrammerAddress - Setting Programmer Address to 0x%X\n", nAddress);

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        PepCtrlLog("lSetProgrammerAddress - Setting Address lines 0 - 3\n");

        if (!lWritePortData(pPortData, (UINT8)nAddress, CUnit2_AddressLines0To3))
        {
            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        PepCtrlLog("lSetProgrammerAddress - Setting Address lines 4 - 7\n");

        if (!lWritePortData(pPortData, (UINT8)nAddress, CUnit3_AddressLines4To7))
        {
            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        PepCtrlLog("lSetProgrammerAddress - Setting Address lines 8 - 11\n");

        if (!lWritePortData(pPortData, (UINT8)nAddress, CUnit4_AddressLines8To11))
        {
            return FALSE;
        }
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if (nLastAddress != nAddress)
    {
        nAddress = MSwapByte(nAddress);
        nData = lVppModeToData(pPortData->Modes.nVppMode);

	    for (nIndex = 0; nIndex < 8; ++nIndex)
	    {
		    nEnable = (nAddress & 1) ? CAddressLines12To19On : CAddressLines12To19Off;

            PepCtrlLog("lSetProgrammerAddress - Setting Address line %d\n", nIndex + 12);

            if (!lWritePortData(pPortData,
                                CDisableAddressLines12To19Unit | nEnable | nData,
                                CUnit5_AddressLines12To19AndVppMode) ||
                !lWritePortData(pPortData,
                                CEnableAddressLines12To19Unit | nEnable | nData,
                                CUnit5_AddressLines12To19AndVppMode))
            {
                return FALSE;
            }

            nAddress >>= 1;
	    }
    }

    return TRUE;
}

static BOOLEAN lSetProgrammerVppMode(
  IN TPepCtrlPortData* pPortData)
{
    UINT8 nData;

    PAGED_CODE()

    PepCtrlLog("lSetProgrammerVppMode called.\n");

    nData = lVppModeToData(pPortData->Modes.nVppMode);

    return lWritePortData(pPortData, CDisableAddressLines12To19Unit | nData,
                          CUnit5_AddressLines12To19AndVppMode);
}

static UINT8 lPinPulseModeToData(
  IN UINT32 nPinPulseMode)
{
    PAGED_CODE()

    PepCtrlLog("lPinPulseModeToData called.\n");

    switch (nPinPulseMode)
    {
        case CPepCtrlPinPulse1Mode:
            return 0;
        case CPepCtrlPinPulse2Mode:
            return CN0;
        case CPepCtrlPinPulse3Mode:
            return CN1;
        case CPepCtrlPinPulse4Mode:
        default:
            return CN0 | CN1;
    }
}

static UINT8 lVppModeToData(
  IN UINT32 nVppMode)
{
    PAGED_CODE()

    PepCtrlLog("lVppModeToData called.\n");

    switch (nVppMode)
    {
        case CPepCtrl12VDCVppMode:
            return CEnable12Vpp;
        case CPepCtrl21VDCVppMode:
            return CEnable21Vpp;
        case CPepCtrl25VDCVppMode:
            return CEnable25Vpp;
    }

    PepCtrlLog("lVppModeToData - Unknown Vpp mode - defaulting to +12VDC.\n");

    return CPepCtrl12VDCVppMode;
}

static BOOLEAN lResetProgrammerState(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

    PepCtrlLog("lResetProgrammerState called.\n");

    if (lWritePortData(pPortData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(CPepCtrl5VDCMode) |
                           MEnableVpp(FALSE),
                       CUnit7_Programmer) &&
        lWritePortData(pPortData, lPinPulseModeToData(pPortData->Modes.nPinPulseMode),
                       CUnit6_LedAndVpp))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOLEAN lEnableProgrammerReadMode(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

    PepCtrlLog("lEnableProgrammerReadMode called.\n");

    if (lWritePortData(pPortData, lPinPulseModeToData(pPortData->Modes.nPinPulseMode) | CN2,
                       CUnit6_LedAndVpp) &&
        lWritePortData(pPortData,
                       MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(pPortData->Modes.nVccMode) |
                           MEnableVpp(FALSE),
                       CUnit7_Programmer))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOLEAN lEnableProgrammerWriteMode(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

    PepCtrlLog("lEnableProgrammerWriteMode called.\n");

    if (lWritePortData(pPortData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(pPortData->Modes.nVccMode) |
                           MEnableVpp(TRUE),
                       CUnit7_Programmer) &&
        lWritePortData(pPortData,
                       lPinPulseModeToData(pPortData->Modes.nPinPulseMode) | CN2 | CN3,
                       CUnit6_LedAndVpp))
    {
        return TRUE;
    }

    return FALSE;
}

static BOOLEAN lWaitForProgramPulse(
  IN TPepCtrlPortData* pPortData)
{
    NTSTATUS status;
    LARGE_INTEGER Interval;
    UINT8 nData;
    BOOLEAN bValue;

    PAGED_CODE()

    PepCtrlLog("lWaitForProgramPulse called.\n");

    switch (pPortData->Modes.nPinPulseMode)
    {
        case CPepCtrlPinPulse1Mode:
            Interval.QuadPart = -11000; /* 1.1 msec */
            break;
        case CPepCtrlPinPulse2Mode:
        case CPepCtrlPinPulse3Mode:
        case CPepCtrlPinPulse4Mode:
            Interval.QuadPart = -3; /* 250 us */
            break;
        default:
            return FALSE;
    }

    PepCtrlLog("lWaitForProgramPulse - Delaying thread execution.\n");

    status = KeDelayExecutionThread(KernelMode, FALSE, &Interval);

    if (STATUS_SUCCESS != status)
    {
        PepCtrlLog("lWaitForProgramPulse - Thread execution delayed has failed.  (0x%X)\n", status);

        return FALSE;
    }

    switch (pPortData->Modes.nPinPulseMode)
    {
        case CPepCtrlPinPulse1Mode:
            nData = MPortData(PFIX, CUnit_DontCare, CUnitOff);
            break;
        case CPepCtrlPinPulse2Mode:
        case CPepCtrlPinPulse3Mode:
        case CPepCtrlPinPulse4Mode:
            nData = MPortData(PVAR, CUnit_DontCare, CUnitOff);
            break;
    }

    PepCtrlLog("lWaitForProgramPulse - Checking if program pulse finished.\n");

    if (pPortData->Funcs.pWritePortFunc(&pPortData->Object, &nData,
                                        sizeof(nData)) &&
        pPortData->Funcs.pReadBitPortFunc(&pPortData->Object, &bValue))
    {
        if (bValue)
        {
            PepCtrlLog("lWaitForProgramPulse - Program pulse has finished.\n");

            return TRUE;
        }

        PepCtrlLog("lWaitForProgramPulse - Program pulse did not finish.\n");
    }
    else
    {
        PepCtrlLog("lWaitForProgramPulse - Unable to retrieve program pulse status.\n");
    }

    return FALSE;
}

BOOLEAN PepCtrlSetProgrammerMode(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nProgrammerMode)
{
    BOOLEAN bResult = FALSE;

    PAGED_CODE()

    PepCtrlLog("PepCtrlSetProgrammerMode called.\n");

    switch (nProgrammerMode)
	{
		case CPepCtrlProgrammerReadMode:
            PepCtrlLog("PepCtrlSetProgrammerMode - Setting the programmer to the read mode.\n");

            pPortData->Modes.nProgrammerMode = nProgrammerMode;

            bResult = lEnableProgrammerReadMode(pPortData);
            break;
		case CPepCtrlProgrammerWriteMode:
            PepCtrlLog("PepCtrlSetProgrammerMode - Setting the programmer to the write mode.\n");

            pPortData->Modes.nProgrammerMode = nProgrammerMode;

            bResult = lEnableProgrammerWriteMode(pPortData);
            break;
        case CPepCtrlProgrammerNoneMode:
            PepCtrlLog("PepCtrlSetProgrammerMode - Setting the programmer to the none mode.\n");

            bResult = PepCtrlInitModes(pPortData) &&
                          lResetProgrammerState(pPortData) &&
                          lSetProgrammerVppMode(pPortData);
            break;
        default:
            PepCtrlLog("PepCtrlSetProgrammerMode - Invalid programmer mode.\n");
            break;
	}

    return bResult;
}

BOOLEAN PepCtrlSetVccMode(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nVccMode)
{
    BOOLEAN bResult = FALSE;

    PAGED_CODE()

    PepCtrlLog("PepCtrlSetVccMode called.\n");

    switch (nVccMode)
	{
		case CPepCtrl5VDCMode:
            PepCtrlLog("PepCtrlSetVccMode - Trying to set the programmer to the +5VDC mode.\n");

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("PepCtrlSetVccMode - Programmer not in the none mode.\n");
            }
			break;
		case CPepCtrl625VDCMode:
            PepCtrlLog("PepCtrlSetVccMode - Trying to set the programmer to the +6.25VDC mode.\n");

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
                PepCtrlLog("PepCtrlSetVccMode - Programmer not in the none mode.\n");
            }
			break;
        default:
            PepCtrlLog("PepCtrlSetVccMode - Invalid VCC mode.\n");
            break;
	}

	return bResult;
}

BOOLEAN PepCtrlSetPinPulseMode(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nPinPulseMode)
{
    BOOLEAN bResult = FALSE;

    PAGED_CODE()

    PepCtrlLog("PepCtrlSetPinPulseMode called.\n");

    switch (nPinPulseMode)
	{
        case CPepCtrlPinPulse1Mode:
            PepCtrlLog("PepCtrlSetPinPulseMode - Trying to set the programmer to the VEN08 and WE08 pin mode.\n");

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                PepCtrlLog("PepCtrlSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        case CPepCtrlPinPulse2Mode:
            PepCtrlLog("PepCtrlSetPinPulseMode - Trying to set the programmer to the ~VP5 and Vpp16 pin mode.\n");

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                PepCtrlLog("PepCtrlSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        case CPepCtrlPinPulse3Mode:
            PepCtrlLog("PepCtrlSetPinPulseMode - Trying to set the programmer to the Vpp32 pin mode.\n");

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                PepCtrlLog("PepCtrlSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        case CPepCtrlPinPulse4Mode:
            PepCtrlLog("PepCtrlSetPinPulseMode - Trying to set the programmer to the Vpp64 pin mode.\n");

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
                PepCtrlLog("PepCtrlSetPinPulseMode - Programmer not in the none mode.\n");
            }
            break;
        default:
            PepCtrlLog("PepCtrlSetPinPulseMode - Invalid pin mode.\n");
            break;
	}

	return bResult;
}

BOOLEAN PepCtrlSetVppMode(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nVppMode)
{
    BOOLEAN bResult = FALSE;

    PAGED_CODE()

    PepCtrlLog("PepCtrlSetVppMode called.\n");

    switch (nVppMode)
	{
        case CPepCtrl12VDCVppMode:
            PepCtrlLog("PepCtrlSetVppMode - Setting the programmer to the +12VDC Vpp mode.\n");

            pPortData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CPepCtrl21VDCVppMode:
            PepCtrlLog("PepCtrlSetVppMode - Setting the programmer to the +21VDC Vpp mode.\n");

            pPortData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CPepCtrl25VDCVppMode:
            PepCtrlLog("PepCtrlSetVppMode - Setting the programmer to the +25VDC Vpp mode.\n");

            pPortData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        default:
            PepCtrlLog("PepCtrlSetVppMode - Invalid Vpp mode.\n");
            break;
	}

    if (bResult)
    {
        lSetProgrammerVppMode(pPortData);
    }

	return bResult;
}

BOOLEAN PepCtrlSetAddress(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nAddress)
{
    PepCtrlLog("PepCtrlSetAddress called.\n");

    PAGED_CODE()

    return lSetProgrammerAddress(pPortData, nAddress);
}

BOOLEAN PepCtrlGetData(
  IN TPepCtrlPortData* pPortData,
  OUT UINT8* pnData)
{
    PepCtrlLog("PepCtrlGetData called.\n");

    PAGED_CODE()

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerReadMode)
    {
        PepCtrlLog("PepCtrlGetData - Error not in programmer read mode.\n");

        return FALSE;
    }

    return lReadByteFromProgrammer(pPortData, pnData);
}

BOOLEAN PepCtrlSetData(
  IN TPepCtrlPortData* pPortData,
  IN UINT8 nData)
{
    PepCtrlLog("PepCtrlSetData called.\n");

    PAGED_CODE()

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerWriteMode)
    {
        PepCtrlLog("PepCtrlSetData - Error not in programmer write mode.\n");

        return FALSE;
    }

    return lWriteByteToProgrammer(pPortData, nData);
}

BOOLEAN PepCtrlTriggerProgram(
  IN TPepCtrlPortData* pPortData,
  OUT PBOOLEAN pbSuccess)
{
    BOOLEAN bResult = FALSE;

    PAGED_CODE()

    PepCtrlLog("PepCtrlTriggerProgram called.\n");

    *pbSuccess = FALSE;

    if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerWriteMode)
    {
        /* Disable reset program pulse */

        if (!lWritePortData(pPortData,
                            MEnableTriggerProgramPulse(FALSE) |
                                MEnableResetProgramPulse(FALSE) |
                                MSelectVccMode(pPortData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        /* Enable trigger program pulse */

        if (!lWritePortData(pPortData,
                            MEnableTriggerProgramPulse(TRUE) |
                                MEnableResetProgramPulse(FALSE) |
                                MSelectVccMode(pPortData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        /* Disable trigger program pulse */

        if (!lWritePortData(pPortData,
                            MEnableTriggerProgramPulse(FALSE) |
                                MEnableResetProgramPulse(FALSE) |
                                MSelectVccMode(pPortData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        *pbSuccess = lWaitForProgramPulse(pPortData);

        /* Disable reset program pulse */

        if (!lWritePortData(pPortData,
                            MEnableTriggerProgramPulse(FALSE) |
                                MEnableResetProgramPulse(TRUE) |
                                MSelectVccMode(pPortData->Modes.nVccMode) |
                                MEnableVpp(TRUE),
                            CUnit7_Programmer))
        {
            return FALSE;
        }

        bResult = TRUE;
    }
    else
    {
        PepCtrlLog("PepCtrlTriggerProgram - Programmer not in the write mode.\n");
    }

    return bResult;
}

BOOLEAN PepCtrlSetOutputEnable(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nOutputEnable)
{
    PepCtrlLog("PepCtrlSetOutputEnable called.\n");

    PAGED_CODE()

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerReadMode)
    {
        PepCtrlLog("PepCtrlSetOutputEnable - Error not in programmer read mode.\n");

        return FALSE;
    }

    return lWritePortData(pPortData,
                          MEnableTriggerProgramPulse(nOutputEnable) |
                              MEnableResetProgramPulse(TRUE) |
                              MSelectVccMode(pPortData->Modes.nVccMode) |
                              MEnableVpp(FALSE),
                          CUnit7_Programmer);
}

BOOLEAN PepCtrlReset(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

    PepCtrlLog("PepCtrlReset called.\n");

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerNoneMode)
    {
        if (PepCtrlInitModes(pPortData) &&
            lResetProgrammerState(pPortData) &&
            lSetProgrammerAddress(pPortData, 0) &&
            lSetProgrammerVppMode(pPortData))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN PepCtrlInitModes(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

    PepCtrlLog("PepCtrlInitModes called.\n");

    pPortData->Modes.nProgrammerMode = CPepCtrlProgrammerNoneMode;
    pPortData->Modes.nVccMode = CPepCtrl5VDCMode;
    pPortData->Modes.nPinPulseMode = CPepCtrlPinPulse4Mode;
    pPortData->Modes.nVppMode = CPepCtrl25VDCVppMode;

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
