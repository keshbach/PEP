#include "PepDevice.h"

/*
#if defined(__XC8) || defined(__18CXX)
#elif defined(__32MX250F128B__)
static int __attribute__ ((address(0xBD000000))) l_Update = 0;
#elif defined(__32MX440F256H__)
static int __attribute__ ((address(0xBD000000))) l_Update = 0;
#else
#error Unknown device configuration
#endif

static void __attribute__((section(".reset.startup"))) lExecuteFirmwareUpdate(void)
{
    if (l_Update)
    {
        // update firmware
        // reset the device
        
        l_Update = 0;
        
        _reset();
    }
}
*/

/*
void _on_reset(void)
{
}
*/

int main(void)
{
    return ExecutePepDevice();
}
