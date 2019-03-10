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
                this.treeViewDevice = new System.Windows.Forms.TreeView();
                this.labelDevice = new System.Windows.Forms.Label();
                this.buttonCancel = new System.Windows.Forms.Button();
                this.buttonOK = new System.Windows.Forms.Button();
                this.textBoxPinDiagram = new System.Windows.Forms.TextBox();
                this.labelPinDiagram = new System.Windows.Forms.Label();
                this.SuspendLayout();
                // 
                // treeViewDevice
                // 
                this.treeViewDevice.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)));
                this.treeViewDevice.Location = new System.Drawing.Point(9, 25);
                this.treeViewDevice.Name = "treeViewDevice";
                this.treeViewDevice.Size = new System.Drawing.Size(248, 326);
                this.treeViewDevice.TabIndex = 1;
                this.treeViewDevice.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewDevice_AfterSelect);
                this.treeViewDevice.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.treeViewDevice_MouseDoubleClick);
                // 
                // labelDevice
                // 
                this.labelDevice.AutoSize = true;
                this.labelDevice.Location = new System.Drawing.Point(9, 9);
                this.labelDevice.Name = "labelDevice";
                this.labelDevice.Size = new System.Drawing.Size(44, 13);
                this.labelDevice.TabIndex = 0;
                this.labelDevice.Text = "&Device:";
                // 
                // buttonCancel
                // 
                this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
                this.buttonCancel.Location = new System.Drawing.Point(477, 357);
                this.buttonCancel.Name = "buttonCancel";
                this.buttonCancel.Size = new System.Drawing.Size(75, 23);
                this.buttonCancel.TabIndex = 3;
                this.buttonCancel.Text = "Cancel";
                this.buttonCancel.UseVisualStyleBackColor = true;
                this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
                // 
                // buttonOK
                // 
                this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.buttonOK.Location = new System.Drawing.Point(396, 357);
                this.buttonOK.Name = "buttonOK";
                this.buttonOK.Size = new System.Drawing.Size(75, 23);
                this.buttonOK.TabIndex = 2;
                this.buttonOK.Text = "OK";
                this.buttonOK.UseVisualStyleBackColor = true;
                this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
                // 
                // textBoxPinDiagram
                // 
                this.textBoxPinDiagram.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
                this.textBoxPinDiagram.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
                this.textBoxPinDiagram.Location = new System.Drawing.Point(266, 25);
                this.textBoxPinDiagram.Multiline = true;
                this.textBoxPinDiagram.Name = "textBoxPinDiagram";
                this.textBoxPinDiagram.ReadOnly = true;
                this.textBoxPinDiagram.ScrollBars = System.Windows.Forms.ScrollBars.Both;
                this.textBoxPinDiagram.Size = new System.Drawing.Size(284, 326);
                this.textBoxPinDiagram.TabIndex = 4;
                this.textBoxPinDiagram.WordWrap = false;
                // 
                // labelPinDiagram
                // 
                this.labelPinDiagram.AutoSize = true;
                this.labelPinDiagram.Location = new System.Drawing.Point(263, 9);
                this.labelPinDiagram.Name = "labelPinDiagram";
                this.labelPinDiagram.Size = new System.Drawing.Size(67, 13);
                this.labelPinDiagram.TabIndex = 5;
                this.labelPinDiagram.Text = "&Pin Diagram:";
                // 
                // SelectDeviceForm
                // 
                this.AcceptButton = this.buttonOK;
                this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.CancelButton = this.buttonCancel;
                this.ClientSize = new System.Drawing.Size(562, 388);
                this.Controls.Add(this.labelPinDiagram);
                this.Controls.Add(this.textBoxPinDiagram);
                this.Controls.Add(this.buttonOK);
                this.Controls.Add(this.buttonCancel);
                this.Controls.Add(this.labelDevice);
                this.Controls.Add(this.treeViewDevice);
                this.MaximizeBox = false;
                this.MinimizeBox = false;
                this.MinimumSize = new System.Drawing.Size(578, 426);
                this.Name = "SelectDeviceForm";
                this.ShowIcon = false;
                this.ShowInTaskbar = false;
                this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
                this.Text = "Select Device";
                this.Load += new System.EventHandler(this.SelectDeviceForm_Load);
                this.ResumeLayout(false);
                this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.TreeView treeViewDevice;
            private System.Windows.Forms.Label labelDevice;
            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.TextBox textBoxPinDiagram;
            private System.Windows.Forms.Label labelPinDiagram;
        }
    }
}