/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

function createDdfFile(name, ddfFile, filePath, filesArray, outputPath)
{
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var outStream, sourceFile, file;

    if (g_Verbose)
    {
        WScript.Echo("Creating the ddf file.");
        WScript.Echo();
    }

    outStream = fso.CreateTextFile(ddfFile, OverwriteIfExist, OpenAsASCII);

    if (g_Verbose)
    {
        WScript.Echo("ddf file created now beginning to populate with data.");
        WScript.Echo();
    }

    outStream.WriteLine(".OPTION EXPLICIT");
    outStream.WriteLine(".Set CabinetNameTemplate=" + name + "*.cab");
    outStream.WriteLine(".Set CabinetName1=" + name + ".cab");
    outStream.WriteLine(".Set ReservePerCabinetSize=8");
    outStream.WriteLine(".Set MaxDiskSize=CDROM");
    outStream.WriteLine(".Set CompressionType=MSZIP");
    outStream.WriteLine(".Set InfFileLineFormat=*file*|*Size*");
    outStream.WriteLine(".Set InfFileName=" + fso.BuildPath(outputPath, name + ".inf"));
    outStream.WriteLine(".Set RptFileName=" + fso.BuildPath(outputPath, name + ".rpt"));
    outStream.WriteLine(".Set InfHeader=");
    outStream.WriteLine(".Set InfFooter=");
    outStream.WriteLine(".Set DiskDirectoryTemplate=" + outputPath);
    outStream.WriteLine(".Set Compress=ON");
    outStream.WriteLine(".Set Cabinet=ON");
    outStream.WriteLine(".Set InfAttr=");
    outStream.WriteLine(".Set SourceDir=" + filePath);

    for (var index = 0; index < filesArray.length; ++index)
    {
        sourceFile = fso.BuildPath(filePath, filesArray[index].name);

        if (g_Verbose)
        {
            WScript.Echo("Source File:  " + sourceFile + "");
        }

        outStream.WriteLine("\"" + filesArray[index].name + "\" " +
                            filesArray[index].nameId);
    }

    if (g_Verbose)
    {
        WScript.Echo();
        WScript.Echo("Closing the ddf file.");
        WScript.Echo();
    }

    outStream.Close();
}

function createCabinetFile(name, outputPath, filePath, filesArray)
{
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var WshShell = WScript.CreateObject("WScript.Shell");
    var cabFile = fso.BuildPath(outputPath, name + ".cab");
    var ddfFile = fso.BuildPath(outputPath, name + ".ddf");
    var oExec;

    if (g_Verbose)
    {
        WScript.Echo("Cabinet File: " + cabFile);
        WScript.Echo("DDf File:     " + ddfFile);
        WScript.Echo();
    }

    if (g_Verbose)
    {
        WScript.Echo("Creating a DDF file for \"" + name + "\".");
        WScript.Echo();
    }

    createDdfFile(name, ddfFile, filePath, filesArray, outputPath);
            
    if (g_Verbose)
    {
        WScript.Echo("Creating a cabinet file for \"" + name + "\" with the MakeCab tool.");
        WScript.Echo();
    }

    oExec = WshShell.Exec("cmd /c MakeCab.exe /f " + ddfFile);

    while (!oExec.StdOut.AtEndOfStream)
    {
        strText = oExec.StdOut.ReadLine();

        if (g_Verbose)
        {        
            WScript.Echo(strText);
        }
    }

    if (g_Verbose)
    {
        WScript.Echo("MakeCab tool has finished.");
        WScript.Echo();
    }

    /*if (oExec.ExitCode != 0)
    {
        if (g_Verbose)
        {
            WScript.Echo("MakeCab tool has failed with the error code " + oExec.ExitCode + ".");
            WScript.Echo();
        }
                
        throw new Error("MakeCab tool failed.  (Error Code: " + oExec.ExitCode + ")");
    }*/
            
    if (g_Verbose)
    {
        WScript.Echo("Cabinet file for " + name + " was successfully created.");
        WScript.Echo();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
