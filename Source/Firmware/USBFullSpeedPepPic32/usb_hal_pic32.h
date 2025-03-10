#if !defined(USB_HAL_PIC32_H)
#define USB_HAL_PIC32_H

#include "Compiler.h"
#include "usb_config.h"

#if (USB_PING_PONG_MODE != USB_PING_PONG__FULL_PING_PONG)
#error "PIC32 only supports full ping pong mode.  A different mode other than full ping pong is selected in the usb_config.h file."
#endif

#define USBSetBDTAddress(addr) {U1BDTP3 = (((UINT32)KVA_TO_PA(addr)) >> 24); U1BDTP2 = (((UINT32)KVA_TO_PA(addr)) >> 16); U1BDTP1 = (((UINT32)KVA_TO_PA(addr)) >> 8);}
#define USBPowerModule() U1PWRCbits.USBPWR = 1;
#define USBPingPongBufferReset U1CONbits.PPBRST

#define USBTransactionCompleteIE U1IEbits.TRNIE
#define USBTransactionCompleteIF U1IRbits.TRNIF
#define USBTransactionCompleteIFReg U1IR
#define USBTransactionCompleteIFBitNum 3

#define USBResetIE  U1IEbits.URSTIE
#define USBResetIF  U1IRbits.URSTIF
#define USBResetIFReg U1IR
#define USBResetIFBitNum 0

#define USBIdleIE U1IEbits.IDLEIE
#define USBIdleIF U1IRbits.IDLEIF
#define USBIdleIFReg U1IR
#define USBIdleIFBitNum 4

#define USBActivityIE U1OTGIEbits.ACTVIE
#define USBActivityIF U1OTGIRbits.ACTVIF
#define USBActivityIFReg U1OTGIR
#define USBActivityIFBitNum 4

#define USBSOFIE U1IEbits.SOFIE
#define USBSOFIF U1IRbits.SOFIF
#define USBSOFIFReg U1IR
#define USBSOFIFBitNum 2

#define USBStallIE U1IEbits.STALLIE
#define USBStallIF U1IRbits.STALLIF
#define USBStallIFReg U1IR
#define USBStallIFBitNum 7

#define USBErrorIE U1IEbits.UERRIE
#define USBErrorIF U1IRbits.UERRIF
#define USBErrorIFReg U1IR
#define USBErrorIFBitNum 1

#define USBSE0Event 0 // U1IRbits.URSTIF//  U1CONbits.SE0
#define USBSuspendControl U1PWRCbits.USUSPEND
#define USBPacketDisable U1CONbits.PKTDIS
#define USBResumeControl U1CONbits.RESUME

/* Buffer Descriptor Status Register Initialization Parameters */

//The _BSTALL definition is changed from 0x04 to 0x00 to
// fix a difference in the PIC18 and PIC24 definitions of this
// bit.  This should be changed back once the definitions are
// synced.
#define _BSTALL     0x04        //Buffer Stall enable
#define _DTSEN      0x08        //Data Toggle Synch enable
#define _DAT0       0x00        //DATA0 packet expected next
#define _DAT1       0x40        //DATA1 packet expected next
#define _DTSMASK    0x40        //DTS Mask
#define _USIE       0x80        //SIE owns buffer
#define _UCPU       0x00        //CPU owns buffer

#define _STAT_MASK  0xFC



#define USTAT_EP0_PP_MASK   ~0x04
#define USTAT_EP_MASK       0xFC
#define USTAT_EP0_OUT       0x00
#define USTAT_EP0_OUT_EVEN  0x00
#define USTAT_EP0_OUT_ODD   0x04

#define USTAT_EP0_IN        0x08
#define USTAT_EP0_IN_EVEN   0x08
#define USTAT_EP0_IN_ODD    0x0C
#define ENDPOINT_MASK       0xF0

#define BDT_BASE_ADDR_TAG   __attribute__ ((aligned (512)))
#define CTRL_TRF_SETUP_ADDR_TAG
#define CTRL_TRF_DATA_ADDR_TAG

#define UEP_STALL 0x0002

#define USB_PULLUP_ENABLE 0x00
#define USB_PULLUP_DISABLE 0x04

#define USB_INTERNAL_TRANSCEIVER 0x00
#define USB_EXTERNAL_TRANSCEIVER 0x01

#define USB_FULL_SPEED 0x04

#define MConvertToPhysicalAddress(a) ((UINT32)KVA_TO_PA(a))
#define MConvertToVirtualAddress(a)  PA_TO_KVA1(a)

#if ((__PIC32_FEATURE_SET__ >= 100) && (__PIC32_FEATURE_SET__ <= 299))
#define USBIE 0x00000008
#elif ((__PIC32_FEATURE_SET__ >= 330) && (__PIC32_FEATURE_SET__ <= 450)) || (__PIC32_FEATURE_SET__ == 470)
// For PIC32MX430,450,470 devices
#define USBIE 0x00000004
#else
#define USBIE 0x02000000
#endif

#define MUSBModuleDisable() {\
    U1CON = 0;\
    U1IE = 0;\
    U1OTGIE = 0;\
    U1PWRCbits.USBPWR = 1;\
    USBDeviceState = DETACHED_STATE;\
}    

/********************************************************************
 * Function (macro): void USBClearInterruptFlag(register, UINT8 if_flag_offset)
 *
 * PreCondition:    None
 *
 * Input:           
 *   register - the register mnemonic for the register holding the interrupt 
 *				flag to be "kleared"
 *   UINT8 if_flag_offset - the bit position offset (for the interrupt flag to 
 *		 					"klear") from the "right of the register"
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Klears the specified USB interrupt flag.
 *
 * Note:    		Individual USB interrupt flag bits are "Kleared" by writing
 *					'1' to the bit
 *******************************************************************/
#define USBClearInterruptFlag(reg_name, if_flag_offset)	(reg_name = (1 << if_flag_offset))	

/********************************************************************
    Function:
        void MDisableNonZeroEndpoints(UINT8 last_ep_num)
        
    Summary:
        Clears the control registers for the specified non-zero endpoints
        
    PreCondition:
        None
        
    Parameters:
        UINT8 last_ep_num - the last endpoint number to clear.  This
        number should include all endpoints used in any configuration.
        
    Return Values:
        None
        
    Remarks:
        None
 
 *******************************************************************/
#define MDisableNonZeroEndpoints(last_ep_num) {\
    UINT8 i;\
    UINT32 *p = (UINT32*)&U1EP1;\
    for (i = 0; i < last_ep_num; i++)\
    {\
        *p = 0;\
        p += 4;\
    }\
}

#define MUSBClearUSBInterrupt() INTClearFlag(INT_USB);
#define MUSBInterruptFlag IFS1bits.USBIF

#if defined(USB_DISABLE_SOF_HANDLER)
    #define USB_SOF_INTERRUPT 0x00
#else
    #define USB_SOF_INTERRUPT 0x04
#endif

#if defined(USB_DISABLE_ERROR_HANDLER)
    #define USB_ERROR_INTERRUPT 0x02
#else
    #define USB_ERROR_INTERRUPT 0x02
#endif

//STALLIE, IDLEIE, TRNIE, and URSTIE are all enabled by default and are required
#if defined(USB_INTERRUPT)
    #if ((__PIC32_FEATURE_SET__ >= 100) && (__PIC32_FEATURE_SET__ <= 299))
    	#define USBEnableInterrupts() {\
            IEC1SET = USBIE;\
            IPC7CLR=0x00FF0000;\
            IPC7SET=0x00100000;\
            INTEnableSystemMultiVectoredInt();\
            INTEnableInterrupts();\
        }
    #elif ((__PIC32_FEATURE_SET__ >= 330) && (__PIC32_FEATURE_SET__ <= 450)) || (__PIC32_FEATURE_SET__ == 470)
        // For PIC32MX430,450,470 devices
        #define USBEnableInterrupts() {\
            IEC1SET = USBIE;\
            IPC7CLR=0x00FF00;\
            IPC7SET=0x001000;\
            INTEnableSystemMultiVectoredInt();\
            INTEnableInterrupts();\
        }
    #else
        #define USBEnableInterrupts() {\
            IEC1SET = USBIE;\
            IPC11CLR = 0x0000FF00;\
            IPC11SET = 0x00001000;\
            INTEnableSystemMultiVectoredInt();\
            INTEnableInterrupts();\
        }
    #endif
#endif

#define MUSBDisableInterrupts() { IEC1CLR = USBIE; }

#if defined(USB_INTERRUPT)
#define MUSBMaskInterrupts() { IEC1CLR = USBIE; }
#define MUSBUnmaskInterrupts() { IEC1SET = USBIE; }
#endif

#define EP_CTRL     0x0C            // Cfg Control pipe for this ep
#define EP_OUT      0x18            // Cfg OUT only pipe for this ep
#define EP_IN       0x14            // Cfg IN only pipe for this ep
#define EP_OUT_IN   0x1C            // Cfg both OUT & IN pipes for this ep
#define HSHK_EN     0x01            // Enable handshake packet
                                    // Handshake should be disable for isoch

#define USB_HANDSHAKE_ENABLED   0x01
#define USB_HANDSHAKE_DISABLED  0x00

#define USB_OUT_ENABLED         0x08
#define USB_OUT_DISABLED        0x00

#define USB_IN_ENABLED          0x04
#define USB_IN_DISABLED         0x00

#define USB_ALLOW_SETUP         0x00
#define USB_DISALLOW_SETUP      0x10

#define USB_STALL_ENDPOINT      0x02

#define MSetConfigurationOptions() {\
    U1CNFG1 = 0;\
    U1EIE = 0x9F;\
    U1IE = 0x99 | USB_SOF_INTERRUPT | USB_ERROR_INTERRUPT;\
    U1OTGCON &= 0x000F;\
    U1OTGCON |= USB_PULLUP_OPTION;\
 }

#define MUSBClearInterruptRegister(reg) reg = 0xFF;

// Buffer Descriptor Status Register layout.
typedef union __attribute__ ((packed)) tagTBufferDescriptorStatusRegister
{
    struct __attribute__ ((packed)){
        unsigned :2;
        unsigned BSTALL:1; //Buffer Stall Enable
        unsigned DTSEN:1;  //Data Toggle Synch Enable
        unsigned :2;       //Reserved - write as 00
        unsigned DTS:1;    //Data Toggle Synch Value
        unsigned UOWN:1;   //USB Ownership
    };
    struct __attribute__ ((packed)){
        unsigned :2;
        unsigned PID0:1;
        unsigned PID1:1;
        unsigned PID2:1;
        unsigned PID3:1;
    };
    struct __attribute__ ((packed)){
        unsigned :2;
        unsigned PID:4; //Packet Identifier
    };
    UINT16 Val;
} TBufferDescriptorStatusRegister;

// BDT Entry Layout
typedef union __attribute__ ((packed))__BDT
{
    struct __attribute__ ((packed))
    {
        TBufferDescriptorStatusRegister STAT;
        UINT16 CNT:10;
        UINT32 ADR;                      //Buffer Address
    };
    struct __attribute__ ((packed))
    {
        UINT32 res:16;
        UINT32 count:10;
    };
    UINT32 w[2];
    UINT16 v[4];
    UINT64 Val;
} BDT_ENTRY;

// USTAT Register Layout
typedef union __USTAT
{
    struct
    {
        UINT8 filler1:2;
        UINT8 ping_pong:1;
        UINT8 direction:1;
        UINT8 endpoint_number:4;
    };
    UINT8 Val;
} USTAT_FIELDS;

//Macros for fetching parameters from a USTAT_FIELDS variable.
#define MUSBHALGetLastEndpoint(stat)  stat.endpoint_number
#define MUSBHALGetLastDirection(stat) stat.direction

extern volatile BDT_ENTRY* pBDTEntryOut[USB_MAX_EP_NUMBER + 1];
extern volatile BDT_ENTRY* pBDTEntryIn[USB_MAX_EP_NUMBER + 1];	

#endif  //USB_HAL_PIC32_H
