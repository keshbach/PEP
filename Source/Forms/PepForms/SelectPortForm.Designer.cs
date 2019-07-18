namespace Pep
{
    namespace Forms
    {
        partial class SelectPortForm
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
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.labelPort = new System.Windows.Forms.Label();
            this.listViewPorts = new Common.Forms.ListView();
            this.columnHeaderPort = ((Common.Forms.ListViewColumnHeader)(new Common.Forms.ListViewColumnHeader()));
            this.buttonRefresh = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // buttonOK
            // 
            this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(166, 126);
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
            this.buttonCancel.Location = new System.Drawing.Point(247, 126);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 4;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // labelPort
            // 
            this.labelPort.AutoSize = true;
            this.labelPort.Location = new System.Drawing.Point(9, 9);
            this.labelPort.Name = "labelPort";
            this.labelPort.Size = new System.Drawing.Size(29, 13);
            this.labelPort.TabIndex = 0;
            this.labelPort.Text = "&Port:";
            // 
            // listViewPorts
            // 
            this.listViewPorts.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewPorts.AutoArrange = false;
            this.listViewPorts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderPort});
            this.listViewPorts.ComboBoxItems = null;
            this.listViewPorts.FullRowSelect = true;
            this.listViewPorts.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.listViewPorts.HideSelection = false;
            this.listViewPorts.LabelEditor = Common.Forms.ListView.ELabelEditor.None;
            this.listViewPorts.LabelWrap = false;
            this.listViewPorts.Location = new System.Drawing.Point(12, 25);
            this.listViewPorts.MultiSelect = false;
            this.listViewPorts.Name = "listViewPorts";
            this.listViewPorts.ShowGroups = false;
            this.listViewPorts.Size = new System.Drawing.Size(310, 85);
            this.listViewPorts.SortArrow = Common.Forms.ListView.ESortArrow.None;
            this.listViewPorts.Sorting = Common.Forms.ListView.ESortOrder.None;
            this.listViewPorts.TabIndex = 1;
            this.listViewPorts.UseCompatibleStateImageBehavior = false;
            this.listViewPorts.View = System.Windows.Forms.View.Details;
            this.listViewPorts.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.listViewPorts_ItemSelectionChanged);
            // 
            // columnHeaderPort
            // 
            this.columnHeaderPort.ControlType = Common.Forms.ListViewColumnHeader.EControlType.Label;
            this.columnHeaderPort.Text = "Port";
            this.columnHeaderPort.Width = 100;
            // 
            // buttonRefresh
            // 
            this.buttonRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonRefresh.Location = new System.Drawing.Point(12, 126);
            this.buttonRefresh.Name = "buttonRefresh";
            this.buttonRefresh.Size = new System.Drawing.Size(75, 23);
            this.buttonRefresh.TabIndex = 2;
            this.buttonRefresh.Text = "Refresh";
            this.buttonRefresh.UseVisualStyleBackColor = true;
            this.buttonRefresh.Click += new System.EventHandler(this.buttonRefresh_Click);
            // 
            // SelectPortForm
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(334, 161);
            this.Controls.Add(this.buttonRefresh);
            this.Controls.Add(this.listViewPorts);
            this.Controls.Add(this.labelPort);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonCancel);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(350, 200);
            this.Name = "SelectPortForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Select Port";
            this.Load += new System.EventHandler(this.SelectPortForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.Label labelPort;
            private Common.Forms.ListView listViewPorts;
            private System.Windows.Forms.Button buttonRefresh;
            private Common.Forms.ListViewColumnHeader columnHeaderPort;
        }
    }
}