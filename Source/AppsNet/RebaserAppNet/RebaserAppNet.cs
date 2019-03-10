/***************************************************************************/
/*  Copyright (C) 2009-2018 Kevin Eshbach                                  */
/***************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;

namespace RebaserAppNet
{
    class Program
    {
        #region "Constants"
        private static string CBaseAddressName = "BaseAddress";
        #endregion

        #region "Enumerations"
        private enum FileType
        {
            Def,
            CsProj
        }
        #endregion

        #region "Structures"
        private struct TFileData
        {
            public FileType FileType;
            public System.String sFile;
        }
        #endregion

        #region "Member Variables"
        private static System.Collections.Generic.Dictionary<System.String, TFileData> s_FileDataDict = new System.Collections.Generic.Dictionary<System.String, TFileData>();
        private static System.String[] s_DllFilesExclude = {"zlibvc", "zlib", "RebaserNet", "PepCustomActions"};
        #endregion

        private static void RebaseFile(
            System.String sFile,
            System.String sSymbolPath,
            System.UInt64 nBaseAddress,
            out System.UInt64 nNewBaseAddress)
        {
            System.String sErrorMsg= "";

            nNewBaseAddress = 0;

            if ((nBaseAddress & 0xFFFF) != 0x0000)
            {
                nBaseAddress &= (System.UInt64.MaxValue - 0xFFFF);
                nBaseAddress += 0x00010000;
            }

            if (false == Common.Rebaser.ReBaseFile(sFile, sSymbolPath,
                                                   nBaseAddress,
                                                   ref sErrorMsg) ||
                false == Common.Rebaser.GetFileImageBase(sFile,
                                                         ref nNewBaseAddress,
                                                         ref sErrorMsg))
            {
                throw new System.Exception(sErrorMsg);
            }
        }

        private static void UpdateDefFile(
            System.String sDefFile,
            System.UInt64 nBaseAddress)
        {
            System.String sTmpDefFile = sDefFile + ".tmp";
            System.String[] sParseChars = {" ", "\t"};
            System.String[] sValues;
            System.String sLine;
            System.IO.StreamReader StreamReader;
            System.IO.StreamWriter StreamWriter;

            if ((System.IO.File.GetAttributes(sDefFile) & System.IO.FileAttributes.ReadOnly) == System.IO.FileAttributes.ReadOnly)
            {
                throw new System.Exception("The def file is read-only.");
            }

            StreamReader = new System.IO.StreamReader(sDefFile);
            StreamWriter = new System.IO.StreamWriter(sTmpDefFile);

            while ((sLine = StreamReader.ReadLine()) != null)
            {
                sValues = sLine.Split(sParseChars, System.StringSplitOptions.RemoveEmptyEntries);

                if (sValues.Length > 1 && sValues[0] == "LIBRARY")
                {
                    sLine = sValues[0] + " " + sValues[1] + " BASE=0x";
                    sLine = sLine + nBaseAddress.ToString("x");
                }

                StreamWriter.WriteLine(sLine);
            }

            StreamWriter.Close();
            StreamReader.Close();

            System.IO.File.Copy(sTmpDefFile, sDefFile, true);
            System.IO.File.Delete(sTmpDefFile);
        }

        private static void UpdateCsProjFile(
            System.String sCsProjFile,
            System.UInt64 nBaseAddress)
        {
            System.String sTmpCsProjFile = sCsProjFile + ".tmp";
            XmlDocument xmlDocument = new XmlDocument();
            XmlNodeList xmlNodeList;
            XmlAttribute xmlAttribute;
            XmlElement xmlElement;
            XmlNode xmlNodeBaseAddress;

            xmlDocument.Load(sCsProjFile);

            xmlNodeList = xmlDocument.GetElementsByTagName("PropertyGroup");

            foreach (XmlNode xmlNode in xmlNodeList)
            {
                xmlAttribute = xmlNode.Attributes["Condition"];

                if (xmlAttribute != null)
                {
                    switch (xmlAttribute.Value)
                    {
#if x86
                        case "'$(Configuration)|$(Platform)' == 'Release|x86'":
                        case "'$(Configuration)|$(Platform)' == 'Debug|x86'":
#elif x64
                        case "'$(Configuration)|$(Platform)' == 'Release|x64'":
                        case "'$(Configuration)|$(Platform)' == 'Debug|x64'":
#else
#error Unknown platform configuration
#endif

                            xmlElement = xmlNode[CBaseAddressName];

                            if (xmlElement == null)
                            {
                                xmlElement = xmlDocument.CreateElement(CBaseAddressName, FindNamespaceURI(xmlDocument));

                                xmlNode.AppendChild(xmlElement);
                            }
                            else
                            {
                                xmlElement.RemoveAll();
                            }

                            xmlNodeBaseAddress = xmlDocument.CreateNode(XmlNodeType.Text, CBaseAddressName, null);

                            xmlNodeBaseAddress.Value = nBaseAddress.ToString();

                            xmlElement.AppendChild(xmlNodeBaseAddress);
                            break;
                    }
                }
            }

            xmlDocument.Save(sTmpCsProjFile);

            System.IO.File.Copy(sTmpCsProjFile, sCsProjFile, true);
            System.IO.File.Delete(sTmpCsProjFile);
        }

        private static string FindNamespaceURI(XmlDocument xmlDocument)
        {
            foreach (XmlNode node in xmlDocument.ChildNodes)
            {
                if (node is XmlElement)
                {
                    return ((XmlElement)node).NamespaceURI;
                }
            }

            return "";
        }

        private static void EnumerateFiles(
            System.String sPath,
            FileType FileType)
        {
            System.String[] sDirectories = System.IO.Directory.GetDirectories(sPath);
            System.String[] sFiles;
            System.String sFileName, sSearchPattern;
            bool bIncludeFile;
            TFileData FileData;

            switch (FileType)
            {
                case FileType.Def:
                    sSearchPattern = "*.def";
                    break;
                case FileType.CsProj:
                    sSearchPattern = "*.csproj";
                    break;
                default:
                    sSearchPattern = "";

                    System.Diagnostics.Debug.Assert(false);
                    break;
            }

            sFiles = System.IO.Directory.GetFiles(sPath, sSearchPattern);

            foreach (string sDir in sDirectories)
            {
                EnumerateFiles(sDir, FileType);
            }

            foreach (string sFile in sFiles)
            {
                bIncludeFile = false;

                sFileName = System.IO.Path.GetFileNameWithoutExtension(sFile);

                if (FileType == FileType.Def)
                {
#if x86
                    bIncludeFile = !sFile.EndsWith("64.def");
#elif x64
                    bIncludeFile = sFile.EndsWith("64.def");

                    sFileName = sFileName.Replace("64", "");
#else
#error Unknown platform configuration
#endif
                }
                else
                {
                    bIncludeFile = true;
                }

                foreach (System.String sFileExclude in s_DllFilesExclude)
                {
                    if (sFileExclude == sFileName)
                    {
                        bIncludeFile = false;

                        break;
                    }
                }

                if (bIncludeFile)
                {
                    FileData = new TFileData();

                    FileData.FileType = FileType;
                    FileData.sFile = sFile;

                    s_FileDataDict.Add(sFileName, FileData);
                }
            }
        }

        private static void DisplayHelp()
        {
            System.Console.WriteLine();
            System.Console.WriteLine("Rebaser v1.00");
            System.Console.WriteLine("Copyright (C) 2009-2018 Kevin Eshbach");
            System.Console.WriteLine();
            System.Console.WriteLine("RebaserAppNet \"Root Path\" \"Binary Path\" \"Base Address\"");
            System.Console.WriteLine();
            System.Console.WriteLine("    \"Root Path\"    - Top level location of the projects");
            System.Console.WriteLine("    \"Binary Path\"  - Location of the dll files");
            System.Console.WriteLine("    \"Base Address\" - Starting address in hex (Example: 0xF00D)");
            System.Console.WriteLine();
        }
        
        static void Main(string[] args)
        {
            System.String sRootPath, sBinaryPath, sBasePath, sFileName;
            System.String[] sDllFiles;
            System.UInt64 nBaseAddress, nTmpBaseAddress;
            System.IO.FileInfo FileInfo;

            if (args.Length != 3)
            {
                DisplayHelp();

                System.Environment.Exit(-1);
            }

            try
            {
                sRootPath = args[0];
                sBinaryPath = args[1];
                sBasePath = args[2];

                System.Console.Write("Parsing the base address argument...");

                if (sBasePath.Length > 2 && sBasePath[0] == '0' && sBasePath[1] == 'x')
                {
                    sBasePath = sBasePath.Substring(2);
                }
                else
                {
                    throw new System.FormatException("Number not in the correct format");
                }

                nBaseAddress = System.UInt64.Parse(sBasePath, System.Globalization.NumberStyles.HexNumber);

                System.Console.WriteLine("Successful");

                System.Console.Write("Searching for all def files in \"" + sRootPath + "\"...");

                EnumerateFiles(sRootPath, FileType.Def);

                System.Console.WriteLine("Successful");

                System.Console.Write("Searching for all C# project files in \"" + sRootPath + "\"...");

                EnumerateFiles(sRootPath, FileType.CsProj);

                System.Console.WriteLine("Successful");

                System.Console.Write("Searching for all dll files in \"" + sBinaryPath + "\"...");

                sDllFiles = System.IO.Directory.GetFiles(sBinaryPath, "*.dll");

                System.Console.WriteLine("Successful");

                foreach (string sFile in sDllFiles)
                {
                    FileInfo = new System.IO.FileInfo(sFile);

                    sFileName = System.IO.Path.GetFileNameWithoutExtension(sFile);

                    System.Console.WriteLine();

                    if (s_FileDataDict.ContainsKey(sFileName))
                    {
                        System.Console.Write(String.Format("Rebasing the file \"{0}\" (Length: 0x{1})...",
                                                           sFileName,
#if x86
                                                           FileInfo.Length.ToString("x8")));
#elif x64
                                                           FileInfo.Length.ToString("x16")));
#else
#error Unknown platform configuration
#endif

                        RebaseFile(sFile, sBinaryPath, nBaseAddress,
                                   out nTmpBaseAddress);

                        System.Console.WriteLine("Successful");

                        System.Console.Write("    New Base Address: 0x");
                        System.Console.WriteLine(nTmpBaseAddress.ToString("x"));

                        switch (s_FileDataDict[sFileName].FileType)
                        {
                            case FileType.Def:
                                System.Console.Write("Updating the def file...");

                                UpdateDefFile(s_FileDataDict[sFileName].sFile, nTmpBaseAddress);

                                System.Console.WriteLine("Successful");
                                break;
                            case FileType.CsProj:
                                System.Console.Write("Updating the C# project file...");

                                UpdateCsProjFile(s_FileDataDict[sFileName].sFile, nTmpBaseAddress);

                                System.Console.WriteLine("Successful");
                                break;
                            default:
                                System.Diagnostics.Debug.Assert(false);
                                break;
                        }

                        nBaseAddress = nTmpBaseAddress + (System.UInt64)FileInfo.Length;
                    }
                    else
                    {
                        System.Console.WriteLine(String.Format("Skipping the file: {0}", sFileName));
                    }
                }
            }

            catch (System.Exception e)
            {
                System.Console.WriteLine("Failed");
                System.Console.WriteLine();
                System.Console.WriteLine("Exception Occurred");
                System.Console.WriteLine("Message: " + e.Message);
                System.Console.WriteLine();

                System.Environment.Exit(1);
            }

            System.Environment.Exit(0);
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2009-2018 Kevin Eshbach                                  */
/***************************************************************************/
