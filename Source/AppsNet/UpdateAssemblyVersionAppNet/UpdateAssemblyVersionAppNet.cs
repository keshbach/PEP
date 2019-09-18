/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/

using System;
using System.Text;

namespace UpdateAssemblyVersionAppNet
{
    class Program
    {
        #region "Constants"
        private static System.Text.Encoding ANSI = Encoding.GetEncoding(1252);

        private static System.String CAssemblyTitle = "AssemblyTitle";
        private static System.String CAssemblyCopyright = "AssemblyCopyright";
        private static System.String CAssemblyVersion = "AssemblyVersion";
        private static System.String CAssemblyFileVersion = "AssemblyFileVersion";
        #endregion

        #region "Structures"
        private struct TAssemblyData
        {
            public System.String sKeyword;
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

        #region "Properties"
        private static string AssemblyTitle
        {
            get
            {
                // Get all Title attributes on this assembly
                object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyTitleAttribute), false);
                // If there is at least one Title attribute
                if (attributes.Length > 0)
                {
                    // Select the first one
                    System.Reflection.AssemblyTitleAttribute titleAttribute = (System.Reflection.AssemblyTitleAttribute)attributes[0];
                    // If it is not an empty string, return it
                    if (titleAttribute.Title != "")
                        return titleAttribute.Title;
                }
                // If there was no Title attribute, or if the Title attribute was the empty string, return the .exe name
                return System.IO.Path.GetFileNameWithoutExtension(System.Reflection.Assembly.GetExecutingAssembly().CodeBase);
            }
        }

        private static string AssemblyVersion
        {
            get
            {
                return System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString();
            }
        }

        private static string AssemblyDescription
        {
            get
            {
                // Get all Description attributes on this assembly
                object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyDescriptionAttribute), false);
                // If there aren't any Description attributes, return an empty string
                if (attributes.Length == 0)
                    return "";
                // If there is a Description attribute, return its value
                return ((System.Reflection.AssemblyDescriptionAttribute)attributes[0]).Description;
            }
        }

        private static string AssemblyProduct
        {
            get
            {
                // Get all Product attributes on this assembly
                object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyProductAttribute), false);
                // If there aren't any Product attributes, return an empty string
                if (attributes.Length == 0)
                    return "";
                // If there is a Product attribute, return its value
                return ((System.Reflection.AssemblyProductAttribute)attributes[0]).Product;
            }
        }

        private static string AssemblyCopyright
        {
            get
            {
                // Get all Copyright attributes on this assembly
                object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyCopyrightAttribute), false);
                // If there aren't any Copyright attributes, return an empty string
                if (attributes.Length == 0)
                    return "";
                // If there is a Copyright attribute, return its value
                return ((System.Reflection.AssemblyCopyrightAttribute)attributes[0]).Copyright;
            }
        }

        private static string AssemblyCompany
        {
            get
            {
                // Get all Company attributes on this assembly
                object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyCompanyAttribute), false);
                // If there aren't any Company attributes, return an empty string
                if (attributes.Length == 0)
                    return "";
                // If there is a Company attribute, return its value
                return ((System.Reflection.AssemblyCompanyAttribute)attributes[0]).Company;
            }
        }
        #endregion

        #region "Private Functions"
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
            System.IO.StreamReader StreamReader = new System.IO.StreamReader(sVersionFileName, ANSI);
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

        private static Nullable<TAssemblyData> FindAssemblyDataFromKeyword(
            System.String sKeyword,
            System.Collections.Generic.List<TAssemblyData> AssemblyDataList)
        {
            foreach (TAssemblyData AssemblyData in AssemblyDataList)
            {
                if (AssemblyData.sKeyword == sKeyword)
                {
                    return AssemblyData;
                }
            }

            return null;
        }

        private static void ReadAssemblyFile(
            System.String sFile,
            out System.Collections.Generic.List<TAssemblyData> AssemblyDataList)
        {
            System.IO.StreamReader StreamReader = new System.IO.StreamReader(sFile, ANSI);
            System.Int32 nLineNumber = 1;
            System.String sLine, sKeyword, sValue;
            TAssemblyData AssemblyData;

            AssemblyDataList = new System.Collections.Generic.List<TAssemblyData>();

            while ((sLine = StreamReader.ReadLine()) != null)
            {
                if (sLine.StartsWith("[assembly:"))
                {
                    ParseAssemblyStatement(sLine, out sKeyword, out sValue);

                    AssemblyData = new TAssemblyData();

                    AssemblyData.sKeyword = sKeyword;
                    AssemblyData.sValue = sValue;
                    AssemblyData.nLineNumber = nLineNumber;

                    AssemblyDataList.Add(AssemblyData);
                }

                ++nLineNumber;
            }

            StreamReader.Close();
        }

        private static System.Boolean WriteAssemblyFile(
            System.String sFile,
            System.Collections.Generic.List<TAssemblyData> AssemblyDataList)
        {
            System.IO.StreamReader StreamReader = new System.IO.StreamReader(sFile, ANSI);
            System.Int32 nLineNumber = 1;
            System.String sLine, sTempFile;
            System.IO.StreamWriter StreamWriter;

            sTempFile = System.IO.Path.GetTempFileName();

            StreamWriter = new System.IO.StreamWriter(sTempFile, false, ANSI);

            while ((sLine = StreamReader.ReadLine()) != null)
            {
                foreach (TAssemblyData AssemblyData in AssemblyDataList)
                {
                    if (AssemblyData.nLineNumber == nLineNumber)
                    {
                        if (AssemblyData.sKeyword == CAssemblyCopyright)
                        {
                            sLine = System.String.Format("[assembly: AssemblyCopyright({0})]", s_sCopyright);
                        }
                        else if (AssemblyData.sKeyword == CAssemblyVersion)
                        {
                            sLine = System.String.Format("[assembly: AssemblyVersion(\"{0}.{1}.0.0\")]", s_sMajorVersion, s_sMinorVersion);
                        }
                        else if (AssemblyData.sKeyword == CAssemblyFileVersion)
                        {
                            sLine = System.String.Format("[assembly: AssemblyFileVersion(\"{0}.{1}.0.0\")]", s_sMajorVersion, s_sMinorVersion);
                        }
                    }
                }

                StreamWriter.WriteLine(sLine);

                ++nLineNumber;
            }

            StreamReader.Close();
            StreamWriter.Close();

            try
            {
                System.IO.File.Copy(sTempFile, sFile, true);
            }
            catch (Exception ex)
            {
                System.Console.WriteLine();
                System.Console.WriteLine("Failed to copy the file \"{0}\" to \"{1}\".", sTempFile, sFile);
                System.Console.WriteLine("Exception: {0}", ex.Message);
                System.Console.WriteLine();

                return false;
            }
            finally
            {
                try
                {
                    System.IO.File.Delete(sTempFile);
                }
                catch (Exception)
                {
                }
            }
            
            return true;
        }

        private static void UpdateAssemblyFiles()
        {
            System.Collections.Generic.List<TAssemblyData> AssemblyDataList;
            Nullable<TAssemblyData> AssemblyData;

            foreach (System.String sFile in m_AssemblyFileCollection)
            {
                ReadAssemblyFile(sFile, out AssemblyDataList);

                AssemblyData = FindAssemblyDataFromKeyword(CAssemblyTitle, AssemblyDataList);

                if (AssemblyData.HasValue)
                {
                    if (!IsAssemblyExcluded(AssemblyData.Value.sValue))
                    {
                        System.Console.WriteLine();
                        System.Console.Write("Updating the assembly file \"{0}\"...", sFile);

                        if (WriteAssemblyFile(sFile, AssemblyDataList))
                        {
                            System.Console.WriteLine("Successful");
                        }
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
            System.Console.WriteLine("Update C# Assembly Version v{0}", AssemblyVersion);
            System.Console.WriteLine(AssemblyCopyright);
            System.Console.WriteLine();
            System.Console.WriteLine("UpdateAssemblyVersionAppNet \"Root Path\" \"Version File\"");
            System.Console.WriteLine();
            System.Console.WriteLine("    \"Root Path\"    - Top level location of the projects");
            System.Console.WriteLine("    \"Version File\" - File with the version information");
            System.Console.WriteLine();
        }
        #endregion

        #region "Main"
        static void Main(string[] args)
        {
            System.String sRootPath, sVersionFileName;

            System.Console.OutputEncoding = Encoding.UTF8;

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

                System.Console.WriteLine();
                System.Console.WriteLine("Finished updating the assembly files.");
            }

            catch (System.Exception e)
            {
                System.Console.WriteLine("Failed");
                System.Console.WriteLine();
                System.Console.WriteLine("Exception Occurred");
                System.Console.WriteLine("Message: {0}", e.Message);
                System.Console.WriteLine();

                System.Environment.Exit(1);
            }

            System.Environment.Exit(0);
        }
        #endregion
    }
}

/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/
