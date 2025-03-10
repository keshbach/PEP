#include "UtPortTypeDefs.h"

#include "Compiler.h"
#include "UtTimeDelay.h" 

#if defined(__32MX250F128B__)
#define SYS_CLK 8000000ul // Hz
#elif defined(__32MX440F256H__)
#define SYS_CLK 8000000ul // Hz
#else
#error Unknown platform architecture
#endif

// Clock frequency values
// These directly influence timed events using the Tick module.  They also are used for UART and SPI baud rate generation.
#define GetSystemClock() SYS_CLK

// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Might need changing if using Doze modes.
#define GetInstructionClock() (GetSystemClock() / 1)

// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Divisor may be different if using a PIC32 since it's configurable.
#define GetPeripheralClock() (GetSystemClock() / 1) // FPBDIV = DIV_1

void UtTimeDelay10us(
  UINT32 tenMicroSecondCounter)
{
    volatile INT32 cyclesRequiredForEntireDelay;    
        
    if (GetInstructionClock() <= 500000) //for all FCY speeds under 500KHz (FOSC <= 1MHz)
    {
        //10 cycles burned through this path (includes return to caller).
        //For FOSC == 1MHZ, it takes 5us.
        //For FOSC == 4MHZ, it takes 0.5us
        //For FOSC == 8MHZ, it takes 0.25us.
        //For FOSC == 10MHZ, it takes 0.2us.
    }    
    else
    {
        //7 cycles burned to this point.
            
        //We want to pre-calculate number of cycles required to delay 10us * tenMicroSecondCounter using a 1 cycle granule.
        cyclesRequiredForEntireDelay = (INT32)(GetInstructionClock() / 100000) * tenMicroSecondCounter;
            
        //We subtract all the cycles used up until we reach the while loop below, where each loop cycle count is subtracted.
        //Also we subtract the 5 cycle function return.
        cyclesRequiredForEntireDelay -= 24; //(19 + 5)
            
        if (cyclesRequiredForEntireDelay <= 0)
        {
            // If we have exceeded the cycle count already, bail!
        }
        else
        {   
            while (cyclesRequiredForEntireDelay > 0) //19 cycles used to this point.
            {
                cyclesRequiredForEntireDelay -= 8; //Subtract cycles burned while doing each delay stage, 8 in this case.
            }
        }
    }
}

void UtTimeDelayMs(
  UINT16 ms)
{
    volatile UINT8 i;
        
    while (ms--)
    {
        i = 4;
        
        while (i--)
        {
            UtTimeDelay10us(25);
        }
    }
}

#if defined(__32MX250F128B__)

static UINT8 l_CounterExpired = 0;

void test()
{
    // pbclk is 8MHz (same as sysclk))
    // 8 MHz means 1 clock takes 125 nanoseconds (1000 MHz per nanosecond / 8 MHz).
    // 40 MHz means 1 clock takes 25 nanoseconds


    // 32MX250F128B internally runs @ 40MHz



    
    T2CON = 0;
    T3CON = 0;
    
    T2CONbits.ON = 0; // disable timer
    T2CONbits.SIDL = 0; // stop in idle mode
    T2CONbits.TCKPS = 0; // Timer Input Clock Prescale (1:1))
    T2CONbits.T32 = 1; // enable 32-bit timer
    T2CONbits.TCS = 0; // Timer Clock Source Select (internal peripheral clock)
    
    TMR2 = 0; // clear timer register (lower 16-bits)
    TMR3 = 0; // clear timer register (upper 16-bits)
    
    PR2 = 0xFFFFFFFF; // load period register (PR2 and PR3))
    
    IPC2SET = 0x0000000C; // Set priority level = 3
    IPC2SET = 0x00000001; // Set subpriority level = 1
                          // Can be done in a single operation by assigning PC2SET = 0x0000000D
    
    IFS0CLR = 0x00000100; // Clear the timer interrupt status flag
    IEC0SET = 0x00000100; // Enable timer interrupts
    
    l_CounterExpired = 0;
    
    T2CONSET = 0x8000; // start 32-bit timer
    
    while (!l_CounterExpired)
    {
    }
}

void __ISR(_TIMER_2_VECTOR, IPL3SOFT) Timer1Handler(void)
{
    IFS0CLR = 0x00000100; // Be sure to clear the Timer1 interrupt status
    
    l_CounterExpired = 1;
}
#elif defined(__32MX440F256H__)

// runs @ 80MHz

#else
#error Unknown platform architecture
#endif
