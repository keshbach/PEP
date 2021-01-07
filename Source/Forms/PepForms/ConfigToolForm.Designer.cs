/***************************************************************************/
/*  Copyright (C) 2010-2010 Kevin Eshbach                                  */
/***************************************************************************/

namespace Pep
{
    namespace Forms
    {
        partial class ConfigToolForm
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
                this.labelApplication = new System.Windows.Forms.Label();
                this.labelArguments = new System.Windows.Forms.Label();
                this.textBoxApplication = new Common.Forms.TextBox();
                this.textBoxArguments = new Common.Forms.TextBox();
                this.buttonApplicationEllipse = new System.Windows.Forms.Button();
                this.buttonOK = new System.Windows.Forms.Button();
                this.buttonCancel = new System.Windows.Forms.Button();
                this.labelDescription2 = new System.Windows.Forms.Label();
                this.labelDescription1 = new System.Windows.Forms.Label();
                this.labelDescription = new System.Windows.Forms.Label();
                this.textBoxDescription = new Common.Forms.TextBox();
                this.SuspendLayout();
                // 
                // labelApplication
                // 
                this.labelApplication.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
                this.labelApplication.AutoSize = true;
                this.labelApplication.Location = new System.Drawing.Point(8, 45);
                this.labelApplication.Name = "labelApplication";
                this.labelApplication.Size = new System.Drawing.Size(62, 13);
                this.labelApplication.TabIndex = 2;
                this.labelApplication.Text = "&Application:";
                // 
                // labelArguments
                // 
                this.labelArguments.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
                this.labelArguments.AutoSize = true;
                this.labelArguments.Location = new System.Drawing.Point(8, 74);
                this.labelArguments.Name = "labelArguments";
                this.labelArguments.Size = new System.Drawing.Size(128, 13);
                this.labelArguments.TabIndex = 5;
                this.labelArguments.Text = "&Command line arguments:";
                // 
                // textBoxApplication
                // 
                this.textBoxApplication.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                            | System.Windows.Forms.AnchorStyles.Right)));
                this.textBoxApplication.Location = new System.Drawing.Point(142, 42);
                this.textBoxApplication.Name = "textBoxApplication";
                this.textBoxApplication.ReadOnly = true;
                this.textBoxApplication.Size = new System.Drawing.Size(250, 20);
                this.textBoxApplication.TabIndex = 3;
                // 
                // textBoxArguments
                // 
                this.textBoxArguments.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                            | System.Windows.Forms.AnchorStyles.Right)));
                this.textBoxArguments.Location = new System.Drawing.Point(142, 71);
                this.textBoxArguments.Name = "textBoxArguments";
                this.textBoxArguments.Size = new System.Drawing.Size(250, 20);
                this.textBoxArguments.TabIndex = 6;
                this.textBoxArguments.TextChanged += new System.EventHandler(this.textBoxArguments_TextChanged);
                // 
                // buttonApplicationEllipse
                // 
                this.buttonApplicationEllipse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonApplicationEllipse.Location = new System.Drawing.Point(398, 42);
                this.buttonApplicationEllipse.Name = "buttonApplicationEllipse";
                this.buttonApplicationEllipse.Size = new System.Drawing.Size(24, 20);
                this.buttonApplicationEllipse.TabIndex = 4;
                this.buttonApplicationEllipse.Text = "...";
                this.buttonApplicationEllipse.UseVisualStyleBackColor = true;
                this.buttonApplicationEllipse.Click += new System.EventHandler(this.buttonApplicationEllipse_Click);
                // 
                // buttonOK
                // 
                this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.buttonOK.Location = new System.Drawing.Point(266, 161);
                this.buttonOK.Name = "buttonOK";
                this.buttonOK.Size = new System.Drawing.Size(75, 23);
                this.buttonOK.TabIndex = 9;
                this.buttonOK.Text = "OK";
                this.buttonOK.UseVisualStyleBackColor = true;
                this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
                // 
                // buttonCancel
                // 
                this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
                this.buttonCancel.Location = new System.Drawing.Point(347, 161);
                this.buttonCancel.Name = "buttonCancel";
                this.buttonCancel.Size = new System.Drawing.Size(75, 23);
                this.buttonCancel.TabIndex = 10;
                this.buttonCancel.Text = "Cancel";
                this.buttonCancel.UseVisualStyleBackColor = true;
                // 
                // labelDescription2
                // 
                this.labelDescription2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
                this.labelDescription2.AutoSize = true;
                this.labelDescription2.Location = new System.Drawing.Point(20, 126);
                this.labelDescription2.Name = "labelDescription2";
                this.labelDescription2.Size = new System.Drawing.Size(122, 13);
                this.labelDescription2.TabIndex = 8;
                this.labelDescription2.Text = "%1 - Current data/zip file";
                // 
                // labelDescription1
                // 
                this.labelDescription1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
                this.labelDescription1.AutoSize = true;
                this.labelDescription1.Location = new System.Drawing.Point(8, 108);
                this.labelDescription1.Name = "labelDescription1";
                this.labelDescription1.Size = new System.Drawing.Size(148, 13);
                this.labelDescription1.TabIndex = 7;
                this.labelDescription1.Text = "Command line argument flags.";
                // 
                // labelDescription
                // 
                this.labelDescription.AutoSize = true;
                this.labelDescription.Location = new System.Drawing.Point(11, 13);
                this.labelDescription.Name = "labelDescription";
                this.labelDescription.Size = new System.Drawing.Size(63, 13);
                this.labelDescription.TabIndex = 0;
                this.labelDescription.Text = "&Description:";
                // 
                // textBoxDescription
                // 
                this.textBoxDescription.Location = new System.Drawing.Point(142, 13);
                this.textBoxDescription.Name = "textBoxDescription";
                this.textBoxDescription.Size = new System.Drawing.Size(250, 20);
                this.textBoxDescription.TabIndex = 1;
                this.textBoxDescription.TextChanged += new System.EventHandler(this.textBoxDescription_TextChanged);
                // 
                // ConfigToolForm
                // 
                this.AcceptButton = this.buttonOK;
                this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.CancelButton = this.buttonCancel;
                this.ClientSize = new System.Drawing.Size(434, 196);
                this.Controls.Add(this.textBoxDescription);
                this.Controls.Add(this.labelDescription);
                this.Controls.Add(this.labelDescription1);
                this.Controls.Add(this.labelDescription2);
                this.Controls.Add(this.buttonCancel);
                this.Controls.Add(this.buttonOK);
                this.Controls.Add(this.buttonApplicationEllipse);
                this.Controls.Add(this.textBoxArguments);
                this.Controls.Add(this.textBoxApplication);
                this.Controls.Add(this.labelArguments);
                this.Controls.Add(this.labelApplication);
                this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
                this.MaximizeBox = false;
                this.MinimizeBox = false;
                this.Name = "ConfigToolForm";
                this.ShowIcon = false;
                this.ShowInTaskbar = false;
                this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
                this.Text = "Configure Tool";
                this.Load += new System.EventHandler(this.ConfigToolForm_Load);
                this.ResumeLayout(false);
                this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.Label labelApplication;
            private System.Windows.Forms.Label labelArguments;
            private Common.Forms.TextBox textBoxApplication;
            private Common.Forms.TextBox textBoxArguments;
            private System.Windows.Forms.Button buttonApplicationEllipse;
            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.Label labelDescription2;
            private System.Windows.Forms.Label labelDescription1;
            private System.Windows.Forms.Label labelDescription;
            private Common.Forms.TextBox textBoxDescription;
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2010-2010 Kevin Eshbach                                  */
/***************************************************************************/
