@echo off

rem
rem Generate Console Using User's Rights Manifest Script File v1.01
rem Copyright (C) 2018-2023 Kevin Eshbach
rem All Rights Reserved
rem

set ScriptsPath=%~dp0

set AssemblyName=%1
set AssemblyDescription=%2
set ProcessorArchitecture=%3
set AssemblyFile=%4
set OutputPath=%5
set ExecutionLevel=asInvoker

pushd %ScriptsPath%

for /F %%i in ('cscript //NoLogo ExtractVersionNumber.wsf /versionFile:%ScriptsPath%\..\Includes\UtVersion.h') do set AssemblyVersion=%%i

cscript //NoLogo GenerateManifestFile.wsf /appType:console /assemblyName:%AssemblyName% /assemblyDescription:%AssemblyDescription% /assemblyVersion:%AssemblyVersion% /processorArchitecture:%ProcessorArchitecture% /executionLevel:%ExecutionLevel% /assemblyFile:%AssemblyFile% /outputPath:%OutputPath% /verbose:+

popd
