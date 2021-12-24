/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include "UtPortTypeDefs.h"

#include "UtSleepFirmware.h"

#if defined(__XC8)
#include <xc.h>
#elif defined(__18CXX)
#pragma code
#endif

BOOL UtSleep(
  PLARGE_INTEGER pIntervalNanoseconds)
{
#if defined(__XC8)
#error UtSleep not implemented
    /*If an accurate delay is required, or if there are other tasks that can be performed during
the delay, then using a timer to generate an interrupt is the best way to proceed.
If these are not issues in your code, then you can use the compiler?s in-built delay
pseudo-functions: _delay, __delay_ms or __delay_us; see Appendix A. Library
Functions. These all expand into in-line assembly instructions or a (nested) loop of
instructions that will consume the specified number of cycles or time. The delay
argument must be a constant and less than 50,463,240.
Note that these code sequences will only use the NOP instruction and/or instructions
which form a loop. The alternate versions of these */
    
    #define _XTAL_FREQ 12000000

    unsigned long count = (unsigned long)(pIntervalNanoseconds->u.LowPart * (_XTAL_FREQ / 4000000.0));
 
    while (count--)
    {
    }

/*
    __delay_us(200);

  3771                           ;UtPepLogic.c: 31:        LARGE_INTEGER* pIntervalNanoseconds);UtPepLogic.c: 32: {;UtPep
      +                          Logic.c: 47:     _delay((unsigned long)((200)*(12000000/4000000.0)));
  3772  003332  0EC8               	movlw	200
  3773  003334                     u4137:
  3774  003334  2EE8               	decfsz	wreg,f,c
  3775  003336  D7FE               	bra	u4137
 */
#elif defined(__18CXX)
    UINT32 nCount;

    nCount = pIntervalNanoseconds->u.LowPart;
    
    while (--nCount > 0)
    {
    }
#endif

	return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
