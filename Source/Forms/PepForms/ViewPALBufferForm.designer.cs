namespace Pep
{
    namespace Forms
    {
        partial class ViewPALBufferForm
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
                this.textBoxBuffer = new System.Windows.Forms.TextBox();
                this.buttonClose = new System.Windows.Forms.Button();
                this.SuspendLayout();
                // 
                // textBoxBuffer
                // 
                this.textBoxBuffer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
                this.textBoxBuffer.CausesValidation = false;
                this.textBoxBuffer.HideSelection = false;
                this.textBoxBuffer.Location = new System.Drawing.Point(13, 13);
                this.textBoxBuffer.Multiline = true;
                this.textBoxBuffer.Name = "textBoxBuffer";
                this.textBoxBuffer.ReadOnly = true;
                this.textBoxBuffer.ScrollBars = System.Windows.Forms.ScrollBars.Both;
                this.textBoxBuffer.Size = new System.Drawing.Size(447, 279);
                this.textBoxBuffer.TabIndex = 0;
                this.textBoxBuffer.WordWrap = false;
                // 
                // buttonClose
                // 
                this.buttonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.buttonClose.Location = new System.Drawing.Point(385, 311);
                this.buttonClose.Name = "buttonClose";
                this.buttonClose.Size = new System.Drawing.Size(75, 23);
                this.buttonClose.TabIndex = 1;
                this.buttonClose.Text = "Close";
                this.buttonClose.UseVisualStyleBackColor = true;
                // 
                // ViewPALBufferForm
                // 
                this.AcceptButton = this.buttonClose;
                this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                this.CancelButton = this.buttonClose;
                this.ClientSize = new System.Drawing.Size(472, 346);
                this.Controls.Add(this.buttonClose);
                this.Controls.Add(this.textBoxBuffer);
                this.MaximizeBox = false;
                this.MinimizeBox = false;
                this.MinimumSize = new System.Drawing.Size(488, 384);
                this.Name = "ViewPALBufferForm";
                this.ShowIcon = false;
                this.ShowInTaskbar = false;
                this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
                this.Text = "View Buffer";
                this.Load += new System.EventHandler(this.ViewPALBufferForm_Load);
                this.ResumeLayout(false);
                this.PerformLayout();

            }

            #endregion

            private System.Windows.Forms.TextBox textBoxBuffer;
            private System.Windows.Forms.Button buttonClose;
        }
    }
}