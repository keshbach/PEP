/***************************************************************************/
/*  Copyright (C) 2014-2021 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace OpenZip
{
    namespace Forms
    {
        public partial class OpenZipForm : System.Windows.Forms.Form
        {
            #region "Enumerations"
            public enum EOpenMode
            {
                Open,
                SaveAs
            }

            private enum EActiveItem
            {
                None,
                TreeViewZipFile,
                TreeViewFolder,
                ListViewFolder,
                ListViewFile
            }
            #endregion

            #region "Structures"
            private struct TTreeViewExpandDragDropData
            {
                public System.Windows.Forms.Timer Timer;
                public System.Drawing.Point Point;
                public System.Windows.Forms.TreeNode TreeNode;
            }

            private struct TTreeViewScrollDragDropData
            {
                public System.Windows.Forms.Timer Timer;
                public System.Drawing.Point Point;
                public Common.Forms.TreeView.EDragDropScrollHitTest ScrollArea;
                public Common.Forms.TreeView TreeView;
            }

            private struct TListViewScrollDragDropData
            {
                public System.Windows.Forms.Timer Timer;
                public System.Drawing.Point Point;
                public Common.Forms.ListView.EDragDropScrollHitTest ScrollArea;
                public Common.Forms.ListView ListView;
            }

            private struct TZipFileData
            {
                public System.String sFileName;
                public System.String sPath;
                public System.String sSrcFileName;
                public System.UInt64 nUncompressedSize;
                public System.UInt32 nCrc;
                public System.String sAttributes;
                public System.DateTime ModDateTime;
                public System.String sComment;
            }
            #endregion

            #region "Delegates"
            private delegate void DelegateTreeViewSort();
            private DelegateTreeViewSort m_DelegateTreeViewSort;

            private delegate void DelegateListViewSort();
            private DelegateListViewSort m_DelegateListViewSort;
            #endregion

            #region "Constants"
            private const System.String CZipFileImageName = "ZipFile";

            private const System.String CSizeFormat = "###,###,##0";
            private const System.String CCrcFormat = "x8";

            private const System.Int32 CBackspaceKeyCode = 0x0008;
            private const System.Int32 CControlKeyCode = 0x0009;
            private const System.Int32 CPasteKeyCode = 0x0016;

            // Timer delays (in milliseconds)

            private const System.Int32 CDragDropExpandNodeDelay = 1000;
            private const System.Int32 CDragDropScrollDelay = 200;

            // Group names

            private const System.String CTreeViewZipFileGroupName = "TreeViewZipFile";
            private const System.String CTreeViewFolderGroupName = "TreeViewFolder";
            private const System.String CListViewFolderGroupName = "ListViewFolder";
            private const System.String CListViewFileGroupName = "ListViewFile";
            #endregion

            #region "Member Variables"
            private EOpenMode m_OpenMode = EOpenMode.Open;
            private EActiveItem m_ActiveItem = EActiveItem.None;

            private System.String m_sCurrentPath = System.IO.Path.DirectorySeparatorChar.ToString();
            private System.String m_sSelection = "";

            private System.Boolean m_bAllowOverwrite = false;
            private System.Boolean m_bEditingSelectFileNameOnly = true;

            private System.String m_sZipFile = null;
            private Common.Zip.File m_ZipFile = null;

            private static System.Boolean s_bInitializeImages = true;
            #endregion

            #region "Static Variables"
            private static System.Windows.Forms.TreeNode s_DragSelectedTreeNode = null;
            private static System.Windows.Forms.TreeNode s_DragTreeNode = null;
            private static System.Windows.Forms.ListViewItem s_DragListViewItem = null;

            private static TTreeViewExpandDragDropData s_TreeViewExpandDragDropData = new TTreeViewExpandDragDropData();
            private static TTreeViewScrollDragDropData s_TreeViewScrollDragDropData = new TTreeViewScrollDragDropData();
            private static TListViewScrollDragDropData s_ListViewScrollDragDropData = new TListViewScrollDragDropData();

            private static System.Collections.Generic.Dictionary<System.Windows.Forms.ListViewItem, System.Windows.Forms.TreeNode> s_DragListViewItemToTreeNodeDict = null;
            #endregion

            #region "Properties"
            public System.String ZipFile
            {
                set
                {
                    m_sZipFile = value;
                }
            }

            public EOpenMode OpenMode
            {
                get
                {
                    return m_OpenMode;
                }

                set
                {
                    m_OpenMode = value;
                }
            }

            public System.String CurrentPath
            {
                get
                {
                    return m_sCurrentPath;
                }
                set
                {
                    m_sCurrentPath = value;
                }
            }

            public System.String Selection
            {
                get
                {
                    return m_sSelection;
                }
            }

            public System.Boolean AllowOverwrite
            {
                get
                {
                    return m_bAllowOverwrite;
                }

                set
                {
                    m_bAllowOverwrite = value;
                }
            }

            public System.Boolean EditingSelectFileNameOnly
            {
                get
                {
                    return m_bEditingSelectFileNameOnly;
                }

                set
                {
                    m_bEditingSelectFileNameOnly = value;
                }
            }
            #endregion

            #region "Constructor"
            public OpenZipForm()
            {
                m_DelegateTreeViewSort = new DelegateTreeViewSort(OnTreeViewSort);
                m_DelegateListViewSort = new DelegateListViewSort(OnListViewSort);

                InitializeComponent();
            }
            #endregion

            #region "Internal Helpers"
            private void TreeViewSort()
            {
                BeginInvoke(m_DelegateTreeViewSort);
            }

            private void ListViewSort()
            {
                BeginInvoke(m_DelegateListViewSort);
            }

            private void OnTreeViewSort()
            {
                using (new Common.Forms.WaitCursor(this))
                {
                    treeViewFolder.Sort();
                }
            }

            private void OnListViewSort()
            {
                System.Windows.Forms.ListViewItem Item = null;

                using (new Common.Forms.WaitCursor(this))
                {
                    if (listViewFolderFile.SelectedItems.Count > 0)
                    {
                        Item = listViewFolderFile.SelectedItems[0];
                    }

                    if (listViewFolderFile.ListViewItemSorter == null)
                    {
                        SetListViewSorter(listViewFolderFile.SortArrow);
                    }
                    else
                    {
                        listViewFolderFile.Sort();
                    }

                    if (Item != null)
                    {
                        Item.EnsureVisible();
                    }
                }
            }

            private void SetListViewSorter(
                Common.Forms.ListView.ESortArrow SortArrow)
            {
                if (listViewFolderFile.SortArrow == SortArrow)
                {
                    listViewFolderFile.ListViewItemSorter = new ListViewNameAscendingComparer();
                }
                else
                {
                    listViewFolderFile.ListViewItemSorter = new ListViewNameDescendingComparer();
                }
            }

            private static TZipFileData? FindZipFileData(
                System.String sName,
                System.Collections.Generic.List<TZipFileData> ZipFileDataList)
            {
                foreach (TZipFileData ZipFileData in ZipFileDataList)
                {
                    if (0 == System.String.Compare(ZipFileData.sFileName, sName, true))
                    {
                        return ZipFileData;
                    }
                }

                return null;
            }

            private static System.Boolean IsNameInUse(
                System.String sName,
                System.Collections.Generic.List<TZipFileData> ZipFileDataList)
            {
                return (null != FindZipFileData(sName, ZipFileDataList)) ? true : false;
            }

            private static System.Boolean IsDropFileNameInUse(
                System.String sName,
                System.Windows.Forms.TreeNode TreeNode)
            {
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;

                if (0 == System.String.Compare(sName, TreeNode.Text, true))
                {
                    return true;
                }

                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Tag;

                return IsNameInUse(sName, ZipFileDataList);
            }

            private static System.Boolean IsTreeNameChildInUse(
                System.String sNewName,
                System.Windows.Forms.TreeNode TreeNode)
            {
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;

                if (null != FindTreeNode(sNewName, TreeNode.Nodes))
                {
                    return true;
                }

                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Tag;

                if (IsNameInUse(sNewName, ZipFileDataList))
                {
                    return true;
                }

                return false;
            }

            private static System.Boolean IsTreeNameParentOrSiblingInUse(
                System.String sNewName,
                System.Windows.Forms.TreeNode TreeNode)
            {
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;

                if (null != FindTreeNode(sNewName, TreeNode.Parent.Nodes))
                {
                    return true;
                }

                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Tag;

                if (IsNameInUse(sNewName, ZipFileDataList))
                {
                    return true;
                }

                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Parent.Tag;

                return IsNameInUse(sNewName, ZipFileDataList);
            }

            private System.Boolean IsListViewNameInUse(
                System.String sNewName)
            {
                foreach (System.Windows.Forms.ListViewItem Item in listViewFolderFile.Items)
                {
                    if (0 == System.String.Compare(Item.Text, sNewName, true))
                    {
                        return true;
                    }
                }

                return false;
            }

            private static System.String CreateNewFolderName(
                System.Windows.Forms.TreeNode ParentTreeNode)
            {
                System.Boolean bNameFound = false;
                System.UInt32 nIndex = 0;
                System.Text.StringBuilder sb = new System.Text.StringBuilder();

                while (bNameFound == false)
                {
                    sb.Clear();

                    sb.Append("Folder");

                    if (nIndex > 0)
                    {
                        sb.AppendFormat(" ({0:#})", nIndex);
                    }

                    if (FindTreeNode(sb.ToString(), ParentTreeNode.Nodes) == null)
                    {
                        bNameFound = true;
                    }
                    else
                    {
                        ++nIndex;
                    }
                }

                return sb.ToString();
            }

            private static void GetFilesInTreeNode(
                System.Windows.Forms.TreeNode TreeNode,
                ref System.Collections.Specialized.StringCollection FilesStringCollection)
            {
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;

                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Tag;

                foreach (TZipFileData ZipFileData in ZipFileDataList)
                {
                    FilesStringCollection.Add(ZipFileData.sSrcFileName);
                }

                foreach (System.Windows.Forms.TreeNode TmpTreeNode in TreeNode.Nodes)
                {
                    GetFilesInTreeNode(TmpTreeNode, ref FilesStringCollection);
                }
            }

            private static System.String GetNodeFilePath(
                System.Windows.Forms.TreeNode TreeNode)
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();

                while (TreeNode.Parent != null)
                {
                    if (sb.Length > 0)
                    {
                        sb.Insert(0, System.IO.Path.DirectorySeparatorChar);
                    }

                    sb.Insert(0, TreeNode.Text);

                    TreeNode = TreeNode.Parent;
                }

                return sb.ToString();
            }

            private static System.Windows.Forms.TreeNode FindTreeNode(
                System.String sName,
                System.Windows.Forms.TreeNodeCollection TreeNodes)
            {
                foreach (System.Windows.Forms.TreeNode TreeNode in TreeNodes)
                {
                    if (0 == System.String.Compare(TreeNode.Text, sName, true))
                    {
                        return TreeNode;
                    }
                }

                return null;
            }

            private static System.Windows.Forms.TreeNode CreateTreeNode(
                System.String sName)
            {
                System.Windows.Forms.TreeNode TreeNode;

                TreeNode = new System.Windows.Forms.TreeNode(sName);

                TreeNode.ImageKey = Common.Forms.ImageManager.FolderImageName;
                TreeNode.SelectedImageKey = Common.Forms.ImageManager.FolderImageName;
                TreeNode.Tag = new System.Collections.Generic.List<TZipFileData>();

                return TreeNode;
            }

            private System.Boolean DeleteTreeNode(
                System.Windows.Forms.TreeNode TreeNode)
            {
                System.Text.StringBuilder sb;

                using (new Common.Forms.WaitCursor(this))
                {
                    try
                    {
                        m_ZipFile.DeleteFolder(GetNodeFilePath(TreeNode));
                    }

                    catch (System.Exception exception)
                    {
                        sb = new System.Text.StringBuilder();

                        sb.Append("The folder could not be deleted.");
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);

                        return false;
                    }

                    TreeNode.Parent.Nodes.Remove(TreeNode);
                }

                return true;
            }

            private System.Windows.Forms.TreeNode AddTreeNode(
                System.String sPath)
            {
                System.String[] sPaths = sPath.Split(System.IO.Path.DirectorySeparatorChar);
                System.Windows.Forms.TreeNode TreeNode = treeViewFolder.Nodes[0];
                System.Windows.Forms.TreeNodeCollection ParentTreeNodes = TreeNode.Nodes;

                foreach (System.String sName in sPaths)
                {
                    if (sName.Length > 0)
                    {
                        TreeNode = FindTreeNode(sName, ParentTreeNodes);

                        if (TreeNode == null)
                        {
                            TreeNode = CreateTreeNode(sName);

                            ParentTreeNodes.Add(TreeNode);
                        }

                        ParentTreeNodes = TreeNode.Nodes;
                    }
                }

                return TreeNode;
            }

            private void AddTreeNodes(
                System.Collections.Specialized.StringCollection FilesStringCollection)
            {
                System.Windows.Forms.TreeNode TreeNode;
                System.String sPath;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;
                TZipFileData ZipFileData;
                Common.Zip.Item ZipItem;

                foreach (System.String sFile in FilesStringCollection)
                {
                    sPath = System.IO.Path.GetDirectoryName(sFile);

                    TreeNode = AddTreeNode(sPath);

                    ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Tag;

                    ZipItem = GetZipItem(sFile);

                    ZipFileData = CreateZipFileData(ZipItem);

                    ZipFileDataList.Add(ZipFileData);
                }
            }

            private System.Windows.Forms.ListViewItem AddListViewItem(
                System.String sText)
            {
                System.Windows.Forms.ListViewItem Item;

                Item = listViewFolderFile.Items.Add(sText, Common.Forms.ImageManager.FolderImageName);

                Item.SubItems.Add("");
                Item.SubItems.Add("");
                Item.SubItems.Add("");
                Item.SubItems.Add("");

                return Item;
            }

            private System.Windows.Forms.ListViewItem AddListViewItem(
                TZipFileData ZipFileData)
            {
                System.Windows.Forms.ListViewItem Item;
                System.String sFileExtension;

                sFileExtension = System.IO.Path.GetExtension(ZipFileData.sFileName);

                if (sFileExtension.Length > 0)
                {
                    Common.Forms.ImageManager.AddFileExtensionSmallImage(sFileExtension, sFileExtension);
                }
                else
                {
                    sFileExtension = Common.Forms.ImageManager.UnknownFileImageName;
                }

                Item = listViewFolderFile.Items.Add(ZipFileData.sFileName, sFileExtension);

                Item.SubItems.Add(ZipFileData.nUncompressedSize.ToString(CSizeFormat));
                Item.SubItems.Add(ZipFileData.nCrc.ToString(CCrcFormat));
                Item.SubItems.Add(ZipFileData.sAttributes);
                Item.SubItems.Add(ZipFileData.ModDateTime.ToString());

                Item.Tag = ZipFileData;

                return Item;
            }

            private System.Boolean DeleteListViewItem(
                System.String sText)
            {
                foreach (System.Windows.Forms.ListViewItem Item in listViewFolderFile.Items)
                {
                    if (0 == System.String.Compare(Item.Text, sText, true))
                    {
                        Item.Tag = null;

                        Item.Remove();

                        return true;
                    }
                }

                return false;
            }

            private void DeleteListViewItem(
                System.Windows.Forms.ListViewItem Item)
            {
                TZipFileData ZipFileData;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;
                System.Windows.Forms.TreeNode TreeNode;
                System.Text.StringBuilder sb;

                using (new Common.Forms.WaitCursor(this))
                {
                    if (IsFolder(Item) == false)
                    {
                        ZipFileData = (TZipFileData)Item.Tag;

                        try
                        {
                            m_ZipFile.DeleteFile(ZipFileData.sSrcFileName);
                        }

                        catch (System.Exception exception)
                        {
                            sb = new System.Text.StringBuilder();

                            sb.Append("The file could not be deleted.");
                            sb.AppendLine();
                            sb.AppendLine();
                            sb.AppendLine(exception.Message);

                            Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                            System.Windows.Forms.MessageBoxButtons.OK,
                                                            System.Windows.Forms.MessageBoxIcon.Error);

                            return;
                        }

                        Item.Remove();

                        ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)treeViewFolder.SelectedNode.Tag;

                        ZipFileDataList.Remove(ZipFileData);
                    }
                    else
                    {
                        TreeNode = FindTreeNode(Item.Text, treeViewFolder.SelectedNode.Nodes);

                        if (DeleteTreeNode(TreeNode))
                        {
                            Item.Remove();
                        }
                    }
                }
            }

            private static System.String ConvertFilePath(
                System.String sFile,
                System.String sOriginalPath,
                System.String sNewPath)
            {
                System.String sNewFile;

                sNewFile = sFile.Remove(0, sOriginalPath.Length);
                sNewFile = sNewFile.Insert(0, sNewPath);

                return sNewFile;
            }

            private static System.Collections.Specialized.StringCollection ConvertFilesPath(
                System.Collections.Specialized.StringCollection FilesStringCollection,
                System.String sOriginalPath,
                System.String sNewPath)
            {
                System.Collections.Specialized.StringCollection NewFilesStringCollection;
                System.String sToFile;

                NewFilesStringCollection = new System.Collections.Specialized.StringCollection();

                foreach (System.String sFromFile in FilesStringCollection)
                {
                    sToFile = ConvertFilePath(sFromFile, sOriginalPath, sNewPath);

                    NewFilesStringCollection.Add(sToFile);
                }

                return NewFilesStringCollection;
            }

            private static System.String TranslateExternalAttributes(
                Common.Zip.Item.EFileAttributes Attributes)
            {
                System.String sResult = "";

                if ((Attributes & Common.Zip.Item.EFileAttributes.Archive) == Common.Zip.Item.EFileAttributes.Archive)
                {
                    sResult += "A";
                }

                if ((Attributes & Common.Zip.Item.EFileAttributes.ReadOnly) == Common.Zip.Item.EFileAttributes.ReadOnly)
                {
                    sResult += "R";
                }

                if ((Attributes & Common.Zip.Item.EFileAttributes.System) == Common.Zip.Item.EFileAttributes.System)
                {
                    sResult += "S";
                }

                if ((Attributes & Common.Zip.Item.EFileAttributes.Hidden) == Common.Zip.Item.EFileAttributes.Hidden)
                {
                    sResult += "H";
                }

                return sResult;
            }

            private static System.Boolean IsValidKeyChar(
                System.Char cKey)
            {
                foreach (System.Char cValue in System.IO.Path.GetInvalidFileNameChars())
                {
                    if (cKey == cValue)
                    {
                        return false;
                    }
                }

                return true;
            }

            private static System.String GetValidClipboardText()
            {
                System.String sText = System.Windows.Forms.Clipboard.GetText();
                System.Int32 nIndex = 0;

                while (nIndex < sText.Length)
                {
                    if (IsValidKeyChar(sText[nIndex]))
                    {
                        ++nIndex;
                    }
                    else
                    {
                        sText = sText.Replace(sText[nIndex].ToString(), "");
                    }
                }

                return sText;
            }

            private static System.String RenamePath(
                System.String sPathChange,
                System.String sOriginalPath,
                System.String sNewPath)
            {
                sPathChange = sPathChange.Remove(0, sOriginalPath.Length);
                sPathChange = sPathChange.Insert(0, sNewPath);

                return sPathChange;
            }

            private void RenameTreeFiles(
                System.Windows.Forms.TreeNode TreeNode,
                System.String sOriginalPath,
                System.String sNewPath)
            {
                System.Collections.Generic.List<TZipFileData> ZipFileDataList, NewZipFileDataList;
                System.Collections.Generic.IEnumerator<TZipFileData> Enum;
                TZipFileData ZipFileData;

                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Tag;
                NewZipFileDataList = new System.Collections.Generic.List<TZipFileData>();
                Enum = ZipFileDataList.GetEnumerator();

                while (Enum.MoveNext())
                {
                    ZipFileData = (TZipFileData)Enum.Current;

                    ZipFileData.sSrcFileName = RenamePath(ZipFileData.sSrcFileName,
                                                          sOriginalPath, sNewPath);
                    ZipFileData.sPath = RenamePath(ZipFileData.sPath,
                                                   sOriginalPath, sNewPath);

                    NewZipFileDataList.Add(ZipFileData);
                }

                TreeNode.Tag = NewZipFileDataList;

                foreach (System.Windows.Forms.TreeNode TmpTreeNode in TreeNode.Nodes)
                {
                    RenameTreeFiles(TmpTreeNode, sOriginalPath, sNewPath);
                }
            }

            private void RenameListViewFiles(
                System.String sOriginalPath,
                System.String sNewPath)
            {
                TZipFileData ZipFileData;

                foreach (System.Windows.Forms.ListViewItem Item in listViewFolderFile.Items)
                {
                    if (Item.Tag != null)
                    {
                        ZipFileData = (TZipFileData)Item.Tag;

                        ZipFileData.sSrcFileName = RenamePath(ZipFileData.sSrcFileName,
                                                              sOriginalPath, sNewPath);
                        ZipFileData.sPath = RenamePath(ZipFileData.sPath,
                                                       sOriginalPath, sNewPath);

                        Item.Tag = ZipFileData;
                    }
                }
            }

            private System.Boolean RenameFile(
                System.String sOriginalName,
                System.String sNewName)
            {
                System.Text.StringBuilder sb;

                try
                {
                    m_ZipFile.ChangeFileName(sOriginalName, sNewName);
                }

                catch (Exception exception)
                {
                    sb = new System.Text.StringBuilder();

                    sb.Append("The file could not be renamed.");
                    sb.AppendLine();
                    sb.AppendLine();
                    sb.AppendLine(exception.Message);

                    Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Error);

                    return false;
                }

                return true;
            }

            private System.Boolean MoveFile(
                System.String sSourceName,
                System.String sDestName)
            {
                System.Text.StringBuilder sb;

                try
                {
                    m_ZipFile.ChangeFileName(sSourceName, sDestName);
                }

                catch (Exception exception)
                {
                    sb = new System.Text.StringBuilder();

                    sb.Append("The file could not be moved.");
                    sb.AppendLine();
                    sb.AppendLine();
                    sb.AppendLine(exception.Message);

                    Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Error);

                    return false;
                }

                return true;
            }

            private System.Boolean CopyFile(
                System.String sSourceName,
                System.String sDestName)
            {
                System.Text.StringBuilder sb;

                try
                {
                    m_ZipFile.CopyFile(sSourceName, sDestName);
                }

                catch (Exception exception)
                {
                    sb = new System.Text.StringBuilder();

                    sb.Append("The file could not be copied.");
                    sb.AppendLine();
                    sb.AppendLine();
                    sb.AppendLine(exception.Message);

                    Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Error);

                    return false;
                }

                return true;
            }

            private static TZipFileData CreateZipFileData(
                Common.Zip.Item ZipItem)
            {
                TZipFileData ZipFileData;

                ZipFileData.sFileName = ZipItem.FileName;
                ZipFileData.sPath = ZipItem.Path;
                ZipFileData.sSrcFileName = ZipItem.SrcFileName;
                ZipFileData.nUncompressedSize = ZipItem.UncompressedSize;
                ZipFileData.nCrc = ZipItem.CRC;
                ZipFileData.sAttributes = TranslateExternalAttributes(ZipItem.FileAttributes);
                ZipFileData.ModDateTime = ZipItem.FileModDateTime;
                ZipFileData.sComment = ZipItem.Comment;

                return ZipFileData;
            }

            private Common.Zip.Item GetZipItem(
                System.String sFile)
            {
                try
                {
                    return m_ZipFile.FindItem(sFile);
                }

                catch (System.Exception)
                {
                }

                return null;
            }

            private void InitZipFile()
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.Windows.Forms.TreeNode TreeNode;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;
                TZipFileData ZipFileData;

                treeViewFolder.BeginUpdate();

                TreeNode = treeViewFolder.Nodes.Add("Zip File");

                TreeNode.ImageKey = CZipFileImageName;
                TreeNode.SelectedImageKey = CZipFileImageName;
                TreeNode.Tag = new System.Collections.Generic.List<TZipFileData>();

                try
                {
                    m_ZipFile = Common.Zip.File.Open(m_sZipFile, Common.Zip.File.EOpenMode.Open);
                }

                catch (Exception exception)
                {
                    sb.Append("The zip file could not be opened.");
                    sb.AppendLine();
                    sb.AppendLine();
                    sb.AppendLine(exception.Message);

                    Common.Forms.MessageBox.Show(sb.ToString(),
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Error);
                }

                if (m_ZipFile != null)
                {
                    try
                    {
                        using (System.Collections.Generic.IEnumerator<Common.Zip.Item> Enum = m_ZipFile.GetEnumerator())
                        {
                            while (Enum.MoveNext())
                            {
                                TreeNode = AddTreeNode(Enum.Current.Path);

                                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeNode.Tag;

                                ZipFileData = CreateZipFileData(Enum.Current);

                                ZipFileDataList.Add(ZipFileData);
                            }
                        }
                    }

                    catch (Exception exception)
                    {
                        sb.Append("The files contained within the zip file could not be retrieved.");
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }
                }

                treeViewFolder.SelectedNode = treeViewFolder.Nodes[0];

                treeViewFolder.Sort();
                treeViewFolder.EndUpdate();
            }

            private void InitImageKeys()
            {
                string sNewFolderImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(OpenZipForms.Resources.Resource.ResourceManager, "AddFolder_16x");
                string sRenameImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(OpenZipForms.Resources.Resource.ResourceManager, "Rename_16x");
                string sDeleteFolderImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(OpenZipForms.Resources.Resource.ResourceManager, "DeleteFolder_16x");
                string sDeleteFileImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(OpenZipForms.Resources.Resource.ResourceManager, "DeleteFile_16x");
                string sPropertiesImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(OpenZipForms.Resources.Resource.ResourceManager, "Property_16x");

                toolStripButtonNewFolder.ImageKey = sNewFolderImageKey;
                toolStripButtonRename.ImageKey = sRenameImageKey;
                toolStripButtonDeleteFolder.ImageKey = sDeleteFolderImageKey;
                toolStripButtonDeleteFile.ImageKey = sDeleteFileImageKey;
                toolStripButtonProperties.ImageKey = sPropertiesImageKey;

                toolStripMenuItemZipFileNewFolder.ImageKey = sNewFolderImageKey;

                toolStripMenuItemFolderNewFolder.ImageKey = sNewFolderImageKey;
                toolStripMenuItemFolderDelete.ImageKey = sDeleteFolderImageKey;
                toolStripMenuItemFolderRename.ImageKey = sRenameImageKey;

                toolStripMenuItemFileDelete.ImageKey = sDeleteFileImageKey;
                toolStripMenuItemFileRename.ImageKey = sRenameImageKey;
                toolStripMenuItemFileProperties.ImageKey = sPropertiesImageKey;
            }

            private void CreateToolStripGroups()
            {
                System.Windows.Forms.ToolStripItem[] TreeViewZipFileGroupItems = {
                    toolStripButtonNewFolder};
                System.Windows.Forms.ToolStripItem[] TreeViewFolderGroupItems = {
                    toolStripButtonNewFolder,
                    toolStripSeparatorToolbar1,
                    toolStripButtonDeleteFolder,
                    toolStripButtonRename};
                System.Windows.Forms.ToolStripItem[] ListViewFolderGroupItems = {
                    toolStripButtonDeleteFolder,
                    toolStripButtonRename};
                System.Windows.Forms.ToolStripItem[] ListViewFileGroupItems = {
                    toolStripButtonDeleteFile,
                    toolStripButtonRename,
                    toolStripSeparatorToolbar2,
                    toolStripButtonProperties};

                toolStripForm.CreateGroup(CTreeViewZipFileGroupName, TreeViewZipFileGroupItems);
                toolStripForm.CreateGroup(CTreeViewFolderGroupName, TreeViewFolderGroupItems);
                toolStripForm.CreateGroup(CListViewFolderGroupName, ListViewFolderGroupItems);
                toolStripForm.CreateGroup(CListViewFileGroupName, ListViewFileGroupItems);
            }

            private void DestroyToolStripGroups()
            {
                toolStripForm.DestroyGroup(CTreeViewZipFileGroupName);
                toolStripForm.DestroyGroup(CTreeViewFolderGroupName);
                toolStripForm.DestroyGroup(CListViewFolderGroupName);
                toolStripForm.DestroyGroup(CListViewFileGroupName);
            }

            private void VerifyFileName()
            {
                System.String sFileName = textBoxFileName.Text;

                switch (m_OpenMode)
                {
                    case EOpenMode.Open:
                        VerifyOpenFileName(sFileName);
                        break;
                    case EOpenMode.SaveAs:
                        VerifySaveAsFileName(sFileName);
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown open mode.");
                        break;
                }
            }

            private void VerifyOpenFileName(
                System.String sFileName)
            {
                AcceptButton = null;
                buttonOK.Enabled = false;

                foreach (System.Windows.Forms.ListViewItem Item in listViewFolderFile.Items)
                {
                    if (IsFolder(Item) == false && 
                        0 == System.String.Compare(sFileName, Item.Text, true))
                    {
                        AcceptButton = buttonOK;
                        buttonOK.Enabled = true;

                        return;
                    }
                }
            }

            private void VerifySaveAsFileName(
                System.String sFileName)
            {
                AcceptButton = null;
                buttonOK.Enabled = false;

                foreach (System.Windows.Forms.ListViewItem Item in listViewFolderFile.Items)
                {
                    if (0 == System.String.Compare(sFileName, Item.Text, true))
                    {
                        if (m_bAllowOverwrite && IsFolder(Item) == false)
                        {
                            AcceptButton = buttonOK;
                            buttonOK.Enabled = true;
                        }

                        return;
                    }
                }

                if (sFileName.Length > 0)
                {
                    AcceptButton = buttonOK;
                    buttonOK.Enabled = true;
                }
            }

            private void InitCurrentPath()
            {
                System.String sCurrentPath;
                System.String[] sPaths;
                System.Windows.Forms.TreeNode TreeNode;
                System.Windows.Forms.TreeNodeCollection TreeNodes;

                if (m_sCurrentPath == null)
                {
                    return;
                }

                if (m_sCurrentPath.Length > 0 && m_sCurrentPath[0] == System.IO.Path.DirectorySeparatorChar)
                {
                    sCurrentPath = m_sCurrentPath.Remove(0, 1);
                }
                else
                {
                    sCurrentPath = m_sCurrentPath;
                }

                sPaths = sCurrentPath.Split(System.IO.Path.DirectorySeparatorChar);

                treeViewFolder.BeginUpdate();

                TreeNodes = treeViewFolder.Nodes[0].Nodes;

                for (System.Int32 nIndex = 0; nIndex < sPaths.Length; ++nIndex)
                {
                    TreeNode = FindTreeNode(sPaths[nIndex], TreeNodes);

                    if (TreeNode != null)
                    {
                        treeViewFolder.SelectedNode = TreeNode;

                        TreeNodes = TreeNode.Nodes;
                    }
                    else
                    {
                        break;
                    }
                }

                treeViewFolder.EndUpdate();
            }

            private static System.String GenerateUniqueName(
                System.String sFileName,
                System.Windows.Forms.TreeNode DropTreeNode)
            {
                System.Boolean bNameFound = false;
                System.Int32 nSuffix = 2;
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.String sBaseFileName, sExtension, sNewName;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;

                sBaseFileName = System.IO.Path.GetFileNameWithoutExtension(sFileName);
                sExtension = System.IO.Path.GetExtension(sFileName);
                sNewName = "";

                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)DropTreeNode.Tag;

                while (bNameFound == false)
                {
                    sb.Append(sBaseFileName);
                    sb.Append(" (");
                    sb.Append(nSuffix.ToString());
                    sb.Append(")");
                    sb.Append(sExtension);

                    sNewName = sb.ToString();

                    sb.Clear();

                    ++nSuffix;

                    if (FindTreeNode(sNewName, DropTreeNode.Nodes) == null &&
                        IsNameInUse(sNewName, ZipFileDataList) == false)
                    {
                        bNameFound = true;
                    }
                }

                return sNewName;
            }

            private void ProcessItemDrag(
                System.Object data)
            {
                System.Windows.Forms.ListViewItem SelectedListViewItem = null;
                System.Windows.Forms.ListViewItem ListViewItem;

                s_DragSelectedTreeNode = treeViewFolder.SelectedNode;

                treeViewFolder.SelectedNode = null;

                if (listViewFolderFile.SelectedItems.Count > 0)
                {
                    SelectedListViewItem = listViewFolderFile.SelectedItems[0];
                }

                listViewFolderFile.SelectedItems.Clear();

                treeViewFolder.BeforeSelect -= new System.Windows.Forms.TreeViewCancelEventHandler(treeViewFolder_BeforeSelect);
                treeViewFolder.AfterSelect -= new System.Windows.Forms.TreeViewEventHandler(treeViewFolder_AfterSelect);

                listViewFolderFile.ItemSelectionChanged -= new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(listViewFolderFile_ItemSelectionChanged);

                s_TreeViewExpandDragDropData.Point.X = -1;
                s_TreeViewExpandDragDropData.Point.Y = -1;

                if (data.GetType() == typeof(System.Windows.Forms.TreeNode))
                {
                    s_DragTreeNode = (System.Windows.Forms.TreeNode)data;
                }
                else if (data.GetType() == typeof(System.Windows.Forms.ListViewItem))
                {
                    ListViewItem = (System.Windows.Forms.ListViewItem)data;

                    if (ListViewItem.Tag != null)
                    {
                        s_DragListViewItem = ListViewItem;
                    }
                    else
                    {
                        s_DragTreeNode = Common.Forms.TreeView.FindTreeNode(
                                                s_DragSelectedTreeNode.Nodes,
                                                ListViewItem.Text, false);
                    }
                }

                s_DragListViewItemToTreeNodeDict = new System.Collections.Generic.Dictionary<System.Windows.Forms.ListViewItem, System.Windows.Forms.TreeNode>();

                foreach (System.Windows.Forms.ListViewItem Item in listViewFolderFile.Items)
                {
                    if (Item.Tag == null)
                    {
                        s_DragListViewItemToTreeNodeDict.Add(Item,
                            Common.Forms.TreeView.FindTreeNode(s_DragSelectedTreeNode.Nodes,
                                                                Item.Text, false));
                    }
                }

                s_TreeViewExpandDragDropData.Timer = new System.Windows.Forms.Timer();
                s_TreeViewScrollDragDropData.Timer = new System.Windows.Forms.Timer();
                s_ListViewScrollDragDropData.Timer = new System.Windows.Forms.Timer();

                s_TreeViewExpandDragDropData.Timer.Tick += new System.EventHandler(TreeViewExpandTimerEventProcessor);
                s_TreeViewScrollDragDropData.Timer.Tick += new System.EventHandler(TreeViewScrollTimerEventProcessor);
                s_ListViewScrollDragDropData.Timer.Tick += new System.EventHandler(ListViewScrollTimerEventProcessor);

                s_TreeViewExpandDragDropData.Timer.Interval = CDragDropExpandNodeDelay;
                s_TreeViewScrollDragDropData.Timer.Interval = CDragDropScrollDelay;
                s_ListViewScrollDragDropData.Timer.Interval = CDragDropScrollDelay;

                DoDragDrop(data,
                           System.Windows.Forms.DragDropEffects.Copy |
                               System.Windows.Forms.DragDropEffects.Move);

                StopTreeViewDragDropExpandTimer();
                StopTreeViewDragDropScrollTimer();
                StopListViewDragDropScrollTimer();

                s_TreeViewExpandDragDropData.Timer.Dispose();
                s_TreeViewScrollDragDropData.Timer.Dispose();
                s_ListViewScrollDragDropData.Timer.Dispose();

                s_TreeViewExpandDragDropData.Timer = null;
                s_TreeViewScrollDragDropData.Timer = null;
                s_ListViewScrollDragDropData.Timer = null;

                s_DragTreeNode = null;

                s_DragListViewItemToTreeNodeDict.Clear();

                s_DragListViewItemToTreeNodeDict = null;

                treeViewFolder.DropHighlightedNode = null;
                listViewFolderFile.DropHighlightedItem = null;

                if (s_DragSelectedTreeNode != null)
                {
                    System.Diagnostics.Debug.Assert(s_DragSelectedTreeNode.TreeView != null);

                    treeViewFolder.SelectedNode = s_DragSelectedTreeNode;

                    if (SelectedListViewItem != null && SelectedListViewItem.ListView != null)
                    {
                        SelectedListViewItem.Selected = true;
                    }
                }

                if (s_DragListViewItem != null)
                {
                    if (s_DragListViewItem.ListView != null)
                    {
                        s_DragListViewItem.Selected = true;
                    }
                }

                listViewFolderFile.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(listViewFolderFile_ItemSelectionChanged);

                treeViewFolder.BeforeSelect += new System.Windows.Forms.TreeViewCancelEventHandler(treeViewFolder_BeforeSelect);
                treeViewFolder.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(treeViewFolder_AfterSelect);

                s_DragSelectedTreeNode = null;
                s_DragListViewItem = null;

                if (listViewFolderFile.SelectedItems.Count > 0)
                {
                    textBoxFileName.Text = listViewFolderFile.SelectedItems[0].Text;
                }
            }

            private void ProcessDropCopy(
                System.Windows.Forms.TreeNode DropTreeNode)
            {
                if (s_DragListViewItem != null)
                {
                    ProcessDropCopyFile(s_DragListViewItem, DropTreeNode);
                }
                else if (s_DragTreeNode != null)
                {
                    ProcessDropCopyFolder(s_DragTreeNode, DropTreeNode);
                }
                else
                {
                    System.Diagnostics.Debug.Assert(false, "Unknown drop copy operation.");
                }
            }

            private void ProcessDropMove(
                System.Windows.Forms.TreeNode DropTreeNode)
            {
                if (s_DragListViewItem != null)
                {
                    ProcessDropMoveFile(s_DragListViewItem, DropTreeNode);
                }
                else if (s_DragTreeNode != null)
                {
                    ProcessDropMoveFolder(s_DragTreeNode, DropTreeNode);
                }
                else
                {
                    System.Diagnostics.Debug.Assert(false, "Unknown drop move operation.");
                }
            }

            private void ProcessDropCopyFile(
                System.Windows.Forms.ListViewItem DragItem,
                System.Windows.Forms.TreeNode DropNode)
            {
                TZipFileData ZipFileData = (TZipFileData)DragItem.Tag;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;
                FileConfirmationForm FileConfirmation;
                System.String sDestPath, sDestFile, sGeneratedFileName, sDestName;
                Common.Zip.Item ZipItem;
                System.Windows.Forms.ListViewItem ListViewItem;

                sDestPath = GetNodeFilePath(DropNode);
                sDestFile = "";

                if (sDestPath.Length > 0)
                {
                    sDestFile = sDestPath + System.IO.Path.DirectorySeparatorChar;
                }

                // Files does not exist
                //     1. Copy file
                // File exists (verify destination is not a folder)
                //     1. Offer to replace
                //     2. Offer to rename by adding " copy (x)" suffix
                //     3. Cancel the copy

                if (!IsDropFileNameInUse(ZipFileData.sFileName, DropNode))
                {
                    sDestFile += DragItem.Text;

                    using (new Common.Forms.WaitCursor(this))
                    {
                        if (CopyFile(ZipFileData.sSrcFileName, sDestFile))
                        {
                            ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)DropNode.Tag;

                            ZipItem = GetZipItem(sDestFile);

                            ZipFileData = CreateZipFileData(ZipItem);

                            ZipFileDataList.Add(ZipFileData);
                        }
                        else
                        {
                            Common.Forms.MessageBox.Show(this, "The file could not be copied.",
                                                            System.Windows.Forms.MessageBoxButtons.OK,
                                                            System.Windows.Forms.MessageBoxIcon.Error);
                        }
                    }
                }
                else
                {
                    if (null != FindTreeNode(ZipFileData.sFileName, DropNode.Nodes))
                    {
                        Common.Forms.MessageBox.Show(this, "A folder with a matching name already exists.",
                                                        System.Windows.Forms.MessageBoxButtons.OK,
                                                        System.Windows.Forms.MessageBoxIcon.Information);

                        return;
                    }

                    sGeneratedFileName = GenerateUniqueName(ZipFileData.sFileName,
                                                            DropNode);

                    FileConfirmation = new FileConfirmationForm();

                    FileConfirmation.ConfirmationMode = FileConfirmationForm.EConfirmationMode.Copy;
                    FileConfirmation.NewFileName = sGeneratedFileName;

                    if (FileConfirmation.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                    {
                        switch (FileConfirmation.ConfirmationChoice)
                        {
                            case FileConfirmationForm.EConfirmationChoice.CopyAndReplace:
                                sDestName = DragItem.Text;
                                break;
                            case FileConfirmationForm.EConfirmationChoice.CopyButKeepBothFiles:
                                sDestName = sGeneratedFileName;
                                break;
                            default:
                                sDestName = "";

                                System.Diagnostics.Debug.Assert(false, "Unknown file confirmation choice.");
                                break;
                        }

                        sDestFile += sDestName;

                        using (new Common.Forms.WaitCursor(this))
                        {
                            if (CopyFile(ZipFileData.sSrcFileName, sDestFile))
                            {
                                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)DropNode.Tag;

                                ZipItem = GetZipItem(sDestFile);

                                switch (FileConfirmation.ConfirmationChoice)
                                {
                                    case FileConfirmationForm.EConfirmationChoice.CopyAndReplace:
                                        ZipFileData = (TZipFileData)FindZipFileData(DragItem.Text, ZipFileDataList);

                                        ZipFileDataList.Remove(ZipFileData);

                                        if (s_DragSelectedTreeNode == DropNode)
                                        {
                                            DeleteListViewItem(DragItem.Text);
                                        }
                                        break;
                                    case FileConfirmationForm.EConfirmationChoice.CopyButKeepBothFiles:
                                        break;
                                    default:
                                        System.Diagnostics.Debug.Assert(false, "Unknown file confirmation choice.");
                                        break;
                                }

                                ZipFileData = CreateZipFileData(ZipItem);

                                ZipFileDataList.Add(ZipFileData);

                                if (s_DragSelectedTreeNode == DropNode)
                                {
                                    ListViewItem = AddListViewItem(ZipFileData);

                                    ListViewSort();
                                }
                            }
                            else
                            {
                                Common.Forms.MessageBox.Show(this, "The file could not be copied.",
                                                                System.Windows.Forms.MessageBoxButtons.OK,
                                                                System.Windows.Forms.MessageBoxIcon.Error);
                            }
                        }
                    }
                }
            }

            private void ProcessDropMoveFile(
                System.Windows.Forms.ListViewItem DragItem,
                System.Windows.Forms.TreeNode DropNode)
            {
                TZipFileData ZipFileData = (TZipFileData)DragItem.Tag;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;
                FileConfirmationForm FileConfirmation;
                System.String sDestPath, sDestFile, sGeneratedFileName;

                sDestPath = GetNodeFilePath(DropNode);
                sDestFile = "";

                if (sDestPath.Length > 0)
                {
                    sDestFile = sDestPath + System.IO.Path.DirectorySeparatorChar;
                }

                // File does not exist
                //     1. Move file
                // File exists (verify destination is not a folder)
                //     1. Offer to replace
                //     2. Offer to rename by adding " (x)" suffix
                //     3. Cancel the move

                if (!IsDropFileNameInUse(ZipFileData.sFileName, DropNode))
                {
                    sDestFile += DragItem.Text;

                    using (new Common.Forms.WaitCursor(this))
                    {
                        if (RenameFile(ZipFileData.sSrcFileName, sDestFile))
                        {
                            ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)s_DragSelectedTreeNode.Tag;

                            ZipFileDataList.Remove(ZipFileData);

                            ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)DropNode.Tag;

                            ZipFileData.sPath = sDestPath;
                            ZipFileData.sSrcFileName = sDestFile;

                            ZipFileDataList.Add(ZipFileData);

                            DragItem.Tag = null;

                            DragItem.Remove();
                        }
                        else
                        {
                            Common.Forms.MessageBox.Show(this, "The file could not be moved.",
                                                            System.Windows.Forms.MessageBoxButtons.OK,
                                                            System.Windows.Forms.MessageBoxIcon.Error);
                        }
                    }
                }
                else
                {
                    if (null != FindTreeNode(ZipFileData.sFileName, DropNode.Nodes))
                    {
                        Common.Forms.MessageBox.Show(this, "A folder with a matching name already exists.",
                                                        System.Windows.Forms.MessageBoxButtons.OK,
                                                        System.Windows.Forms.MessageBoxIcon.Information);

                        return;
                    }

                    sGeneratedFileName = GenerateUniqueName(ZipFileData.sFileName,
                                                            DropNode);

                    FileConfirmation = new FileConfirmationForm();

                    FileConfirmation.ConfirmationMode = FileConfirmationForm.EConfirmationMode.Move;
                    FileConfirmation.NewFileName = sGeneratedFileName;

                    if (FileConfirmation.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                    {
                        switch (FileConfirmation.ConfirmationChoice)
                        {
                            case FileConfirmationForm.EConfirmationChoice.MoveAndReplace:
                                sDestFile += DragItem.Text;
                                break;
                            case FileConfirmationForm.EConfirmationChoice.MoveButKeepBothFiles:
                                sDestFile += sGeneratedFileName;
                                break;
                            default:
                                System.Diagnostics.Debug.Assert(false, "Unknown file confirmation choice.");
                                return;
                        }

                        using (new Common.Forms.WaitCursor(this))
                        {
                            if (MoveFile(ZipFileData.sSrcFileName, sDestFile))
                            {
                                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)s_DragSelectedTreeNode.Tag;

                                ZipFileDataList.Remove(ZipFileData);

                                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)DropNode.Tag;

                                ZipFileData.sPath = sDestPath;
                                ZipFileData.sSrcFileName = sDestFile;

                                switch (FileConfirmation.ConfirmationChoice)
                                {
                                    case FileConfirmationForm.EConfirmationChoice.MoveAndReplace:
                                        break;
                                    case FileConfirmationForm.EConfirmationChoice.MoveButKeepBothFiles:
                                        ZipFileData.sFileName = sGeneratedFileName;
                                        break;
                                    default:
                                        System.Diagnostics.Debug.Assert(false, "Unknown file confirmation choice.");
                                        return;
                                }

                                ZipFileDataList.Add(ZipFileData);

                                DragItem.Tag = null;

                                DragItem.Remove();
                            }
                            else
                            {
                                Common.Forms.MessageBox.Show(this, "The file could not be moved.",
                                                                System.Windows.Forms.MessageBoxButtons.OK,
                                                                System.Windows.Forms.MessageBoxIcon.Error);
                            }
                        }
                    }
                }
            }

            private void ProcessDropCopyFolder(
                System.Windows.Forms.TreeNode DragTreeNode,
                System.Windows.Forms.TreeNode DropTreeNode)
            {
                System.Collections.Specialized.StringCollection FilesStringCollection = new System.Collections.Specialized.StringCollection();
                System.String sDragPath, sDropPath, sGeneratedFolderName;
                System.Text.StringBuilder sb;

                using (new Common.Forms.WaitCursor(this))
                {
                    sGeneratedFolderName = GenerateUniqueName(DragTreeNode.Text,
                                                              DropTreeNode);

                    GetFilesInTreeNode(DragTreeNode, ref FilesStringCollection);

                    sDragPath = GetNodeFilePath(DragTreeNode);
                    sDropPath = GetNodeFilePath(DropTreeNode);

                    if (sDropPath.Length > 0)
                    {
                        sDropPath += System.IO.Path.DirectorySeparatorChar;
                    }

                    sDropPath += sGeneratedFolderName;

                    try
                    {
                        m_ZipFile.CopyFolder(sDragPath, sDropPath);
                    }

                    catch (System.Exception exception)
                    {
                        sb = new System.Text.StringBuilder();

                        sb.Append("The files could not be copied.");
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);

                        return;
                    }

                    if (DropTreeNode == s_DragSelectedTreeNode)
                    {
                        AddListViewItem(sGeneratedFolderName);
                    }

                    FilesStringCollection = ConvertFilesPath(FilesStringCollection,
                                                             sDragPath, sDropPath);

                    AddTreeNodes(FilesStringCollection);
                }
            }

            private void ProcessDropMoveFolder(
                System.Windows.Forms.TreeNode DragTreeNode,
                System.Windows.Forms.TreeNode DropTreeNode)
            {
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;
                System.String sDragPath, sDropPath;
                System.Windows.Forms.TreeNode TmpTreeNode;
                System.Text.StringBuilder sb;

                if (!IsTreeNameChildInUse(DragTreeNode.Text, DropTreeNode))
                {
                    using (new Common.Forms.WaitCursor(this))
                    {
                        sDragPath = GetNodeFilePath(DragTreeNode);
                        sDropPath = GetNodeFilePath(DropTreeNode);

                        if (sDropPath.Length > 0)
                        {
                            sDropPath += System.IO.Path.DirectorySeparatorChar;
                        }

                        sDropPath += System.IO.Path.GetFileName(sDragPath);

                        try
                        {
                            m_ZipFile.RenameFolder(sDragPath, sDropPath);
                        }

                        catch (System.Exception exception)
                        {
                            sb = new System.Text.StringBuilder();

                            sb.Append("The files could not be moved.");
                            sb.AppendLine();
                            sb.AppendLine();
                            sb.AppendLine(exception.Message);

                            Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Error);

                            return;
                        }

                        if (DragTreeNode.Parent == s_DragSelectedTreeNode)
                        {
                            DeleteListViewItem(DragTreeNode.Text);
                        }
                        else if (DropTreeNode == s_DragSelectedTreeNode)
                        {
                            AddListViewItem(DragTreeNode.Text);
                        }

                        treeViewFolder.BeginUpdate();

                        DragTreeNode.Parent.Nodes.Remove(DragTreeNode);

                        DropTreeNode.Nodes.Add(DragTreeNode);

                        if (!DropTreeNode.IsExpanded)
                        {
                            DropTreeNode.Expand();
                        }

                        RenameTreeFiles(DragTreeNode, sDragPath, sDropPath);

                        treeViewFolder.EndUpdate();
                    }
                }
                else
                {
                    ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)DropTreeNode.Tag;

                    if (IsDropFileNameInUse(DragTreeNode.Text, DropTreeNode))
                    {
                        Common.Forms.MessageBox.Show(this, "A file with a matching name already exists.",
                                                        System.Windows.Forms.MessageBoxButtons.OK,
                                                        System.Windows.Forms.MessageBoxIcon.Information);

                        return;
                    }

                    // Future enhancement - Instead of replacing offer to do a merge instead.

                    if (System.Windows.Forms.DialogResult.No == Common.Forms.MessageBox.Show(this,
                            "A folder with a matching name already exists.  Replace the existing folder?",
                            System.Windows.Forms.MessageBoxButtons.YesNo,
                            System.Windows.Forms.MessageBoxIcon.Information))
                    {
                        return;
                    }

                    using (new Common.Forms.WaitCursor(this))
                    {
                        TmpTreeNode = FindTreeNode(DragTreeNode.Text, DropTreeNode.Nodes);

                        sDragPath = GetNodeFilePath(DragTreeNode);
                        sDropPath = GetNodeFilePath(DropTreeNode);

                        if (sDropPath.Length > 0)
                        {
                            sDropPath += System.IO.Path.DirectorySeparatorChar;
                        }

                        sDropPath += System.IO.Path.GetFileName(sDragPath);

                        try
                        {
                            m_ZipFile.RenameFolder(sDragPath, sDropPath);
                        }

                        catch (System.Exception exception)
                        {
                            sb = new System.Text.StringBuilder();

                            sb.Append("The files could not be moved.");
                            sb.AppendLine();
                            sb.AppendLine();
                            sb.AppendLine(exception.Message);

                            Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Error);

                            return;
                        }

                        if (DragTreeNode.Parent == s_DragSelectedTreeNode)
                        {
                            DeleteListViewItem(DragTreeNode.Text);
                        }

                        treeViewFolder.BeginUpdate();

                        TmpTreeNode.Remove();

                        DragTreeNode.Parent.Nodes.Remove(DragTreeNode);

                        DropTreeNode.Nodes.Add(DragTreeNode);

                        if (!DropTreeNode.IsExpanded)
                        {
                            DropTreeNode.Expand();
                        }

                        RenameTreeFiles(DragTreeNode, sDragPath, sDropPath);

                        treeViewFolder.EndUpdate();
                    }
                }
            }

            private static System.Boolean ProcessTreeViewDragDropExpand(
                System.Windows.Forms.TreeNode TreeNode,
                System.Drawing.Point Point)
            {
                if (TreeNode.Nodes.Count > 0 && TreeNode.IsExpanded == false)
                {
                    if (Point != s_TreeViewExpandDragDropData.Point)
                    {
                        StartTreeViewDragDropExpandTimer(TreeNode, Point);

                        return true;
                    }
                }
                else
                {
                    StopTreeViewDragDropExpandTimer();
                }

                return false;
            }

            private static void StartTreeViewDragDropExpandTimer(
                System.Windows.Forms.TreeNode TreeNode,
                System.Drawing.Point Point)
            {
                s_TreeViewExpandDragDropData.Point = Point;
                s_TreeViewExpandDragDropData.TreeNode = TreeNode;

                s_TreeViewExpandDragDropData.Timer.Start();
            }

            private static void StopTreeViewDragDropExpandTimer()
            {
                s_TreeViewExpandDragDropData.Timer.Stop();

                s_TreeViewExpandDragDropData.TreeNode = null;

                s_TreeViewExpandDragDropData.Point.X = -1;
                s_TreeViewExpandDragDropData.Point.Y = -1;
            }

            private static System.Boolean ProcessTreeViewDragDropScroll(
                Common.Forms.TreeView TreeView,
                System.Drawing.Point Point)
            {
                switch (TreeView.DragDropScrollHitTest(Point))
                {
                    case Common.Forms.TreeView.EDragDropScrollHitTest.UpArea:
                        if (TreeView.CanScrollUp)
                        {
                            StartTreeViewDragDropScrollTimer(Point,
                                                                Common.Forms.TreeView.EDragDropScrollHitTest.UpArea,
                                                                TreeView);

                            return true;
                        }
                        break;
                    case Common.Forms.TreeView.EDragDropScrollHitTest.DownArea:
                        if (TreeView.CanScrollDown)
                        {
                            StartTreeViewDragDropScrollTimer(Point,
                                                                Common.Forms.TreeView.EDragDropScrollHitTest.DownArea,
                                                                TreeView);

                            return true;
                        }
                        break;
                    case Common.Forms.TreeView.EDragDropScrollHitTest.None:
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown tree view scroll hit test area value.");
                        break;
                }

                return false;
            }

            private static void StartTreeViewDragDropScrollTimer(
                System.Drawing.Point Point,
                Common.Forms.TreeView.EDragDropScrollHitTest ScrollArea,
                Common.Forms.TreeView TreeView)
            {
                s_TreeViewScrollDragDropData.Point = Point;
                s_TreeViewScrollDragDropData.ScrollArea = ScrollArea;
                s_TreeViewScrollDragDropData.TreeView = TreeView;

                s_TreeViewScrollDragDropData.Timer.Start();
            }

            private static void StopTreeViewDragDropScrollTimer()
            {
                s_TreeViewScrollDragDropData.Timer.Stop();

                s_TreeViewScrollDragDropData.Point.X = -1;
                s_TreeViewScrollDragDropData.Point.Y = -1;

                s_TreeViewScrollDragDropData.TreeView = null;
            }

            private static System.Boolean ProcessListViewDragDropScroll(
                Common.Forms.ListView ListView,
                System.Drawing.Point Point)
            {
                switch (ListView.DragDropScrollHitTest(Point))
                {
                    case Common.Forms.ListView.EDragDropScrollHitTest.UpArea:
                        if (ListView.CanScrollUp)
                        {
                            StartListViewDragDropScrollTimer(Point,
                                                                Common.Forms.ListView.EDragDropScrollHitTest.UpArea,
                                                                ListView);

                            return true;
                        }
                        break;
                    case Common.Forms.ListView.EDragDropScrollHitTest.DownArea:
                        if (ListView.CanScrollDown)
                        {
                            StartListViewDragDropScrollTimer(Point,
                                                                Common.Forms.ListView.EDragDropScrollHitTest.DownArea,
                                                                ListView);

                            return true;
                        }
                        break;
                    case Common.Forms.ListView.EDragDropScrollHitTest.None:
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown list view scroll hit test area value.");
                        break;
                }

                return false;
            }

            private static void StartListViewDragDropScrollTimer(
                System.Drawing.Point Point,
                Common.Forms.ListView.EDragDropScrollHitTest ScrollArea,
                Common.Forms.ListView ListView)
            {
                s_ListViewScrollDragDropData.Point = Point;
                s_ListViewScrollDragDropData.ScrollArea = ScrollArea;
                s_ListViewScrollDragDropData.ListView = ListView;

                s_ListViewScrollDragDropData.Timer.Start();
            }

            private static void StopListViewDragDropScrollTimer()
            {
                s_ListViewScrollDragDropData.Timer.Stop();

                s_ListViewScrollDragDropData.ListView = null;

                s_ListViewScrollDragDropData.Point.X = -1;
                s_ListViewScrollDragDropData.Point.Y = -1;
            }

            private static void TreeViewExpandTimerEventProcessor(
                System.Object sender,
                System.EventArgs e)
            {
                s_TreeViewExpandDragDropData.TreeNode.TreeView.BeginUpdate();
                s_TreeViewExpandDragDropData.TreeNode.Expand();
                s_TreeViewExpandDragDropData.TreeNode.EnsureVisible();
                s_TreeViewExpandDragDropData.TreeNode.TreeView.EndUpdate();

                StopTreeViewDragDropExpandTimer();
            }

            private static void TreeViewScrollTimerEventProcessor(
                System.Object sender,
                System.EventArgs e)
            {
                switch (s_TreeViewScrollDragDropData.ScrollArea)
                {
                    case Common.Forms.TreeView.EDragDropScrollHitTest.UpArea:
                        s_TreeViewScrollDragDropData.TreeView.ScrollUp();
                        break;
                    case Common.Forms.TreeView.EDragDropScrollHitTest.DownArea:
                        s_TreeViewScrollDragDropData.TreeView.ScrollDown();
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown tree view scroll area value.");
                        break;
                }
            }

            private static void ListViewScrollTimerEventProcessor(
                System.Object sender,
                System.EventArgs e)
            {
                switch (s_ListViewScrollDragDropData.ScrollArea)
                {
                    case Common.Forms.ListView.EDragDropScrollHitTest.UpArea:
                        s_ListViewScrollDragDropData.ListView.ScrollUp();
                        break;
                    case Common.Forms.ListView.EDragDropScrollHitTest.DownArea:
                        s_ListViewScrollDragDropData.ListView.ScrollDown();
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown list view scroll area value.");
                        break;
                }
            }

            private void UpdateListViewSelectedItem(
                System.Windows.Forms.ListViewItem Item)
            {
                AcceptButton = null;
                buttonOK.Enabled = false;
                textBoxFileName.Text = "";

                if (IsFolder(Item) == false)
                {
                    switch (m_OpenMode)
                    {
                        case EOpenMode.Open:
                            AcceptButton = buttonOK;
                            buttonOK.Enabled = true;
                            break;
                        case EOpenMode.SaveAs:
                            if (m_bAllowOverwrite)
                            {
                                AcceptButton = buttonOK;
                                buttonOK.Enabled = true;
                            }
                            break;
                        default:
                            System.Diagnostics.Debug.Assert(false, "Unknown open mode.");
                            break;
                    }

                    textBoxFileName.Text = Item.Text;

                    listViewFolderFile.ContextMenuStrip = contextMenuStripFile;
                }
                else
                {
                    listViewFolderFile.ContextMenuStrip = contextMenuStripFolder;
                }
            }

            private void UpdateActiveItem(
                bool treeViewFolderFocused,
                bool listViewFolderFileFocused)
            {
                m_ActiveItem = EActiveItem.None;

                if (treeViewFolderFocused && treeViewFolder.SelectedNode != null)
                {
                    m_ActiveItem = IsFolder(treeViewFolder.SelectedNode) ? EActiveItem.TreeViewFolder : EActiveItem.TreeViewZipFile;
                }

                if (listViewFolderFileFocused && listViewFolderFile.SelectedItems.Count > 0)
                {
                    m_ActiveItem = IsFolder(listViewFolderFile.SelectedItems[0]) ? EActiveItem.ListViewFolder : EActiveItem.ListViewFile;
                }
            }

            private void UpdateMenuAndToolStrips()
            {
                System.Windows.Forms.ToolStripItem[] ToolStripItems = {
                    toolStripButtonNewFolder,
                    toolStripButtonRename,
                    toolStripButtonDeleteFolder,
                    toolStripButtonDeleteFile,
                    toolStripButtonProperties};
                System.Boolean bEnable = true;

                switch (m_ActiveItem)
                {
                    case EActiveItem.None:
                        bEnable = false;
                        break;
                    case EActiveItem.TreeViewZipFile:
                        toolStripForm.ActiveGroup = CTreeViewZipFileGroupName;
                        break;
                    case EActiveItem.TreeViewFolder:
                        toolStripForm.ActiveGroup = CTreeViewFolderGroupName;

                        toolStripMenuItemFolderNewFolder.Visible = true;
                        toolStripSeparatorFolder1.Visible = true;
                        break;
                    case EActiveItem.ListViewFolder:
                        toolStripForm.ActiveGroup = CListViewFolderGroupName;

                        toolStripMenuItemFolderNewFolder.Visible = false;
                        toolStripSeparatorFolder1.Visible = false;
                        break;
                    case EActiveItem.ListViewFile:
                        toolStripForm.ActiveGroup = CListViewFileGroupName;
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown active item");
                        break;
                }

                foreach (System.Windows.Forms.ToolStripItem ToolStripItem in ToolStripItems)
                {
                    ToolStripItem.Enabled = bEnable;
                }
            }

            private static bool IsFolder(
                System.Windows.Forms.TreeNode TreeNode)
            {
                return TreeNode.Parent != null;
            }

            private static bool IsFolder(
                System.Windows.Forms.ListViewItem ListViewItem)
            {
                return ListViewItem.Tag == null;
            }

            private void NewFolder()
            {
                System.Windows.Forms.TreeNode ParentTreeNode = treeViewFolder.SelectedNode;
                System.String sNewFolderName = CreateNewFolderName(ParentTreeNode);
                System.Windows.Forms.TreeNode TreeNode;
                System.Windows.Forms.ListViewItem ListViewItem;

                TreeNode = CreateTreeNode(sNewFolderName);

                ParentTreeNode.Nodes.Add(TreeNode);

                listViewFolderFile.BeginUpdate();

                listViewFolderFile.Enabled = true;

                listViewFolderFile.ListViewItemSorter = null;

                ListViewItem = AddListViewItem(sNewFolderName);

                ListViewItem.EnsureVisible();

                listViewFolderFile.EndUpdate();

                ListViewItem.BeginEdit();
            }

            private void DeleteFolder()
            {
                switch (m_ActiveItem)
                {
                    case EActiveItem.TreeViewFolder:
                        DeleteTreeNode(treeViewFolder.SelectedNode);
                        break;
                    case EActiveItem.ListViewFolder:
                        DeleteListViewItem(listViewFolderFile.FocusedItem);
                        break;
                }
            }

            private void DeleteFile()
            {
                DeleteListViewItem(listViewFolderFile.FocusedItem);
            }

            private void RenameFolder()
            {
                switch (m_ActiveItem)
                {
                    case EActiveItem.TreeViewFolder:
                        treeViewFolder.SelectedNode.BeginEdit();
                        break;
                    case EActiveItem.ListViewFolder:
                        listViewFolderFile.FocusedItem.BeginEdit();
                        break;
                }
            }

            private void RenameFile()
            {
                listViewFolderFile.FocusedItem.BeginEdit();
            }

            private void PropertiesFile()
            {
                OpenZip.Forms.PropertiesForm PropertiesForm = new OpenZip.Forms.PropertiesForm();
                System.Windows.Forms.ListViewItem Item;
                TZipFileData ZipFileData;
                System.Text.StringBuilder sb;

                Item = listViewFolderFile.SelectedItems[0];

                ZipFileData = (TZipFileData)Item.Tag;

                PropertiesForm.PropertiesMode = Forms.PropertiesForm.EPropertiesMode.File;
                PropertiesForm.FileName = Item.Text;
                PropertiesForm.ReadOnlyFile = false;
                PropertiesForm.Comment = ZipFileData.sComment;

                if (PropertiesForm.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    ZipFileData.sComment = PropertiesForm.Comment;

                    Item.Tag = ZipFileData;

                    using (new Common.Forms.WaitCursor(this))
                    {
                        try
                        {
                            m_ZipFile.ChangeFileComment(ZipFileData.sSrcFileName, ZipFileData.sComment);
                        }
                        catch (System.Exception exception)
                        {
                            sb = new System.Text.StringBuilder();

                            sb.Append("The file's comment could not be changed.");
                            sb.AppendLine();
                            sb.AppendLine();
                            sb.AppendLine(exception.Message);

                            Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Error);
                        }
                    }
                }
            }
            #endregion

            #region "Form Event Handlers"
            private void OpenZipForm_Load(
                object sender,
                System.EventArgs e)
            {
                switch (m_OpenMode)
                {
                    case EOpenMode.Open:
                        Text = "Open";
                        buttonOK.Text = "&Open";
                        break;
                    case EOpenMode.SaveAs:
                        Text = "Save As";
                        buttonOK.Text = "&Save";
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown open mode.");
                        break;
                }

                treeViewFolder.ImageList = Common.Forms.ImageManager.CreateFileSmallBorderImageList(0, 1, 0, 1);
                listViewFolderFile.SmallImageList = Common.Forms.ImageManager.CreateFileSmallBorderImageList(0, 1, 0, 0);
                toolStripForm.ImageList = Common.Forms.ImageManager.ToolbarSmallImageList;
                contextMenuStripFile.ImageList = Common.Forms.ImageManager.ToolbarSmallImageList;
                contextMenuStripFolder.ImageList = Common.Forms.ImageManager.ToolbarSmallImageList;
                contextMenuStripZipFile.ImageList = Common.Forms.ImageManager.ToolbarSmallImageList;

                if (s_bInitializeImages)
                {
                    Common.Forms.ImageManager.AddToolbarSmallImages(OpenZipForms.Resources.Resource.ResourceManager);

                    Common.Forms.ImageManager.AddFileSmallImage(m_sZipFile, CZipFileImageName);

                    s_bInitializeImages = false;
                }

                InitImageKeys();

                CreateToolStripGroups();

                SetListViewSorter(listViewFolderFile.SortArrow);

                m_ActiveItem = EActiveItem.TreeViewZipFile;

                UpdateMenuAndToolStrips();

                InitZipFile();

                InitCurrentPath();
            }

            private void OpenZipForm_FormClosed(
                object sender,
                System.Windows.Forms.FormClosedEventArgs e)
            {
                Common.Forms.ImageManager.DestroyFileSmallBorderImageList(treeViewFolder.ImageList);
                Common.Forms.ImageManager.DestroyFileSmallBorderImageList(listViewFolderFile.SmallImageList);

                treeViewFolder.ImageList = null;
                listViewFolderFile.SmallImageList = null;
                toolStripForm.ImageList = null;
                contextMenuStripFile.ImageList = null;
                contextMenuStripFolder.ImageList = null;
                contextMenuStripZipFile.ImageList = null;

                DestroyToolStripGroups();

                try
                {
                    if (m_ZipFile != null)
                    {
                        m_ZipFile.Close();
                    }
                }

                catch (System.Exception)
                {
                }

                m_ZipFile = null;
            }
            #endregion

            #region "Button Click Event Handlers"
            private void buttonOK_Click(
                object sender,
                EventArgs e)
            {
                System.Windows.Forms.TreeNode TreeNode;
                System.Text.StringBuilder sb;
                System.String sRootPath, sNewPath;

                if (listViewFolderFile.SelectedItems.Count > 0 &&
                    0 == System.String.Compare(listViewFolderFile.SelectedItems[0].Text,
                                                textBoxFileName.Text, true))
                {
                    sb = new System.Text.StringBuilder();
                    TreeNode = treeViewFolder.SelectedNode;

                    while (TreeNode.Parent != null)
                    {
                        sb.Insert(0, TreeNode.Text);
                        sb.Insert(0, System.IO.Path.DirectorySeparatorChar);

                        TreeNode = TreeNode.Parent;
                    }

                    m_sCurrentPath = sb.ToString();

                    if (m_sCurrentPath.Length > 0 && m_sCurrentPath[0] == System.IO.Path.DirectorySeparatorChar)
                    {
                        m_sCurrentPath = m_sCurrentPath.Remove(0, 1);
                    }

                    m_sSelection = System.IO.Path.Combine(m_sCurrentPath, listViewFolderFile.SelectedItems[0].Text);
                }
                else
                {
                    sRootPath = GetNodeFilePath(treeViewFolder.SelectedNode);
                    sNewPath = Common.IO.Path.Canonicalize(sRootPath,
                                                           textBoxFileName.Text);

                    m_sSelection = sNewPath;

                    m_sCurrentPath = System.IO.Path.GetDirectoryName(sNewPath);
                }
            }
            #endregion

            #region "Tree View Event Handlers"
            private void treeViewFolder_AfterSelect(
                object sender,
                System.Windows.Forms.TreeViewEventArgs e)
            {
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;

                using (new Common.Forms.WaitCursor(this))
                {
                    ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)e.Node.Tag;

                    if (IsFolder(e.Node))
                    {
                        e.Node.ImageKey = Common.Forms.ImageManager.OpenFolderImageName;
                        e.Node.SelectedImageKey = Common.Forms.ImageManager.OpenFolderImageName;

                        treeViewFolder.ContextMenuStrip = contextMenuStripFolder;
                    }
                    else
                    {
                        treeViewFolder.ContextMenuStrip = contextMenuStripZipFile;
                    }

                    listViewFolderFile.BeginUpdate();
                    listViewFolderFile.Items.Clear();

                    listViewFolderFile.ListViewItemSorter = null;

                    foreach (System.Windows.Forms.TreeNode TreeNode in e.Node.Nodes)
                    {
                        AddListViewItem(TreeNode.Text);
                    }

                    foreach (TZipFileData ZipFileData in ZipFileDataList)
                    {
                        AddListViewItem(ZipFileData);
                    }

                    listViewFolderFile.Enabled = (listViewFolderFile.Items.Count > 0) ? true : false;

                    if (listViewFolderFile.Items.Count > 0)
                    {
                        SetListViewSorter(listViewFolderFile.SortArrow);

                        listViewFolderFile.AutosizeColumns();
                    }

                    listViewFolderFile.EndUpdate();

                    buttonOK.Enabled = false;
                    textBoxFileName.Text = "";
                }

                UpdateActiveItem(treeViewFolder.Focused, listViewFolderFile.Focused);
                UpdateMenuAndToolStrips();
            }

            private void treeViewFolder_BeforeSelect(
                object sender,
                System.Windows.Forms.TreeViewCancelEventArgs e)
            {
                if (treeViewFolder.SelectedNode != null && IsFolder(treeViewFolder.SelectedNode))
                {
                    treeViewFolder.SelectedNode.ImageKey = Common.Forms.ImageManager.FolderImageName;
                    treeViewFolder.SelectedNode.SelectedImageKey = Common.Forms.ImageManager.FolderImageName;
                }
            }

            private void treeViewFolder_AfterLabelEdit(
                object sender,
                System.Windows.Forms.NodeLabelEditEventArgs e)
            {
                System.Collections.Specialized.StringCollection FilesStringCollection;
                System.String sOriginalPath, sNewPath;
                System.Text.StringBuilder sb;

                if (e.Label != null && !IsTreeNameParentOrSiblingInUse(e.Label, e.Node))
                {
                    FilesStringCollection = new System.Collections.Specialized.StringCollection();

                    GetFilesInTreeNode(e.Node, ref FilesStringCollection);

                    sOriginalPath = GetNodeFilePath(e.Node);
                    sNewPath = GetNodeFilePath(e.Node.Parent);

                    if (sNewPath.Length > 0)
                    {
                        sNewPath += System.IO.Path.DirectorySeparatorChar;
                    }

                    sNewPath += e.Label;

                    using (new Common.Forms.WaitCursor(this))
                    {
                        try
                        {
                            m_ZipFile.RenameFolder(sOriginalPath, sNewPath);
                        }

                        catch (System.Exception exception)
                        {
                            sb = new System.Text.StringBuilder();

                            sb.Append("The folder could not be renamed.");
                            sb.AppendLine();
                            sb.AppendLine();
                            sb.AppendLine(exception.Message);

                            Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Error);

                            e.CancelEdit = true;

                            return;
                        }
                        
                        RenameTreeFiles(e.Node, sOriginalPath, sNewPath);

                        RenameListViewFiles(sOriginalPath, sNewPath);

                        TreeViewSort();
                    }
                }
                else
                {
                    e.CancelEdit = true;
                }
            }

            private void treeViewFolder_BeforeLabelEdit(
                object sender,
                System.Windows.Forms.NodeLabelEditEventArgs e)
            {
                if (IsFolder(e.Node) == false)
                {
                    e.CancelEdit = true;
                }
            }

            private void treeViewFolder_KeyDown(
                object sender,
                System.Windows.Forms.KeyEventArgs e)
            {
                System.Windows.Forms.TreeNode TreeNode = treeViewFolder.SelectedNode;
                System.Drawing.Point ItemPoint = new System.Drawing.Point();
                System.Drawing.Rectangle ItemRect;

                ItemRect = treeViewFolder.SelectedNode.Bounds;

                ItemPoint.X = ItemRect.X;
                ItemPoint.Y = ItemRect.Y + (ItemRect.Height / 2);

                switch (e.KeyCode)
                {
                    case System.Windows.Forms.Keys.Apps:
                        e.Handled = true;

                        treeViewFolder.ContextMenuStrip.Show(treeViewFolder, ItemPoint);
                        break;
                    case System.Windows.Forms.Keys.F10:
                        if (e.Shift == true)
                        {
                            e.Handled = true;

                            treeViewFolder.ContextMenuStrip.Show(treeViewFolder, ItemPoint);
                        }
                        break;
                }
            }

            private void treeViewFolder_KeyPressLabelEdit(
                object sender,
                System.Windows.Forms.KeyPressEventArgs e)
            {
                if (e.KeyChar == CBackspaceKeyCode)
                {
                }
                else if (e.KeyChar == CPasteKeyCode)
                {
                    treeViewFolder.ChangeLabelEditText(GetValidClipboardText());

                    e.Handled = true;
                }
                else
                {
                    if (!IsValidKeyChar(e.KeyChar))
                    {
                        e.Handled = true;
                    }
                }
            }

            private void treeViewFolder_ItemDrag(
                object sender,
                System.Windows.Forms.ItemDragEventArgs e)
            {
                System.Windows.Forms.TreeNode TreeNode = (System.Windows.Forms.TreeNode)e.Item;

                if (IsFolder(TreeNode))
                {
                    ProcessItemDrag(e.Item);
                }
            }

            private void treeViewFolder_DragOver(
                object sender,
                System.Windows.Forms.DragEventArgs e)
            {
                System.Windows.Forms.TreeViewHitTestInfo TreeHitTestInfo;
                System.Drawing.Point Point;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;

                Point = new System.Drawing.Point(e.X, e.Y);

                Point = treeViewFolder.PointToClient(Point);

                TreeHitTestInfo = treeViewFolder.HitTest(Point);

                treeViewFolder.DropHighlightedNode = TreeHitTestInfo.Node;

                if (true == ProcessTreeViewDragDropScroll(treeViewFolder, Point))
                {
                    StopTreeViewDragDropExpandTimer();
                }
                else
                {
                    StopTreeViewDragDropScrollTimer();

                    if (TreeHitTestInfo.Node != null)
                    {
                        ProcessTreeViewDragDropExpand(TreeHitTestInfo.Node, Point);
                    }
                    else
                    {
                        StopTreeViewDragDropExpandTimer();
                    }
                }

                e.Effect = System.Windows.Forms.DragDropEffects.None;

                if (TreeHitTestInfo.Node != null)
                {
                    if (s_DragListViewItem != null)
                    {
                        ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)TreeHitTestInfo.Node.Tag;

                        if (ZipFileDataList.Contains((TZipFileData)s_DragListViewItem.Tag))
                        {
                            if (e.KeyState == CControlKeyCode)
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Copy;
                            }
                        }
                        else
                        {
                            if (e.KeyState == CControlKeyCode)
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Copy;
                            }
                            else
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Move;
                            }
                        }
                    }
                    else if (s_DragTreeNode != null)
                    {
                        if (TreeHitTestInfo.Node == s_DragTreeNode ||
                            TreeHitTestInfo.Node == s_DragTreeNode.Parent ||
                            true == Common.Forms.TreeView.IsTreeNodeChild(s_DragTreeNode,
                                                                            TreeHitTestInfo.Node))
                        {
                            if (e.KeyState == CControlKeyCode)
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Copy;
                            }
                        }
                        else
                        {
                            if (e.KeyState == CControlKeyCode)
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Copy;
                            }
                            else
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Move;
                            }
                        }
                    }
                }
            }

            private void treeViewFolder_DragLeave(
                object sender,
                System.EventArgs e)
            {
                StopTreeViewDragDropExpandTimer();
                StopTreeViewDragDropScrollTimer();
            }

            private void treeViewFolder_DragDrop(
                object sender,
                System.Windows.Forms.DragEventArgs e)
            {
                System.Windows.Forms.TreeViewHitTestInfo TreeHitTestInfo;
                System.Drawing.Point Point;

                Point = new System.Drawing.Point(e.X, e.Y);

                Point = treeViewFolder.PointToClient(Point);

                TreeHitTestInfo = treeViewFolder.HitTest(Point);

                if (e.Effect == System.Windows.Forms.DragDropEffects.Copy)
                {
                    ProcessDropCopy(TreeHitTestInfo.Node);
                }
                else if (e.Effect == System.Windows.Forms.DragDropEffects.Move)
                {
                    ProcessDropMove(TreeHitTestInfo.Node);
                }
                else
                {
                    System.Diagnostics.Debug.Assert(false, "Unknown drop operation.");
                }
            }

            private void treeViewFolder_Enter(object sender, EventArgs e)
            {
                if (listViewFolderFile.SelectedItems.Count > 0)
                {
                    UpdateListViewSelectedItem(listViewFolderFile.SelectedItems[0]);
                }

                UpdateActiveItem(true, false);
                UpdateMenuAndToolStrips();
            }

            private void treeViewFolder_Leave(object sender, EventArgs e)
            {
                UpdateActiveItem(false, false);
                UpdateMenuAndToolStrips();
            }
            #endregion

            #region "List View Event Handlers"
            private void listViewFolderFile_AfterLabelEdit(
                object sender,
                System.Windows.Forms.LabelEditEventArgs e)
            {
                TZipFileData ZipFileData;
                System.String sNewFileName, sOriginalPath, sNewPath, sFileExtension;
                System.Collections.Generic.List<TZipFileData> ZipFileDataList;
                System.Windows.Forms.ListViewItem ListViewItem;
                System.Int32 nIndex;
                System.Windows.Forms.TreeNode TreeNode;
                System.Text.StringBuilder sb;

                if (e.Label != null && !IsListViewNameInUse(e.Label))
                {
                    using (new Common.Forms.WaitCursor(this))
                    {
                        ListViewItem = listViewFolderFile.Items[e.Item];

                        if (IsFolder(ListViewItem) == false)
                        {
                            // Renaming file

                            ZipFileData = (TZipFileData)ListViewItem.Tag;

                            sNewFileName = ZipFileData.sPath;

                            if (sNewFileName.Length > 0)
                            {
                                sNewFileName += System.IO.Path.DirectorySeparatorChar;
                            }

                            sNewFileName += e.Label;

                            if (RenameFile(ZipFileData.sSrcFileName, sNewFileName))
                            {
                                if (0 == System.String.Compare(ZipFileData.sFileName,
                                                                textBoxFileName.Text, true))
                                {
                                    textBoxFileName.TextChanged -= new System.EventHandler(textBoxFileName_TextChanged);

                                    textBoxFileName.Text = e.Label;

                                    textBoxFileName.TextChanged += new System.EventHandler(textBoxFileName_TextChanged);
                                }

                                ZipFileDataList = (System.Collections.Generic.List<TZipFileData>)treeViewFolder.SelectedNode.Tag;

                                nIndex = ZipFileDataList.IndexOf(ZipFileData);

                                ZipFileData.sFileName = e.Label;
                                ZipFileData.sSrcFileName = sNewFileName;

                                ZipFileDataList[nIndex] = ZipFileData;

                                ListViewItem.Tag = ZipFileData;

                                sFileExtension = System.IO.Path.GetExtension(e.Label);

                                if (sFileExtension.Length > 0)
                                {
                                    Common.Forms.ImageManager.AddFileExtensionSmallImage(sFileExtension, sFileExtension);
                                }
                                else
                                {
                                    sFileExtension = Common.Forms.ImageManager.UnknownFileImageName;
                                }

                                ListViewItem.ImageKey = sFileExtension;
                            }
                        }
                        else
                        {
                            // Renaming directory

                            TreeNode = FindTreeNode(ListViewItem.Text,
                                                    treeViewFolder.SelectedNode.Nodes);

                            sOriginalPath = GetNodeFilePath(TreeNode);
                            sNewPath = GetNodeFilePath(TreeNode.Parent);

                            if (sNewPath.Length > 0)
                            {
                                sNewPath += System.IO.Path.DirectorySeparatorChar;
                            }

                            sNewPath += e.Label;

                            try
                            {
                                m_ZipFile.RenameFolder(sOriginalPath, sNewPath);

                                RenameTreeFiles(TreeNode, sOriginalPath, sNewPath);

                                TreeNode.Text = e.Label;

                                TreeViewSort();
                            }

                            catch (System.Exception exception)
                            {
                                sb = new System.Text.StringBuilder();

                                sb.Append("The folder could not be renamed.");
                                sb.AppendLine();
                                sb.AppendLine();
                                sb.AppendLine(exception.Message);

                                Common.Forms.MessageBox.Show(this, sb.ToString(),
                                                             System.Windows.Forms.MessageBoxButtons.OK,
                                                             System.Windows.Forms.MessageBoxIcon.Error);

                                e.CancelEdit = true;
                            }
                        }
                    }

                    ListViewSort();
                }
                else
                {
                    e.CancelEdit = true;
                }
            }

            private void listViewFolderFile_BeforeLabelEdit(
                object sender,
                System.Windows.Forms.LabelEditEventArgs e)
            {
                System.Windows.Forms.ListViewItem ListViewItem;
                System.String sFileName, sExtension;

                if (m_bEditingSelectFileNameOnly)
                {
                    ListViewItem = listViewFolderFile.Items[e.Item];
                    sFileName = ListViewItem.Text;
                    sExtension = System.IO.Path.GetExtension(sFileName);

                    listViewFolderFile.ChangeLabelEditSelection(0, sFileName.Length - sExtension.Length);
                }
            }

            private void listViewFolderFile_ItemSelectionChanged(
                object sender,
                System.Windows.Forms.ListViewItemSelectionChangedEventArgs e)
            {
                textBoxFileName.TextChanged -= new System.EventHandler(textBoxFileName_TextChanged);

                AcceptButton = null;
                buttonOK.Enabled = false;
                textBoxFileName.Text = "";

                if (e.IsSelected)
                {
                    UpdateListViewSelectedItem(e.Item);
                }
                else
                {
                    listViewFolderFile.ContextMenuStrip = null;
                }

                textBoxFileName.TextChanged += new System.EventHandler(textBoxFileName_TextChanged);

                UpdateActiveItem(treeViewFolder.Focused, listViewFolderFile.Focused);
                UpdateMenuAndToolStrips();
            }

            private void listViewFolderFile_KeyDown(
                object sender,
                System.Windows.Forms.KeyEventArgs e)
            {
                System.Drawing.Point ItemPoint = new System.Drawing.Point();
                System.Drawing.Rectangle ItemRect;
                System.Windows.Forms.ListViewItem ListViewItem;

                if (listViewFolderFile.SelectedItems.Count > 0)
                {
                    ListViewItem = listViewFolderFile.SelectedItems[0];

                    ItemRect = ListViewItem.GetBounds(System.Windows.Forms.ItemBoundsPortion.Label);

                    ItemPoint.X = ItemRect.X;
                    ItemPoint.Y = ItemRect.Y + (ItemRect.Height / 2);
                }
                else
                {
                    ListViewItem = null;
                }

                switch (e.KeyCode)
                {
                    case System.Windows.Forms.Keys.Apps:
                        e.Handled = true;

                        if (ListViewItem != null)
                        {
                            listViewFolderFile.ContextMenuStrip.Show(listViewFolderFile, ItemPoint);
                        }
                        break;
                    case System.Windows.Forms.Keys.F10:
                        if (e.Shift == true)
                        {
                            e.Handled = true;

                            if (ListViewItem != null)
                            {
                                listViewFolderFile.ContextMenuStrip.Show(listViewFolderFile, ItemPoint);
                            }
                        }
                        break;
                }
            }

            private void listViewFolderFile_KeyPressLabelEdit(
                object sender,
                System.Windows.Forms.KeyPressEventArgs e)
            {
                if (e.KeyChar == CBackspaceKeyCode)
                {
                }
                else if (e.KeyChar == CPasteKeyCode)
                {
                    listViewFolderFile.ChangeLabelEditText(GetValidClipboardText());

                    e.Handled = true;
                }
                else
                {
                    if (!IsValidKeyChar(e.KeyChar))
                    {
                        e.Handled = true;
                    }
                }
            }

            private void listViewFolderFile_ColumnClick(
                object sender,
                System.Windows.Forms.ColumnClickEventArgs e)
            {
                if (e.Column == 0)
                {
                    using (new Common.Forms.WaitCursor(this))
                    {
                        if (listViewFolderFile.SortArrow == Common.Forms.ListView.ESortArrow.Up)
                        {
                            listViewFolderFile.ListViewItemSorter = new ListViewNameDescendingComparer();
                            listViewFolderFile.SortArrow = Common.Forms.ListView.ESortArrow.Down;
                        }
                        else
                        {
                            listViewFolderFile.ListViewItemSorter = new ListViewNameAscendingComparer();
                            listViewFolderFile.SortArrow = Common.Forms.ListView.ESortArrow.Up;
                        }
                    }
                }
            }

            private void listViewFolderFile_DoubleClick(
                object sender,
                System.EventArgs e)
            {
                System.Windows.Forms.TreeNode TreeNode;

                TreeNode = FindTreeNode(listViewFolderFile.SelectedItems[0].Text,
                                        treeViewFolder.SelectedNode.Nodes);

                if (TreeNode != null)
                {
                    TreeNode.EnsureVisible();

                    treeViewFolder.SelectedNode = TreeNode;

                    UpdateActiveItem(true, false);
                    UpdateMenuAndToolStrips();
                    UpdateActiveItem(false, false);
                    UpdateMenuAndToolStrips();
                }
            }

            private void listViewFolderFile_ItemDrag(
                object sender,
                System.Windows.Forms.ItemDragEventArgs e)
            {
                ProcessItemDrag(e.Item);
            }

            private void listViewFolderFile_DragOver(
                object sender,
                System.Windows.Forms.DragEventArgs e)
            {
                System.Windows.Forms.ListViewHitTestInfo ListHitTestInfo;
                System.Drawing.Point Point;

                Point = new System.Drawing.Point(e.X, e.Y);

                Point = listViewFolderFile.PointToClient(Point);

                ListHitTestInfo = listViewFolderFile.HitTest(Point);

                listViewFolderFile.DropHighlightedItem = ListHitTestInfo.Item;

                if (false == ProcessListViewDragDropScroll(listViewFolderFile, Point))
                {
                    StopListViewDragDropScrollTimer();
                }

                e.Effect = System.Windows.Forms.DragDropEffects.None;

                if (s_DragListViewItem != null)
                {
                    if (s_DragListViewItem.Tag == null)
                    {
                        // Dragging a folder

                        if (e.KeyState == CControlKeyCode)
                        {
                            e.Effect = System.Windows.Forms.DragDropEffects.Copy;
                        }
                        else
                        {
                            e.Effect = System.Windows.Forms.DragDropEffects.Move;
                        }
                    }
                    else
                    {
                        // Dragging a file

                        if (e.KeyState == CControlKeyCode)
                        {
                            e.Effect = System.Windows.Forms.DragDropEffects.Copy;
                        }
                        else
                        {
                            if (ListHitTestInfo.Item != null &&
                                ListHitTestInfo.Item.Tag == null)
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Move;
                            }
                        }
                    }
                }
                else if (s_DragTreeNode != null)
                {
                    // Dragging a folder

                    if (e.KeyState == CControlKeyCode)
                    {
                        e.Effect = System.Windows.Forms.DragDropEffects.Copy;
                    }
                    else
                    {
                        if (ListHitTestInfo.Item != null)
                        {
                            if (s_DragListViewItemToTreeNodeDict.ContainsKey(ListHitTestInfo.Item) &&
                                s_DragTreeNode != s_DragListViewItemToTreeNodeDict[ListHitTestInfo.Item] &&
                                s_DragTreeNode.Parent != s_DragListViewItemToTreeNodeDict[ListHitTestInfo.Item])
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Move;
                            }
                        }
                        else
                        {
                            if (s_DragTreeNode != s_DragSelectedTreeNode)
                            {
                                e.Effect = System.Windows.Forms.DragDropEffects.Move;
                            }
                        }
                    }
                }
            }

            private void listViewFolderFile_DragLeave(
                object sender,
                System.EventArgs e)
            {
                StopListViewDragDropScrollTimer();
            }

            private void listViewFolderFile_DragDrop(
                object sender,
                System.Windows.Forms.DragEventArgs e)
            {
                System.Windows.Forms.ListViewHitTestInfo ListHitTestInfo;
                System.Windows.Forms.TreeNode TreeNode;
                System.Drawing.Point Point;

                Point = new System.Drawing.Point(e.X, e.Y);

                Point = listViewFolderFile.PointToClient(Point);

                ListHitTestInfo = listViewFolderFile.HitTest(Point);

                if (ListHitTestInfo.Item != null)
                {
                    TreeNode = s_DragListViewItemToTreeNodeDict[ListHitTestInfo.Item];
                }
                else
                {
                    TreeNode = s_DragSelectedTreeNode;
                }

                if (e.Effect == System.Windows.Forms.DragDropEffects.Copy)
                {
                    ProcessDropCopy(TreeNode);
                }
                else if (e.Effect == System.Windows.Forms.DragDropEffects.Move)
                {
                    ProcessDropMove(TreeNode);
                }
                else
                {
                    System.Diagnostics.Debug.Assert(false, "Unknown drop operation.");
                }
            }

            private void listViewFolderFile_Enter(object sender, EventArgs e)
            {
                if (listViewFolderFile.SelectedItems.Count > 0)
                {
                    UpdateListViewSelectedItem(listViewFolderFile.SelectedItems[0]);
                }

                UpdateActiveItem(false, true);
                UpdateMenuAndToolStrips();
            }

            private void listViewFolderFile_Leave(object sender, EventArgs e)
            {
                UpdateActiveItem(false, false);
                UpdateMenuAndToolStrips();
            }
            #endregion

            #region "Text Box Event Handlers"
            private void textBoxFileName_TextChanged(
                object sender,
                System.EventArgs e)
            {
                VerifyFileName();
            }
            #endregion

            #region "Tool Strip Menu Item Event Handlers"
            private void toolStripMenuItemZipFileNewFolder_Click(
                object sender,
                EventArgs e)
            {
                NewFolder();
            }

            private void toolStripMenuItemFolderNewFolder_Click(
                object sender,
                System.EventArgs e)
            {
                NewFolder();
            }

            private void toolStripMenuItemFolderDelete_Click(
                object sender,
                System.EventArgs e)
            {
                DeleteFolder();
            }

            private void toolStripMenuItemFolderRename_Click(
                object sender,
                System.EventArgs e)
            {
                RenameFolder();
            }

            private void toolStripMenuItemFileDelete_Click(
                object sender,
                System.EventArgs e)
            {
                DeleteFile();
            }

            private void toolStripMenuItemFileRename_Click(
                object sender,
                System.EventArgs e)
            {
                RenameFile();
            }

            private void toolStripMenuItemFileProperties_Click(
                object sender,
                EventArgs e)
            {
                PropertiesFile();
            }
            #endregion

            #region "Tool Strip Button Event Handlers"
            private void toolStripButtonNewFolder_Click(
                object sender,
                EventArgs e)
            {
                NewFolder();
            }

            private void toolStripButtonDeleteFolder_Click(
                object sender,
                EventArgs e)
            {
                DeleteFolder();
            }

            private void toolStripButtonDeleteFile_Click(
                object sender,
                EventArgs e)
            {
                DeleteFile();
            }

            private void toolStripButtonRename_Click(
                object sender,
                EventArgs e)
            {
                switch (m_ActiveItem)
                {
                    case EActiveItem.TreeViewFolder:
                    case EActiveItem.ListViewFolder:
                        RenameFolder();
                        break;
                    case EActiveItem.ListViewFile:
                        RenameFile();
                        break;
                }
            }

            private void toolStripButtonProperties_Click(
                object sender,
                EventArgs e)
            {
                PropertiesFile();
            }
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2021 Kevin Eshbach                                  */
/***************************************************************************/
