#ifndef __COMPILER_H
#define __COMPILER_H

// Include proper device header file
#if defined(__PIC32MX__)	// Microchip C32 compiler
    #define COMPILER_MPLAB_C32

	#include <p32xxxx.h>
        
    //#include <plib.h> /* Below are the contents of these header file */
        
    //#include <peripheral/adc10.h>
    //#include <peripheral/bmx.h>
    //#include <peripheral/cmp.h>
    //#include <peripheral/cvref.h>
    //#include <peripheral/dma.h>
    //#include <peripheral/i2c.h>
    //#include <peripheral/incap.h>
    #include <peripheral/int.h>
    #include <peripheral/nvm.h>
    //#include <peripheral/outcompare.h>
    #include <peripheral/pcache.h>
    #include <peripheral/pmp.h>
    //#include <peripheral/ports.h>
    #include <peripheral/power.h>
    #include <peripheral/reset.h>
    //#include <peripheral/rtcc.h>
    //#include <peripheral/spi.h>
    #include <peripheral/system.h>
    //#include <peripheral/timer.h>
    //#include <peripheral/uart.h>
    #include <peripheral/wdt.h>
    //#include <peripheral/eth.h>
    //#include <peripheral/CAN.h>
#else
	#error Unknown processor or compiler.  See Compiler.h
#endif

#include <stddef.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define	ROM const

// 32-bit specific defines (PIC32)
#if defined(__PIC32MX__)
//	#define Reset()				SoftReset()
//	#define ClrWdt()			(WDTCONSET = _WDTCON_WDTCLR_MASK)
#endif

#endif
