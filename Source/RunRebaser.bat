@echo off

rem
rem Update dll base address script file
rem Copyright (C) 2014-2018 Kevin Eshbach
rem

set RebaserApp32=.\bin\Debug\x86\RebaserAppNet.exe
set RebaserApp64=.\bin\Debug\x64\RebaserAppNet.exe
set RootPath=.
set BinaryPath32=.\bin\Debug\x86
set BinaryPath64=.\bin\Debug\x64
set BaseAddress32=0x20000000
set BaseAddress64=0x400000000

if not exist %RebaserApp32% goto Rebase64

echo "Rebasing 32-bit dll's"

%RebaserApp32% %RootPath% %BinaryPath32% %BaseAddress32%

:Rebase64

if not exist %RebaserApp64% goto End

echo "Rebasing 64-bit dll's"

%RebaserApp64% %RootPath% %BinaryPath64% %BaseAddress64%

:End
