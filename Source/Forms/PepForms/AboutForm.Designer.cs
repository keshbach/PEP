namespace Pep
{
    namespace Forms
    {
        partial class AboutForm
        {
            /// <summary>
            /// Required designer variable.
            /// </summary>
            private System.ComponentModel.IContainer components = null;

            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
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
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AboutForm));
        this.labelProductName = new System.Windows.Forms.Label();
        this.labelProductVersion = new System.Windows.Forms.Label();
        this.labelProductCopyright = new System.Windows.Forms.Label();
        this.label4 = new System.Windows.Forms.Label();
        this.buttonClose = new System.Windows.Forms.Button();
        this.label1 = new System.Windows.Forms.Label();
        this.listViewPlugins = new Common.Forms.ListView();
        this.columnHeaderName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        this.columnHeaderVersion = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        this.textBoxOther = new System.Windows.Forms.TextBox();
        this.textBoxEmail = new System.Windows.Forms.TextBox();
        this.SuspendLayout();
        // 
        // labelProductName
        // 
        this.labelProductName.AutoSize = true;
        this.labelProductName.Location = new System.Drawing.Point(13, 9);
        this.labelProductName.MinimumSize = new System.Drawing.Size(338, 0);
        this.labelProductName.Name = "labelProductName";
        this.labelProductName.Size = new System.Drawing.Size(338, 13);
        this.labelProductName.TabIndex = 0;
        this.labelProductName.Text = "Product Name";
        this.labelProductName.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelProductVersion
        // 
        this.labelProductVersion.AutoSize = true;
        this.labelProductVersion.Location = new System.Drawing.Point(13, 26);
        this.labelProductVersion.MinimumSize = new System.Drawing.Size(338, 0);
        this.labelProductVersion.Name = "labelProductVersion";
        this.labelProductVersion.Size = new System.Drawing.Size(338, 13);
        this.labelProductVersion.TabIndex = 1;
        this.labelProductVersion.Text = "Product Version";
        this.labelProductVersion.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelProductCopyright
        // 
        this.labelProductCopyright.AutoSize = true;
        this.labelProductCopyright.Location = new System.Drawing.Point(13, 43);
        this.labelProductCopyright.MinimumSize = new System.Drawing.Size(338, 0);
        this.labelProductCopyright.Name = "labelProductCopyright";
        this.labelProductCopyright.Size = new System.Drawing.Size(338, 13);
        this.labelProductCopyright.TabIndex = 2;
        this.labelProductCopyright.Text = "Product Copyright";
        this.labelProductCopyright.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // label4
        // 
        this.label4.AutoSize = true;
        this.label4.Location = new System.Drawing.Point(13, 60);
        this.label4.MinimumSize = new System.Drawing.Size(338, 0);
        this.label4.Name = "label4";
        this.label4.Size = new System.Drawing.Size(338, 13);
        this.label4.TabIndex = 3;
        this.label4.Text = "All Rights Reserved";
        this.label4.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // buttonClose
        // 
        this.buttonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
        this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
        this.buttonClose.Location = new System.Drawing.Point(279, 331);
        this.buttonClose.Name = "buttonClose";
        this.buttonClose.Size = new System.Drawing.Size(75, 23);
        this.buttonClose.TabIndex = 8;
        this.buttonClose.Text = "Close";
        this.buttonClose.UseVisualStyleBackColor = true;
        // 
        // label1
        // 
        this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
        this.label1.AutoSize = true;
        this.label1.Location = new System.Drawing.Point(15, 183);
        this.label1.Name = "label1";
        this.label1.Size = new System.Drawing.Size(81, 13);
        this.label1.TabIndex = 6;
        this.label1.Text = "&Device Plugins:";
        // 
        // listViewPlugins
        // 
        this.listViewPlugins.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
        | System.Windows.Forms.AnchorStyles.Right)));
        this.listViewPlugins.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
        this.columnHeaderName,
        this.columnHeaderVersion});
        this.listViewPlugins.ComboBoxItems = ((System.Collections.Specialized.StringCollection)(resources.GetObject("listViewPlugins.ComboBoxItems")));
        this.listViewPlugins.FullRowSelect = true;
        this.listViewPlugins.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
        this.listViewPlugins.HideSelection = false;
        this.listViewPlugins.LabelEdit = true;
        this.listViewPlugins.LabelEditor = Common.Forms.ListView.ELabelEditor.Label;
        this.listViewPlugins.LabelWrap = false;
        this.listViewPlugins.Location = new System.Drawing.Point(15, 199);
        this.listViewPlugins.MultiSelect = false;
        this.listViewPlugins.Name = "listViewPlugins";
        this.listViewPlugins.Size = new System.Drawing.Size(339, 118);
        this.listViewPlugins.SortArrow = Common.Forms.ListView.ESortArrow.None;
        this.listViewPlugins.Sorting = Common.Forms.ListView.ESortOrder.None;
        this.listViewPlugins.TabIndex = 7;
        this.listViewPlugins.UseCompatibleStateImageBehavior = false;
        this.listViewPlugins.View = System.Windows.Forms.View.Details;
        // 
        // columnHeaderName
        // 
        this.columnHeaderName.Text = "Name";
        // 
        // columnHeaderVersion
        // 
        this.columnHeaderVersion.Text = "Version";
        // 
        // textBoxOther
        // 
        this.textBoxOther.Location = new System.Drawing.Point(12, 108);
        this.textBoxOther.Multiline = true;
        this.textBoxOther.Name = "textBoxOther";
        this.textBoxOther.ReadOnly = true;
        this.textBoxOther.Size = new System.Drawing.Size(341, 64);
        this.textBoxOther.TabIndex = 5;
        // 
        // textBoxEmail
        // 
        this.textBoxEmail.BackColor = System.Drawing.SystemColors.ButtonFace;
        this.textBoxEmail.BorderStyle = System.Windows.Forms.BorderStyle.None;
        this.textBoxEmail.Location = new System.Drawing.Point(12, 84);
        this.textBoxEmail.MaxLength = 100;
        this.textBoxEmail.Name = "textBoxEmail";
        this.textBoxEmail.ReadOnly = true;
        this.textBoxEmail.Size = new System.Drawing.Size(342, 13);
        this.textBoxEmail.TabIndex = 9;
        this.textBoxEmail.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
        this.textBoxEmail.WordWrap = false;
        // 
        // AboutForm
        // 
        this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        this.CancelButton = this.buttonClose;
        this.ClientSize = new System.Drawing.Size(366, 364);
        this.Controls.Add(this.textBoxEmail);
        this.Controls.Add(this.textBoxOther);
        this.Controls.Add(this.listViewPlugins);
        this.Controls.Add(this.label1);
        this.Controls.Add(this.buttonClose);
        this.Controls.Add(this.label4);
        this.Controls.Add(this.labelProductCopyright);
        this.Controls.Add(this.labelProductVersion);
        this.Controls.Add(this.labelProductName);
        this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
        this.MaximizeBox = false;
        this.MinimizeBox = false;
        this.Name = "AboutForm";
        this.Padding = new System.Windows.Forms.Padding(9);
        this.ShowIcon = false;
        this.ShowInTaskbar = false;
        this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
        this.Text = "AboutForm";
        this.Load += new System.EventHandler(this.AboutForm_Load);
        this.ResumeLayout(false);
        this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.Label labelProductName;
            private System.Windows.Forms.Label labelProductVersion;
            private System.Windows.Forms.Label labelProductCopyright;
            private System.Windows.Forms.Label label4;
            private System.Windows.Forms.Button buttonClose;
            private System.Windows.Forms.Label label1;
            private Common.Forms.ListView listViewPlugins;
            private System.Windows.Forms.ColumnHeader columnHeaderName;
            private System.Windows.Forms.ColumnHeader columnHeaderVersion;
            private System.Windows.Forms.TextBox textBoxOther;
            private System.Windows.Forms.TextBox textBoxEmail;
        }
    }
}
