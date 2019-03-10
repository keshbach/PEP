/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
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
            this.textBoxFileName = new System.Windows.Forms.TextBox();
            this.splitContainerFolderFile = new System.Windows.Forms.SplitContainer();
            this.treeViewFolder = new Common.Forms.TreeView();
            this.contextMenuStripTreeView = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemTreeViewNewFolder = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparatorTreeView = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuItemTreeViewDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemTreeViewRename = new System.Windows.Forms.ToolStripMenuItem();
            this.listViewFolderFile = new Common.Forms.ListView();
            this.columnHeaderName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderSize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderCRC = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderAttributes = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderModified = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStripListView = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemListViewDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemListViewRename = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparatorListView = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuItemListViewProperties = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerFolderFile)).BeginInit();
            this.splitContainerFolderFile.Panel1.SuspendLayout();
            this.splitContainerFolderFile.Panel2.SuspendLayout();
            this.splitContainerFolderFile.SuspendLayout();
            this.contextMenuStripTreeView.SuspendLayout();
            this.contextMenuStripListView.SuspendLayout();
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
            this.splitContainerFolderFile.Location = new System.Drawing.Point(12, 12);
            this.splitContainerFolderFile.Name = "splitContainerFolderFile";
            // 
            // splitContainerFolderFile.Panel1
            // 
            this.splitContainerFolderFile.Panel1.Controls.Add(this.treeViewFolder);
            // 
            // splitContainerFolderFile.Panel2
            // 
            this.splitContainerFolderFile.Panel2.Controls.Add(this.listViewFolderFile);
            this.splitContainerFolderFile.Size = new System.Drawing.Size(650, 227);
            this.splitContainerFolderFile.SplitterDistance = 262;
            this.splitContainerFolderFile.TabIndex = 4;
            // 
            // treeViewFolder
            // 
            this.treeViewFolder.AllowDrop = true;
            this.treeViewFolder.ContextMenuStrip = this.contextMenuStripTreeView;
            this.treeViewFolder.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeViewFolder.HideSelection = false;
            this.treeViewFolder.LabelEdit = true;
            this.treeViewFolder.Location = new System.Drawing.Point(0, 0);
            this.treeViewFolder.Name = "treeViewFolder";
            this.treeViewFolder.Size = new System.Drawing.Size(262, 227);
            this.treeViewFolder.Sorted = true;
            this.treeViewFolder.Sorting = Common.Forms.TreeView.ESortOrder.Ascending;
            this.treeViewFolder.TabIndex = 0;
            this.treeViewFolder.KeyPressLabelEdit += new Common.Forms.TreeView.KeyPressLabelEditHandler(this.treeViewFolder_KeyPressLabelEdit);
            this.treeViewFolder.BeforeLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.treeViewFolder_BeforeLabelEdit);
            this.treeViewFolder.AfterLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.treeViewFolder_AfterLabelEdit);
            this.treeViewFolder.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.treeViewFolder_ItemDrag);
            this.treeViewFolder.BeforeSelect += new System.Windows.Forms.TreeViewCancelEventHandler(this.treeViewFolder_BeforeSelect);
            this.treeViewFolder.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewFolder_AfterSelect);
            this.treeViewFolder.DragDrop += new System.Windows.Forms.DragEventHandler(this.treeViewFolder_DragDrop);
            this.treeViewFolder.DragOver += new System.Windows.Forms.DragEventHandler(this.treeViewFolder_DragOver);
            this.treeViewFolder.DragLeave += new System.EventHandler(this.treeViewFolder_DragLeave);
            this.treeViewFolder.KeyDown += new System.Windows.Forms.KeyEventHandler(this.treeViewFolder_KeyDown);
            // 
            // contextMenuStripTreeView
            // 
            this.contextMenuStripTreeView.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemTreeViewNewFolder,
            this.toolStripSeparatorTreeView,
            this.toolStripMenuItemTreeViewDelete,
            this.toolStripMenuItemTreeViewRename});
            this.contextMenuStripTreeView.Name = "contextMenuStrip1";
            this.contextMenuStripTreeView.Size = new System.Drawing.Size(137, 76);
            // 
            // toolStripMenuItemTreeViewNewFolder
            // 
            this.toolStripMenuItemTreeViewNewFolder.Name = "toolStripMenuItemTreeViewNewFolder";
            this.toolStripMenuItemTreeViewNewFolder.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemTreeViewNewFolder.Text = "New folder";
            this.toolStripMenuItemTreeViewNewFolder.Click += new System.EventHandler(this.toolStripMenuItemTreeViewNewFolder_Click);
            // 
            // toolStripSeparatorTreeView
            // 
            this.toolStripSeparatorTreeView.Name = "toolStripSeparatorTreeView";
            this.toolStripSeparatorTreeView.Size = new System.Drawing.Size(133, 6);
            // 
            // toolStripMenuItemTreeViewDelete
            // 
            this.toolStripMenuItemTreeViewDelete.Name = "toolStripMenuItemTreeViewDelete";
            this.toolStripMenuItemTreeViewDelete.ShortcutKeys = System.Windows.Forms.Keys.Delete;
            this.toolStripMenuItemTreeViewDelete.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemTreeViewDelete.Text = "&Delete";
            this.toolStripMenuItemTreeViewDelete.Click += new System.EventHandler(this.toolStripMenuItemTreeViewDelete_Click);
            // 
            // toolStripMenuItemTreeViewRename
            // 
            this.toolStripMenuItemTreeViewRename.Name = "toolStripMenuItemTreeViewRename";
            this.toolStripMenuItemTreeViewRename.ShortcutKeys = System.Windows.Forms.Keys.F2;
            this.toolStripMenuItemTreeViewRename.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemTreeViewRename.Text = "&Rename";
            this.toolStripMenuItemTreeViewRename.Click += new System.EventHandler(this.toolStripMenuItemTreeViewRename_Click);
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
            this.listViewFolderFile.ContextMenuStrip = this.contextMenuStripListView;
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
            this.listViewFolderFile.Size = new System.Drawing.Size(384, 227);
            this.listViewFolderFile.SortArrow = Common.Forms.ListView.ESortArrow.Up;
            this.listViewFolderFile.Sorting = Common.Forms.ListView.ESortOrder.UserDefined;
            this.listViewFolderFile.TabIndex = 0;
            this.listViewFolderFile.UseCompatibleStateImageBehavior = false;
            this.listViewFolderFile.View = System.Windows.Forms.View.Details;
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
            this.listViewFolderFile.KeyDown += new System.Windows.Forms.KeyEventHandler(this.listViewFolderFile_KeyDown);
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
            // contextMenuStripListView
            // 
            this.contextMenuStripListView.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemListViewDelete,
            this.toolStripMenuItemListViewRename,
            this.toolStripSeparatorListView,
            this.toolStripMenuItemListViewProperties});
            this.contextMenuStripListView.Name = "contextMenuStripListView";
            this.contextMenuStripListView.Size = new System.Drawing.Size(137, 76);
            this.contextMenuStripListView.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStripListView_Opening);
            // 
            // toolStripMenuItemListViewDelete
            // 
            this.toolStripMenuItemListViewDelete.Name = "toolStripMenuItemListViewDelete";
            this.toolStripMenuItemListViewDelete.ShortcutKeys = System.Windows.Forms.Keys.Delete;
            this.toolStripMenuItemListViewDelete.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemListViewDelete.Text = "&Delete";
            this.toolStripMenuItemListViewDelete.Click += new System.EventHandler(this.toolStripMenuItemListViewDelete_Click);
            // 
            // toolStripMenuItemListViewRename
            // 
            this.toolStripMenuItemListViewRename.Name = "toolStripMenuItemListViewRename";
            this.toolStripMenuItemListViewRename.ShortcutKeys = System.Windows.Forms.Keys.F2;
            this.toolStripMenuItemListViewRename.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemListViewRename.Text = "&Rename";
            this.toolStripMenuItemListViewRename.Click += new System.EventHandler(this.toolStripMenuItemListViewRename_Click);
            // 
            // toolStripSeparatorListView
            // 
            this.toolStripSeparatorListView.Name = "toolStripSeparatorListView";
            this.toolStripSeparatorListView.Size = new System.Drawing.Size(133, 6);
            // 
            // toolStripMenuItemListViewProperties
            // 
            this.toolStripMenuItemListViewProperties.Name = "toolStripMenuItemListViewProperties";
            this.toolStripMenuItemListViewProperties.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemListViewProperties.Text = "&Properties";
            this.toolStripMenuItemListViewProperties.Click += new System.EventHandler(this.toolStripMenuItemListViewProperties_Click);
            // 
            // OpenZipForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(674, 312);
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
            this.Load += new System.EventHandler(this.OpenZipForm2_Load);
            this.splitContainerFolderFile.Panel1.ResumeLayout(false);
            this.splitContainerFolderFile.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerFolderFile)).EndInit();
            this.splitContainerFolderFile.ResumeLayout(false);
            this.contextMenuStripTreeView.ResumeLayout(false);
            this.contextMenuStripListView.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Label labelFileName;
            private System.Windows.Forms.TextBox textBoxFileName;
            private System.Windows.Forms.SplitContainer splitContainerFolderFile;
            private Common.Forms.TreeView treeViewFolder;
            private Common.Forms.ListView listViewFolderFile;
            private System.Windows.Forms.ColumnHeader columnHeaderName;
            private System.Windows.Forms.ColumnHeader columnHeaderSize;
            private System.Windows.Forms.ColumnHeader columnHeaderCRC;
            private System.Windows.Forms.ColumnHeader columnHeaderAttributes;
            private System.Windows.Forms.ColumnHeader columnHeaderModified;
            private System.Windows.Forms.ContextMenuStrip contextMenuStripListView;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemListViewDelete;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemListViewRename;
            private System.Windows.Forms.ContextMenuStrip contextMenuStripTreeView;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemTreeViewNewFolder;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorTreeView;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemTreeViewDelete;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemTreeViewRename;
            private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemListViewProperties;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorListView;
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/
