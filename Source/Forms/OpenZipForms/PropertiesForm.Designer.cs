/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/

namespace OpenZip
{
    namespace Forms
    {
        partial class PropertiesForm
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
            this.tabControlProperties = new System.Windows.Forms.TabControl();
            this.tabPageComment = new System.Windows.Forms.TabPage();
            this.textBoxComment = new System.Windows.Forms.TextBox();
            this.tabControlProperties.SuspendLayout();
            this.tabPageComment.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonOK
            // 
            this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(196, 358);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 1;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(277, 357);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 2;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // tabControlProperties
            // 
            this.tabControlProperties.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControlProperties.Controls.Add(this.tabPageComment);
            this.tabControlProperties.Location = new System.Drawing.Point(12, 12);
            this.tabControlProperties.Name = "tabControlProperties";
            this.tabControlProperties.SelectedIndex = 0;
            this.tabControlProperties.Size = new System.Drawing.Size(340, 340);
            this.tabControlProperties.TabIndex = 0;
            // 
            // tabPageComment
            // 
            this.tabPageComment.Controls.Add(this.textBoxComment);
            this.tabPageComment.Location = new System.Drawing.Point(4, 22);
            this.tabPageComment.Name = "tabPageComment";
            this.tabPageComment.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageComment.Size = new System.Drawing.Size(332, 314);
            this.tabPageComment.TabIndex = 0;
            this.tabPageComment.Text = "Comment";
            this.tabPageComment.UseVisualStyleBackColor = true;
            // 
            // textBoxComment
            // 
            this.textBoxComment.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxComment.Location = new System.Drawing.Point(6, 6);
            this.textBoxComment.MaxLength = 65536;
            this.textBoxComment.Multiline = true;
            this.textBoxComment.Name = "textBoxComment";
            this.textBoxComment.Size = new System.Drawing.Size(323, 302);
            this.textBoxComment.TabIndex = 0;
            this.textBoxComment.WordWrap = false;
            // 
            // PropertiesForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(364, 392);
            this.Controls.Add(this.tabControlProperties);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PropertiesForm";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Properties";
            this.Load += new System.EventHandler(this.PropertiesForm_Load);
            this.tabControlProperties.ResumeLayout(false);
            this.tabPageComment.ResumeLayout(false);
            this.tabPageComment.PerformLayout();
            this.ResumeLayout(false);

            }

            #endregion

            private System.Windows.Forms.Button buttonOK;
            private System.Windows.Forms.Button buttonCancel;
            private System.Windows.Forms.TabControl tabControlProperties;
            private System.Windows.Forms.TabPage tabPageComment;
            private System.Windows.Forms.TextBox textBoxComment;
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/
