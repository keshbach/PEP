/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <p18cxxx.h>

#include "UtPortTypeDefs.h"

#include "UtUsbDefs.h"
#include "UtUsbHidDefs.h"

#include "usb.h"

#include "UsbTransferData.h"

#pragma udata
static UINT8 l_nIdleRate;
static UINT8 l_nActiveProtocol;               // [0] Boot Protocol [1] Report Protocol
static UINT8 l_nHidRptRxLen;

static void lHIDGetReportHandler(void);
static void lHIDSetReportHandler(void);

#pragma code

/** C L A S S  S P E C I F I C  R E Q ****************************************/
/******************************************************************************
 * Function:        void USBCheckHIDRequest(void)
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
void USBCheckHIDRequest(void)
{
    if (SetupPkt.Recipient != RCPT_INTF) return;
    
    if (SetupPkt.bIntfID != HID_INTF_ID) return;
    
    if (SetupPkt.bRequest == CUsbRequestCodeGetDescriptor)
    {
        switch (SetupPkt.bDscType)
        {
            case CUsbHidClassDescriptorTypeHID:
                g_nControlTransferSessionOwner = MUID_HID;
                mUSBGetHIDDscAdr(g_pSrc.pbyRom);      // See usbcfg.h
                g_wCount = sizeof(USB_HID_DSC);
                break;
            case CUsbHidClassDescriptorTypeReport:
                g_nControlTransferSessionOwner = MUID_HID;
                mUSBGetHIDRptDscAdr(g_pSrc.pbyRom);   // See usbcfg.h
                mUSBGetHIDRptDscSize(g_wCount);       // See usbcfg.h
                break;
            case CUsbHidClassDescriptorTypePhysicalDescriptor:
                // g_ControlTransferSessionOwner = MUID_HID;
                break;
        }//end switch(SetupPkt.bDscType)
        
        g_UsbDeviceStatus.ctrl_trf_mem = _ROM;
    }
    
    if (SetupPkt.RequestType != CLASS) return;
    
    switch (SetupPkt.bRequest)
    {
        case CUsbHidClassRequestGetReport:
            lHIDGetReportHandler();
            break;
        case CUsbHidClassRequestSetReport:
            lHIDSetReportHandler();
            break;
        case CUsbHidClassRequestGetIdle:
            g_nControlTransferSessionOwner = MUID_HID;
            g_pSrc.pbyRam = &l_nIdleRate;          // Set source
            g_UsbDeviceStatus.ctrl_trf_mem = _RAM; // Set memory type
            g_wCount = 1;                          // Set data count
            break;
        case CUsbHidClassRequestSetIdle:
            g_nControlTransferSessionOwner = MUID_HID;
            l_nIdleRate = MHighByteFromWord(SetupPkt.W_Value);
            break;
        case CUsbHidClassRequestGetProtocol:
            g_nControlTransferSessionOwner = MUID_HID;
            g_pSrc.pbyRam = &l_nActiveProtocol;    // Set source
            g_UsbDeviceStatus.ctrl_trf_mem = _RAM; // Set memory type
            g_wCount = 1;                          // Set data count
            break;
        case CUsbHidClassRequestSetProtocol:
            g_nControlTransferSessionOwner = MUID_HID;
            l_nActiveProtocol = MLowByteFromWord(SetupPkt.W_Value);
            break;
    }
}

void lHIDGetReportHandler(void)
{
    // g_nControlTransferSessionOwner = MUID_HID;
}

void lHIDSetReportHandler(void)
{
    // g_nControlTransferSessionOwner = MUID_HID;
    // g_pDst.bRam = (byte*)&g_nHidReportOut;
}

/** U S E R  A P I ***********************************************************/

/******************************************************************************
 * Function:        void HIDInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        HIDInitEP initializes HID endpoints, buffer descriptors,
 *                  internal state-machine, and variables.
 *                  It should be called after the USB host has sent out a
 *                  SET_CONFIGURATION request.
 *                  See USBStdSetCfgHandler() in usb9.c for examples.
 *
 * Note:            None
 *****************************************************************************/
void HIDInitEP(void)
{   
    l_nHidRptRxLen = 0;
    
    HID_UEP = EP_OUT_IN | HSHK_EN;                 // Enable 2 data pipes
    
    HID_BD_OUT.Cnt = sizeof(g_nHidReportOut);      // Set buffer size
    HID_BD_OUT.ADR = g_nHidReportOut;              // Set buffer address
    HID_BD_OUT.Stat._byte = _USIE | _DAT0 |_DTSEN; // Set status

    /*
     * Do not have to init Cnt of IN pipes here.
     * Reason:  Number of bytes to send to the host
     *          varies from one transaction to
     *          another. Cnt should equal the exact
     *          number of bytes to transmit for
     *          a given IN transaction.
     *          This number of bytes will only
     *          be known right before the data is
     *          sent.
     */
    HID_BD_IN.ADR = g_nHidReportIn;       // Set buffer address
    HID_BD_IN.Stat._byte = _UCPU | _DAT1; // Set status
}

/******************************************************************************
 * Function:        void HIDTxReport(char *buffer, byte nLen)
 *
 * PreCondition:    mHIDTxIsBusy() must return false.
 *
 *                  Value of 'nLen' must be equal to or smaller than
 *                  HID_INT_IN_EP_SIZE
 *                  For an interrupt endpoint, the largest buffer size is
 *                  64 bytes.
 *
 * Input:           buffer  : Pointer to the starting location of data bytes
 *                  nLen    : Number of bytes to be transferred
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Use this macro to transfer data located in data memory.
 *
 *                  Remember: mHIDTxIsBusy() must return false before user
 *                  can call this function.
 *                  Unexpected behavior will occur if this function is called
 *                  when mHIDTxIsBusy() == 0
 *
 *                  Typical Usage:
 *                  if(!mHIDTxIsBusy())
 *                      HIDTxReport(buffer, 3);
 *
 * Note:            None
 *****************************************************************************/
void HIDTxReport(char *buffer, UINT8 nLen)
{
	UINT8 i;
	
    /*
     * Value of nLen should be equal to or smaller than HID_INT_IN_EP_SIZE.
     * This check forces the value of nLen to meet the precondition.
     */
	if (nLen > HID_INT_IN_EP_SIZE)
    {
	    nLen = HID_INT_IN_EP_SIZE;
    }

   /*
    * Copy data from user's buffer to dual-ram buffer
    */
    for (i = 0; i < nLen; i++)
    {
    	g_nHidReportIn[i] = buffer[i];
    }

    HID_BD_IN.Cnt = nLen;
    
    mUSBBufferReady(HID_BD_IN);
}

/******************************************************************************
 * Function:        byte HIDRxReport(char *buffer, byte nLen)
 *
 * PreCondition:    Value of input argument 'len' should be smaller than the
 *                  maximum endpoint size responsible for receiving report
 *                  data from USB host for HID class.
 *                  Input argument 'buffer' should point to a buffer area that
 *                  is bigger or equal to the size specified by 'nLen'.
 *
 * Input:           buffer  : Pointer to where received bytes are to be stored
 *                  nLen    : The number of bytes expected.
 *
 * Output:          The number of bytes copied to buffer.
 *
 * Side Effects:    Publicly accessible variable hid_rpt_rx_len is updated
 *                  with the number of bytes copied to buffer.
 *                  Once HIDRxReport is called, subsequent retrieval of
 *                  hid_rpt_rx_len can be done by calling macro
 *                  mHIDGetRptRxLength().
 *
 * Overview:        HIDRxReport copies a string of bytes received through
 *                  USB HID OUT endpoint to a user's specified location. 
 *                  It is a non-blocking function. It does not wait
 *                  for data if there is no data available. Instead it returns
 *                  '0' to notify the caller that there is no data available.
 *
 * Note:            If the actual number of bytes received is larger than the
 *                  number of bytes expected (nLen), only the expected number
 *                  of bytes specified will be copied to buffer.
 *                  If the actual number of bytes received is smaller than the
 *                  number of bytes expected (nLen), only the actual number
 *                  of bytes received will be copied to buffer.
 *****************************************************************************/
byte HIDRxReport(char *buffer, UINT8 nLen)
{
    l_nHidRptRxLen = 0;
    
    if (!mHIDRxIsBusy())
    {
        /*
         * Adjust the expected number of bytes to equal
         * the actual number of bytes received.
         */
        if (nLen > HID_BD_OUT.Cnt)
        {
            nLen = HID_BD_OUT.Cnt;
        }
        
        /*
         * Copy data from dual-ram buffer to user's buffer
         */
        for (l_nHidRptRxLen = 0; l_nHidRptRxLen < nLen; l_nHidRptRxLen++)
        {
            buffer[l_nHidRptRxLen] = g_nHidReportOut[l_nHidRptRxLen];
        }

        /*
         * Prepare dual-ram buffer for next OUT transaction
         */
        HID_BD_OUT.Cnt = sizeof(g_nHidReportOut);
        
        mUSBBufferReady(HID_BD_OUT);
    }//end if
    
    return l_nHidRptRxLen;
}

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
