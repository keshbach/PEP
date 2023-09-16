@echo off

rem
rem Build zlib Script File
rem Copyright (C) 2014-2023 Kevin Eshbach
rem

set ZlibDirectory=zlib-1.3

echo Cleaning the zlib project

pushd ..\%ZlibDirectory%

nmake /NOLOGO -f Win32/Makefile.msc clean

echo Building the zlib project

nmake /NOLOGO -f Win32/Makefile.msc LOC=/Qspectre-

popd

rd /s /q include 2>&1 > nul
rd /s /q lib\%VSCMD_ARG_TGT_ARCH% 2>&1 > nul

md include 2>&1 > nul
md lib 2>&1 > nul
md lib\%VSCMD_ARG_TGT_ARCH% 2>&1 > nul

copy ..\%ZlibDirectory%\zlib.lib lib\%VSCMD_ARG_TGT_ARCH% 2>&1 > nul
copy ..\%ZlibDirectory%\zlib*.pdb lib\%VSCMD_ARG_TGT_ARCH% 2>&1 > nul
copy ..\%ZlibDirectory%\zlib.h include 2>&1 > nul
copy ..\%ZlibDirectory%\zconf.h include 2>&1 > nul
