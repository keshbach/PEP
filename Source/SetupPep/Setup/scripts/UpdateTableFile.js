/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

function appendMediaTable(cabinetPath, templatePath, dataArray)
{
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var lastSequence = 0;
    var mediaFile, outStream, cabNamesArray, infFile, filesArray;

    mediaFile = fso.BuildPath(templatePath, "Media.idt");

    if (g_Verbose)
    {
        WScript.Echo("Media File: " + mediaFile);
        WScript.Echo();
        WScript.Echo("Opening the Media table file.");
        WScript.Echo();
    }

    outStream = fso.OpenTextFile(mediaFile, ForAppending, false, TristateFalse);

    for (var index = 0; index < dataArray.length; ++index)
    {
        lastSequence += dataArray[index].filesArray.length;

        outStream.Write(index + 1);
        outStream.Write("\t" + lastSequence);
        outStream.Write("\tDisk 1");
        outStream.Write("\t#" + dataArray[index].name + ".cab");
        outStream.WriteLine("\t\t");
    }

    if (g_Verbose)
    {
        WScript.Echo("Closing the Media table file.");
        WScript.Echo();
    }
            
    outStream.Close();
}

function appendFileTable(cabinetPath, templatePath, dataArray)
{
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var sequence = 1;
    var fileFile, outStream, cabNamesArray;

    fileFile = fso.BuildPath(templatePath, "File.idt");

    if (g_Verbose)
    {
        WScript.Echo("File File: " + fileFile);
        WScript.Echo();
        WScript.Echo("Opening the File table file.");
        WScript.Echo();
    }

    outStream = fso.OpenTextFile(fileFile, ForAppending, false, TristateFalse);

    for (var index = 0; index < dataArray.length; ++index)
    {
        filesArray = dataArray[index].filesArray;

        for (var indexFile = 0; indexFile < filesArray.length; ++indexFile)
        {
            outStream.Write(filesArray[indexFile].nameId);
            outStream.Write("\t" + dataArray[index].componentName);
            outStream.Write("\t" +
                            generateFileTableFileName(dataArray[index].filesPath,
                                                      filesArray[indexFile].name));
            outStream.Write("\t" + filesArray[indexFile].size);
            outStream.Write("\t\t\t16896\t");
            outStream.WriteLine(sequence);
                    
            ++sequence;
        }
    }

    if (g_Verbose)
    {
        WScript.Echo("Closing the File table file.");
        WScript.Echo();
    }
            
    outStream.Close();
}

function generateFileTableFileName(sourcePath, name)
{
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var file = fso.GetFile(fso.BuildPath(sourcePath, name));
            
    return file.ShortName + "|" + name;
}

/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
