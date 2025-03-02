# PEP

Unofficial Windows application for the EPROM+ System from Andromeda Research Labs

Copyright (C) 2006-2025 Kevin Eshbach  

## Requirements

Windows 10 or Windows 11

The application currently only works with a real parallel port.  A USB to Parallel Port adapter is not currently supported.  For laptops the Quatech (now B&B Electronics) SPPXP-100 ExpressCard or SPP-100 PC card can be used.

I just had to replace my laptop back in March 2019 and found that laptops do not come with an ExpressCard slot, but instead a Thunderbolt 3 connector.  The Quatech SPPXP-100 ExpressCard will work with the Sonnet Echo Pro ExpressCard/34 Thunderbolt Adapter.  The Sonnet Echo Pro ExpressCard/34 Thunderbolt Adapter uses a Thunderbolt 2 connector  (no Thunderbolt 2 cable is included and must be purchased separately) and also needs a Thunderbolt 2 to 3 adapter.  (I purchased Apple's Thunderbolt 2 cable and Thunderbolt 2 to 3 adapter.)  I also had to disable PCI Express power management for the card to function properly.

A USB based hardware replacement solution is currently in progress.

## Building

- Install Visual Studio 2022
- Install Windows 10 SDK, version 10.0.22621.0
- Install Windows WDK, version 10.0.26100.2454
- Launch a Command Prompt and go to the 'Source' directory
- Run the command "cscript build.wsf /verbose:+ /binaryType:Release" (without the quotes) to build the release configuration (To build the debug configuration replace "Release" with "Debug".)

## Programming Languages

The application is a combination of C, Managed C++ and C#.
