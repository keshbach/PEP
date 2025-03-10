#if !defined(_USB_H_)
#define _USB_H_

#include "Compiler.h"

#include "usb_config.h"     // Must be defined by the application

#include "usb_common.h"     // Common USB library definitions
#include "usb_ch9.h"        // USB device framework definitions

#include "usb_device.h"     // USB Device abstraction layer interface

#if defined(__PIC32MX__)
#include "usb_hal_pic32.h"
#else
#error Unknown device configuration
#endif

#endif // _USB_H_
