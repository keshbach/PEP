/***************************************************************************/
/*  Copyright (C) 2014-2020 Kevin Eshbach                                  */
/***************************************************************************/

namespace OpenZip
{
    namespace Forms
    {
        partial class OpenZipForm
        {
            /// <summary>
            /// Required designer variable.
            /// </summary>
            private System.ComponentModel.IContainer components = null;

            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
            protected override void Dispose(bool disposing)
            {
                if (disposing && (components != null))
                {
                    components.Dispose();
                }
                base.Dispose(disposing);
            }

            #region Windows Form Designer generated code

            /// <summary>
            /// Required method for Designer support - do not modify
            /// the contents of this method with the code editor.
            /// </summary>
            private void InitializeComponent()
            {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(OpenZipForm));
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.labelFileName = new System.Windows.Forms.Label();
            this.textBoxFileName = new Common.Forms.TextBox();
            this.splitContainerFolderFile = new System.Windows.Forms.SplitContainer();
            this.treeViewFolder = new Common.Forms.TreeView();
            this.contextMenuStripFolder = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemFolderNewFolder = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparatorFolder1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuItemFolderDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemFolderRename = new System.Windows.Forms.ToolStripMenuItem();
            this.listViewFolderFile = new Common.Forms.ListView();
            this.columnHeaderName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderSize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderCRC = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderAttributes = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderModified = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStripFile = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemFileDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemFileRename = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparatorFile1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuItemFileProperties = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripForm = new Common.Forms.ToolStrip();
            this.toolStripButtonNewFolder = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparatorToolbar1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonDeleteFolder = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonDeleteFile = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonRename = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparatorToolbar2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonProperties = new System.Windows.Forms.ToolStripButton();
            this.contextMenuStripZipFile = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemZipFileNewFolder = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerFolderFile)).BeginInit();
            this.splitContainerFolderFile.Panel1.SuspendLayout();
            this.splitContainerFolderFile.Panel2.SuspendLayout();
            this.splitContainerFolderFile.SuspendLayout();
            this.contextMenuStripFolder.SuspendLayout();
            this.contextMenuStripFile.SuspendLayout();
            this.toolStripForm.SuspendLayout();
            this.contextMenuStripZipFile.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(587, 277);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 3;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(506, 277);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 2;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // labelFileName
            // 
            this.labelFileName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelFileName.AutoSize = true;
            this.labelFileName.Location = new System.Drawing.Point(12, 254);
            this.labelFileName.Name = "labelFileName";
            this.labelFileName.Size = new System.Drawing.Size(55, 13);
            this.labelFileName.TabIndex = 0;
            this.labelFileName.Text = "&File name:";
            // 
            // textBoxFileName
            // 
            this.textBoxFileName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxFileName.Location = new System.Drawing.Point(73, 251);
            this.textBoxFileName.MaxLength = 256;
            this.textBoxFileName.Name = "textBoxFileName";
            this.textBoxFileName.Size = new System.Drawing.Size(589, 20);
            this.textBoxFileName.TabIndex = 1;
            this.textBoxFileName.WordWrap = false;
            this.textBoxFileName.TextChanged += new System.EventHandler(this.textBoxFileName_TextChanged);
            // 
            // splitContainerFolderFile
            // 
            this.splitContainerFolderFile.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainerFolderFile.Location = new System.Drawing.Point(12, 34);
            this.splitContainerFolderFile.Name = "splitContainerFolderFile";
            // 
            // splitContainerFolderFile.Panel1
            // 
            this.splitContainerFolderFile.Panel1.Controls.Add(this.treeViewFolder);
            // 
            // splitContainerFolderFile.Panel2
            // 
            this.splitContainerFolderFile.Panel2.Controls.Add(this.listViewFolderFile);
            this.splitContainerFolderFile.Size = new System.Drawing.Size(650, 205);
            this.splitContainerFolderFile.SplitterDistance = 262;
            this.splitContainerFolderFile.TabIndex = 4;
            // 
            // treeViewFolder
            // 
            this.treeViewFolder.AllowDrop = true;
            this.treeViewFolder.ContextMenuStrip = this.contextMenuStripFolder;
            this.treeViewFolder.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeViewFolder.HideSelection = false;
            this.treeViewFolder.LabelEdit = true;
            this.treeViewFolder.Location = new System.Drawing.Point(0, 0);
            this.treeViewFolder.Name = "treeViewFolder";
            this.treeViewFolder.Size = new System.Drawing.Size(262, 205);
            this.treeViewFolder.Sorted = true;
            this.treeViewFolder.Sorting = Common.Forms.TreeView.ESortOrder.Ascending;
            this.treeViewFolder.TabIndex = 0;
            this.treeViewFolder.PasteLabelEdit += new Common.Forms.TreeView.PasteLabelEditHandler(this.treeViewFolder_PasteLabelEdit);
            this.treeViewFolder.KeyPressLabelEdit += new Common.Forms.TreeView.KeyPressLabelEditHandler(this.treeViewFolder_KeyPressLabelEdit);
            this.treeViewFolder.BeforeLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.treeViewFolder_BeforeLabelEdit);
            this.treeViewFolder.AfterLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.treeViewFolder_AfterLabelEdit);
            this.treeViewFolder.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.treeViewFolder_ItemDrag);
            this.treeViewFolder.BeforeSelect += new System.Windows.Forms.TreeViewCancelEventHandler(this.treeViewFolder_BeforeSelect);
            this.treeViewFolder.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewFolder_AfterSelect);
            this.treeViewFolder.DragDrop += new System.Windows.Forms.DragEventHandler(this.treeViewFolder_DragDrop);
            this.treeViewFolder.DragOver += new System.Windows.Forms.DragEventHandler(this.treeViewFolder_DragOver);
            this.treeViewFolder.DragLeave += new System.EventHandler(this.treeViewFolder_DragLeave);
            this.treeViewFolder.Enter += new System.EventHandler(this.treeViewFolder_Enter);
            this.treeViewFolder.KeyDown += new System.Windows.Forms.KeyEventHandler(this.treeViewFolder_KeyDown);
            this.treeViewFolder.Leave += new System.EventHandler(this.treeViewFolder_Leave);
            // 
            // contextMenuStripFolder
            // 
            this.contextMenuStripFolder.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemFolderNewFolder,
            this.toolStripSeparatorFolder1,
            this.toolStripMenuItemFolderDelete,
            this.toolStripMenuItemFolderRename});
            this.contextMenuStripFolder.Name = "contextMenuStrip1";
            this.contextMenuStripFolder.Size = new System.Drawing.Size(137, 76);
            // 
            // toolStripMenuItemFolderNewFolder
            // 
            this.toolStripMenuItemFolderNewFolder.Name = "toolStripMenuItemFolderNewFolder";
            this.toolStripMenuItemFolderNewFolder.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFolderNewFolder.Text = "New folder";
            this.toolStripMenuItemFolderNewFolder.Click += new System.EventHandler(this.toolStripMenuItemFolderNewFolder_Click);
            // 
            // toolStripSeparatorFolder1
            // 
            this.toolStripSeparatorFolder1.Name = "toolStripSeparatorFolder1";
            this.toolStripSeparatorFolder1.Size = new System.Drawing.Size(133, 6);
            // 
            // toolStripMenuItemFolderDelete
            // 
            this.toolStripMenuItemFolderDelete.Name = "toolStripMenuItemFolderDelete";
            this.toolStripMenuItemFolderDelete.ShortcutKeys = System.Windows.Forms.Keys.Delete;
            this.toolStripMenuItemFolderDelete.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFolderDelete.Text = "&Delete";
            this.toolStripMenuItemFolderDelete.Click += new System.EventHandler(this.toolStripMenuItemFolderDelete_Click);
            // 
            // toolStripMenuItemFolderRename
            // 
            this.toolStripMenuItemFolderRename.Name = "toolStripMenuItemFolderRename";
            this.toolStripMenuItemFolderRename.ShortcutKeys = System.Windows.Forms.Keys.F2;
            this.toolStripMenuItemFolderRename.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFolderRename.Text = "&Rename";
            this.toolStripMenuItemFolderRename.Click += new System.EventHandler(this.toolStripMenuItemFolderRename_Click);
            // 
            // listViewFolderFile
            // 
            this.listViewFolderFile.AllowDrop = true;
            this.listViewFolderFile.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderName,
            this.columnHeaderSize,
            this.columnHeaderCRC,
            this.columnHeaderAttributes,
            this.columnHeaderModified});
            this.listViewFolderFile.ComboBoxItems = ((System.Collections.Specialized.StringCollection)(resources.GetObject("listViewFolderFile.ComboBoxItems")));
            this.listViewFolderFile.ContextMenuStrip = this.contextMenuStripFile;
            this.listViewFolderFile.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listViewFolderFile.FullRowSelect = true;
            this.listViewFolderFile.HideSelection = false;
            this.listViewFolderFile.LabelEdit = true;
            this.listViewFolderFile.LabelEditor = Common.Forms.ListView.ELabelEditor.Label;
            this.listViewFolderFile.LabelWrap = false;
            this.listViewFolderFile.Location = new System.Drawing.Point(0, 0);
            this.listViewFolderFile.MultiSelect = false;
            this.listViewFolderFile.Name = "listViewFolderFile";
            this.listViewFolderFile.ShowGroups = false;
            this.listViewFolderFile.ShowItemToolTips = true;
            this.listViewFolderFile.Size = new System.Drawing.Size(384, 205);
            this.listViewFolderFile.SortArrow = Common.Forms.ListView.ESortArrow.Up;
            this.listViewFolderFile.Sorting = Common.Forms.ListView.ESortOrder.UserDefined;
            this.listViewFolderFile.TabIndex = 0;
            this.listViewFolderFile.UseCompatibleStateImageBehavior = false;
            this.listViewFolderFile.View = System.Windows.Forms.View.Details;
            this.listViewFolderFile.PasteLabelEdit += new Common.Forms.ListView.PasteLabelEditHandler(this.listViewFolderFile_PasteLabelEdit);
            this.listViewFolderFile.KeyPressLabelEdit += new Common.Forms.ListView.KeyPressLabelEditHandler(this.listViewFolderFile_KeyPressLabelEdit);
            this.listViewFolderFile.AfterLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.listViewFolderFile_AfterLabelEdit);
            this.listViewFolderFile.BeforeLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.listViewFolderFile_BeforeLabelEdit);
            this.listViewFolderFile.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listViewFolderFile_ColumnClick);
            this.listViewFolderFile.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.listViewFolderFile_ItemDrag);
            this.listViewFolderFile.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.listViewFolderFile_ItemSelectionChanged);
            this.listViewFolderFile.DragDrop += new System.Windows.Forms.DragEventHandler(this.listViewFolderFile_DragDrop);
            this.listViewFolderFile.DragOver += new System.Windows.Forms.DragEventHandler(this.listViewFolderFile_DragOver);
            this.listViewFolderFile.DragLeave += new System.EventHandler(this.listViewFolderFile_DragLeave);
            this.listViewFolderFile.DoubleClick += new System.EventHandler(this.listViewFolderFile_DoubleClick);
            this.listViewFolderFile.Enter += new System.EventHandler(this.listViewFolderFile_Enter);
            this.listViewFolderFile.KeyDown += new System.Windows.Forms.KeyEventHandler(this.listViewFolderFile_KeyDown);
            this.listViewFolderFile.Leave += new System.EventHandler(this.listViewFolderFile_Leave);
            // 
            // columnHeaderName
            // 
            this.columnHeaderName.Text = "Name";
            // 
            // columnHeaderSize
            // 
            this.columnHeaderSize.Text = "Size";
            // 
            // columnHeaderCRC
            // 
            this.columnHeaderCRC.Text = "CRC";
            // 
            // columnHeaderAttributes
            // 
            this.columnHeaderAttributes.Text = "Attributes";
            // 
            // columnHeaderModified
            // 
            this.columnHeaderModified.Text = "Modified";
            // 
            // contextMenuStripFile
            // 
            this.contextMenuStripFile.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemFileDelete,
            this.toolStripMenuItemFileRename,
            this.toolStripSeparatorFile1,
            this.toolStripMenuItemFileProperties});
            this.contextMenuStripFile.Name = "contextMenuStripListView";
            this.contextMenuStripFile.Size = new System.Drawing.Size(137, 76);
            // 
            // toolStripMenuItemFileDelete
            // 
            this.toolStripMenuItemFileDelete.Name = "toolStripMenuItemFileDelete";
            this.toolStripMenuItemFileDelete.ShortcutKeys = System.Windows.Forms.Keys.Delete;
            this.toolStripMenuItemFileDelete.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFileDelete.Text = "&Delete";
            this.toolStripMenuItemFileDelete.Click += new System.EventHandler(this.toolStripMenuItemFileDelete_Click);
            // 
            // toolStripMenuItemFileRename
            // 
            this.toolStripMenuItemFileRename.Name = "toolStripMenuItemFileRename";
            this.toolStripMenuItemFileRename.ShortcutKeys = System.Windows.Forms.Keys.F2;
            this.toolStripMenuItemFileRename.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFileRename.Text = "&Rename";
            this.toolStripMenuItemFileRename.Click += new System.EventHandler(this.toolStripMenuItemFileRename_Click);
            // 
            // toolStripSeparatorFile1
            // 
            this.toolStripSeparatorFile1.Name = "toolStripSeparatorFile1";
            this.toolStripSeparatorFile1.Size = new System.Drawing.Size(133, 6);
            // 
            // toolStripMenuItemFileProperties
            // 
            this.toolStripMenuItemFileProperties.Name = "toolStripMenuItemFileProperties";
            this.toolStripMenuItemFileProperties.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFileProperties.Text = "&Properties";
            this.toolStripMenuItemFileProperties.Click += new System.EventHandler(this.toolStripMenuItemFileProperties_Click);
            // 
            // toolStripForm
            // 
            this.toolStripForm.ActiveGroup = "";
            this.toolStripForm.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonNewFolder,
            this.toolStripSeparatorToolbar1,
            this.toolStripButtonDeleteFolder,
            this.toolStripButtonDeleteFile,
            this.toolStripButtonRename,
            this.toolStripSeparatorToolbar2,
            this.toolStripButtonProperties});
            this.toolStripForm.Location = new System.Drawing.Point(0, 0);
            this.toolStripForm.Name = "toolStripForm";
            this.toolStripForm.Size = new System.Drawing.Size(674, 25);
            this.toolStripForm.TabIndex = 5;
            // 
            // toolStripButtonNewFolder
            // 
            this.toolStripButtonNewFolder.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonNewFolder.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonNewFolder.Name = "toolStripButtonNewFolder";
            this.toolStripButtonNewFolder.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonNewFolder.ToolTipText = "New folder";
            this.toolStripButtonNewFolder.Click += new System.EventHandler(this.toolStripButtonNewFolder_Click);
            // 
            // toolStripSeparatorToolbar1
            // 
            this.toolStripSeparatorToolbar1.Name = "toolStripSeparatorToolbar1";
            this.toolStripSeparatorToolbar1.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonDeleteFolder
            // 
            this.toolStripButtonDeleteFolder.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonDeleteFolder.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonDeleteFolder.Name = "toolStripButtonDeleteFolder";
            this.toolStripButtonDeleteFolder.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonDeleteFolder.ToolTipText = "Delete (Del)";
            this.toolStripButtonDeleteFolder.Click += new System.EventHandler(this.toolStripButtonDeleteFolder_Click);
            // 
            // toolStripButtonDeleteFile
            // 
            this.toolStripButtonDeleteFile.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonDeleteFile.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonDeleteFile.Name = "toolStripButtonDeleteFile";
            this.toolStripButtonDeleteFile.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonDeleteFile.ToolTipText = "Delete (Del)";
            this.toolStripButtonDeleteFile.Click += new System.EventHandler(this.toolStripButtonDeleteFile_Click);
            // 
            // toolStripButtonRename
            // 
            this.toolStripButtonRename.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonRename.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonRename.Name = "toolStripButtonRename";
            this.toolStripButtonRename.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonRename.ToolTipText = "Rename (F2)";
            this.toolStripButtonRename.Click += new System.EventHandler(this.toolStripButtonRename_Click);
            // 
            // toolStripSeparatorToolbar2
            // 
            this.toolStripSeparatorToolbar2.Name = "toolStripSeparatorToolbar2";
            this.toolStripSeparatorToolbar2.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonProperties
            // 
            this.toolStripButtonProperties.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonProperties.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonProperties.Name = "toolStripButtonProperties";
            this.toolStripButtonProperties.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonProperties.ToolTipText = "Properties";
            this.toolStripButtonProperties.Click += new System.EventHandler(this.toolStripButtonProperties_Click);
            // 
            // contextMenuStripZipFile
            // 
            this.contextMenuStripZipFile.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemZipFileNewFolder});
            this.contextMenuStripZipFile.Name = "contextMenuStripZipFile";
            this.contextMenuStripZipFile.Size = new System.Drawing.Size(133, 26);
            // 
            // toolStripMenuItemZipFileNewFolder
            // 
            this.toolStripMenuItemZipFileNewFolder.Name = "toolStripMenuItemZipFileNewFolder";
            this.toolStripMenuItemZipFileNewFolder.Size = new System.Drawing.Size(132, 22);
            this.toolStripMenuItemZipFileNewFolder.Text = "New folder";
            this.toolStripMenuItemZipFileNewFolder.Click += new System.EventHandler(this.toolStripMenuItemZipFileNewFolder_Click);
            // 
            // OpenZipForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(674, 312);
            this.Controls.Add(this.toolStripForm);
            this.Controls.Add(this.splitContainerFolderFile);
            this.Controls.Add(this.textBoxFileName);
            this.Controls.Add(this.labelFileName);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonCancel);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(690, 350);
            this.Name = "OpenZipForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Open Zip File";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.OpenZipForm_FormClosed);
            this.Load += new System.EventHandler(this.OpenZipForm_Load);
            this.splitContainerFolderFile.Panel1.ResumeLayout(false);
            this.splitContainerFolderFile.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerFolderFile)).EndInit();
            this.splitContainerFolderFile.ResumeLayout(false);
            this.contextMenuStripFolder.ResumeLayout(false);
            this.contextMenuStripFile.ResumeLayout(false);
            this.toolStripForm.ResumeLayout(false);
            this.toolStripForm.PerformLayout();
            this.contextMenuStripZipFile.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Label labelFileName;
            private Common.Forms.TextBox textBoxFileName;
            private System.Windows.Forms.SplitContainer splitContainerFolderFile;
            private Common.Forms.TreeView treeViewFolder;
            private Common.Forms.ListView listViewFolderFile;
            private System.Windows.Forms.ColumnHeader columnHeaderName;
            private System.Windows.Forms.ColumnHeader columnHeaderSize;
            private System.Windows.Forms.ColumnHeader columnHeaderCRC;
            private System.Windows.Forms.ColumnHeader columnHeaderAttributes;
            private System.Windows.Forms.ColumnHeader columnHeaderModified;
            private System.Windows.Forms.ContextMenuStrip contextMenuStripFile;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFileDelete;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFileRename;
            private System.Windows.Forms.ContextMenuStrip contextMenuStripFolder;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFolderNewFolder;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorFolder1;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFolderDelete;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFolderRename;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFileProperties;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorFile1;
            private Common.Forms.ToolStrip toolStripForm;
            private System.Windows.Forms.ToolStripButton toolStripButtonNewFolder;
            private System.Windows.Forms.ToolStripButton toolStripButtonRename;
            private System.Windows.Forms.ToolStripButton toolStripButtonDeleteFolder;
            private System.Windows.Forms.ToolStripButton toolStripButtonProperties;
            private System.Windows.Forms.ContextMenuStrip contextMenuStripZipFile;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemZipFileNewFolder;
            private System.Windows.Forms.ToolStripButton toolStripButtonDeleteFile;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorToolbar1;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorToolbar2;
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2020 Kevin Eshbach                                  */
/***************************************************************************/
