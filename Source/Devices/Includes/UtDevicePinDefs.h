/***************************************************************************/
/*  Copyright (C) 2009-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtDevicePinDefs_H)
#define UtDevicePinDefs_H

/*
    Device Pin Names
*/

#define CDevicePin_Ground L"GND"

#define CDevicePin_Vcc L"VCC"
#define CDevicePin_Vdd L"VDD"
#define CDevicePin_Vbb L"VBB"
#define CDevicePin_Vgg L"VGG"
#define CDevicePin_Vpp L"VPP"

#define CDevicePin_Input L"I"
#define CDevicePin_Output L"O"
#define CDevicePin_InputOutput L"I/O"
#define CDevicePin_InputClock L"I/CLK"
#define CDevicePin_Clock L"CLK"
#define CDevicePin_RegisteredOutput L"Q"
#define CDevicePin_InputOutputRegisteredOutput L"I/O/Q"
#define CDevicePin_NotUsed L"N/C"
#define CDevicePin_Strobe L"STROBE"
#define CDevicePin_LowWriteEnable L"~WE"
#define CDevicePin_LowReset L"~RST"

#define CDevicePin_HighProgram L"PGM"
#define CDevicePin_LowProgram L"~PGM"

#define CDevicePin_LowWriteEnable_LowProgram L"~WE/~PGM"

#define CDevicePin_LowOutputEnable_Vpp L"~OE/VPP"
#define CDevicePin_LowChipSelect1_Vpp L"~CS1/VPP"
#define CDevicePin_LowByte_Vpp L"~BYTE/VPP"
#define CDevicePin_LowReset_Vpp L"~RST/VPP"

#define CDevicePin_LowOutputEnable L"~OE"

#define CDevicePin_LowChipEnable L"~CE"
#define CDevicePin_LowChipEnable1 L"~CE1"
#define CDevicePin_LowChipEnable2 L"~CE2"
#define CDevicePin_LowChipEnable_Vpp L"~CE/VPP"

#define CDevicePin_HighChipEnable L"CE"
#define CDevicePin_HighChipEnable1 L"CE1"
#define CDevicePin_HighChipEnable2 L"CE2"
#define CDevicePin_HighChipEnable3 L"CE3"
#define CDevicePin_HighChipEnable4 L"CE4"

#define CDevicePin_LowChipSelect L"~CS"
#define CDevicePin_LowChipSelect1 L"~CS1"
#define CDevicePin_LowChipSelect2 L"~CS2"
#define CDevicePin_HighChipSelect2 L"CS2"

#define CDevicePin_HighFE1 L"FE1"
#define CDevicePin_HighFE2 L"FE2"

#define CDevicePin_LowChipSelect_ProgramEnable L"~CS/PE"
#define CDevicePin_LowChipSelect_Vpp L"~CS/VPP"

#define CDevicePin_Vcc_ProgramEnable L"VCC/PE"

/*
    Address Pin Names
*/

#define CDevicePin_Address0 L"A0"
#define CDevicePin_Address1 L"A1"
#define CDevicePin_Address2 L"A2"
#define CDevicePin_Address3 L"A3"
#define CDevicePin_Address4 L"A4"
#define CDevicePin_Address5 L"A5"
#define CDevicePin_Address6 L"A6"
#define CDevicePin_Address7 L"A7"
#define CDevicePin_Address8 L"A8"
#define CDevicePin_Address9 L"A9"
#define CDevicePin_Address10 L"A10"
#define CDevicePin_Address11 L"A11"
#define CDevicePin_Address12 L"A12"
#define CDevicePin_Address13 L"A13"
#define CDevicePin_Address14 L"A14"
#define CDevicePin_Address15 L"A15"
#define CDevicePin_Address16 L"A16"
#define CDevicePin_Address17 L"A17"
#define CDevicePin_Address18 L"A18"
#define CDevicePin_Address19 L"A19"
#define CDevicePin_Address20 L"A20"
#define CDevicePin_Address21 L"A21"

#define CDevicePin_Address15_A1 L"A15/A-1"

/*
    Data Pin Names
*/

#define CDevicePin_Data0 L"D0"
#define CDevicePin_Data1 L"D1"
#define CDevicePin_Data2 L"D2"
#define CDevicePin_Data3 L"D3"
#define CDevicePin_Data4 L"D4"
#define CDevicePin_Data5 L"D5"
#define CDevicePin_Data6 L"D6"
#define CDevicePin_Data7 L"D7"
#define CDevicePin_Data8 L"D8"
#define CDevicePin_Data9 L"D9"
#define CDevicePin_Data10 L"D10"
#define CDevicePin_Data11 L"D11"
#define CDevicePin_Data12 L"D12"
#define CDevicePin_Data13 L"D13"
#define CDevicePin_Data14 L"D14"
#define CDevicePin_Data15 L"D15"

#define CDevicePin_Data15_A1 L"D15/A-1"

/*
    Data Input/Output Pin Names
*/

#define CDevicePin_DataInOut0 L"D0/O0"
#define CDevicePin_DataInOut1 L"D1/O1"
#define CDevicePin_DataInOut2 L"D2/O2"

#endif /* end of UtDevicePinDefs_H */

/***************************************************************************/
/*  Copyright (C) 2009-2020 Kevin Eshbach                                  */
/***************************************************************************/
