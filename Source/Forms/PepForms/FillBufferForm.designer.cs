namespace Pep
{
    namespace Forms
    {
        partial class FillBufferForm
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
                this.buttonCancel = new System.Windows.Forms.Button();
                this.buttonOK = new System.Windows.Forms.Button();
                this.labelValue = new System.Windows.Forms.Label();
                this.maskedTextBoxValue = new System.Windows.Forms.MaskedTextBox();
                this.labelInstructions = new System.Windows.Forms.Label();
                this.SuspendLayout();
                // 
                // buttonCancel
                // 
                this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
                this.buttonCancel.Location = new System.Drawing.Point(95, 65);
                this.buttonCancel.Name = "buttonCancel";
                this.buttonCancel.Size = new System.Drawing.Size(75, 23);
                this.buttonCancel.TabIndex = 4;
                this.buttonCancel.Text = "Cancel";
                this.buttonCancel.UseVisualStyleBackColor = true;
                this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
                // 
                // buttonOK
                // 
                this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.buttonOK.Location = new System.Drawing.Point(14, 65);
                this.buttonOK.Name = "buttonOK";
                this.buttonOK.Size = new System.Drawing.Size(75, 23);
                this.buttonOK.TabIndex = 3;
                this.buttonOK.Text = "OK";
                this.buttonOK.UseVisualStyleBackColor = true;
                this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
                // 
                // labelValue
                // 
                this.labelValue.AutoSize = true;
                this.labelValue.Location = new System.Drawing.Point(13, 13);
                this.labelValue.Name = "labelValue";
                this.labelValue.Size = new System.Drawing.Size(37, 13);
                this.labelValue.TabIndex = 0;
                this.labelValue.Text = "&Value:";
                // 
                // maskedTextBoxValue
                // 
                this.maskedTextBoxValue.Location = new System.Drawing.Point(56, 10);
                this.maskedTextBoxValue.Mask = "000";
                this.maskedTextBoxValue.Name = "maskedTextBoxValue";
                this.maskedTextBoxValue.Size = new System.Drawing.Size(24, 20);
                this.maskedTextBoxValue.TabIndex = 1;
                this.maskedTextBoxValue.TextChanged += new System.EventHandler(this.maskedTextBoxValue_TextChanged);
                // 
                // labelInstructions
                // 
                this.labelInstructions.AutoSize = true;
                this.labelInstructions.Location = new System.Drawing.Point(13, 33);
                this.labelInstructions.Name = "labelInstructions";
                this.labelInstructions.Size = new System.Drawing.Size(144, 13);
                this.labelInstructions.TabIndex = 2;
                this.labelInstructions.Text = "(Enter a value from 0 to 255.)";
                // 
                // FillBufferForm
                // 
                this.AcceptButton = this.buttonOK;
                this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.CancelButton = this.buttonCancel;
                this.ClientSize = new System.Drawing.Size(182, 100);
                this.Controls.Add(this.labelInstructions);
                this.Controls.Add(this.maskedTextBoxValue);
                this.Controls.Add(this.labelValue);
                this.Controls.Add(this.buttonOK);
                this.Controls.Add(this.buttonCancel);
                this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
                this.MaximizeBox = false;
                this.MinimizeBox = false;
                this.Name = "FillBufferForm";
                this.ShowIcon = false;
                this.ShowInTaskbar = false;
                this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
                this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
                this.Text = "Fill";
                this.Load += new System.EventHandler(this.FillBufferForm_Load);
                this.ResumeLayout(false);
                this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Label labelValue;
            private System.Windows.Forms.MaskedTextBox maskedTextBoxValue;
            private System.Windows.Forms.Label labelInstructions;
        }
    }
}