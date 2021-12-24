/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>

#include "UtTypeDefs.h"
#include "UtPortTypeDefs.h"

#include "UtUsbDefs.h"

#include "usb.h"
#include "Util.h"

#include "UsbTransferData.h"

#include <Includes/UtMacros.h>

#pragma udata

static void lUSBStdGetDscHandler(void);
static void lUSBStdSetCfgHandler(void);
static void lUSBStdGetStatusHandler(void);
static void lUSBStdFeatureReqHandler(void);

#pragma code
/******************************************************************************
 * Function:        void USBCheckStdRequest(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks the setup data packet to see if it
 *                  knows how to handle it
 *
 * Note:            None
 *****************************************************************************/
void USBCheckStdRequest(void)
{   
    if (SetupPkt.RequestType != STANDARD) return;
    
    switch (SetupPkt.bRequest)
    {
        case CUsbRequestCodeSetAddress:
            g_nControlTransferSessionOwner = MUID_USB9;
            g_nUsbDeviceState = ADR_PENDING_STATE;       // Update state only
            /* See USBCtrlTrfInHandler() in usbctrltrf.c for the next step */
            break;
        case CUsbRequestCodeGetDescriptor:
            lUSBStdGetDscHandler();
            break;
        case CUsbRequestCodeSetConfiguration:
            lUSBStdSetCfgHandler();
            break;
        case CUsbRequestCodeGetConfiguration:
            g_nControlTransferSessionOwner = MUID_USB9;
            g_pSrc.pbyRam = &g_nUsbActiveCfg;      // Set Source
            g_UsbDeviceStatus.ctrl_trf_mem = _RAM; // Set memory type
            g_wCount = 1;                          // Set data count
            break;
        case CUsbRequestCodeGetStatus:
            lUSBStdGetStatusHandler();
            break;
        case CUsbRequestCodeClearFeature:
        case CUsbRequestCodeSetFeature:
            lUSBStdFeatureReqHandler();
            break;
        case CUsbRequestCodeGetInterface:
            g_nControlTransferSessionOwner = MUID_USB9;
            g_pSrc.pbyRam = (UINT8*)&g_nUsbAlternateInterface + SetupPkt.bIntfID; // Set source
            g_UsbDeviceStatus.ctrl_trf_mem = _RAM;                                // Set memory type
            g_wCount = 1;                                                         // Set data count
            break;
        case CUsbRequestCodeSetInterface:
            g_nControlTransferSessionOwner = MUID_USB9;
            g_nUsbAlternateInterface[SetupPkt.bIntfID] = SetupPkt.bAltID;
            break;
        case CUsbRequestCodeSetDescriptor:
        case CUsbRequestCodeSynchFrame:
        default:
            break;
    }
}

/******************************************************************************
 * Function:        void USBStdGetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard GET_DESCRIPTOR request.
 *                  It utilizes tables dynamically looks up descriptor size.
 *                  This routine should never have to be modified if the tables
 *                  in usbdsc.c are declared correctly.
 *
 * Note:            None
 *****************************************************************************/
static void lUSBStdGetDscHandler(void)
{
    if (SetupPkt.bmRequestType == 0x80)
    {
        switch (SetupPkt.bDscType)
        {
            case CUsbDescriptorTypeDevice:
                g_nControlTransferSessionOwner = MUID_USB9;
                g_pSrc.pbyRom = (rom UINT8*)&g_DeviceDescriptor;
                g_wCount = sizeof(g_DeviceDescriptor); // Set data count
                break;
            case CUsbDescriptorTypeConfiguration:
                g_nControlTransferSessionOwner = MUID_USB9;
                g_pSrc.pbyRom = *(g_USB_CD_Ptr + SetupPkt.bDscIndex);
                g_wCount = *(g_pSrc.pwRom + 1); // Set data count
                break;
            case CUsbDescriptorTypeString:
                g_nControlTransferSessionOwner = MUID_USB9;
                g_pSrc.pbyRom = *(g_USB_SD_Ptr + SetupPkt.bDscIndex);
                g_wCount = *g_pSrc.pbyRom; // Set data count
                break;
        }
        
        g_UsbDeviceStatus.ctrl_trf_mem = _ROM; // Set memory type
    }
}

/******************************************************************************
 * Function:        void USBStdSetCfgHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine first disables all endpoints by clearing
 *                  UEP registers. It then configures (initializes) endpoints
 *                  specified in the modifiable section.
 *
 * Note:            None
 *****************************************************************************/
static void lUSBStdSetCfgHandler(void)
{
    g_nControlTransferSessionOwner = MUID_USB9;
    
    mDisableEP1to15();                          // See usbdrv.h
    
    UtClearArray(g_nUsbAlternateInterface, MArrayLen(g_nUsbAlternateInterface));
    
    g_nUsbActiveCfg = SetupPkt.bCfgValue;
    
    if (SetupPkt.bCfgValue == 0)
    {
        g_nUsbDeviceState = ADDRESS_STATE;
    }
    else
    {
        g_nUsbDeviceState = CONFIGURED_STATE;

        HIDInitEP();
    }//end if(SetupPkt.bcfgValue == 0)
}

/******************************************************************************
 * Function:        void USBStdGetStatusHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard GET_STATUS request
 *
 * Note:            None
 *****************************************************************************/
static void lUSBStdGetStatusHandler(void)
{
    CtrlTrfData._byte0 = 0;                         // Initialize content
    CtrlTrfData._byte1 = 0;
        
    switch (SetupPkt.Recipient)
    {
        case RCPT_DEV:
            g_nControlTransferSessionOwner = MUID_USB9;
            /*
             * _byte0: bit0: Self-Powered Status [0] Bus-Powered [1] Self-Powered
             *         bit1: RemoteWakeup        [0] Disabled    [1] Enabled
             */

            CtrlTrfData._byte0 |= 0b00000001;     // Set bit0

            if (g_UsbDeviceStatus.RemoteWakeup == 1) // usb_stat defined in usbmmap.c
            {
                CtrlTrfData._byte0 |= 0b00000010;    // Set bit1
            }
            break;
        case RCPT_INTF:
            g_nControlTransferSessionOwner = MUID_USB9;     // No data to update
            break;
        case RCPT_EP:
            g_nControlTransferSessionOwner = MUID_USB9;
            /*
             * _byte0: bit0: Halt Status [0] Not Halted [1] Halted
             */
            g_pDst.pbyRam = (UINT8*)&ep0Bo + (SetupPkt.EPNum * 8) + (SetupPkt.EPDir * 4);
            if (*g_pDst.pbyRam & _BSTALL)  // Use _BSTALL as a bit mask
            {
                CtrlTrfData._byte0 = 0x01; // Set bit0
            }
            break;
    }//end switch
    
    if (g_nControlTransferSessionOwner == MUID_USB9)
    {
        g_pSrc.pbyRam = (UINT8*)&CtrlTrfData;  // Set Source
        g_UsbDeviceStatus.ctrl_trf_mem = _RAM; // Set memory type
        g_wCount = 2;                          // Set data count
    }
}

/******************************************************************************
 * Function:        void USBStdFeatureReqHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard SET & CLEAR FEATURES
 *                  requests
 *
 * Note:            None
 *****************************************************************************/
static void lUSBStdFeatureReqHandler(void)
{
    if ((SetupPkt.bFeature == CUsbFeatureSelectorDeviceRemoteWakeup) &&
        (SetupPkt.Recipient == RCPT_DEV))
    {
        g_nControlTransferSessionOwner = MUID_USB9;
        
        if (SetupPkt.bRequest == CUsbRequestCodeSetFeature)
        {
            g_UsbDeviceStatus.RemoteWakeup = 1;
        }
        else
        {
            g_UsbDeviceStatus.RemoteWakeup = 0;
        }
    }
    
    if ((SetupPkt.bFeature == CUsbFeatureSelectorEndpointHalt) &&
        (SetupPkt.Recipient == RCPT_EP) &&
        (SetupPkt.EPNum != 0))
    {
        g_nControlTransferSessionOwner = MUID_USB9;
        
        /* Must do address calculation here */
        g_pDst.pbyRam = (UINT8*)&ep0Bo + (SetupPkt.EPNum * 8) + (SetupPkt.EPDir * 4);
        
        if (SetupPkt.bRequest == CUsbRequestCodeSetFeature)
        {
            *g_pDst.pbyRam = _USIE | _BSTALL;
        }
        else
        {
/********************************************************************
Bug Fix: January 4, 2008
*********************************************************************
Added '|DAT1' to force a halted IN endpoint to reinitialize to send
out DAT0 on the next transaction.
********************************************************************/
            if (SetupPkt.EPDir == 1) // IN
            {
                *g_pDst.pbyRam = _UCPU | _DAT1;
            }
            else
            {
                *g_pDst.pbyRam = _USIE | _DAT0 | _DTSEN;
            }
/*******************************************************************/
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
