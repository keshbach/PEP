/***************************************************************************/
/*  Copyright (C) 2021-2023 Kevin Eshbach                                  */
/***************************************************************************/

#include "UtPortTypeDefs.h"

#include "UtSleepFirmware.h"

#if defined(__32MX250F128B__)
#include <p32xxxx.h>
#include <cp0defs.h>
#elif defined(__32MX440F256H__)
#else
#error Unknown device configuration
#endif

BOOL UtSleep(
  UINT32 nIntervalNanoseconds)
{
#if defined(__32MX250F128B__)
    // 32MX250F128B sys clk runs @ 40MHz
    //    
    // 40 MHz means 1 clock takes 25 nanoseconds
    
    if (nIntervalNanoseconds > 100)
    {
        volatile UINT32 cyclesRequired = nIntervalNanoseconds / 25;

        if (cyclesRequired > 20)
        {
            while (cyclesRequired--)
            {
            }
        }
    }
#elif defined(__32MX440F256H__)
    // if using PIC32MX3XX or greater then cpu runs at 80 MHz
    // and have Cache and Instruction Pre-fetch logic
    
    /*    
    // ? cycles for instructions generated before the while loop
    
    volatile UINT32 ticksRequired = nIntervalNanoseconds / 50;
   
    _CP0_SET_COUNT(0);
    _CP0_SET_CAUSE(_CP0_GET_CAUSE() & ~0x08000000); // increment counter every other clock
    
    while (_CP0_GET_COUNT() != ticksRequired)
    {
    }
*/
    
    volatile INT32 cyclesRequired = (INT32)nIntervalNanoseconds / 12;

    while (cyclesRequired--)
    {
        // loop does not take into account other instructions being executed
    }
#else
#error Unknown device configuration
#endif

	return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2021-2023 Kevin Eshbach                                  */
/***************************************************************************/
