/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace UpdateAssemblyVersionAppNet
{
    class Program
    {
        #region "Constants"
        private static System.String CAssemblyTitle = "AssemblyTitle";
        private static System.String CAssemblyCopyright = "AssemblyCopyright";
        private static System.String CAssemblyVersion = "AssemblyVersion";
        private static System.String CAssemblyFileVersion = "AssemblyFileVersion";
        #endregion

        #region "Structures"
        private struct TFileData
        {
            public System.String sValue;
            public System.Int32 nLineNumber;
        }
        #endregion

        #region "Member Variables"
        private static System.String s_sMajorVersion;
        private static System.String s_sMinorVersion;
        private static System.String s_sVersion;
        private static System.String s_sCopyright;

        private static System.Collections.Specialized.StringCollection m_AssemblyFileCollection = new System.Collections.Specialized.StringCollection();
        private static System.String[] s_AssemblyTitleExclude = { "DotZLib" };
        #endregion

        private static System.Boolean IsAssemblyExcluded(
            System.String sAssemblyTitle)
        {
            foreach (System.String sAssemblyTitleExclude in s_AssemblyTitleExclude)
            {
                if (0 == System.String.Compare(sAssemblyTitleExclude, sAssemblyTitle, true))
                {
                    return true;
                }
            }

            return false;
        }

        private static void ParseDefineStatement(
            System.String sData,
            out System.String sKeyword,
            out System.String sValue)
        {
            System.Int32 nIndex = 7;

            sKeyword = "";
            sValue = "";

            while (sData[nIndex] == ' ' || sData[nIndex] == '\t')
            {
                ++nIndex;
            }

            while (nIndex < sData.Length && sData[nIndex] != ' ' && sData[nIndex] != '\t' &&
                   sData[nIndex] != '\r' && sData[nIndex] != '\n')
            {
                sKeyword += sData[nIndex];

                ++nIndex;
            }

            while ((nIndex < sData.Length && sData[nIndex] != '\r' && sData[nIndex] != '\n') &&
                   (sData[nIndex] == ' ' || sData[nIndex] == '\t'))
            {
                ++nIndex;
            }

            while (nIndex < sData.Length && sData[nIndex] != '\r' && sData[nIndex] != '\n')
            {
                sValue += sData[nIndex];

                ++nIndex;
            }

            sValue = sValue.Trim();

            if (sValue.Length > 0 && sValue[0] == 'L')
            {
                sValue = sValue.Remove(0, 1);
            }
        }

        private static void ParseAssemblyStatement(
            System.String sData,
            out System.String sKeyword,
            out System.String sValue)
        {
            System.Int32 nIndex = 10;

            sKeyword = "";
            sValue = "";

            while (nIndex < sData.Length && sData[nIndex] == ' ')
            {
                ++nIndex;
            }

            while (nIndex < sData.Length && sData[nIndex] != '(')
            {
                sKeyword += sData[nIndex];

                ++nIndex;
            }

            sKeyword = sKeyword.Trim();

            while (nIndex < sData.Length && sData[nIndex] != '\"')
            {
                ++nIndex;
            }

            ++nIndex;

            while (nIndex < sData.Length && sData[nIndex] != '\"')
            {
                sValue += sData[nIndex];

                ++nIndex;
            }
        }

        private static System.Boolean ParseVersionFileName(
            System.String sVersionFileName)
        {
            System.IO.StreamReader StreamReader = new System.IO.StreamReader(sVersionFileName);
            System.String sLine, sKeyword, sValue;

            while ((sLine = StreamReader.ReadLine()) != null)
            {
                if (sLine.StartsWith("#define"))
                {
                    ParseDefineStatement(sLine, out sKeyword, out sValue);

                    switch (sKeyword)
                    {
                        case "CUtMajorVersion":
                            s_sMajorVersion = sValue;
                            break;
                        case "CUtMinorVersion":
                            s_sMinorVersion = sValue;
                            break;
                        case "CUtVersionString":
                            s_sVersion = sValue;
                            break;
                        case "CUtCopyrightString":
                            s_sCopyright = sValue;
                            break;
                    }
                }
            }

            StreamReader.Close();

            if (System.String.IsNullOrWhiteSpace(s_sMajorVersion) ||
                System.String.IsNullOrWhiteSpace(s_sMinorVersion) ||
                System.String.IsNullOrWhiteSpace(s_sVersion) ||
                System.String.IsNullOrWhiteSpace(s_sCopyright))
            {
                return false;
            }

            return true;
        }

        private static void EnumerateFiles(
            System.String sPath)
        {
            System.String[] sDirectories = System.IO.Directory.GetDirectories(sPath);
            System.String[] sFiles;

            sFiles = System.IO.Directory.GetFiles(sPath, "AssemblyInfo.cs");

            foreach (string sDir in sDirectories)
            {
                EnumerateFiles(sDir);
            }

            foreach (string sFile in sFiles)
            {
                m_AssemblyFileCollection.Add(sFile);
            }
        }

        private static void ReadAssemblyFile(
            System.String sFile,
            out System.Collections.Generic.Dictionary<System.String, TFileData> AssemblyDict)
        {
            System.IO.StreamReader StreamReader = new System.IO.StreamReader(sFile);
            System.Int32 nLineNumber = 1;
            System.String sLine, sKeyword, sValue;
            TFileData FileData;

            AssemblyDict = new System.Collections.Generic.Dictionary<System.String, TFileData>();

            while ((sLine = StreamReader.ReadLine()) != null)
            {
                if (sLine.StartsWith("[assembly:"))
                {
                    ParseAssemblyStatement(sLine, out sKeyword, out sValue);

                    FileData = new TFileData();

                    FileData.sValue = sValue;
                    FileData.nLineNumber = nLineNumber;

                    AssemblyDict.Add(sKeyword, FileData);
                }

                ++nLineNumber;
            }

            StreamReader.Close();
        }

        private static System.Boolean WriteAssemblyFile(
            System.String sFile,
            System.Collections.Generic.Dictionary<System.String, TFileData> AssemblyDict)
        {
            System.IO.StreamReader StreamReader = new System.IO.StreamReader(sFile);
            System.Int32 nLineNumber = 1;
            System.String sLine, sTempFile;
            System.IO.StreamWriter StreamWriter;

            sTempFile = System.IO.Path.GetTempFileName();

            StreamWriter = new System.IO.StreamWriter(sTempFile);

            while ((sLine = StreamReader.ReadLine()) != null)
            {
                foreach (TFileData FileData in AssemblyDict.Values)
                {
                    if (FileData.nLineNumber == nLineNumber)
                    {
                        if (FileData.sValue == CAssemblyCopyright)
                        {
                            sLine = System.String.Format("[assembly: AssemblyCopyright({0})]", s_sCopyright);
                        }
                        else if (FileData.sValue == CAssemblyVersion)
                        {
                        }
                        else if (FileData.sValue == CAssemblyFileVersion)
                        {
                        }
                    }
                }

                StreamWriter.WriteLine(sLine);

                ++nLineNumber;
            }

            StreamReader.Close();
            StreamWriter.Close();

            // move temp file

            System.IO.File.Delete(sTempFile);

            return true;
        }

        private static void UpdateAssemblyFiles()
        {
            System.Collections.Generic.Dictionary<System.String, TFileData> AssemblyDict;

            foreach (System.String sFile in m_AssemblyFileCollection)
            {
                ReadAssemblyFile(sFile, out AssemblyDict);

                if (AssemblyDict.ContainsKey(CAssemblyTitle))
                {
                    if (!IsAssemblyExcluded(AssemblyDict[CAssemblyTitle].sValue))
                    {
                        System.Console.WriteLine();
                        System.Console.Write("Updating the assembly file \"{0}\"...", sFile);

                        System.Console.WriteLine(WriteAssemblyFile(sFile, AssemblyDict) ? "Successful" : "Failed");
                    }
                    else
                    {
                        System.Console.WriteLine();
                        System.Console.WriteLine("Excluding the assembly file \"{0}\".", sFile);
                    }
                }
            }
        }

         private static void DisplayHelp()
        {
            System.Console.WriteLine();
            System.Console.WriteLine("Update C# Assembly Version v1.00");
            System.Console.WriteLine("Copyright (C) 2019 Kevin Eshbach");
            System.Console.WriteLine();
            System.Console.WriteLine("UpdateAssemblyVersionAppNet \"Root Path\" \"Version File\"");
            System.Console.WriteLine();
            System.Console.WriteLine("    \"Root Path\"    - Top level location of the projects");
            System.Console.WriteLine("    \"Version File\" - File with the version information");
            System.Console.WriteLine();
        }

        static void Main(string[] args)
        {
            System.String sRootPath, sVersionFileName;

            if (args.Length != 2)
            {
                DisplayHelp();

                System.Environment.Exit(-1);
            }

            try
            {
                sRootPath = args[0];
                sVersionFileName = args[1];

                System.Console.Write("Parsing the version file name...");

                if (ParseVersionFileName(sVersionFileName))
                {
                    System.Console.WriteLine("Successful");
                }
                else
                {
                    throw new Exception("Failed to parse version file");
                }

                System.Console.WriteLine();
                System.Console.WriteLine("Major Version: {0}", s_sMajorVersion);
                System.Console.WriteLine("Minor Version: {0}", s_sMinorVersion);
                System.Console.WriteLine("Version:       {0}", s_sVersion);
                System.Console.WriteLine("Copyright:     {0}", s_sCopyright);
                System.Console.WriteLine();

                System.Console.Write("Searching for AssemblyInfo files in \"" + sRootPath + "\"...");

                EnumerateFiles(sRootPath);

                System.Console.WriteLine("Successful");
                System.Console.WriteLine();
                System.Console.WriteLine("Updating assembly files.");

                UpdateAssemblyFiles();
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
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/
