'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Copyright (C) 2014-2014 Kevin Eshbach
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

' FileSystemObject.CreateTextFile and FileSystemObject.OpenTextFile
const OpenAsASCII = 0
const OpenAsUnicode = -1

' FileSystemObject.CreateTextFile
const OverwriteIfExist = -1
const FailIfExist = 0

' FileSystemObject.OpenTextFile
const OpenAsDefault = -2
const CreateIfNotExist = -1
const FailIfNotExist = 0
const ForReading = 1 ' Open a file for reading only. You can't write to this file.
const ForWriting = 2
const ForAppending = 8 ' Open a file and write to the end of the file.

' OpenTextFile Format constants
const TristateUseDefault = 2 ' Opens the file using the system default.
const TristateTrue = 1 ' Opens the file as Unicode.
const TristateFalse = 0 ' Opens the file as ASCII.
               
' MSI
const msiOpenDatabaseModeReadOnly = 0
const msiOpenDatabaseModeTransact = 1
const msiOpenDatabaseModeCreate = 3
        
const msiViewModifyInsert = 1
const msiViewModifyUpdate = 2
const msiViewModifyAssign = 3
const msiViewModifyReplace = 4
const msiViewModifyDelete = 6

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Copyright (C) 2014-2014 Kevin Eshbach
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
