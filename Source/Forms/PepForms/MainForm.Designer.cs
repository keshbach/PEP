namespace Pep
{
    namespace Forms
    {
        partial class MainForm
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

                if (m_CancelBtnMutex != null)
                {
                    m_CancelBtnMutex.Close();

                    m_CancelBtnMutex = null;
                }

                if (m_MessageCacheMutex != null)
                {
                    m_MessageCacheMutex.Close();

                    m_MessageCacheMutex = null;
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.statusStrip = new Common.Forms.StatusStrip();
            this.toolStripConnectedStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripSeparatorStatusBar1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripZipStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripSeparatorStatusBar2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripProgressBar = new Common.Forms.ToolStripProgressBar();
            this.toolStripSeparatorStatusBar3 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripTimeStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripSeparatorStatusBar4 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripCancelButton = new System.Windows.Forms.ToolStripDropDownButton();
            this.menuAppStrip = new Common.Forms.MenuStrip();
            this.menuItemFile = new Common.Forms.ToolStripMenuItem();
            this.menuItemFileNewZip = new Common.Forms.ToolStripMenuItem();
            this.menuItemFileOpenZip = new Common.Forms.ToolStripMenuItem();
            this.menuItemFileCloseZip = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorFile1 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemFileOpen = new Common.Forms.ToolStripMenuItem();
            this.menuItemFileSaveAs = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorFile2 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemFileOpenIntelHex = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorFile3 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemFileSaveLogAs = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorFile4 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemFileExit = new Common.Forms.ToolStripMenuItem();
            this.menuItemEdit = new Common.Forms.ToolStripMenuItem();
            this.menuItemEditCopy = new Common.Forms.ToolStripMenuItem();
            this.menuItemEditDelete = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemEditSelectAll = new Common.Forms.ToolStripMenuItem();
            this.menuItemDevice = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceCancel = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorDevice1 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemDeviceRead = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceProgram = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceVerify = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceTest = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceTestBuffer = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceTestZeroes = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceTestOnes = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceTestRandom = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorDevice2 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemDeviceSelectDevice = new Common.Forms.ToolStripMenuItem();
            this.menuItemDeviceConfigureDevice = new Common.Forms.ToolStripMenuItem();
            this.menuItemBuffer = new Common.Forms.ToolStripMenuItem();
            this.menuItemBufferView = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorBuffer1 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemBufferFill = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorBuffer2 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemBufferTestEmpty = new Common.Forms.ToolStripMenuItem();
            this.menuItemBufferTestStuckBits = new Common.Forms.ToolStripMenuItem();
            this.menuItemBufferTestOverdump = new Common.Forms.ToolStripMenuItem();
            this.menuItemBufferTestAll = new Common.Forms.ToolStripMenuItem();
            this.toolStripSeparatorBuffer3 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemBufferTestZip = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools0 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools1 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools2 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools3 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools4 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools5 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools6 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools7 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools8 = new Common.Forms.ToolStripMenuItem();
            this.menuItemTools9 = new Common.Forms.ToolStripMenuItem();
            this.menuItemToolsSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemToolsConfigure = new Common.Forms.ToolStripMenuItem();
            this.menuItemToolsSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.menuItemToolsOptions = new Common.Forms.ToolStripMenuItem();
            this.menuItemHelp = new Common.Forms.ToolStripMenuItem();
            this.menuItemHelpAbout = new Common.Forms.ToolStripMenuItem();
            this.textBoxMessages = new Common.Forms.TextBoxMessages();
            this.contextMenuMessageWindowStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.timerUpdater = new System.Windows.Forms.Timer(this.components);
            this.timerElapsedTime = new System.Windows.Forms.Timer(this.components);
            this.deviceInfo = new Pep.Forms.DeviceInfo(this.components);
            this.toolStripContainerMain = new System.Windows.Forms.ToolStripContainer();
            this.toolStripFile = new System.Windows.Forms.ToolStrip();
            this.toolStripEdit = new System.Windows.Forms.ToolStrip();
            this.toolStripDevice = new System.Windows.Forms.ToolStrip();
            this.toolStripBuffer = new System.Windows.Forms.ToolStrip();
            this.toolStripTools = new System.Windows.Forms.ToolStrip();
            this.statusStrip.SuspendLayout();
            this.menuAppStrip.SuspendLayout();
            this.toolStripContainerMain.BottomToolStripPanel.SuspendLayout();
            this.toolStripContainerMain.ContentPanel.SuspendLayout();
            this.toolStripContainerMain.TopToolStripPanel.SuspendLayout();
            this.toolStripContainerMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // statusStrip
            // 
            this.statusStrip.ActiveGroup = "";
            this.statusStrip.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.statusStrip.Dock = System.Windows.Forms.DockStyle.None;
            this.statusStrip.GripStyle = System.Windows.Forms.ToolStripGripStyle.Visible;
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripConnectedStatusLabel,
            this.toolStripSeparatorStatusBar1,
            this.toolStripZipStatusLabel,
            this.toolStripSeparatorStatusBar2,
            this.toolStripProgressBar,
            this.toolStripSeparatorStatusBar3,
            this.toolStripTimeStatusLabel,
            this.toolStripSeparatorStatusBar4,
            this.toolStripCancelButton});
            this.statusStrip.Location = new System.Drawing.Point(0, 0);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(513, 23);
            this.statusStrip.TabIndex = 3;
            // 
            // toolStripConnectedStatusLabel
            // 
            this.toolStripConnectedStatusLabel.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.toolStripConnectedStatusLabel.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenInner;
            this.toolStripConnectedStatusLabel.Name = "toolStripConnectedStatusLabel";
            this.toolStripConnectedStatusLabel.Size = new System.Drawing.Size(88, 18);
            this.toolStripConnectedStatusLabel.Text = "Not Connected";
            // 
            // toolStripSeparatorStatusBar1
            // 
            this.toolStripSeparatorStatusBar1.Name = "toolStripSeparatorStatusBar1";
            this.toolStripSeparatorStatusBar1.Size = new System.Drawing.Size(6, 23);
            // 
            // toolStripZipStatusLabel
            // 
            this.toolStripZipStatusLabel.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.toolStripZipStatusLabel.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenInner;
            this.toolStripZipStatusLabel.Name = "toolStripZipStatusLabel";
            this.toolStripZipStatusLabel.Size = new System.Drawing.Size(43, 18);
            this.toolStripZipStatusLabel.Text = "not zip";
            // 
            // toolStripSeparatorStatusBar2
            // 
            this.toolStripSeparatorStatusBar2.Name = "toolStripSeparatorStatusBar2";
            this.toolStripSeparatorStatusBar2.Size = new System.Drawing.Size(6, 23);
            // 
            // toolStripProgressBar
            // 
            this.toolStripProgressBar.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.toolStripProgressBar.AutoSize = false;
            this.toolStripProgressBar.Margin = new System.Windows.Forms.Padding(1, 4, 1, 2);
            this.toolStripProgressBar.Name = "toolStripProgressBar";
            this.toolStripProgressBar.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.toolStripProgressBar.Size = new System.Drawing.Size(200, 17);
            this.toolStripProgressBar.Step = 1;
            // 
            // toolStripSeparatorStatusBar3
            // 
            this.toolStripSeparatorStatusBar3.Name = "toolStripSeparatorStatusBar3";
            this.toolStripSeparatorStatusBar3.Size = new System.Drawing.Size(6, 23);
            // 
            // toolStripTimeStatusLabel
            // 
            this.toolStripTimeStatusLabel.Name = "toolStripTimeStatusLabel";
            this.toolStripTimeStatusLabel.Size = new System.Drawing.Size(49, 18);
            this.toolStripTimeStatusLabel.Text = "00:00:00";
            // 
            // toolStripSeparatorStatusBar4
            // 
            this.toolStripSeparatorStatusBar4.Name = "toolStripSeparatorStatusBar4";
            this.toolStripSeparatorStatusBar4.Size = new System.Drawing.Size(6, 23);
            // 
            // toolStripCancelButton
            // 
            this.toolStripCancelButton.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.toolStripCancelButton.AutoToolTip = false;
            this.toolStripCancelButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.toolStripCancelButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.toolStripCancelButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripCancelButton.Name = "toolStripCancelButton";
            this.toolStripCancelButton.ShowDropDownArrow = false;
            this.toolStripCancelButton.Size = new System.Drawing.Size(47, 21);
            this.toolStripCancelButton.Text = "Cancel";
            this.toolStripCancelButton.Click += new System.EventHandler(this.toolStripCancelButton_Click);
            // 
            // menuAppStrip
            // 
            this.menuAppStrip.Dock = System.Windows.Forms.DockStyle.None;
            this.menuAppStrip.GripStyle = System.Windows.Forms.ToolStripGripStyle.Visible;
            this.menuAppStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemFile,
            this.menuItemEdit,
            this.menuItemDevice,
            this.menuItemBuffer,
            this.menuItemTools,
            this.menuItemHelp});
            this.menuAppStrip.Location = new System.Drawing.Point(0, 0);
            this.menuAppStrip.Name = "menuAppStrip";
            this.menuAppStrip.Size = new System.Drawing.Size(513, 24);
            this.menuAppStrip.TabIndex = 0;
            // 
            // menuItemFile
            // 
            this.menuItemFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemFileNewZip,
            this.menuItemFileOpenZip,
            this.menuItemFileCloseZip,
            this.toolStripSeparatorFile1,
            this.menuItemFileOpen,
            this.menuItemFileSaveAs,
            this.toolStripSeparatorFile2,
            this.menuItemFileOpenIntelHex,
            this.toolStripSeparatorFile3,
            this.menuItemFileSaveLogAs,
            this.toolStripSeparatorFile4,
            this.menuItemFileExit});
            this.menuItemFile.HelpText = "Options for the various file actions.";
            this.menuItemFile.Name = "menuItemFile";
            this.menuItemFile.Size = new System.Drawing.Size(37, 20);
            this.menuItemFile.Text = "&File";
            // 
            // menuItemFileNewZip
            // 
            this.menuItemFileNewZip.HelpText = "Create a new zip file to contain the contents  of multiple buffers.";
            this.menuItemFileNewZip.Name = "menuItemFileNewZip";
            this.menuItemFileNewZip.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileNewZip.Text = "New Zip...";
            this.menuItemFileNewZip.Click += new System.EventHandler(this.FileNewZipMenuItem_Click);
            // 
            // menuItemFileOpenZip
            // 
            this.menuItemFileOpenZip.HelpText = "Open an existing zip file.";
            this.menuItemFileOpenZip.Name = "menuItemFileOpenZip";
            this.menuItemFileOpenZip.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileOpenZip.Text = "Open Zip...";
            this.menuItemFileOpenZip.Click += new System.EventHandler(this.FileOpenZipMenuItem_Click);
            // 
            // menuItemFileCloseZip
            // 
            this.menuItemFileCloseZip.HelpText = "Close the active zip file.";
            this.menuItemFileCloseZip.Name = "menuItemFileCloseZip";
            this.menuItemFileCloseZip.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileCloseZip.Text = "Close Zip";
            this.menuItemFileCloseZip.Click += new System.EventHandler(this.FileCloseZipMenuItem_Click);
            // 
            // toolStripSeparatorFile1
            // 
            this.toolStripSeparatorFile1.Name = "toolStripSeparatorFile1";
            this.toolStripSeparatorFile1.Size = new System.Drawing.Size(160, 6);
            // 
            // menuItemFileOpen
            // 
            this.menuItemFileOpen.HelpText = null;
            this.menuItemFileOpen.Name = "menuItemFileOpen";
            this.menuItemFileOpen.ShortcutKeyDisplayString = "Ctrl+O";
            this.menuItemFileOpen.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.menuItemFileOpen.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileOpen.Text = "&Open...";
            this.menuItemFileOpen.Click += new System.EventHandler(this.FileOpenMenuItem_Click);
            // 
            // menuItemFileSaveAs
            // 
            this.menuItemFileSaveAs.HelpText = null;
            this.menuItemFileSaveAs.Name = "menuItemFileSaveAs";
            this.menuItemFileSaveAs.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.menuItemFileSaveAs.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileSaveAs.Text = "&Save As...";
            this.menuItemFileSaveAs.Click += new System.EventHandler(this.FileSaveAsMenuItem_Click);
            // 
            // toolStripSeparatorFile2
            // 
            this.toolStripSeparatorFile2.Name = "toolStripSeparatorFile2";
            this.toolStripSeparatorFile2.Size = new System.Drawing.Size(160, 6);
            // 
            // menuItemFileOpenIntelHex
            // 
            this.menuItemFileOpenIntelHex.HelpText = "Open an existing Intel Hex file.";
            this.menuItemFileOpenIntelHex.Name = "menuItemFileOpenIntelHex";
            this.menuItemFileOpenIntelHex.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileOpenIntelHex.Text = "Open Intel Hex...";
            this.menuItemFileOpenIntelHex.Click += new System.EventHandler(this.FileOpenIntelHexMenuItem_Click);
            // 
            // toolStripSeparatorFile3
            // 
            this.toolStripSeparatorFile3.Name = "toolStripSeparatorFile3";
            this.toolStripSeparatorFile3.Size = new System.Drawing.Size(160, 6);
            // 
            // menuItemFileSaveLogAs
            // 
            this.menuItemFileSaveLogAs.HelpText = null;
            this.menuItemFileSaveLogAs.Name = "menuItemFileSaveLogAs";
            this.menuItemFileSaveLogAs.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileSaveLogAs.Text = "Save Log As...";
            this.menuItemFileSaveLogAs.Click += new System.EventHandler(this.FileSaveLogAsMenuItem_Click);
            // 
            // toolStripSeparatorFile4
            // 
            this.toolStripSeparatorFile4.Name = "toolStripSeparatorFile4";
            this.toolStripSeparatorFile4.Size = new System.Drawing.Size(160, 6);
            // 
            // menuItemFileExit
            // 
            this.menuItemFileExit.HelpText = "Quit the application.";
            this.menuItemFileExit.Name = "menuItemFileExit";
            this.menuItemFileExit.Size = new System.Drawing.Size(163, 22);
            this.menuItemFileExit.Text = "E&xit";
            this.menuItemFileExit.Click += new System.EventHandler(this.FileExitMenuItem_Click);
            // 
            // menuItemEdit
            // 
            this.menuItemEdit.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemEditCopy,
            this.menuItemEditDelete,
            this.toolStripSeparator1,
            this.menuItemEditSelectAll});
            this.menuItemEdit.HelpText = "Options for messages.";
            this.menuItemEdit.Name = "menuItemEdit";
            this.menuItemEdit.Size = new System.Drawing.Size(39, 20);
            this.menuItemEdit.Text = "&Edit";
            // 
            // menuItemEditCopy
            // 
            this.menuItemEditCopy.HelpText = "Copy the selected message(s) into the clipboard.";
            this.menuItemEditCopy.Name = "menuItemEditCopy";
            this.menuItemEditCopy.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.C)));
            this.menuItemEditCopy.Size = new System.Drawing.Size(164, 22);
            this.menuItemEditCopy.Text = "&Copy";
            this.menuItemEditCopy.Click += new System.EventHandler(this.EditCopyMenuItem_Click);
            // 
            // menuItemEditDelete
            // 
            this.menuItemEditDelete.HelpText = "Remove all messages.";
            this.menuItemEditDelete.Name = "menuItemEditDelete";
            this.menuItemEditDelete.ShortcutKeys = System.Windows.Forms.Keys.Delete;
            this.menuItemEditDelete.Size = new System.Drawing.Size(164, 22);
            this.menuItemEditDelete.Text = "&Delete";
            this.menuItemEditDelete.Click += new System.EventHandler(this.EditDeleteMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(161, 6);
            // 
            // menuItemEditSelectAll
            // 
            this.menuItemEditSelectAll.HelpText = "Select all messages.";
            this.menuItemEditSelectAll.Name = "menuItemEditSelectAll";
            this.menuItemEditSelectAll.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.A)));
            this.menuItemEditSelectAll.Size = new System.Drawing.Size(164, 22);
            this.menuItemEditSelectAll.Text = "Select &All";
            this.menuItemEditSelectAll.Click += new System.EventHandler(this.EditSelectAllMenuItem_Click);
            // 
            // menuItemDevice
            // 
            this.menuItemDevice.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemDeviceCancel,
            this.toolStripSeparatorDevice1,
            this.menuItemDeviceRead,
            this.menuItemDeviceProgram,
            this.menuItemDeviceVerify,
            this.menuItemDeviceTest,
            this.toolStripSeparatorDevice2,
            this.menuItemDeviceSelectDevice,
            this.menuItemDeviceConfigureDevice});
            this.menuItemDevice.HelpText = "Options to select a device or execute a device action.";
            this.menuItemDevice.Name = "menuItemDevice";
            this.menuItemDevice.Size = new System.Drawing.Size(54, 20);
            this.menuItemDevice.Text = "&Device";
            // 
            // menuItemDeviceCancel
            // 
            this.menuItemDeviceCancel.Enabled = false;
            this.menuItemDeviceCancel.HelpText = "Stop the active device action.";
            this.menuItemDeviceCancel.Name = "menuItemDeviceCancel";
            this.menuItemDeviceCancel.ShortcutKeyDisplayString = "Escape";
            this.menuItemDeviceCancel.Size = new System.Drawing.Size(194, 22);
            this.menuItemDeviceCancel.Text = "&Cancel";
            this.menuItemDeviceCancel.Click += new System.EventHandler(this.DeviceCancelMenuItem_Click);
            // 
            // toolStripSeparatorDevice1
            // 
            this.toolStripSeparatorDevice1.Name = "toolStripSeparatorDevice1";
            this.toolStripSeparatorDevice1.Size = new System.Drawing.Size(191, 6);
            // 
            // menuItemDeviceRead
            // 
            this.menuItemDeviceRead.HelpText = "Read the contents of the active device into the buffer.";
            this.menuItemDeviceRead.Name = "menuItemDeviceRead";
            this.menuItemDeviceRead.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.R)));
            this.menuItemDeviceRead.Size = new System.Drawing.Size(194, 22);
            this.menuItemDeviceRead.Text = "&Read";
            this.menuItemDeviceRead.Click += new System.EventHandler(this.DeviceReadMenuItem_Click);
            // 
            // menuItemDeviceProgram
            // 
            this.menuItemDeviceProgram.HelpText = "Program the contents of the buffer into the active device.";
            this.menuItemDeviceProgram.Name = "menuItemDeviceProgram";
            this.menuItemDeviceProgram.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.P)));
            this.menuItemDeviceProgram.Size = new System.Drawing.Size(194, 22);
            this.menuItemDeviceProgram.Text = "&Program";
            this.menuItemDeviceProgram.Click += new System.EventHandler(this.DeviceProgramMenuItem_Click);
            // 
            // menuItemDeviceVerify
            // 
            this.menuItemDeviceVerify.HelpText = "Verify that the contents of the buffer are identical to the contents of the activ" +
    "e device.";
            this.menuItemDeviceVerify.Name = "menuItemDeviceVerify";
            this.menuItemDeviceVerify.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.V)));
            this.menuItemDeviceVerify.Size = new System.Drawing.Size(194, 22);
            this.menuItemDeviceVerify.Text = "&Verify";
            this.menuItemDeviceVerify.Click += new System.EventHandler(this.DeviceVerifyMenuItem_Click);
            // 
            // menuItemDeviceTest
            // 
            this.menuItemDeviceTest.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemDeviceTestBuffer,
            this.menuItemDeviceTestZeroes,
            this.menuItemDeviceTestOnes,
            this.menuItemDeviceTestRandom});
            this.menuItemDeviceTest.HelpText = null;
            this.menuItemDeviceTest.Name = "menuItemDeviceTest";
            this.menuItemDeviceTest.Size = new System.Drawing.Size(194, 22);
            this.menuItemDeviceTest.Text = "&Test (Read/Write)";
            // 
            // menuItemDeviceTestBuffer
            // 
            this.menuItemDeviceTestBuffer.HelpText = "";
            this.menuItemDeviceTestBuffer.Name = "menuItemDeviceTestBuffer";
            this.menuItemDeviceTestBuffer.Size = new System.Drawing.Size(119, 22);
            this.menuItemDeviceTestBuffer.Text = "&Buffer";
            this.menuItemDeviceTestBuffer.Click += new System.EventHandler(this.DeviceTestBufferMenuItem_Click);
            // 
            // menuItemDeviceTestZeroes
            // 
            this.menuItemDeviceTestZeroes.HelpText = "";
            this.menuItemDeviceTestZeroes.Name = "menuItemDeviceTestZeroes";
            this.menuItemDeviceTestZeroes.Size = new System.Drawing.Size(119, 22);
            this.menuItemDeviceTestZeroes.Text = "&Zeroes";
            this.menuItemDeviceTestZeroes.Click += new System.EventHandler(this.DeviceTestZeroesMenuItem_Click);
            // 
            // menuItemDeviceTestOnes
            // 
            this.menuItemDeviceTestOnes.HelpText = "";
            this.menuItemDeviceTestOnes.Name = "menuItemDeviceTestOnes";
            this.menuItemDeviceTestOnes.Size = new System.Drawing.Size(119, 22);
            this.menuItemDeviceTestOnes.Text = "&Ones";
            this.menuItemDeviceTestOnes.Click += new System.EventHandler(this.DeviceTestOnesMenuItem_Click);
            // 
            // menuItemDeviceTestRandom
            // 
            this.menuItemDeviceTestRandom.HelpText = "";
            this.menuItemDeviceTestRandom.Name = "menuItemDeviceTestRandom";
            this.menuItemDeviceTestRandom.Size = new System.Drawing.Size(119, 22);
            this.menuItemDeviceTestRandom.Text = "&Random";
            this.menuItemDeviceTestRandom.Click += new System.EventHandler(this.DeviceTestRandomMenuItem_Click);
            // 
            // toolStripSeparatorDevice2
            // 
            this.toolStripSeparatorDevice2.Name = "toolStripSeparatorDevice2";
            this.toolStripSeparatorDevice2.Size = new System.Drawing.Size(191, 6);
            // 
            // menuItemDeviceSelectDevice
            // 
            this.menuItemDeviceSelectDevice.HelpText = "Change the active device.";
            this.menuItemDeviceSelectDevice.Name = "menuItemDeviceSelectDevice";
            this.menuItemDeviceSelectDevice.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.D)));
            this.menuItemDeviceSelectDevice.Size = new System.Drawing.Size(194, 22);
            this.menuItemDeviceSelectDevice.Text = "&Select Device...";
            this.menuItemDeviceSelectDevice.Click += new System.EventHandler(this.DeviceSelectDeviceMenuItem_Click);
            // 
            // menuItemDeviceConfigureDevice
            // 
            this.menuItemDeviceConfigureDevice.HelpText = "Change the settings of the active device.";
            this.menuItemDeviceConfigureDevice.Name = "menuItemDeviceConfigureDevice";
            this.menuItemDeviceConfigureDevice.Size = new System.Drawing.Size(194, 22);
            this.menuItemDeviceConfigureDevice.Text = "Configure Device...";
            this.menuItemDeviceConfigureDevice.Click += new System.EventHandler(this.DeviceConfigureDeviceMenuItem_Click);
            // 
            // menuItemBuffer
            // 
            this.menuItemBuffer.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemBufferView,
            this.toolStripSeparatorBuffer1,
            this.menuItemBufferFill,
            this.toolStripSeparatorBuffer2,
            this.menuItemBufferTestEmpty,
            this.menuItemBufferTestStuckBits,
            this.menuItemBufferTestOverdump,
            this.menuItemBufferTestAll,
            this.toolStripSeparatorBuffer3,
            this.menuItemBufferTestZip});
            this.menuItemBuffer.HelpText = "Options that can be performed on the buffer.";
            this.menuItemBuffer.Name = "menuItemBuffer";
            this.menuItemBuffer.Size = new System.Drawing.Size(51, 20);
            this.menuItemBuffer.Text = "&Buffer";
            // 
            // menuItemBufferView
            // 
            this.menuItemBufferView.HelpText = "View the contents of the buffer.";
            this.menuItemBufferView.Name = "menuItemBufferView";
            this.menuItemBufferView.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.B)));
            this.menuItemBufferView.Size = new System.Drawing.Size(296, 22);
            this.menuItemBufferView.Text = "&View...";
            this.menuItemBufferView.Click += new System.EventHandler(this.BufferViewMenuItem_Click);
            // 
            // toolStripSeparatorBuffer1
            // 
            this.toolStripSeparatorBuffer1.Name = "toolStripSeparatorBuffer1";
            this.toolStripSeparatorBuffer1.Size = new System.Drawing.Size(293, 6);
            // 
            // menuItemBufferFill
            // 
            this.menuItemBufferFill.HelpText = "Fill the buffer with a specified value.";
            this.menuItemBufferFill.Name = "menuItemBufferFill";
            this.menuItemBufferFill.Size = new System.Drawing.Size(296, 22);
            this.menuItemBufferFill.Text = "&Fill...";
            this.menuItemBufferFill.Click += new System.EventHandler(this.BufferFillMenuItem_Click);
            // 
            // toolStripSeparatorBuffer2
            // 
            this.toolStripSeparatorBuffer2.Name = "toolStripSeparatorBuffer2";
            this.toolStripSeparatorBuffer2.Size = new System.Drawing.Size(293, 6);
            // 
            // menuItemBufferTestEmpty
            // 
            this.menuItemBufferTestEmpty.HelpText = "Test if the buffer is blank.";
            this.menuItemBufferTestEmpty.Name = "menuItemBufferTestEmpty";
            this.menuItemBufferTestEmpty.Size = new System.Drawing.Size(296, 22);
            this.menuItemBufferTestEmpty.Text = "Test &Empty";
            this.menuItemBufferTestEmpty.Click += new System.EventHandler(this.BufferTestEmptyMenuItem_Click);
            // 
            // menuItemBufferTestStuckBits
            // 
            this.menuItemBufferTestStuckBits.HelpText = "Test if the buffer contains any stuck address lines.";
            this.menuItemBufferTestStuckBits.Name = "menuItemBufferTestStuckBits";
            this.menuItemBufferTestStuckBits.Size = new System.Drawing.Size(296, 22);
            this.menuItemBufferTestStuckBits.Text = "Test &Stuck Bits";
            this.menuItemBufferTestStuckBits.Click += new System.EventHandler(this.BufferTestStuckBitsMenuItem_Click);
            // 
            // menuItemBufferTestOverdump
            // 
            this.menuItemBufferTestOverdump.HelpText = "Test if the buffer\'s first and second half are identical.";
            this.menuItemBufferTestOverdump.Name = "menuItemBufferTestOverdump";
            this.menuItemBufferTestOverdump.Size = new System.Drawing.Size(296, 22);
            this.menuItemBufferTestOverdump.Text = "Test &Overdump";
            this.menuItemBufferTestOverdump.Click += new System.EventHandler(this.BufferTestOverdumpMenuItem_Click);
            // 
            // menuItemBufferTestAll
            // 
            this.menuItemBufferTestAll.HelpText = "Test the buffer if empty, has stuck bits or contains an overdump.";
            this.menuItemBufferTestAll.Name = "menuItemBufferTestAll";
            this.menuItemBufferTestAll.Size = new System.Drawing.Size(296, 22);
            this.menuItemBufferTestAll.Text = "Test &All (Empty, Stuck Bits and Overdump)";
            this.menuItemBufferTestAll.Click += new System.EventHandler(this.BufferTestAllMenuItem_Click);
            // 
            // toolStripSeparatorBuffer3
            // 
            this.toolStripSeparatorBuffer3.Name = "toolStripSeparatorBuffer3";
            this.toolStripSeparatorBuffer3.Size = new System.Drawing.Size(293, 6);
            // 
            // menuItemBufferTestZip
            // 
            this.menuItemBufferTestZip.HelpText = "Run all tests (Empty, Stuck Bits and Overdump) on the contents of the active zip " +
    "file.";
            this.menuItemBufferTestZip.Name = "menuItemBufferTestZip";
            this.menuItemBufferTestZip.Size = new System.Drawing.Size(296, 22);
            this.menuItemBufferTestZip.Text = "Test Zip";
            this.menuItemBufferTestZip.Click += new System.EventHandler(this.BufferTestZipMenuItem_Click);
            // 
            // menuItemTools
            // 
            this.menuItemTools.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemTools0,
            this.menuItemTools1,
            this.menuItemTools2,
            this.menuItemTools3,
            this.menuItemTools4,
            this.menuItemTools5,
            this.menuItemTools6,
            this.menuItemTools7,
            this.menuItemTools8,
            this.menuItemTools9,
            this.menuItemToolsSeparator,
            this.menuItemToolsConfigure,
            this.menuItemToolsSeparator1,
            this.menuItemToolsOptions});
            this.menuItemTools.HelpText = "Options to configure and launch external applications.";
            this.menuItemTools.Name = "menuItemTools";
            this.menuItemTools.Size = new System.Drawing.Size(46, 20);
            this.menuItemTools.Text = "&Tools";
            // 
            // menuItemTools0
            // 
            this.menuItemTools0.HelpText = "Launch this tool.";
            this.menuItemTools0.Name = "menuItemTools0";
            this.menuItemTools0.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools0.Text = "&0";
            this.menuItemTools0.Click += new System.EventHandler(this.Tools0MenuItem_Click);
            // 
            // menuItemTools1
            // 
            this.menuItemTools1.HelpText = "Launch this tool.";
            this.menuItemTools1.Name = "menuItemTools1";
            this.menuItemTools1.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools1.Text = "&1";
            this.menuItemTools1.Click += new System.EventHandler(this.Tools1MenuItem_Click);
            // 
            // menuItemTools2
            // 
            this.menuItemTools2.HelpText = "Launch this tool.";
            this.menuItemTools2.Name = "menuItemTools2";
            this.menuItemTools2.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools2.Text = "&2";
            this.menuItemTools2.Click += new System.EventHandler(this.Tools2MenuItem_Click);
            // 
            // menuItemTools3
            // 
            this.menuItemTools3.HelpText = "Launch this tool.";
            this.menuItemTools3.Name = "menuItemTools3";
            this.menuItemTools3.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools3.Text = "&3";
            this.menuItemTools3.Click += new System.EventHandler(this.Tools3MenuItem_Click);
            // 
            // menuItemTools4
            // 
            this.menuItemTools4.HelpText = "Launch this tool.";
            this.menuItemTools4.Name = "menuItemTools4";
            this.menuItemTools4.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools4.Text = "&4";
            this.menuItemTools4.Click += new System.EventHandler(this.Tools4MenuItem_Click);
            // 
            // menuItemTools5
            // 
            this.menuItemTools5.HelpText = "Launch this tool.";
            this.menuItemTools5.Name = "menuItemTools5";
            this.menuItemTools5.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools5.Text = "&5";
            this.menuItemTools5.Click += new System.EventHandler(this.Tools5MenuItem_Click);
            // 
            // menuItemTools6
            // 
            this.menuItemTools6.HelpText = "Launch this tool.";
            this.menuItemTools6.Name = "menuItemTools6";
            this.menuItemTools6.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools6.Text = "&6";
            this.menuItemTools6.Click += new System.EventHandler(this.Tools6MenuItem_Click);
            // 
            // menuItemTools7
            // 
            this.menuItemTools7.HelpText = "Launch this tool.";
            this.menuItemTools7.Name = "menuItemTools7";
            this.menuItemTools7.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools7.Text = "&7";
            this.menuItemTools7.Click += new System.EventHandler(this.Tools7MenuItem_Click);
            // 
            // menuItemTools8
            // 
            this.menuItemTools8.HelpText = "Launch this tool.";
            this.menuItemTools8.Name = "menuItemTools8";
            this.menuItemTools8.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools8.Text = "&8";
            this.menuItemTools8.Click += new System.EventHandler(this.Tools8MenuItem_Click);
            // 
            // menuItemTools9
            // 
            this.menuItemTools9.HelpText = "Launch this tool.";
            this.menuItemTools9.Name = "menuItemTools9";
            this.menuItemTools9.Size = new System.Drawing.Size(136, 22);
            this.menuItemTools9.Text = "&9";
            this.menuItemTools9.Click += new System.EventHandler(this.Tools9MenuItem_Click);
            // 
            // menuItemToolsSeparator
            // 
            this.menuItemToolsSeparator.Name = "menuItemToolsSeparator";
            this.menuItemToolsSeparator.Size = new System.Drawing.Size(133, 6);
            // 
            // menuItemToolsConfigure
            // 
            this.menuItemToolsConfigure.HelpText = "Configure the various tools.";
            this.menuItemToolsConfigure.Name = "menuItemToolsConfigure";
            this.menuItemToolsConfigure.Size = new System.Drawing.Size(136, 22);
            this.menuItemToolsConfigure.Text = "&Configure...";
            this.menuItemToolsConfigure.Click += new System.EventHandler(this.ToolsConfigureMenuItem_Click);
            // 
            // menuItemToolsSeparator1
            // 
            this.menuItemToolsSeparator1.Name = "menuItemToolsSeparator1";
            this.menuItemToolsSeparator1.Size = new System.Drawing.Size(133, 6);
            // 
            // menuItemToolsOptions
            // 
            this.menuItemToolsOptions.HelpText = "Configure the various settings of the application.";
            this.menuItemToolsOptions.Name = "menuItemToolsOptions";
            this.menuItemToolsOptions.Size = new System.Drawing.Size(136, 22);
            this.menuItemToolsOptions.Text = "&Options...";
            this.menuItemToolsOptions.Click += new System.EventHandler(this.ToolsOptionsMenuItem_Click);
            // 
            // menuItemHelp
            // 
            this.menuItemHelp.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemHelpAbout});
            this.menuItemHelp.HelpText = "Options to provide assistance on the application.";
            this.menuItemHelp.Name = "menuItemHelp";
            this.menuItemHelp.Size = new System.Drawing.Size(44, 20);
            this.menuItemHelp.Text = "&Help";
            // 
            // menuItemHelpAbout
            // 
            this.menuItemHelpAbout.HelpText = "Display general information on the application.";
            this.menuItemHelpAbout.Name = "menuItemHelpAbout";
            this.menuItemHelpAbout.Size = new System.Drawing.Size(116, 22);
            this.menuItemHelpAbout.Text = "&About...";
            this.menuItemHelpAbout.Click += new System.EventHandler(this.HelpAboutMenuItem_Click);
            // 
            // textBoxMessages
            // 
            this.textBoxMessages.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxMessages.ContextMenuStrip = this.contextMenuMessageWindowStrip;
            this.textBoxMessages.HideSelection = false;
            this.textBoxMessages.Location = new System.Drawing.Point(8, 91);
            this.textBoxMessages.MaxLength = 200000000;
            this.textBoxMessages.Multiline = true;
            this.textBoxMessages.Name = "textBoxMessages";
            this.textBoxMessages.ReadOnly = true;
            this.textBoxMessages.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxMessages.Size = new System.Drawing.Size(497, 0);
            this.textBoxMessages.TabIndex = 2;
            this.textBoxMessages.WordWrap = false;
            this.textBoxMessages.TextSelected += new Common.Forms.TextBoxMessages.TextSelectedHandler(this.textBoxMessages_TextSelected);
            this.textBoxMessages.TextChanged += new System.EventHandler(this.textBoxMessages_TextChanged);
            // 
            // contextMenuMessageWindowStrip
            // 
            this.contextMenuMessageWindowStrip.Name = "contextMenuMessageWindowStrip";
            this.contextMenuMessageWindowStrip.Size = new System.Drawing.Size(181, 26);
            // 
            // timerUpdater
            // 
            this.timerUpdater.Interval = 50;
            this.timerUpdater.Tick += new System.EventHandler(this.timerUpdater_Tick);
            // 
            // timerElapsedTime
            // 
            this.timerElapsedTime.Interval = 1000;
            this.timerElapsedTime.Tick += new System.EventHandler(this.timerElapsedTime_Tick);
            // 
            // deviceInfo
            // 
            this.deviceInfo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.deviceInfo.DeviceAdapter = "----";
            this.deviceInfo.DeviceBits = "----";
            this.deviceInfo.DeviceChipEnable = "----";
            this.deviceInfo.DeviceDipSwitches = new bool[] {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false};
            this.deviceInfo.DeviceName = "----";
            this.deviceInfo.DeviceOutputEnable = "----";
            this.deviceInfo.DeviceSize = "----";
            this.deviceInfo.DeviceVpp = "----";
            this.deviceInfo.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.deviceInfo.Location = new System.Drawing.Point(8, 8);
            this.deviceInfo.Name = "deviceInfo";
            this.deviceInfo.Size = new System.Drawing.Size(497, 77);
            this.deviceInfo.TabIndex = 1;
            // 
            // toolStripContainerMain
            // 
            this.toolStripContainerMain.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            // 
            // toolStripContainerMain.BottomToolStripPanel
            // 
            this.toolStripContainerMain.BottomToolStripPanel.Controls.Add(this.statusStrip);
            // 
            // toolStripContainerMain.ContentPanel
            // 
            this.toolStripContainerMain.ContentPanel.Controls.Add(this.deviceInfo);
            this.toolStripContainerMain.ContentPanel.Controls.Add(this.textBoxMessages);
            this.toolStripContainerMain.ContentPanel.Size = new System.Drawing.Size(513, 98);
            this.toolStripContainerMain.Location = new System.Drawing.Point(0, 0);
            this.toolStripContainerMain.Name = "toolStripContainerMain";
            this.toolStripContainerMain.Size = new System.Drawing.Size(513, 245);
            this.toolStripContainerMain.TabIndex = 4;
            // 
            // toolStripContainerMain.TopToolStripPanel
            // 
            this.toolStripContainerMain.TopToolStripPanel.Controls.Add(this.menuAppStrip);
            this.toolStripContainerMain.TopToolStripPanel.Controls.Add(this.toolStripFile);
            this.toolStripContainerMain.TopToolStripPanel.Controls.Add(this.toolStripEdit);
            this.toolStripContainerMain.TopToolStripPanel.Controls.Add(this.toolStripDevice);
            this.toolStripContainerMain.TopToolStripPanel.Controls.Add(this.toolStripBuffer);
            this.toolStripContainerMain.TopToolStripPanel.Controls.Add(this.toolStripTools);
            // 
            // toolStripFile
            // 
            this.toolStripFile.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStripFile.Location = new System.Drawing.Point(3, 24);
            this.toolStripFile.Name = "toolStripFile";
            this.toolStripFile.Size = new System.Drawing.Size(111, 25);
            this.toolStripFile.TabIndex = 0;
            // 
            // toolStripEdit
            // 
            this.toolStripEdit.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStripEdit.Location = new System.Drawing.Point(114, 24);
            this.toolStripEdit.Name = "toolStripEdit";
            this.toolStripEdit.Size = new System.Drawing.Size(111, 25);
            this.toolStripEdit.TabIndex = 1;
            // 
            // toolStripDevice
            // 
            this.toolStripDevice.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStripDevice.Location = new System.Drawing.Point(39, 49);
            this.toolStripDevice.Name = "toolStripDevice";
            this.toolStripDevice.Size = new System.Drawing.Size(111, 25);
            this.toolStripDevice.TabIndex = 2;
            // 
            // toolStripBuffer
            // 
            this.toolStripBuffer.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStripBuffer.Location = new System.Drawing.Point(39, 74);
            this.toolStripBuffer.Name = "toolStripBuffer";
            this.toolStripBuffer.Size = new System.Drawing.Size(111, 25);
            this.toolStripBuffer.TabIndex = 3;
            // 
            // toolStripTools
            // 
            this.toolStripTools.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStripTools.Location = new System.Drawing.Point(39, 99);
            this.toolStripTools.Name = "toolStripTools";
            this.toolStripTools.Size = new System.Drawing.Size(111, 25);
            this.toolStripTools.TabIndex = 4;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(512, 246);
            this.Controls.Add(this.toolStripContainerMain);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuAppStrip;
            this.MinimumSize = new System.Drawing.Size(520, 280);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Device Programmer";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.Shown += new System.EventHandler(this.PepMainForm_Shown);
            this.SystemColorsChanged += new System.EventHandler(this.MainForm_SystemColorsChanged);
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.menuAppStrip.ResumeLayout(false);
            this.menuAppStrip.PerformLayout();
            this.toolStripContainerMain.BottomToolStripPanel.ResumeLayout(false);
            this.toolStripContainerMain.BottomToolStripPanel.PerformLayout();
            this.toolStripContainerMain.ContentPanel.ResumeLayout(false);
            this.toolStripContainerMain.ContentPanel.PerformLayout();
            this.toolStripContainerMain.TopToolStripPanel.ResumeLayout(false);
            this.toolStripContainerMain.TopToolStripPanel.PerformLayout();
            this.toolStripContainerMain.ResumeLayout(false);
            this.toolStripContainerMain.PerformLayout();
            this.ResumeLayout(false);

            }

            #endregion

            private Common.Forms.StatusStrip statusStrip;
            private Common.Forms.MenuStrip menuAppStrip;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorFile4;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorDevice1;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorDevice2;
            private System.Windows.Forms.ToolStripStatusLabel toolStripConnectedStatusLabel;
            private Common.Forms.ToolStripMenuItem menuItemFile;
            private Common.Forms.ToolStripMenuItem menuItemFileOpen;
            private Common.Forms.ToolStripMenuItem menuItemFileSaveAs;
            private Common.Forms.ToolStripMenuItem menuItemFileExit;
            private Common.Forms.ToolStripMenuItem menuItemDevice;
            private Common.Forms.ToolStripMenuItem menuItemDeviceCancel;
            private Common.Forms.ToolStripMenuItem menuItemDeviceRead;
            private Common.Forms.ToolStripMenuItem menuItemDeviceTest;
            private Common.Forms.ToolStripMenuItem menuItemDeviceSelectDevice;
            private Common.Forms.ToolStripMenuItem menuItemHelp;
            private Common.Forms.ToolStripMenuItem menuItemHelpAbout;
            private Common.Forms.TextBoxMessages textBoxMessages;
            private System.Windows.Forms.ToolStripDropDownButton toolStripCancelButton;
            private Common.Forms.ToolStripMenuItem menuItemDeviceVerify;
            private Common.Forms.ToolStripMenuItem menuItemFileNewZip;
            private Common.Forms.ToolStripMenuItem menuItemFileOpenZip;
            private Common.Forms.ToolStripMenuItem menuItemFileCloseZip;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorFile1;
            private System.Windows.Forms.ToolStripStatusLabel toolStripZipStatusLabel;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorStatusBar1;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorStatusBar2;
            private Common.Forms.ToolStripMenuItem menuItemDeviceProgram;
            private Common.Forms.ToolStripMenuItem menuItemDeviceTestBuffer;
            private Common.Forms.ToolStripMenuItem menuItemDeviceTestZeroes;
            private Common.Forms.ToolStripMenuItem menuItemDeviceTestOnes;
            private Common.Forms.ToolStripMenuItem menuItemDeviceTestRandom;
            private System.Windows.Forms.ContextMenuStrip contextMenuMessageWindowStrip;
            private Common.Forms.ToolStripMenuItem menuItemDeviceConfigureDevice;
            private Common.Forms.ToolStripMenuItem menuItemTools;
            private Common.Forms.ToolStripMenuItem menuItemToolsConfigure;
            private Common.Forms.ToolStripMenuItem menuItemTools0;
            private Common.Forms.ToolStripMenuItem menuItemTools1;
            private Common.Forms.ToolStripMenuItem menuItemTools2;
            private Common.Forms.ToolStripMenuItem menuItemTools3;
            private Common.Forms.ToolStripMenuItem menuItemTools4;
            private Common.Forms.ToolStripMenuItem menuItemTools5;
            private Common.Forms.ToolStripMenuItem menuItemTools6;
            private Common.Forms.ToolStripMenuItem menuItemTools7;
            private Common.Forms.ToolStripMenuItem menuItemTools8;
            private Common.Forms.ToolStripMenuItem menuItemTools9;
            private System.Windows.Forms.ToolStripSeparator menuItemToolsSeparator;
            private Common.Forms.ToolStripMenuItem menuItemEdit;
            private Common.Forms.ToolStripMenuItem menuItemEditCopy;
            private Common.Forms.ToolStripMenuItem menuItemEditDelete;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
            private Common.Forms.ToolStripMenuItem menuItemEditSelectAll;
            private Common.Forms.ToolStripMenuItem menuItemFileSaveLogAs;
            private Common.Forms.ToolStripProgressBar toolStripProgressBar;
            private System.Windows.Forms.Timer timerUpdater;
            private System.Windows.Forms.ToolStripSeparator menuItemToolsSeparator1;
            private Common.Forms.ToolStripMenuItem menuItemToolsOptions;
            private Common.Forms.ToolStripMenuItem menuItemBuffer;
            private Common.Forms.ToolStripMenuItem menuItemBufferView;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorBuffer1;
            private Common.Forms.ToolStripMenuItem menuItemBufferFill;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorBuffer2;
            private Common.Forms.ToolStripMenuItem menuItemBufferTestEmpty;
            private Common.Forms.ToolStripMenuItem menuItemBufferTestStuckBits;
            private Common.Forms.ToolStripMenuItem menuItemBufferTestOverdump;
            private System.Windows.Forms.ToolStripStatusLabel toolStripTimeStatusLabel;
            private System.Windows.Forms.Timer timerElapsedTime;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorStatusBar3;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorStatusBar4;
            private Common.Forms.ToolStripMenuItem menuItemBufferTestAll;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorBuffer3;
            private Common.Forms.ToolStripMenuItem menuItemBufferTestZip;
            private DeviceInfo deviceInfo;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorFile3;
            private System.Windows.Forms.ToolStripSeparator toolStripSeparatorFile2;
            private Common.Forms.ToolStripMenuItem menuItemFileOpenIntelHex;
            private System.Windows.Forms.ToolStripContainer toolStripContainerMain;
            private System.Windows.Forms.ToolStrip toolStripFile;
            private System.Windows.Forms.ToolStrip toolStripEdit;
            private System.Windows.Forms.ToolStrip toolStripDevice;
            private System.Windows.Forms.ToolStrip toolStripBuffer;
            private System.Windows.Forms.ToolStrip toolStripTools;
        }
    }
}
