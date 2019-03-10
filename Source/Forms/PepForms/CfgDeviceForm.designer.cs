namespace Pep
{
    namespace Forms
    {
        partial class CfgDeviceForm
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
                System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CfgDeviceForm));
                this.buttonOK = new System.Windows.Forms.Button();
                this.buttonCancel = new System.Windows.Forms.Button();
                this.listViewPins = new Common.Forms.ListView();
                this.columnHeaderPinNumber = ((Common.Forms.ListViewColumnHeader)(new Common.Forms.ListViewColumnHeader()));
                this.columnHeaderPinType = ((Common.Forms.ListViewColumnHeader)(new Common.Forms.ListViewColumnHeader()));
                this.columnHeaderPinPolarity = ((Common.Forms.ListViewColumnHeader)(new Common.Forms.ListViewColumnHeader()));
                this.SuspendLayout();
                // 
                // buttonOK
                // 
                this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.buttonOK.Location = new System.Drawing.Point(66, 278);
                this.buttonOK.Name = "buttonOK";
                this.buttonOK.Size = new System.Drawing.Size(75, 23);
                this.buttonOK.TabIndex = 3;
                this.buttonOK.Text = "OK";
                this.buttonOK.UseVisualStyleBackColor = true;
                this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
                // 
                // buttonCancel
                // 
                this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
                this.buttonCancel.Location = new System.Drawing.Point(147, 278);
                this.buttonCancel.Name = "buttonCancel";
                this.buttonCancel.Size = new System.Drawing.Size(75, 23);
                this.buttonCancel.TabIndex = 4;
                this.buttonCancel.Text = "Cancel";
                this.buttonCancel.UseVisualStyleBackColor = true;
                this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
                // 
                // listViewPins
                // 
                this.listViewPins.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
                this.listViewPins.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderPinNumber,
            this.columnHeaderPinType,
            this.columnHeaderPinPolarity});
                this.listViewPins.FullRowSelect = true;
                this.listViewPins.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
                this.listViewPins.HideSelection = false;
                this.listViewPins.LabelWrap = false;
                this.listViewPins.ComboBoxItems = ((System.Collections.Specialized.StringCollection)(resources.GetObject("listViewPins.ComboBoxItems")));
                this.listViewPins.Location = new System.Drawing.Point(13, 13);
                this.listViewPins.MultiSelect = false;
                this.listViewPins.Name = "listViewPins";
                this.listViewPins.ShowGroups = false;
                this.listViewPins.Size = new System.Drawing.Size(208, 254);
                this.listViewPins.Sorting = Common.Forms.ListView.ESortOrder.None;
                this.listViewPins.TabIndex = 0;
                this.listViewPins.UseCompatibleStateImageBehavior = false;
                this.listViewPins.LabelEditor = Common.Forms.ListView.ELabelEditor.ComboBox;
                this.listViewPins.View = System.Windows.Forms.View.Details;
                this.listViewPins.AfterComboBoxEdit += new Common.Forms.ListView.AfterComboBoxEditHandler(this.listViewPins_AfterComboBoxEdit);
                this.listViewPins.BeforeComboBoxEdit += new Common.Forms.ListView.BeforeComboBoxEditHandler(this.listViewPins_BeforeComboBoxEdit);
                // 
                // columnHeaderPinNumber
                // 
                this.columnHeaderPinNumber.ControlType = Common.Forms.ListViewColumnHeader.EControlType.Label;
                this.columnHeaderPinNumber.Text = "Pin #";
                // 
                // columnHeaderPinType
                // 
                this.columnHeaderPinType.ControlType = Common.Forms.ListViewColumnHeader.EControlType.ComboBox;
                this.columnHeaderPinType.Text = "Type";
                this.columnHeaderPinType.Width = 90;
                // 
                // columnHeaderPinPolarity
                // 
                this.columnHeaderPinPolarity.ControlType = Common.Forms.ListViewColumnHeader.EControlType.ComboBox;
                this.columnHeaderPinPolarity.Text = "Polarity";
                // 
                // CfgDeviceForm
                // 
                this.AcceptButton = this.buttonOK;
                this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.CancelButton = this.buttonCancel;
                this.ClientSize = new System.Drawing.Size(234, 312);
                this.Controls.Add(this.buttonCancel);
                this.Controls.Add(this.buttonOK);
                this.Controls.Add(this.listViewPins);
                this.MaximizeBox = false;
                this.MinimizeBox = false;
                this.MinimumSize = new System.Drawing.Size(250, 350);
                this.Name = "CfgDeviceForm";
                this.ShowIcon = false;
                this.ShowInTaskbar = false;
                this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
                this.Text = "Configure Device";
                this.Load += new System.EventHandler(this.CfgDeviceForm_Load);
                this.ResumeLayout(false);

            }

            #endregion

            private Common.Forms.ListView listViewPins;
            private Common.Forms.ListViewColumnHeader columnHeaderPinNumber;
            private Common.Forms.ListViewColumnHeader columnHeaderPinType;
            private Common.Forms.ListViewColumnHeader columnHeaderPinPolarity;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Button buttonCancel;
        }
    }
}