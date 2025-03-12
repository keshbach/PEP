# PEP

Unofficial Windows application for the EPROM+ System from Andromeda Research Labs

Copyright (C) 2006-2025 Kevin Eshbach  

## Requirements

- Windows 10 or Windows 11
- PIC32MX250F128B based USB to EPROM+ adapter is required.

The PCB layout and schematics for the USB adapter are located on GitHub in the [USB to EPROM+ Adapter](https://github.com/keshbach/PCB123/tree/master/USB%20To%20EPROM%2B%20Adapter/PIC32MX250F128B) repository.

## Building

- Install Visual Studio 2022
- Install Windows 10 SDK, version 10.0.22621.0
- Install Windows WDK, version 10.0.26100.2454
- Launch a Command Prompt and go to the 'Source' directory
- Run the command "cscript build.wsf /verbose:+ /binaryType:Release" (without the quotes) to build the release configuration (To build the debug configuration replace "Release" with "Debug".)

## Programming Languages

The application is a combination of C, Managed C++ and C#.
