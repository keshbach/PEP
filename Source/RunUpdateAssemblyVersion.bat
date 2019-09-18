@echo off

rem
rem Update Microsoft Visual Studio C# Project Assembly files
rem Copyright (C) 2019 Kevin Eshbach
rem

set UpdateApp32=.\bin\Debug\x86\UpdateAssemblyVersionAppNet.exe
set UpdateApp64=.\bin\Debug\x64\UpdateAssemblyVersionAppNet.exe
set RootPath=.
set VersionFile=.\Includes\UtVersion.h

if not exist %UpdateApp32% goto Update64

echo "Updating C# Project Assembly Files with the 32-bit app"

%UpdateApp32% %RootPath% %VersionFile%

goto End

:Update64

if not exist %RebaserApp64% goto End

echo "Updating C# Project Assembly Files with the 64-bit app"

%UpdateApp64% %RootPath% %VersionFile%

:End
