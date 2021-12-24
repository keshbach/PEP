/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>
#include "UtTypeDefs.h"                   
#include "usb.h"                         
#include "ProcessCommand.h"

#pragma config CPUDIV = NOCLKDIV
#pragma config USBDIV = OFF
#pragma config FOSC   = HS
#pragma config PLLEN  = ON
#pragma config FCMEN  = OFF
#pragma config IESO   = OFF
#pragma config PWRTEN = OFF
#pragma config BOREN  = OFF
#pragma config BORV   = 30
#pragma config WDTEN  = OFF
#pragma config WDTPS  = 32768
#pragma config MCLRE  = OFF
#pragma config HFOFST = OFF
#pragma config STVREN = ON
#pragma config LVP    = OFF
#pragma config XINST  = OFF
#pragma config BBSIZ  = OFF
#pragma config CP0    = OFF
#pragma config CP1    = OFF
#pragma config CPB    = OFF
#pragma config WRT0   = OFF
#pragma config WRT1   = OFF
#pragma config WRTB   = OFF
#pragma config WRTC   = OFF
#pragma config EBTR0  = OFF
#pragma config EBTR1  = OFF
#pragma config EBTRB  = OFF      

#pragma code

static void lInitializeUSBDriver(void)
{
    UCFGbits.PPB0 = 0;  // Even/odd ping-pong buffers disabled
    UCFGbits.PPB1 = 0;
    UCFGbits.FSEN = 1;  // Full-Speed enabled
    UCFGbits.UPUEN = 1; // USB On-Chip pull-up enabled
    UCFGbits.UTEYE = 0; // USB Eye Pattern Test disabled
   
    g_nUsbDeviceState = DETACHED_STATE;
    g_UsbDeviceStatus._byte = 0x00;
    g_nUsbActiveCfg = 0x00;
}

static void lUSBTasks(void)
{
    /*
     * Servicing Hardware
     */
    USBCheckBusStatus();                    // Must use polling method
    USBDriverService();              	    // Interrupt or polling method
}

void main(void)
{ 
    lInitializeUSBDriver();
    
    InitProcessCommand();
    
    while (1)
    {
		ClrWdt();
        
	    lUSBTasks();         					// Need to call USBTasks() periodically
	    										// it handles SETUP packets needed for enumeration
		
	    if ((g_nUsbDeviceState == CONFIGURED_STATE) && (UCONbits.SUSPND != 1))
	    {
 	       ProcessCommand();
 	    }
    }
}

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
