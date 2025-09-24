# PEP

Unofficial Windows application for the EPROM+ System from Andromeda Research Labs

Copyright (C) 2006-2025 Kevin Eshbach  

## Requirements

- Microsoft Windows 10 or Windows 11
- USB to EPROM+ adapter based on the Microchip PIC32MX250F128B is required.

![USB To Eprom+ Adapter Image](https://github.com/keshbach/PCB123/blob/master/USB%20To%20EPROM%2B%20Adapter/PIC32MX250F128B/USB%20to%20EPROM%2B%20Adapter%20Image.png)

The PCB layout and schematics for the USB adapter are located on GitHub in the [USB to EPROM+ Adapter](https://github.com/keshbach/PCB123/tree/master/USB%20To%20EPROM%2B%20Adapter/PIC32MX250F128B) repository.

## Building App (and Tools)

- Install Microsoft Visual Studio 2022
- Install Microsoft Windows 10 SDK, version 10.0.22621.0
- Install Microsoft Windows WDK, version 10.0.26100.2454
- Launch a Command Prompt and go to the 'Source' directory
- Run the command "cscript build.wsf /verbose:+ /binaryType:Release" (without the quotes) to build the release configuration (To build the debug configuration replace "Release" with "Debug".)

## Building Firmware

- Install Microchip MPLAB X IDE version 5.40
- Install Microchip MPLAB C32 C Compiler version 2.02
- Firmware source code is located in the 'Source/Firmware/USBFullSpeedPepPic32' directory

## Programming Languages

The application is a combination of C, Managed C++ and C#.
