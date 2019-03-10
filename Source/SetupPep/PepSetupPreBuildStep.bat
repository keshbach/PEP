@echo off

rem
rem Pep Setup Pre Build Step Script File
rem Copyright (C) 2014-2018 Kevin Eshbach
rem

set BinaryType=%1
set SolutionPath=%2

pushd %SolutionPath%

cscript SetupPep\Setup\scripts\CreateSetup.wsf //NoLogo /binaryType:%BinaryType% /verbose:+

popd
