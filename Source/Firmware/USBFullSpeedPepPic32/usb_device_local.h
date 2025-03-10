#if !defined(_USB_DEVICE_LOCAL_H_)
#define _USB_DEVICE_LOCAL_H_

/* Short Packet States - Used by Control Transfer Read  - CTRL_TRF_TX */
#define SHORT_PKT_NOT_USED  0
#define SHORT_PKT_PENDING   1
#define SHORT_PKT_SENT      2

/* Control Transfer States */
#define WAIT_SETUP          0
#define CTRL_TRF_TX         1
#define CTRL_TRF_RX         2

typedef union tagTEndPointStatus
{
    struct
    {
        UINT8 ping_pong_state :1;
        UINT8 transfer_terminated :1;
    } bits;
    UINT8 Val;
} TEndPointStatus;

#if (USB_PING_PONG_MODE == USB_PING_PONG__NO_PING_PONG)
    #define USB_NEXT_EP0_OUT_PING_PONG 0x0000   // Used in USB Device Mode only
    #define USB_NEXT_EP0_IN_PING_PONG 0x0000    // Used in USB Device Mode only
    #define USB_NEXT_PING_PONG 0x0000           // Used in USB Device Mode only

    #define EP0_OUT_EVEN    0                   // Used in USB Device Mode only
    #define EP0_OUT_ODD     0                   // Used in USB Device Mode only
    #define EP0_IN_EVEN     1                   // Used in USB Device Mode only
    #define EP0_IN_ODD      1                   // Used in USB Device Mode only

    #define EP(ep, dir, pp) (2 * ep + dir)         // Used in USB Device Mode only
    #define BD(ep, dir, pp) ((8 * ep) + (4 * dir)) // Used in USB Device Mode only

#elif (USB_PING_PONG_MODE == USB_PING_PONG__EP0_OUT_ONLY)
    #define USB_NEXT_EP0_OUT_PING_PONG 0x0004
    #define USB_NEXT_EP0_IN_PING_PONG 0x0000
    #define USB_NEXT_PING_PONG 0x0000

    #define EP0_OUT_EVEN    0
    #define EP0_OUT_ODD     1
    #define EP0_IN_EVEN     2
    #define EP0_IN_ODD      2

    #define EP(ep,dir,pp) (2u*ep+dir+(((ep==0)&&(dir==0))?pp:1))
    #define BD(ep,dir,pp) (4u*((2u*ep)+dir+(((ep==0)&&(dir==0))?pp:1)))

#elif (USB_PING_PONG_MODE == USB_PING_PONG__FULL_PING_PONG)
    #define USB_NEXT_EP0_OUT_PING_PONG 0x0008
    #define USB_NEXT_EP0_IN_PING_PONG 0x0008
    #define USB_NEXT_PING_PONG 0x0008

    #define EP0_OUT_EVEN    0
    #define EP0_OUT_ODD     1
    #define EP0_IN_EVEN     2
    #define EP0_IN_ODD      3

    #define EP(ep, dir, pp) (4 * ep + 2 * dir + pp)
    #define BD(ep, dir, pp) (8 * (4 * ep + 2 * dir + pp))

#elif (USB_PING_PONG_MODE == USB_PING_PONG__ALL_BUT_EP0)
    #define USB_NEXT_EP0_OUT_PING_PONG 0x0000
    #define USB_NEXT_EP0_IN_PING_PONG 0x0000
    #define USB_NEXT_PING_PONG 0x0004

    #define EP0_OUT_EVEN    0
    #define EP0_OUT_ODD     0
    #define EP0_IN_EVEN     1
    #define EP0_IN_ODD      1

    #define EP(ep, dir, pp) (4 * ep + 2 * dir + ((ep == 0) ? 0 : (pp - 2)))
    #define BD(ep, dir, pp) (4 * (4 * ep + 2 * dir + ((ep == 0) ? 0 : (pp - 2))))
#else
    #error "No ping pong mode defined."
#endif

/****** Event callback enabling/disabling macros ********************
    This section of code is used to disable specific USB events that may not be
    desired by the user.  This can save code size and increase throughput and
    decrease CPU utilization.
********************************************************************/
#if defined USB_DISABLE_SUSPEND_HANDLER
    #define USB_SUSPEND_HANDLER(event, pointer, size)
    
    #warning "Disabling the suspend handler is not recommended.  Proper suspend handling is required to create a compliant USB device."                
#else
    #define USB_SUSPEND_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#if defined USB_DISABLE_WAKEUP_FROM_SUSPEND_HANDLER
    #define USB_WAKEUP_FROM_SUSPEND_HANDLER(event, pointer, size)

    #warning "Disabling the wake from suspend handler is not recommended.  Proper suspend handling is required to create a compliant USB device."                
#else
    #define USB_WAKEUP_FROM_SUSPEND_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#if defined USB_DISABLE_SOF_HANDLER
    #define USB_SOF_HANDLER(event, pointer, size)
#else
    #define USB_SOF_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#if defined USB_DISABLE_TRANSFER_TERMINATED_HANDLER
    #define USB_TRANSFER_TERMINATED_HANDLER(event, pointer, size)
#else
    #define USB_TRANSFER_TERMINATED_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#if defined USB_DISABLE_ERROR_HANDLER 
    #define USB_ERROR_HANDLER(event, pointer, size)
#else
    #define USB_ERROR_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#if defined USB_DISABLE_NONSTANDARD_EP0_REQUEST_HANDLER 
    #define USB_NONSTANDARD_EP0_REQUEST_HANDLER(event, pointer, size)
#else
    #define USB_NONSTANDARD_EP0_REQUEST_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#if defined USB_DISABLE_SET_DESCRIPTOR_HANDLER 
    #define USB_SET_DESCRIPTOR_HANDLER(event, pointer, size)
#else
    #define USB_SET_DESCRIPTOR_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size) 
#endif

#if defined USB_DISABLE_SET_CONFIGURATION_HANDLER
    #define USB_SET_CONFIGURATION_HANDLER(event, pointer, size)
#else
    #define USB_SET_CONFIGURATION_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#if defined USB_DISABLE_TRANSFER_COMPLETE_HANDLER 
    #define USB_TRANSFER_COMPLETE_HANDLER(event, pointer, size)
#else
    #define USB_TRANSFER_COMPLETE_HANDLER(event, pointer, size) USER_USB_CALLBACK_EVENT_HANDLER(event, pointer, size)
#endif

#endif // end of _USB_DEVICE_LOCAL_H_
