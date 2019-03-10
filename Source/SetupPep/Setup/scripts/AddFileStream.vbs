'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Copyright (C) 2014-2014 Kevin Eshbach
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

sub addFileStream(database, file, name)
    dim installer, view, record
        
    set installer = Wscript.CreateObject("WindowsInstaller.Installer")
    set view = database.OpenView("SELECT Name, Data FROM _Streams")

    if g_Verbose then
        WScript.Echo "Opening a view to the database's _Streams table."
        WScript.Echo
    end if

    set record = installer.CreateRecord(2)
            
    record.StringData(1) = name
            
    view.Execute record

    record.SetStream 2, file

    view.Modify msiViewModifyAssign, record
            
    set record = nothing
    Set view = Nothing
    Set installer = nothing

    if g_Verbose then
        WScript.Echo "File added to the stream."
        WScript.Echo
    end if
end sub

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Copyright (C) 2014-2014 Kevin Eshbach
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
