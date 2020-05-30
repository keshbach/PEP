namespace Pep
{
    namespace Forms
    {
        partial class SelectDeviceForm
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
            this.splitContainerDevice = new System.Windows.Forms.SplitContainer();
            this.treeViewDevice = new System.Windows.Forms.TreeView();
            this.labelDevice = new System.Windows.Forms.Label();
            this.textBoxPinDiagram = new System.Windows.Forms.TextBox();
            this.labelPinDiagram = new System.Windows.Forms.Label();
            this.labelFilter = new System.Windows.Forms.Label();
            this.checkedListBoxFilter = new Pep.Forms.CheckedListBox(this.components);
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.labelSearch = new System.Windows.Forms.Label();
            this.textBoxSearch = new Common.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerDevice)).BeginInit();
            this.splitContainerDevice.Panel1.SuspendLayout();
            this.splitContainerDevice.Panel2.SuspendLayout();
            this.splitContainerDevice.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainerDevice
            // 
            this.splitContainerDevice.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainerDevice.Location = new System.Drawing.Point(8, 8);
            this.splitContainerDevice.Margin = new System.Windows.Forms.Padding(0);
            this.splitContainerDevice.Name = "splitContainerDevice";
            // 
            // splitContainerDevice.Panel1
            // 
            this.splitContainerDevice.Panel1.Controls.Add(this.treeViewDevice);
            this.splitContainerDevice.Panel1.Controls.Add(this.labelDevice);
            // 
            // splitContainerDevice.Panel2
            // 
            this.splitContainerDevice.Panel2.Controls.Add(this.textBoxPinDiagram);
            this.splitContainerDevice.Panel2.Controls.Add(this.labelPinDiagram);
            this.splitContainerDevice.Size = new System.Drawing.Size(402, 305);
            this.splitContainerDevice.SplitterDistance = 212;
            this.splitContainerDevice.TabIndex = 0;
            // 
            // treeViewDevice
            // 
            this.treeViewDevice.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.treeViewDevice.HideSelection = false;
            this.treeViewDevice.Location = new System.Drawing.Point(0, 16);
            this.treeViewDevice.Name = "treeViewDevice";
            this.treeViewDevice.Size = new System.Drawing.Size(209, 289);
            this.treeViewDevice.TabIndex = 1;
            this.treeViewDevice.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewDevice_AfterSelect);
            this.treeViewDevice.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.treeViewDevice_MouseDoubleClick);
            // 
            // labelDevice
            // 
            this.labelDevice.AutoSize = true;
            this.labelDevice.Location = new System.Drawing.Point(-3, 1);
            this.labelDevice.Name = "labelDevice";
            this.labelDevice.Size = new System.Drawing.Size(44, 13);
            this.labelDevice.TabIndex = 0;
            this.labelDevice.Text = "&Device:";
            // 
            // textBoxPinDiagram
            // 
            this.textBoxPinDiagram.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxPinDiagram.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxPinDiagram.Location = new System.Drawing.Point(-1, 16);
            this.textBoxPinDiagram.Multiline = true;
            this.textBoxPinDiagram.Name = "textBoxPinDiagram";
            this.textBoxPinDiagram.ReadOnly = true;
            this.textBoxPinDiagram.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxPinDiagram.Size = new System.Drawing.Size(187, 289);
            this.textBoxPinDiagram.TabIndex = 1;
            this.textBoxPinDiagram.WordWrap = false;
            // 
            // labelPinDiagram
            // 
            this.labelPinDiagram.AutoSize = true;
            this.labelPinDiagram.Location = new System.Drawing.Point(0, 0);
            this.labelPinDiagram.Name = "labelPinDiagram";
            this.labelPinDiagram.Size = new System.Drawing.Size(67, 13);
            this.labelPinDiagram.TabIndex = 0;
            this.labelPinDiagram.Text = "&Pin Diagram:";
            // 
            // labelFilter
            // 
            this.labelFilter.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelFilter.AutoSize = true;
            this.labelFilter.Location = new System.Drawing.Point(417, 8);
            this.labelFilter.Name = "labelFilter";
            this.labelFilter.Size = new System.Drawing.Size(32, 13);
            this.labelFilter.TabIndex = 0;
            this.labelFilter.Text = "&Filter:";
            // 
            // checkedListBoxFilter
            // 
            this.checkedListBoxFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.checkedListBoxFilter.Location = new System.Drawing.Point(420, 24);
            this.checkedListBoxFilter.Name = "checkedListBoxFilter";
            this.checkedListBoxFilter.Size = new System.Drawing.Size(132, 289);
            this.checkedListBoxFilter.TabIndex = 1;
            this.checkedListBoxFilter.CheckStateChange += new System.EventHandler<Pep.Forms.CheckStateChangedEventArgs>(this.checkedListBoxFilter_CheckStateChange);
            // 
            // buttonOK
            // 
            this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(396, 356);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 4;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(477, 356);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 5;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // labelSearch
            // 
            this.labelSearch.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelSearch.AutoSize = true;
            this.labelSearch.Location = new System.Drawing.Point(8, 329);
            this.labelSearch.Name = "labelSearch";
            this.labelSearch.Size = new System.Drawing.Size(44, 13);
            this.labelSearch.TabIndex = 2;
            this.labelSearch.Text = "&Search:";
            // 
            // textBoxSearch
            // 
            this.textBoxSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxSearch.KeyPressTimerDelay = 0.5D;
            this.textBoxSearch.Location = new System.Drawing.Point(59, 326);
            this.textBoxSearch.MaxLength = 10;
            this.textBoxSearch.Name = "textBoxSearch";
            this.textBoxSearch.Size = new System.Drawing.Size(493, 20);
            this.textBoxSearch.TabIndex = 3;
            this.textBoxSearch.KeyPressTimerExpired += new Common.Forms.TextBox.KeyPressTimerExpiredHandler(this.textBoxSearch_KeyPressTimerExpired);
            // 
            // SelectDeviceForm
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(562, 387);
            this.Controls.Add(this.textBoxSearch);
            this.Controls.Add(this.labelSearch);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.checkedListBoxFilter);
            this.Controls.Add(this.labelFilter);
            this.Controls.Add(this.splitContainerDevice);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(578, 426);
            this.Name = "SelectDeviceForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Select Device";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.SelectDeviceForm_FormClosed);
            this.Load += new System.EventHandler(this.SelectDeviceForm_Load);
            this.splitContainerDevice.Panel1.ResumeLayout(false);
            this.splitContainerDevice.Panel1.PerformLayout();
            this.splitContainerDevice.Panel2.ResumeLayout(false);
            this.splitContainerDevice.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerDevice)).EndInit();
            this.splitContainerDevice.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

            } 

            #endregion
            private System.Windows.Forms.SplitContainer splitContainerDevice;
            private System.Windows.Forms.Label labelDevice;
            private System.Windows.Forms.TreeView treeViewDevice;
            private System.Windows.Forms.Label labelPinDiagram;
            private System.Windows.Forms.TextBox textBoxPinDiagram;
            private System.Windows.Forms.Label labelFilter;
            private Pep.Forms.CheckedListBox checkedListBoxFilter;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.Label labelSearch;
            private Common.Forms.TextBox textBoxSearch;
        }
    }
}
