@echo off

rem
rem Generate Console Requiring Admin Rights Manifest Script File
rem Copyright (C) 2018 Kevin Eshbach
rem

set ScriptsPath=%~dp0

set AssemblyName=%1
set AssemblyDescription=%2
set AssemblyVersion=%3
set ProcessorArchitecture=%4
set AssemblyFile=%5
set OutputPath=%6
set ExecutionLevel=requireAdministrator

pushd %ScriptsPath%

cscript //NoLogo GenerateManifestFile.wsf /appType:console /assemblyName:%AssemblyName% /assemblyDescription:%AssemblyDescription% /assemblyVersion:%AssemblyVersion% /processorArchitecture:%ProcessorArchitecture% /executionLevel:%ExecutionLevel% /assemblyFile:%AssemblyFile% /outputPath:%OutputPath% /verbose:+

popd
