namespace Pep
{
    namespace Forms
    {
        partial class ViewBufferForm
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
                this.bufferViewer = new Pep.Forms.BufferViewer();
                this.comboBoxOrganization = new Common.Forms.ComboBox();
                this.labelOrganization = new System.Windows.Forms.Label();
                this.buttonFill = new System.Windows.Forms.Button();
                this.buttonOK = new System.Windows.Forms.Button();
                this.buttonCancel = new System.Windows.Forms.Button();
                this.SuspendLayout();
                //
                // bufferViewer
                //
                this.bufferViewer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                            | System.Windows.Forms.AnchorStyles.Left)
                            | System.Windows.Forms.AnchorStyles.Right)));
                this.bufferViewer.AutoValidate = System.Windows.Forms.AutoValidate.Disable;
                this.bufferViewer.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
                this.bufferViewer.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
                this.bufferViewer.Buffer = null;
                this.bufferViewer.DataOrganization = Pep.Forms.BufferViewer.EDataOrganization.Byte;
                this.bufferViewer.FontPtSize = ((uint)(10u));
                this.bufferViewer.Location = new System.Drawing.Point(12, 12);
                this.bufferViewer.Name = "bufferViewer";
                this.bufferViewer.Size = new System.Drawing.Size(568, 210);
                this.bufferViewer.TabIndex = 0;
                // 
                // labelOrganization
                // 
                this.labelOrganization.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
                this.labelOrganization.AutoSize = true;
                this.labelOrganization.Location = new System.Drawing.Point(9, 238);
                this.labelOrganization.Name = "labelOrganization";
                this.labelOrganization.Size = new System.Drawing.Size(69, 13);
                this.labelOrganization.TabIndex = 1;
                this.labelOrganization.Text = "&Organization:";
                // 
                // comboBoxOrganization
                // 
                this.comboBoxOrganization.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
                this.comboBoxOrganization.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
                this.comboBoxOrganization.FormattingEnabled = true;
                this.comboBoxOrganization.Location = new System.Drawing.Point(84, 235);
                this.comboBoxOrganization.Name = "comboBoxOrganization";
                this.comboBoxOrganization.Size = new System.Drawing.Size(121, 21);
                this.comboBoxOrganization.TabIndex = 2;
                this.comboBoxOrganization.SelectedIndexChanged += new System.EventHandler(this.comboBoxOrganization_SelectedIndexChanged);
                // 
                // buttonFill
                // 
                this.buttonFill.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
                this.buttonFill.Location = new System.Drawing.Point(13, 271);
                this.buttonFill.Name = "buttonFill";
                this.buttonFill.Size = new System.Drawing.Size(75, 23);
                this.buttonFill.TabIndex = 3;
                this.buttonFill.Text = "&Fill...";
                this.buttonFill.UseVisualStyleBackColor = true;
                this.buttonFill.Click += new System.EventHandler(this.buttonFill_Click);
                // 
                // buttonOK
                // 
                this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.buttonOK.Location = new System.Drawing.Point(424, 271);
                this.buttonOK.Name = "buttonOK";
                this.buttonOK.Size = new System.Drawing.Size(75, 23);
                this.buttonOK.TabIndex = 4;
                this.buttonOK.Text = "OK";
                this.buttonOK.UseVisualStyleBackColor = true;
                // 
                // buttonCancel
                // 
                this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
                this.buttonCancel.Location = new System.Drawing.Point(505, 271);
                this.buttonCancel.Name = "buttonCancel";
                this.buttonCancel.Size = new System.Drawing.Size(75, 23);
                this.buttonCancel.TabIndex = 5;
                this.buttonCancel.Text = "Cancel";
                this.buttonCancel.UseVisualStyleBackColor = true;
                // 
                // ViewBufferForm
                // 
                this.AcceptButton = this.buttonOK;
                this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.CancelButton = this.buttonCancel;
                this.ClientSize = new System.Drawing.Size(592, 306);
                this.Controls.Add(this.bufferViewer);
                this.Controls.Add(this.labelOrganization);
                this.Controls.Add(this.comboBoxOrganization);
                this.Controls.Add(this.buttonFill);
                this.Controls.Add(this.buttonOK);
                this.Controls.Add(this.buttonCancel);
                this.MaximizeBox = false;
                this.MinimizeBox = false;
                this.MinimumSize = new System.Drawing.Size(608, 344);
                this.Name = "ViewBufferForm";
                this.ShowIcon = false;
                this.ShowInTaskbar = false;
                this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
                this.Text = "View Buffer";
                this.Load += new System.EventHandler(this.ViewBufferForm_Load);
                this.ResumeLayout(false);
                this.PerformLayout();
            }

            #endregion

            private System.Windows.Forms.Label labelOrganization;
            private Common.Forms.ComboBox comboBoxOrganization;
            private System.Windows.Forms.Button buttonFill;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Button buttonCancel;
            private Pep.Forms.BufferViewer bufferViewer;
        }
    }
}