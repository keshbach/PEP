/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/

namespace OpenZip.Forms
{
    partial class FileConfirmationForm
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
            this.label1 = new System.Windows.Forms.Label();
            this.radioButtonCopyAndReplace = new System.Windows.Forms.RadioButton();
            this.radioButtonCopyButKeepBothFiles = new System.Windows.Forms.RadioButton();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.labelCopyAndReplace = new System.Windows.Forms.Label();
            this.labelCopyButKeepBothFiles = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 10);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(253, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "There already is a file with that name at this location.";
            // 
            // radioButtonCopyAndReplace
            // 
            this.radioButtonCopyAndReplace.AutoSize = true;
            this.radioButtonCopyAndReplace.Location = new System.Drawing.Point(16, 38);
            this.radioButtonCopyAndReplace.Name = "radioButtonCopyAndReplace";
            this.radioButtonCopyAndReplace.Size = new System.Drawing.Size(113, 17);
            this.radioButtonCopyAndReplace.TabIndex = 4;
            this.radioButtonCopyAndReplace.TabStop = true;
            this.radioButtonCopyAndReplace.Text = "Copy and Replace";
            this.radioButtonCopyAndReplace.UseVisualStyleBackColor = true;
            // 
            // radioButtonCopyButKeepBothFiles
            // 
            this.radioButtonCopyButKeepBothFiles.AutoSize = true;
            this.radioButtonCopyButKeepBothFiles.Location = new System.Drawing.Point(16, 86);
            this.radioButtonCopyButKeepBothFiles.Name = "radioButtonCopyButKeepBothFiles";
            this.radioButtonCopyButKeepBothFiles.Size = new System.Drawing.Size(145, 17);
            this.radioButtonCopyButKeepBothFiles.TabIndex = 6;
            this.radioButtonCopyButKeepBothFiles.TabStop = true;
            this.radioButtonCopyButKeepBothFiles.Text = "Copy, but keep both files.";
            this.radioButtonCopyButKeepBothFiles.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(236, 137);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 7;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(317, 137);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 8;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // labelCopyAndReplace
            // 
            this.labelCopyAndReplace.AutoSize = true;
            this.labelCopyAndReplace.Location = new System.Drawing.Point(32, 60);
            this.labelCopyAndReplace.Name = "labelCopyAndReplace";
            this.labelCopyAndReplace.Size = new System.Drawing.Size(336, 13);
            this.labelCopyAndReplace.TabIndex = 1;
            this.labelCopyAndReplace.Text = "(Replace the file in the destination folder with the file you are copying.)";
            // 
            // labelCopyButKeepBothFiles
            // 
            this.labelCopyButKeepBothFiles.AutoSize = true;
            this.labelCopyButKeepBothFiles.Location = new System.Drawing.Point(32, 108);
            this.labelCopyButKeepBothFiles.Name = "labelCopyButKeepBothFiles";
            this.labelCopyButKeepBothFiles.Size = new System.Drawing.Size(219, 13);
            this.labelCopyButKeepBothFiles.TabIndex = 3;
            this.labelCopyButKeepBothFiles.Text = "(The file being copied will be renamed \" (2)\".)";
            // 
            // FileConfirmationForm
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(404, 172);
            this.Controls.Add(this.labelCopyButKeepBothFiles);
            this.Controls.Add(this.labelCopyAndReplace);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.radioButtonCopyButKeepBothFiles);
            this.Controls.Add(this.radioButtonCopyAndReplace);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "FileConfirmationForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Copy File";
            this.Load += new System.EventHandler(this.FileConfirmationForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.RadioButton radioButtonCopyAndReplace;
        private System.Windows.Forms.RadioButton radioButtonCopyButKeepBothFiles;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label labelCopyAndReplace;
        private System.Windows.Forms.Label labelCopyButKeepBothFiles;
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/
