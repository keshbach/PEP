/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

/******************************************************************************
 * USB Definitions: Endpoint 0 Buffer
 *****************************************************************************/

#if !defined(USBDEFS_EP0_BUFF_H)
#define USBDEFS_EP0_BUFF_H

#include "UtTypeDefs.h"
#include "usbcfg.h"       // usbcfg.h contains required definitions

/******************************************************************************
 * CTRL_TRF_SETUP:
 *
 * Every setup packet has 8 bytes.
 * However, the buffer size has to equal the EP0_BUFF_SIZE value specified
 * in autofiles\usbcfg.h
 * The value of EP0_BUFF_SIZE can be 8, 16, 32, or 64.
 *
 * First 8 bytes are defined to be directly addressable to improve speed
 * and reduce code size.
 * Bytes beyond the 8th byte have to be accessed using indirect addressing.
 *****************************************************************************/
typedef union _CTRL_TRF_SETUP
{
    /** Array for indirect addressing ****************************************/
    struct
    {
        UINT8 _byte[EP0_BUFF_SIZE];
    };
    
    /** Standard Device Requests *********************************************/
    struct
    {
        UINT8 bmRequestType;
        UINT8 bRequest;    
        UINT16 wValue;
        UINT16 wIndex;
        UINT16 wLength;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        UINT16 W_Value;
        UINT16 W_Index;
        UINT16 W_Length;
    };
    struct
    {
        unsigned Recipient:5;           //Device,Interface,Endpoint,Other
        unsigned RequestType:2;         //Standard,Class,Vendor,Reserved
        unsigned DataDir:1;             //Host-to-device,Device-to-host
        unsigned :8;
        UINT8 bFeature;                  //DEVICE_REMOTE_WAKEUP,ENDPOINT_HALT
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        UINT8 bDscIndex;                 //For Configuration and String DSC Only
        UINT8 bDscType;                  //Device,Configuration,String
        UINT16 wLangID;                   //Language ID
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        UINT8 bDevADR;                   //Device Address 0-127
        UINT8 bDevADRH;                  //Must equal zero
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        UINT8 bCfgValue;                 //Configuration Value 0-255
        UINT8 bCfgRSD;                   //Must equal zero (Reserved)
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        UINT8 bAltID;                    //Alternate Setting Value 0-255
        UINT8 bAltID_H;                  //Must equal zero
        UINT8 bIntfID;                   //Interface Number Value 0-255
        UINT8 bIntfID_H;                 //Must equal zero
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        UINT8 bEPID;                     //Endpoint ID (Number & Direction)
        UINT8 bEPID_H;                   //Must equal zero
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned EPNum:4;               //Endpoint Number 0-15
        unsigned :3;
        unsigned EPDir:1;               //Endpoint Direction: 0-OUT, 1-IN
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    /** End: Standard Device Requests ****************************************/
    
} CTRL_TRF_SETUP;

/******************************************************************************
 * CTRL_TRF_DATA:
 *
 * Buffer size has to equal the EP0_BUFF_SIZE value specified
 * in autofiles\usbcfg.h
 * The value of EP0_BUFF_SIZE can be 8, 16, 32, or 64.
 *
 * First 8 bytes are defined to be directly addressable to improve speed
 * and reduce code size.
 * Bytes beyond the 8th byte have to be accessed using indirect addressing.
 *****************************************************************************/
typedef union _CTRL_TRF_DATA
{
    /** Array for indirect addressing ****************************************/
    struct
    {
        UINT8 _byte[EP0_BUFF_SIZE];
    };
    
    /** First 8-byte direct addressing ***************************************/
    struct
    {
        UINT8 _byte0;
        UINT8 _byte1;
        UINT8 _byte2;
        UINT8 _byte3;
        UINT8 _byte4;
        UINT8 _byte5;
        UINT8 _byte6;
        UINT8 _byte7;
    };
    struct
    {
        UINT16 _word0;
        UINT16 _word1;
        UINT16 _word2;
        UINT16 _word3;
    };
} CTRL_TRF_DATA;

#endif //USBDEFS_EP0_BUFF_H

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
