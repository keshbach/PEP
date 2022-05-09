/***************************************************************************/
/*  Copyright (C) 2021-2022 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>

#include "usb.h"

#include "UtPortTypeDefs.h"

#include "UtSleepFirmware.h"

#include <UtilsPep/UtPepLogic.h>

#include <Firmware/PepFirmwareDefs.h>

#include <Includes/UtMacros.h>

/* Process Command State Constants */

#define	CProcessCommandStateIdle 0x00
#define CProcessCommandStateNotIdle 0x01

/* Local variables */

#pragma romdata
static rom UINT8 l_nVersion[] = "18F14K50 1.00";

#pragma udata SomeSectionName1
static UINT8 l_nBootState;
static TUtPepLogicData l_PepLogicData;

#pragma udata PepCommandDataSectionName
static TUtPepCommandData l_CommandData;

#pragma udata PepResponseDataSectionName
static TUtPepResponseData l_ResponseData;

/* Local Functions */

#pragma code

static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(
  _In_ PVOID pvContext,
  _Out_ PBOOLEAN pbValue)
{
    // Must invert the bit to match how the Busy bit of the parallel port works
    
    *pbValue = PORTBbits.RB4 ? FALSE : TRUE;

    return TRUE;
}
    
static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(
  _In_ PVOID pvContext,
  _In_ PUCHAR pucData,
  _In_ ULONG ulDataLen,
  _In_ ULONG nWaitNanoSeconds)
{
    ULONG ulDataIndex;
    LARGE_INTEGER Interval;
   
    Interval.u.HighPart = 0;
    Interval.u.LowPart = nWaitNanoSeconds;

    for (ulDataIndex = 0; ulDataIndex < ulDataLen; ++ulDataIndex)
    {
        LATC = pucData[ulDataIndex];
        
        UtSleep(&Interval);
    }

    return TRUE;
}
 
static void lProcessVersionCommand(void)
{
    UINT8 nIndex;

    l_ResponseData.ErrorCode = CPepErrorSuccess;
    
    for (nIndex = 0; nIndex < MArrayLen(l_nVersion); ++nIndex)
    {
        l_ResponseData.Data.Version[nIndex] = l_nVersion[nIndex];
    }
}

static void lProcessResetCommand(void)
{
    l_ResponseData.ErrorCode = UtPepLogicReset(&l_PepLogicData) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetProgrammerModeCommand(void)
{
    l_ResponseData.ErrorCode = UtPepLogicSetProgrammerMode(&l_PepLogicData,
                                                           l_CommandData.Data.nProgrammerMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetVccModeCommand(void)
{
    l_ResponseData.ErrorCode = UtPepLogicSetVccMode(&l_PepLogicData,
                                                    l_CommandData.Data.nVccMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetPinPulseModeCommand(void)
{
    l_ResponseData.ErrorCode = UtPepLogicSetPinPulseMode(&l_PepLogicData,
                                                         l_CommandData.Data.nPinPulseMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetVppModeCommand(void)
{
    l_ResponseData.ErrorCode = UtPepLogicSetVppMode(&l_PepLogicData,
                                                    l_CommandData.Data.nVppMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessReadDataCommand(void)
{
    UINT8 nIndex;

    if (l_CommandData.Data.ReadData.nDataLen > MArrayLen(l_ResponseData.Data.nData))
    {
        l_ResponseData.ErrorCode = CPepErrorInvalidLength;
        
        return;
    }
    
    for (nIndex = 0; nIndex < l_CommandData.Data.ReadData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetOutputEnable(&l_PepLogicData, FALSE) ||
            !UtPepLogicSetAddress(&l_PepLogicData,
                                  l_CommandData.Data.ReadData.nAddress + nIndex) ||
            !UtPepLogicSetOutputEnable(&l_PepLogicData, TRUE) ||
            !UtPepLogicGetData(&l_PepLogicData,
                               &l_ResponseData.Data.nData[nIndex]))
        {
            l_ResponseData.ErrorCode = CPepErrorFailed;
        
            return;
        }
    }

    l_ResponseData.ErrorCode = CPepErrorSuccess;
}

static void lProcessReadUserDataCommand(void)
{
    UINT8 nIndex;

    if (l_CommandData.Data.ReadUserData.nDataLen > MArrayLen(l_CommandData.Data.ReadUserData.Data))
    {
        l_ResponseData.ErrorCode = CPepErrorInvalidLength;
        
        return;
    }

    for (nIndex = 0; nIndex < l_CommandData.Data.ReadUserData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddress(&l_PepLogicData,
                                  l_CommandData.Data.ReadUserData.Data[nIndex].nAddress) ||
            !UtPepLogicSetOutputEnable(&l_PepLogicData,
                                       l_CommandData.Data.ReadUserData.Data[nIndex].nEnableOutputEnable))
        {
            l_ResponseData.ErrorCode = CPepErrorFailed;
        
            return;
        }
        
        if (l_CommandData.Data.ReadUserData.Data[nIndex].nPerformRead)
        {
            if (!UtPepLogicGetData(&l_PepLogicData,
                                   &l_ResponseData.Data.nData[nIndex]))
            {
                l_ResponseData.ErrorCode = CPepErrorFailed;
        
                return;
            }
        }
    }    

    l_ResponseData.ErrorCode = CPepErrorSuccess;
}

static void lProcessReadUserDataWithDelayCommand(void)
{
    UINT8 nIndex;

    if (l_CommandData.Data.ReadUserDataWithDelay.nDataLen > MArrayLen(l_CommandData.Data.ReadUserDataWithDelay.Data))
    {
        l_ResponseData.ErrorCode = CPepErrorInvalidLength;
        
        return;
    }

    for (nIndex = 0; nIndex < l_CommandData.Data.ReadUserDataWithDelay.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddressWithDelay(&l_PepLogicData,
                                           l_CommandData.Data.ReadUserDataWithDelay.Data[nIndex].nAddress,
                                           l_CommandData.Data.ReadUserDataWithDelay.Data[nIndex].nDelayNanoSeconds))
        {
            l_ResponseData.ErrorCode = CPepErrorFailed;
        
            return;
        }

        if (l_CommandData.Data.ReadUserDataWithDelay.Data[nIndex].nPerformRead)
        {
            if (!UtPepLogicGetData(&l_PepLogicData,
                                   &l_ResponseData.Data.nData[nIndex]))
            {
                l_ResponseData.ErrorCode = CPepErrorFailed;
        
                return;
            }
        }
    }    

    l_ResponseData.ErrorCode = CPepErrorSuccess;
}

static void lProcessProgramDataCommand(void)
{
    UINT8 nIndex;
    BOOLEAN bSuccess;

    if (l_CommandData.Data.ProgramData.nDataLen > MArrayLen(l_CommandData.Data.ProgramData.nData))
    {
        l_ResponseData.ErrorCode = CPepErrorInvalidLength;
        
        return;
    }
    
    for (nIndex = 0; nIndex < l_CommandData.Data.ProgramData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddress(&l_PepLogicData,
                                  l_CommandData.Data.ProgramData.nAddress + nIndex) ||
            !UtPepLogicSetData(&l_PepLogicData,
                               l_CommandData.Data.ProgramData.nData[nIndex]) ||
            !UtPepLogicTriggerProgram(&l_PepLogicData,
                                      &bSuccess) ||
            !bSuccess)
        {
            l_ResponseData.ErrorCode = CPepErrorFailed;
        
            return;
        }
    }

    l_ResponseData.ErrorCode = CPepErrorSuccess;
}

static void lProcessProgramUserDataCommand(void)
{
    UINT8 nIndex;
    BOOLEAN bSuccess;

    if (l_CommandData.Data.ProgramUserData.nDataLen > MArrayLen(l_CommandData.Data.ProgramUserData.Data))
    {
        l_ResponseData.ErrorCode = CPepErrorInvalidLength;
        
        return;
    }
    
    for (nIndex = 0; nIndex < l_CommandData.Data.ProgramUserData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddress(&l_PepLogicData,
                                  l_CommandData.Data.ProgramUserData.Data[nIndex].nAddress))
        {
            l_ResponseData.ErrorCode = CPepErrorFailed;
        
            return;
        }

        if (l_CommandData.Data.ProgramUserData.Data[nIndex].nPerformProgram)
        {
            if (!UtPepLogicSetData(&l_PepLogicData,
                                   l_CommandData.Data.ProgramUserData.Data[nIndex].nData) ||
                !UtPepLogicTriggerProgram(&l_PepLogicData,
                                          &bSuccess) ||
                !bSuccess)
            {
                l_ResponseData.ErrorCode = CPepErrorFailed;

                return;
            }
        }
    }

    l_ResponseData.ErrorCode = CPepErrorSuccess;
}

static void lProcessSetDelaysCommand(void)
{
    l_ResponseData.ErrorCode = UtPepLogicSetDelays(&l_PepLogicData,
                                                   l_CommandData.Data.Delays.nChipEnableNanoSeconds,
                                                   l_CommandData.Data.Delays.nOutputEnableNanoSeconds) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessUnknownCommand(void)
{
    l_ResponseData.ErrorCode = CPepErrorInvalidCommand;
}

void InitProcessCommand(void)
{
    TRISBbits.TRISB4 = 0x01; // Set Port B, bit 4 as input
    WPUBbits.WPUB4 = 0x00; // Disable Weak Pull-up on Port B, bit 4
    IOCBbits.IOCB4 = 0x00; // Disable Interrupt-on-change on Port B, bit 4
    ANSELHbits.ANS10 = 0x00; // Disable Analog on Port B, bit 4
    
    TRISC = 0x00; // Set Port C bits as output

	l_nBootState = CProcessCommandStateIdle;
    
    l_PepLogicData.pvLogicContext = UtPepLogicAllocLogicContext();
    l_PepLogicData.pReadBitPortFunc = lPepLogicReadBitPort;
    l_PepLogicData.pWritePortFunc = lPepLogicWritePort;
    l_PepLogicData.pvDeviceContext = 0;
}

void ProcessCommand(void)
{
	if (l_nBootState == CProcessCommandStateIdle)
	{
		if (!mHIDRxIsBusy()) // Did we receive a command?
		{
			HIDRxReport((char *)&l_CommandData, sizeof(l_CommandData));
            
			l_nBootState = CProcessCommandStateNotIdle;
			
            UtClearArray((UINT8*)&l_ResponseData, sizeof(l_ResponseData));
		}
	}
    else if (l_nBootState == CProcessCommandStateNotIdle)
	{
		switch (l_CommandData.Command)
		{
            case CPepVersionCommand:
                lProcessVersionCommand();
                break;
            case CPepResetCommand:
                lProcessResetCommand();
                break;
            case CPepSetProgrammerModeCommand:
                lProcessSetProgrammerModeCommand();
                break;
            case CPepSetVccModeCommand:
                lProcessSetVccModeCommand();
                break;
            case CPepSetPinPulseModeCommand:
                lProcessSetPinPulseModeCommand();
                break;
            case CPepSetVppModeCommand:
                lProcessSetVppModeCommand();
                break;
            case CPepReadDataCommand:
                lProcessReadDataCommand();
                break;
            case CPepReadUserDataCommand:
                lProcessReadUserDataCommand();
                break;
            case CPepReadUserDataWithDelayCommand:
                lProcessReadUserDataWithDelayCommand();
                break;
            case CPepProgramDataCommand:
                lProcessProgramDataCommand();
                break;
            case CPepProgramUserDataCommand:
                lProcessProgramUserDataCommand();
                break;
            case CPepSetDelaysCommand:
                lProcessSetDelaysCommand();
                break;
            default:
                lProcessUnknownCommand();
                break;
		}

        if (!mHIDTxIsBusy())
    	{
			HIDTxReport((char *)&l_ResponseData, sizeof(l_ResponseData));
            
			l_nBootState = CProcessCommandStateIdle;
		}
	}
}

/***************************************************************************/
/*  Copyright (C) 2021-2022 Kevin Eshbach                                  */
/***************************************************************************/
