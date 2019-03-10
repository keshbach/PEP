/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include <Drivers/PepCtrlIOCTL.h>

#include <Includes/UtMacros.h>

#include "PepCtrlPortData.h"
#include "PepCtrlLogic.h"

/*
    Byte Reversal Macro
*/

#define MSwapByte(data) \
    (UCHAR)( ((data & 0x80) >> 7) | ((data & 0x40) >> 5) | ((data & 0x20) >> 3) | ((data & 0x10) >> 1) | \
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
    (UCHAR)((data & 0x0F) | ((unit & 0x07) << 4) | ((enable & 0x01) << 7))

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

static VOID lWritePortData(IN TPepCtrlPortData* pPortData, IN UCHAR ucData, IN UCHAR ucUnit);
static UCHAR lReadByteFromProgrammer(IN TPepCtrlPortData* pPortData);
static VOID lWriteByteToProgrammer(IN TPepCtrlPortData* pPortData, IN UCHAR ucByte);
static VOID lSetProgrammerAddress(IN TPepCtrlPortData* pPortData, IN UINT32 ulAddress);
static VOID lSetProgrammerVppMode(IN TPepCtrlPortData* pPortData);
static UCHAR lPinPulseModeToData(IN UINT32 nPinPulseMode);
static UCHAR lVppModeToData(IN UINT32 nVppMode);
static VOID lResetProgrammerState(IN TPepCtrlPortData* pPortData);
static VOID lEnableProgrammerReadMode(IN TPepCtrlPortData* pPortData);
static VOID lEnableProgrammerWriteMode(IN TPepCtrlPortData* pPortData);
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

static VOID lWritePortData(
  IN TPepCtrlPortData* pPortData,
  IN UCHAR ucData,
  IN UCHAR ucUnit)
{
    UCHAR ucTmpData[3];

    PAGED_CODE()

#if defined(PEPCTRL_ALL_MESSAGES)
    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lWritePortData called.\n") );
	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Writing Data: 0x%X, Unit: 0x%X\n", (ULONG)ucData,
               (ULONG)ucUnit) );
#endif

    if (!pPortData->bPortEjected)
    {
        ucTmpData[0] = MPortData(ucData, ucUnit, CUnitOff);
        ucTmpData[1] = MPortData(ucData, ucUnit, CUnitOn);
        ucTmpData[2] = MPortData(ucData, ucUnit, CUnitOff);

        pPortData->Funcs.pWritePortFunc(&pPortData->Object, ucTmpData,
                                        MArrayLen(ucTmpData));
    }
    else
    {
#if defined(PEPCTRL_ALL_MESSAGES)
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		           "PepCtrl: No data written because the port was ejected.\n") );
#endif
    }
}

static UCHAR lReadByteFromProgrammer(
  IN TPepCtrlPortData* pPortData)
{
    UCHAR ucBitPosition = 0;
    UCHAR ucData = 0;
    UCHAR ucTmpData, ucPortOutput;
    BOOLEAN bValue;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: lReadByteFromProgrammer called.\n") );

    if (pPortData->bPortEjected)
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		           "PepCtrl: No data read because port was ejected.\n") );

        return 0;
    }

    while (ucBitPosition < 8)
    {
        ucTmpData = MPortData(ucBitPosition, CUnit_DontCare, CUnitOff);

        pPortData->Funcs.pWritePortFunc(&pPortData->Object, &ucTmpData,
                                        sizeof(ucTmpData));

        pPortData->Funcs.pReadBitPortFunc(&pPortData->Object, &bValue);

        ucPortOutput = (bValue == TRUE) ? 0x01 : 0x00;

        ucPortOutput <<= ucBitPosition;

        ucData |= ucPortOutput;

        ++ucBitPosition;
    }

    ucData = ~ucData;

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Retrieved the byte 0x%X\n", (ULONG)ucData) );

    return ucData;
}

static VOID lWriteByteToProgrammer(
  IN TPepCtrlPortData* pPortData,
  IN UCHAR ucByte)
{
    UCHAR ucDataLow = ucByte & 0x0F;
    UCHAR ucDataHigh = ucByte >> 4;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		       "PepCtrl: lWriteByteToProgrammer called.\n") );

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Writing the byte 0x%X\n", (ULONG)ucByte) );

    lWritePortData(pPortData, ucDataLow, CUnit0_DataBits0To3);
    lWritePortData(pPortData, ucDataHigh, CUnit1_DataBits4To7);
}

static VOID lSetProgrammerAddress(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nAddress)
{
    UINT32 nLastAddress = pPortData->nLastAddress;
	UINT32 nIndex;
    UCHAR ucEnable, ucData;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lSetProgrammerAddress called.\n") );

    pPortData->nLastAddress = nAddress;
    
    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
               "PepCtrl: Setting Programmer Address to 0x%X\n", nAddress) );

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Setting Address lines 0 - 3\n") );

        lWritePortData(pPortData, (UCHAR)nAddress, CUnit2_AddressLines0To3);
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Setting Address lines 4 - 7\n") );

        lWritePortData(pPortData, (UCHAR)nAddress, CUnit3_AddressLines4To7);
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if ((nLastAddress & 0x0F) != (nAddress & 0x0F))
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Setting Address lines 8 - 11\n") );

        lWritePortData(pPortData, (UCHAR)nAddress, CUnit4_AddressLines8To11);
    }

    nLastAddress >>= 4;
    nAddress >>= 4;

    if (nLastAddress != nAddress)
    {
        nAddress = MSwapByte(nAddress);
        ucData = lVppModeToData(pPortData->Modes.nVppMode);

	    for (nIndex = 0; nIndex < 8; ++nIndex)
	    {
		    ucEnable = (nAddress & 1) ? CAddressLines12To19On : CAddressLines12To19Off;

            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting Address line %d\n", nIndex + 12) );

            lWritePortData(pPortData,
                           CDisableAddressLines12To19Unit | ucEnable | ucData,
                           CUnit5_AddressLines12To19AndVppMode);

            lWritePortData(pPortData,
                           CEnableAddressLines12To19Unit | ucEnable | ucData,
                           CUnit5_AddressLines12To19AndVppMode);

            nAddress >>= 1;
	    }
    }
}

static VOID lSetProgrammerVppMode(
  IN TPepCtrlPortData* pPortData)
{
    UCHAR ucData;

    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lSetProgrammerVppMode called.\n") );

    ucData = lVppModeToData(pPortData->Modes.nVppMode);

    lWritePortData(pPortData, CDisableAddressLines12To19Unit | ucData,
                   CUnit5_AddressLines12To19AndVppMode);
}

static UCHAR lPinPulseModeToData(
  IN UINT32 nPinPulseMode)
{
    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lPinPulseModeToData called.\n") );

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

static UCHAR lVppModeToData(
  IN UINT32 nVppMode)
{
    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lVppModeToData called.\n") );

    switch (nVppMode)
    {
        case CPepCtrl12VDCVppMode:
            return CEnable12Vpp;
        case CPepCtrl21VDCVppMode:
            return CEnable21Vpp;
        case CPepCtrl25VDCVppMode:
            return CEnable25Vpp;
    }

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: Unknown Vpp mode - defaulting to +12VDC.\n") );

    return CPepCtrl12VDCVppMode;
}

static VOID lResetProgrammerState(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lResetProgrammerState called.\n") );

    lWritePortData(pPortData,
                   MEnableTriggerProgramPulse(FALSE) |
                       MEnableResetProgramPulse(TRUE) |
                       MSelectVccMode(CPepCtrl5VDCMode) |
                       MEnableVpp(FALSE),
                   CUnit7_Programmer);

    lWritePortData(pPortData, lPinPulseModeToData(pPortData->Modes.nPinPulseMode),
                   CUnit6_LedAndVpp);
}

static VOID lEnableProgrammerReadMode(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lEnableProgrammerReadMode called.\n") );

    lWritePortData(pPortData, lPinPulseModeToData(pPortData->Modes.nPinPulseMode) | CN2,
                   CUnit6_LedAndVpp);

    lWritePortData(pPortData,
                   MEnableResetProgramPulse(TRUE) | 
                       MSelectVccMode(pPortData->Modes.nVccMode) |
                       MEnableVpp(FALSE),
                   CUnit7_Programmer);
}

static VOID lEnableProgrammerWriteMode(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lEnableProgrammerWriteMode called.\n") );

    lWritePortData(pPortData,
                   MEnableTriggerProgramPulse(FALSE) |
                       MEnableResetProgramPulse(TRUE) |
                       MSelectVccMode(pPortData->Modes.nVccMode) |
                       MEnableVpp(TRUE),
                   CUnit7_Programmer);

    lWritePortData(pPortData,
                   lPinPulseModeToData(pPortData->Modes.nPinPulseMode) | CN2 | CN3,
                   CUnit6_LedAndVpp);
}

static BOOLEAN lWaitForProgramPulse(
  IN TPepCtrlPortData* pPortData)
{
    NTSTATUS status;
    LARGE_INTEGER Interval;
    UCHAR ucData;
    BOOLEAN bValue;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: lWaitForProgramPulse called.\n") );

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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: Delaying thread execution.\n") );

    status = KeDelayExecutionThread(KernelMode, FALSE, &Interval);

    if (STATUS_SUCCESS != status)
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                    "PepCtrl: Thread execution delayed has failed.  (0x%X)\n",
                    status) );

        return FALSE;
    }

    switch (pPortData->Modes.nPinPulseMode)
    {
        case CPepCtrlPinPulse1Mode:
            ucData = MPortData(PFIX, CUnit_DontCare, CUnitOff);
            break;
        case CPepCtrlPinPulse2Mode:
        case CPepCtrlPinPulse3Mode:
        case CPepCtrlPinPulse4Mode:
            ucData = MPortData(PVAR, CUnit_DontCare, CUnitOff);
            break;
    }

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: Checking if program pulse finished.\n") );

    pPortData->Funcs.pWritePortFunc(&pPortData->Object, &ucData,
                                    sizeof(ucData));

    if (pPortData->Funcs.pReadBitPortFunc(&pPortData->Object, &bValue))
    {
        if (bValue)
        {
	        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	                   "PepCtrl: Program pulse has finished.\n") );

            return TRUE;
        }

	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	               "PepCtrl: Program pulse did not finish.\n") );
    }
    else
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	               "PepCtrl: Unable to retrieve program pulse status.\n") );
    }

    return FALSE;
}

BOOLEAN PepCtrlSetProgrammerMode(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nProgrammerMode)
{
    BOOLEAN bResult = FALSE;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlSetProgrammerMode called.\n") );

    switch (nProgrammerMode)
	{
		case CPepCtrlProgrammerReadMode:
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting the programmer to the read mode.\n") );

            pPortData->Modes.nProgrammerMode = nProgrammerMode;

            lEnableProgrammerReadMode(pPortData);

            bResult = TRUE;
            break;
		case CPepCtrlProgrammerWriteMode:
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting the programmer to the write mode.\n") );

            pPortData->Modes.nProgrammerMode = nProgrammerMode;

            lEnableProgrammerWriteMode(pPortData);

            bResult = TRUE;
            break;
        case CPepCtrlProgrammerNoneMode:
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting the programmer to the none mode.\n") );

            PepCtrlInitModes(pPortData);

            lResetProgrammerState(pPortData);
            lSetProgrammerVppMode(pPortData);

            bResult = TRUE;
            break;
        default:
       	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Invalid programmer mode.\n") );
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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlSetVccMode called.\n") );

    switch (nVccMode)
	{
		case CPepCtrl5VDCMode:
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Trying to set the programmer to the +5VDC mode.\n") );

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
            	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                           "PepCtrl: Programmer not in the none mode.\n") );
            }
			break;
		case CPepCtrl625VDCMode:
        	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Trying to set the programmer to the +6.25VDC mode.\n") );

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nVccMode = nVccMode;

                bResult = TRUE;
            }
            else
            {
            	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                           "PepCtrl: Programmer not in the none mode.\n") );
            }
			break;
        default:
       	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Invalid VCC mode.\n") );
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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlSetPinPulseMode called.\n") );

    switch (nPinPulseMode)
	{
        case CPepCtrlPinPulse1Mode:
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Trying to set the programmer to the VEN08 and WE08 pin mode.\n") );

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
            	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                            "PepCtrl: Programmer not in the none mode.\n") );
            }
            break;
        case CPepCtrlPinPulse2Mode:
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Trying to set the programmer to the ~VP5 and Vpp16 pin mode.\n") );

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
            	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                            "PepCtrl: Programmer not in the none mode.\n") );
            }
            break;
        case CPepCtrlPinPulse3Mode:
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Trying to set the programmer to the Vpp32 pin mode.\n") );

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
            	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                            "PepCtrl: Programmer not in the none mode.\n") );
            }
            break;
        case CPepCtrlPinPulse4Mode:
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                        "PepCtrl: Trying to set the programmer to the Vpp64 pin mode.\n") );

            if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerNoneMode)
            {
                pPortData->Modes.nPinPulseMode = nPinPulseMode;

			    bResult = TRUE;
            }
            else
            {
            	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                            "PepCtrl: Programmer not in the none mode.\n") );
            }
            break;
        default:
       	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Invalid pin mode.\n") );
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

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlSetVppMode called.\n") );

    switch (nVppMode)
	{
        case CPepCtrl12VDCVppMode:
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting the programmer to the +12VDC Vpp mode.\n") );

            pPortData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CPepCtrl21VDCVppMode:
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting the programmer to the +21VDC Vpp mode.\n") );

            pPortData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        case CPepCtrl25VDCVppMode:
            KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Setting the programmer to the +25VDC Vpp mode.\n") );

            pPortData->Modes.nVppMode = nVppMode;

		    bResult = TRUE;
            break;
        default:
       	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                       "PepCtrl: Invalid Vpp mode.\n") );
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
    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlSetAddress called.\n") );

    PAGED_CODE()

    lSetProgrammerAddress(pPortData, nAddress);

    return TRUE;
}

BOOLEAN PepCtrlGetData(
  IN TPepCtrlPortData* pPortData,
  OUT UCHAR* pucData)
{
	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlGetData called.\n") );

    PAGED_CODE()

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerReadMode)
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	               "PepCtrl: Error not in programmer read mode.\n") );

        return FALSE;
    }

    *pucData = lReadByteFromProgrammer(pPortData);

    return TRUE;
}

BOOLEAN PepCtrlSetData(
  IN TPepCtrlPortData* pPortData,
  IN UCHAR ucData)
{
	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlSetData called.\n") );

    PAGED_CODE()

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerWriteMode)
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	               "PepCtrl: Error not in programmer write mode.\n") );

        return FALSE;
    }

    lWriteByteToProgrammer(pPortData, ucData);

    return TRUE;
}

BOOLEAN PepCtrlTriggerProgram(
  IN TPepCtrlPortData* pPortData,
  OUT PBOOLEAN pbSuccess)
{
    BOOLEAN bResult = FALSE;

    PAGED_CODE()

	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlTriggerProgram called.\n") );

    *pbSuccess = FALSE;

    if (pPortData->Modes.nProgrammerMode == CPepCtrlProgrammerWriteMode)
    {
        /* Disable reset program pulse */

        lWritePortData(pPortData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(FALSE) |
                           MSelectVccMode(pPortData->Modes.nVccMode) |
                           MEnableVpp(TRUE),
                       CUnit7_Programmer);

        /* Enable trigger program pulse */

        lWritePortData(pPortData,
                       MEnableTriggerProgramPulse(TRUE) |
                           MEnableResetProgramPulse(FALSE) |
                           MSelectVccMode(pPortData->Modes.nVccMode) |
                           MEnableVpp(TRUE),
                       CUnit7_Programmer);

        /* Disable trigger program pulse */

        lWritePortData(pPortData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(FALSE) |
                           MSelectVccMode(pPortData->Modes.nVccMode) |
                           MEnableVpp(TRUE),
                       CUnit7_Programmer);

        *pbSuccess = lWaitForProgramPulse(pPortData);

        /* Disable reset program pulse */

        lWritePortData(pPortData,
                       MEnableTriggerProgramPulse(FALSE) |
                           MEnableResetProgramPulse(TRUE) |
                           MSelectVccMode(pPortData->Modes.nVccMode) |
                           MEnableVpp(TRUE),
                       CUnit7_Programmer);

        bResult = TRUE;
    }
    else
    {
        KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                   "PepCtrl: Programmer not in the write mode.\n") );
    }

    return bResult;
}

BOOLEAN PepCtrlSetOutputEnable(
  IN TPepCtrlPortData* pPortData,
  IN UINT32 nOutputEnable)
{
	KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlSetOutputEnable called.\n") );

    PAGED_CODE()

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerReadMode)
    {
	    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	               "PepCtrl: Error not in programmer read mode.\n") );

        return FALSE;
    }

    lWritePortData(pPortData,
                   MEnableTriggerProgramPulse(nOutputEnable) |
                       MEnableResetProgramPulse(TRUE) |
                       MSelectVccMode(pPortData->Modes.nVccMode) |
                       MEnableVpp(FALSE),
                   CUnit7_Programmer);

    return TRUE;
}

BOOLEAN PepCtrlReset(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlReset called.\n") );

    if (pPortData->Modes.nProgrammerMode != CPepCtrlProgrammerNoneMode)
    {
        PepCtrlInitModes(pPortData);

        lResetProgrammerState(pPortData);

        lSetProgrammerAddress(pPortData, 0);

        lSetProgrammerVppMode(pPortData);
    }

    return TRUE;
}

BOOLEAN PepCtrlInitModes(
  IN TPepCtrlPortData* pPortData)
{
    PAGED_CODE()

    KdPrintEx( (DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
	           "PepCtrl: PepCtrlInitModes called.\n") );

    pPortData->Modes.nProgrammerMode = CPepCtrlProgrammerNoneMode;
    pPortData->Modes.nVccMode = CPepCtrl5VDCMode;
    pPortData->Modes.nPinPulseMode = CPepCtrlPinPulse4Mode;
    pPortData->Modes.nVppMode = CPepCtrl25VDCVppMode;

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2013 Kevin Eshbach                                  */
/***************************************************************************/
