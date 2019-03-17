# PEP

Unofficial Windows application for the EPROM+ System from Andromeda Research Labs

## Requirements

The application currently only works with a real parallel port.  A USB to Parallel Port adapter is not currently supported.  For laptops the Quatech (now B&B Electronics) SPPXP-100 ExpressCard or SPP-100 PC card can be used.

## Building

- Install Visual Studio 2017
- Launch a Command Prompt and go to the 'Source' directory
- Run the command 'cscript build.wsf /verbose:+ /binaryType:Release' to build the release configuration (to build the debug configuration replace 'Release' with 'Debug'
