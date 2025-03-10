/***************************************************************************/
/*  Copyright (C) 2021-2024 Kevin Eshbach                                  */
/***************************************************************************/

#include "UtPortTypeDefs.h"

#include "usb.h"

#include "UtSleepFirmware.h"

#include <UtilsPep/UtPepLogic.h>

#include <Firmware/PepFirmwareDefs.h>

#include <Includes/UtMacros.h>

#define CDeviceStateUpdatedNanoSeconds 150

#define DISABLE_PARALLEL_PORT

/* Local variables */

#if defined(__32MX250F128B__)
static UINT8 l_nVersion[] = "32MX250F128B 1.00";
#elif defined(__32MX440F256H__)
static UINT8 l_nVersion[] = "32MX440F256H 1.00";
#else
#error Unknown device configuration
#endif

static TUtPepLogicData l_PepLogicData;

/* Local Functions */

static BOOLEAN TUTPEPLOGICAPI lPepLogicReadBitPort(
  _Out_ PBOOLEAN pbValue)
{
    // Must invert the bit to match how the Busy bit of the parallel port works

#if defined(__32MX250F128B__)
    *pbValue = PORTBbits.RB4 ? FALSE : TRUE;
#elif defined(__32MX440F256H__)
    *pbValue = PORTBbits.RB4 ? FALSE : TRUE;
#else
#error Unknown device configuration
#endif

    return TRUE;
}
    
static BOOLEAN TUTPEPLOGICAPI lPepLogicWritePort(
  _In_ PUINT8 pnData,
  _In_ UINT32 nDataLen,
  _In_ UINT32 nWaitNanoSeconds)
{
    UINT8 nDataIndex;
   
    for (nDataIndex = 0; nDataIndex < nDataLen; ++nDataIndex)
    {
#if defined(__32MX250F128B__)
#if defined(DISABLE_PARALLEL_PORT)
        LATA = 0x00;
        LATB = 0x00;
        
        // D0 - D2
        LATB = pnData[nDataIndex] & 0x0007;
        
        // D3 
        if (pnData[nDataIndex] & 0x0008)
        {
            LATBbits.LATB9 = 0x01;
        }

        // D4
        if (pnData[nDataIndex] & 0x0010)
        {
            LATBbits.LATB8 = 0x01;
        }

        // D5
        if (pnData[nDataIndex] & 0x0020)
        {
            LATBbits.LATB7 = 0x01;
        }
        
        // D6
        if (pnData[nDataIndex] & 0x0040) {
            LATAbits.LATA1 = 0x01;
        }
        
        // D7
        if (pnData[nDataIndex] & 0x0080) {
            LATAbits.LATA0 = 0x01;
        }
        
        // PMWR (Strobe clock pin of 74LS374, data loaded on the rising edge of the clock)
        Nop();
        Nop();
        Nop();
        Nop();

        LATBbits.LATB3 = 0x01;

        // Need a max of 28 ns for the data to be loaded into the LS374
        
        Nop();
        Nop();
        Nop();

        LATBbits.LATB3 = 0x00;
#else
        while (PMMODEbits.BUSY);

        PMDIN = pnData[nDataIndex];

        //PMDOUT = pnData[nDataIndex];

        //PMADDRbits.ADDR = pnData[nDataIndex];
#endif
#elif defined(__32MX440F256H__)
#if defined(DISABLE_PARALLEL_PORT)
        LATE = pnData[nDataIndex];
#else
        PMDIN = pnData[nDataIndex];

        //while (PMMODEbits.BUSY);
#endif
#else
#error Unknown device configuration
#endif

        UtSleep(CDeviceStateUpdatedNanoSeconds);
    }
            
    return TRUE;
}
 
static void lProcessVersionCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    UINT8 nIndex;

    memset(pResponseData->Data.Version, 0x00, sizeof(pResponseData->Data.Version));
 
    for (nIndex = 0; nIndex < MArrayLen(l_nVersion); ++nIndex)
    {
        if (nIndex < MArrayLen(pResponseData->Data.Version))
        {
            pResponseData->Data.Version[nIndex] = l_nVersion[nIndex];
        }
    }

    pResponseData->ErrorCode = CPepErrorSuccess;
}

static void lProcessResetCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    pResponseData->ErrorCode = UtPepLogicReset(&l_PepLogicData) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetProgrammerModeCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    pResponseData->ErrorCode = UtPepLogicSetProgrammerMode(&l_PepLogicData,
                                                           pCommandData->Data.nProgrammerMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetVccModeCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    pResponseData->ErrorCode = UtPepLogicSetVccMode(&l_PepLogicData,
                                                    pCommandData->Data.nVccMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetPinPulseModeCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    pResponseData->ErrorCode = UtPepLogicSetPinPulseMode(&l_PepLogicData,
                                                         pCommandData->Data.nPinPulseMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessSetVppModeCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    pResponseData->ErrorCode = UtPepLogicSetVppMode(&l_PepLogicData,
                                                    pCommandData->Data.nVppMode) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessReadDataCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    UINT8 nIndex;

    if (pCommandData->Data.ReadData.nDataLen > MArrayLen(pResponseData->Data.nData))
    {
        pResponseData->ErrorCode = CPepErrorInvalidLength;
        
        return;
    }
  
    for (nIndex = 0; nIndex < pCommandData->Data.ReadData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetOutputEnable(&l_PepLogicData, FALSE) ||
            !UtPepLogicSetAddress(&l_PepLogicData,
                                  pCommandData->Data.ReadData.nAddress + nIndex) ||
            !UtPepLogicSetOutputEnable(&l_PepLogicData, TRUE) ||
            !UtPepLogicGetData(&l_PepLogicData,
                               &pResponseData->Data.nData[nIndex]))
        {
            pResponseData->ErrorCode = CPepErrorFailed;
        
            return;
        }
    }

    pResponseData->ErrorCode = CPepErrorSuccess;
}

static void lProcessReadUserDataCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    UINT8 nIndex;

    if (pCommandData->Data.ReadUserData.nDataLen > MArrayLen(pCommandData->Data.ReadUserData.Data))
    {
        pResponseData->ErrorCode = CPepErrorInvalidLength;
        
        return;
    }

    for (nIndex = 0; nIndex < pCommandData->Data.ReadUserData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddress(&l_PepLogicData,
                                  pCommandData->Data.ReadUserData.Data[nIndex].nAddress) ||
            !UtPepLogicSetOutputEnable(&l_PepLogicData,
                                       pCommandData->Data.ReadUserData.Data[nIndex].nEnableOutputEnable))
        {
            pResponseData->ErrorCode = CPepErrorFailed;
        
            return;
        }
        
        if (pCommandData->Data.ReadUserData.Data[nIndex].nPerformRead)
        {
            if (!UtPepLogicGetData(&l_PepLogicData,
                                   &pResponseData->Data.nData[nIndex]))
            {
                pResponseData->ErrorCode = CPepErrorFailed;
        
                return;
            }
        }
    }    

    pResponseData->ErrorCode = CPepErrorSuccess;
}

static void lProcessReadUserDataWithDelayCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    UINT8 nIndex;
    
    if (pCommandData->Data.ReadUserDataWithDelay.nDataLen > MArrayLen(pCommandData->Data.ReadUserDataWithDelay.Data))
    {
        pResponseData->ErrorCode = CPepErrorInvalidLength;
        
        return;
    }

    for (nIndex = 0; nIndex < pCommandData->Data.ReadUserDataWithDelay.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddressWithDelay(&l_PepLogicData,
                                           pCommandData->Data.ReadUserDataWithDelay.Data[nIndex].nAddress,
                                           pCommandData->Data.ReadUserDataWithDelay.Data[nIndex].nDelayNanoSeconds))
        {
            pResponseData->ErrorCode = CPepErrorFailed;
        
            return;
        }

        if (pCommandData->Data.ReadUserDataWithDelay.Data[nIndex].nPerformRead)
        {
            if (!UtPepLogicGetData(&l_PepLogicData,
                                   &pResponseData->Data.nData[nIndex]))
            {
                pResponseData->ErrorCode = CPepErrorFailed;
        
                return;
            }
        }
    }    

    pResponseData->ErrorCode = CPepErrorSuccess;
}

static void lProcessProgramDataCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    UINT8 nIndex;
    BOOLEAN bSuccess;

    if (pCommandData->Data.ProgramData.nDataLen > MArrayLen(pCommandData->Data.ProgramData.nData))
    {
        pResponseData->ErrorCode = CPepErrorInvalidLength;
        
        return;
    }
    
    for (nIndex = 0; nIndex < pCommandData->Data.ProgramData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddress(&l_PepLogicData,
                                  pCommandData->Data.ProgramData.nAddress + nIndex) ||
            !UtPepLogicSetData(&l_PepLogicData,
                               pCommandData->Data.ProgramData.nData[nIndex]) ||
            !UtPepLogicTriggerProgram(&l_PepLogicData,
                                      &bSuccess) ||
            !bSuccess)
        {
            pResponseData->ErrorCode = CPepErrorFailed;
        
            return;
        }
    }

    pResponseData->ErrorCode = CPepErrorSuccess;
}

static void lProcessProgramUserDataCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    UINT8 nIndex;
    BOOLEAN bSuccess;
    
    if (pCommandData->Data.ProgramUserData.nDataLen > MArrayLen(pCommandData->Data.ProgramUserData.Data))
    {
        pResponseData->ErrorCode = CPepErrorInvalidLength;
        
        return;
    }
    
    for (nIndex = 0; nIndex < pCommandData->Data.ProgramUserData.nDataLen; ++nIndex)
    {
        if (!UtPepLogicSetAddress(&l_PepLogicData,
                                  pCommandData->Data.ProgramUserData.Data[nIndex].nAddress))
        {
            pResponseData->ErrorCode = CPepErrorFailed;
        
            return;
        }

        if (pCommandData->Data.ProgramUserData.Data[nIndex].nPerformProgram)
        {
            if (!UtPepLogicSetData(&l_PepLogicData,
                                   pCommandData->Data.ProgramUserData.Data[nIndex].nData) ||
                !UtPepLogicTriggerProgram(&l_PepLogicData,
                                          &bSuccess) ||
                !bSuccess)
            {
                pResponseData->ErrorCode = CPepErrorFailed;

                return;
            }
        }
    }

    pResponseData->ErrorCode = CPepErrorSuccess;
}

static void lProcessSetDelaysCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    pResponseData->ErrorCode = UtPepLogicSetDelays(&l_PepLogicData,
                                                   pCommandData->Data.Delays.nChipEnableNanoSeconds,
                                                   pCommandData->Data.Delays.nOutputEnableNanoSeconds) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessDebugWritePortDataCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    UINT8 nData[] = {pCommandData->Data.DebugWritePortData.nWritePortData};
    
    pResponseData->ErrorCode = lPepLogicWritePort(nData,
                                                  MArrayLen(nData),
                                                  0) ? CPepErrorSuccess : CPepErrorFailed;
}

static void lProcessUnknownCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    pResponseData->ErrorCode = CPepErrorInvalidCommand;
}

void InitProcessCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
#if defined(__32MX250F128B__)
    ANSELA = 0x00; // Disable all analog inputs on Port A
    ANSELB = 0x00; // Disable all analog inputs on Port B

    ODCA = 0x00; // Disable Open-Drain Output on Port A
    ODCB = 0x00; // Disable Open-Drain Output on POrt B

    DMACONbits.ON = 0x00; // Disable DMA Controller

    CNCONAbits.ON = 0x00; // Disable Change Notice on Port A
    CNCONBbits.ON = 0x00; // Disable Change Notice on Port B

    CNPDA = 0x00; // Disable Change Notification Pull-Down on Port A
    CNPDB = 0x00; // Disable Change Notification Pull-Down on Port B

    CNPUA = 0x00; // Disable Change Notification Pull-Up on Port A
    CNPUB = 0x00; // Disable Change Notification Pull-Up on Port B

    CNENA = 0x00; // Disable Change Notification Interrupt on Port A
    CNENB = 0x00; // Disable Change Notification Interrupt on Port B
    
    I2C1CONbits.ON = 0x00; // Turn off I2C1 module
    I2C2CONbits.ON = 0x00; // Turn off I2C2 module
    
    T1CONbits.ON = 0x00; // Turn off Timer 1
    T2CONbits.ON = 0x00; // Turn off Timer 2
    T3CONbits.ON = 0x00; // Turn off Timer 3
    T4CONbits.ON = 0x00; // Turn off Timer 4
    T5CONbits.ON = 0x00; // Turn off Timer 5
    
    OC1CONbits.ON = 0x00; // Turn off Output Compare Peripheral
    OC2CONbits.ON = 0x00; // Turn off Output Compare Peripheral
    OC3CONbits.ON = 0x00; // Turn off Output Compare Peripheral
    OC4CONbits.ON = 0x00; // Turn off Output Compare Peripheral
    OC5CONbits.ON = 0x00; // Turn off Output Compare Peripheral

    SPI1CONbits.ON = 0x00; // Turn off Framed SPI 1 support
    SPI2CONbits.ON = 0x00; // Turn off Framed SPI 2 support
    
    U1MODEbits.ON = 0x00; // Turn off UART1
    U2MODEbits.ON = 0x00; // Turn off UART2

    RTCCONbits.ON = 0x00; // Turn off RTC module

    RTCALRMbits.ALRMEN = 0x00; // Turn off RTC alarm
    
    AD1CON1bits.ON = 0x00; // Turn off ADC
 
    CM1CONbits.ON = 0x00; // Turn off Comparator 1
    CM2CONbits.ON = 0x00; // Turn off Comparator 2
    CM3CONbits.ON = 0x00; // Turn off Comparator 3
    
    CVRCONbits.ON = 0x00; // Turn off Comparator Voltage Reference

    CTMUCONbits.ON = 0x00; // Turn off CTMU
   
    // Pin 11 - SOSCI/RPB4/RB4

    TRISBbits.TRISB4 = 0x01; // Set Port B, bit 4 as input

#if defined(DISABLE_PARALLEL_PORT)
    IEC1bits.PMPIE = 0x00; // disable PMP interrupts
    PMCONbits.ON = 0x00; // Turn off Parallel Master Port
    
    // Pin 4 PGED1/AN2/C1IND/C2INB/C3IND/RPB0/PMD0/RB0

    TRISBbits.TRISB0 = 0x00; // Set Port B, bit 0 as output
    
    // Pin 5 - PGEC1/AN1/C1INC/C2INA/RPB1/CTED12/PMD1/RB1

    TRISBbits.TRISB1 = 0x00; // Set Port B, bit 1 as output

    // Pin 6 - AN4/C1INB/C2IND/RPB2/SDA2/CTED13/PMD2/RB2

    TRISBbits.TRISB2 = 0x00; // Set Port B, bit 2 as output
    
    // Pin 18 - TDO/RPB9/SDA1/CTED4/PMD3/RB9

    TRISBbits.TRISB9 = 0x00; // Set Port B, bit 9 as output
    
    // Pin 17 - TCK/RPB8/SCL1/CTED10/PMD4/RB8

    TRISBbits.TRISB8 = 0x00; // Set Port B, bit 8 as output
    
    // Pin 16 - TDI/RPB7/CTED3/PMD5/INT0/RB7
    
    TRISBbits.TRISB7 = 0x00; // Set Port B, bit 7 as output
    
    // Pin 3 - PGEC3/VREF-/CVREF-/AN1/RPA1/CTED2/PMD6/RA1

    TRISAbits.TRISA1 = 0x00; // Set Port A, bit 1 as output
    
    // Pin 2 - PGED3/VREF+/CVREF+/AN0/C3INC/RPA0/CTED1/PMD7/RA0

    TRISAbits.TRISA0 = 0x00; // Set Port A, bit 0 as output

    // Pin 7 - AN5/C1INA/C2INC/RTCC/RPB3/SCL2/PMWR/RB3

    TRISBbits.TRISB3 = 0x00; // Set Port B, bit 3 as output

    LATA = 0x00;
    LATB = 0x00;
#else
    /* Parallel Port Configuration */

    IEC1bits.PMPIE = 0x00; // disable PMP interrupts
    
    PMCON = 0x00; // Stop PMP module and clear control register
    PMAEN = 0x00; // Disable all address bits
    
    PMCONbits.PMPEN = 0x00;  // Parallel Port Master Enable as off
    PMCONbits.SIDL = 0x00;   // Stop in Idle Mode as off 
    PMCONbits.ADRMUX = 0x00; // Address/Data Multiplexing as address and data on separate pins
    PMCONbits.PMPTTL = 0x01; // PMP Module TTL Input as TTL input buffer
    PMCONbits.PTWREN = 0x01; // Write Enable Strobe Port Enable as on
    PMCONbits.PTRDEN = 0x01; // Read/Write Strobe Port Enable as on
    PMCONbits.CSF = 0x02;    // Chip Select Function as PMCS2/PMCS2A and PMCS1/PMCS1A function as chip select]
    PMCONbits.ALP = 0x00;    // Address Latch Polarity as active-low
    PMCONbits.CS1P = 0x01;   // Chip Select 1/1A Polarity as active-high
    PMCONbits.WRSP = 0x01;   // Write Strobe Polarity as Enable strobe active-high (PMWR)
    PMCONbits.RDSP = 0x01;   // Read Strobe Polarity as Read/write strobe active-high (PMRD)

    PMMODEbits.IRQM = 0x00;  // Interrupt Request Mode as off
    PMMODEbits.INCM = 0x00;  // Increment Mode as off
    PMMODEbits.MODE = 0x02;  // Parallel Port Mode Selected as Master Mode 2 (PMCSx, PMRD, PMWR, PMDx)
    PMMODEbits.WAITB = 0x03; // Data Setup to Read/Write Strobe Wait States as 4TPB
    PMMODEbits.WAITM = 0x03; // Data Read/Write Strobe Wait States as 1TPB
    PMMODEbits.WAITE = 0x00; // Data Hold After Read/Write Strobe Wait States as 1TPB
    
    PMADDRbits.ADDR = 0x00; // PMA Address not active
    PMADDRbits.CS1 = 0x00;  // Chip Select 1 is active
    
    PMAENbits.PTEN0 = 0x01; // PMD0 enable bit
    PMAENbits.PTEN1 = 0x01; // PMD1 enable bit
    PMAENbits.PTEN2 = 0x01; // PMD2 enable bit
    PMAENbits.PTEN3 = 0x01; // PMD3 enable bit
    PMAENbits.PTEN4 = 0x01; // PMD4 enable bit
    PMAENbits.PTEN5 = 0x01; // PMD5 enable bit
    PMAENbits.PTEN6 = 0x01; // PMD6 enable bit
    PMAENbits.PTEN7 = 0x01; // PMD7 enable bit
    PMAENbits.PTEN14 = 0x01; // Chip Select enable bit (enable/disable))
    
    PMCONbits.PMPEN = 0x01; // Parallel Port Master Enable as on
#endif
#elif defined(__32MX440F256H__)
    /* Busy (Input) Pin Configuration */

    AD1PCFGbits.PCFG4 = 0x01; // Analog Bit 4 as off
    
    TRISBbits.TRISB4 = 0x01; // Set Port B Bit 4 as input
    ODCBbits.ODCB4 = 0x00;   // Open-Drain configuration as 5V tolerant

    /* Output Pin Configuration */
    
#if defined(DISABLE_PARALLEL_PORT)
    TRISE = 0x00; // Set Port E bits as output
    ODCE = 0x00; // Open-Drain configuration as 5V tolerant
    CNPUE = 0x00; // Change Notification Enable as off
#else
    /* Parallel Port Configuration */
    
    IEC1bits.PMPIE = 0;    // disable PMP interrupts
    
    PMCON = 0x0000; // Stop PMP module and clear control register
    PMAEN = 0x0000; // Disable all address bits
    
    PMCONbits.PMPEN = 0x00; // default state at boot
    PMCONbits.SIDL = 0x00;  // Stop in Idle Mode as off 
    
    PMMODEbits.IRQM = 0x00;   // Interrupt Request Mode as off)
    PMMODEbits.INCM = 0x00;   // Increment Mode as off
    PMMODEbits.MODE16 = 0x00; // 8-bit mode
    PMMODEbits.MODE = 0x02;   // Parallel Port Mode Selected as Master Mode 2 (PMCSx, PMRD, PMWR,, PMDx)
    PMMODEbits.WAITB = 0x00;  // Data Setup to Read/Write Strobe Wait States as 1TPB
    PMMODEbits.WAITM = 0x00;  // Data Read/Write Strobe Wait States as 1TPB
    PMMODEbits.WAITE = 0x00;  // Data Hold After Read/Write Strobe Wait States as 1TPB

    PMCONbits.ADRMUX = 0x00; // Address/Data Multiplex Mode is Demultiplexed
    PMCONbits.PMPTTL = 0x01; // PMP Module TTL Input as TTL input buffer
    PMCONbits.PTWREN = 0x01; // Write Enable Strobe Port Enable as on
    PMCONbits.PTRDEN = 0x01; // Read/Write Strobe Port Enable as on
    PMCONbits.CSF = 0x02;    // Chip Select Function as PMCS2/PMCS2A and PMCS1/PMCS1A function as chip select]
    PMCONbits.ALP = 0x00;    // Address Latch Polarity as active-low
    PMCONbits.CS2P = 0x01;   // Chip Select 2/2A Polarity as active-high
    PMCONbits.CS1P = 0x01;   // Chip Select 1/1A Polarity as active-high
    PMCONbits.WRSP = 0x01;   // Write Strobe Polarity as Enable strobe active-high (PMWR)
    PMCONbits.RDSP = 0x01;   // Read Strobe Polarity as Read/write strobe active-high (PMRD)

    PMADDRbits.CS1 = 0x01;   // Chip Select 1 is Active
    PMADDRbits.CS2 = 0x00;   // Chip Select 2 is Inactive

    PMAENbits.PTEN14 = 0x01; // Enable Chip Select 1 Enable Strobe to enable
    PMAENbits.PTEN15 = 0x00; // Enable Chip Select 2 Enable Strobe to enable
    
    PMCONbits.PMPEN = 0x01;  // Start PMP module
#endif
#else
#error Unknown device configuration
#endif
    
    l_PepLogicData.pvLogicContext = UtPepLogicAllocLogicContext();
    l_PepLogicData.pReadBitPortFunc = lPepLogicReadBitPort;
    l_PepLogicData.pWritePortFunc = lPepLogicWritePort;
    
    memset(pResponseData, CPepErrorInitializeData, sizeof(*pResponseData));
    memset(pCommandData, CPepErrorInitializeData, sizeof(*pCommandData));
    
    // In the future add way to check if the EPROM+ is attached 
    
    UtPepLogicReset(&l_PepLogicData);
}

void ProcessCommand(
  TUtPepCommandData* pCommandData,
  TUtPepResponseData* pResponseData)
{
    switch (pCommandData->Command)
    {
        case CPepVersionCommand:
            lProcessVersionCommand(pCommandData, pResponseData);
            break;
        case CPepResetCommand:
            lProcessResetCommand(pCommandData, pResponseData);
            break;
        case CPepSetProgrammerModeCommand:
            lProcessSetProgrammerModeCommand(pCommandData, pResponseData);
            break;
        case CPepSetVccModeCommand:
            lProcessSetVccModeCommand(pCommandData, pResponseData);
            break;
        case CPepSetPinPulseModeCommand:
            lProcessSetPinPulseModeCommand(pCommandData, pResponseData);
            break;
        case CPepSetVppModeCommand:
            lProcessSetVppModeCommand(pCommandData, pResponseData);
            break;
        case CPepReadDataCommand:
            lProcessReadDataCommand(pCommandData, pResponseData);
            break;
        case CPepReadUserDataCommand:
            lProcessReadUserDataCommand(pCommandData, pResponseData);
            break;
        case CPepReadUserDataWithDelayCommand:
            lProcessReadUserDataWithDelayCommand(pCommandData, pResponseData);
            break;
        case CPepProgramDataCommand:
            lProcessProgramDataCommand(pCommandData, pResponseData);
            break;
        case CPepProgramUserDataCommand:
            lProcessProgramUserDataCommand(pCommandData, pResponseData);
            break;
        case CPepSetDelaysCommand:
            lProcessSetDelaysCommand(pCommandData, pResponseData);
            break;
        case CPepDebugWritePortDataCommand:
            lProcessDebugWritePortDataCommand(pCommandData, pResponseData);
            break;
        default:
            lProcessUnknownCommand(pCommandData, pResponseData);
            break;
    }
}

/***************************************************************************/
/*  Copyright (C) 2021-2024 Kevin Eshbach                                  */
/***************************************************************************/
