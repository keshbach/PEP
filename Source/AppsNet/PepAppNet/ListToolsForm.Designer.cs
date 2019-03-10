namespace Pep
{
    namespace Application
    {
        namespace Forms
        {
            partial class ListToolsForm
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
                    System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ListToolsForm));
                    this.listViewTools = new Common.Forms.ListView();
                    this.columnHeaderPosition = new System.Windows.Forms.ColumnHeader();
                    this.columnHeaderDescription = new System.Windows.Forms.ColumnHeader();
                    this.buttonOK = new System.Windows.Forms.Button();
                    this.buttonDown = new System.Windows.Forms.Button();
                    this.buttonUp = new System.Windows.Forms.Button();
                    this.buttonEdit = new System.Windows.Forms.Button();
                    this.buttonClear = new System.Windows.Forms.Button();
                    this.buttonCancel = new System.Windows.Forms.Button();
                    this.SuspendLayout();
                    // 
                    // listViewTools
                    // 
                    this.listViewTools.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                                | System.Windows.Forms.AnchorStyles.Right)));
                    this.listViewTools.AutoArrange = false;
                    this.listViewTools.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderPosition,
            this.columnHeaderDescription});
                    this.listViewTools.FullRowSelect = true;
                    this.listViewTools.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
                    this.listViewTools.HideSelection = false;
                    this.listViewTools.LabelWrap = false;
                    this.listViewTools.ComboBoxItems = ((System.Collections.Specialized.StringCollection)(resources.GetObject("listViewTools.ComboBoxItems")));
                    this.listViewTools.Location = new System.Drawing.Point(12, 12);
                    this.listViewTools.MultiSelect = false;
                    this.listViewTools.Name = "listViewTools";
                    this.listViewTools.ShowGroups = false;
                    this.listViewTools.Size = new System.Drawing.Size(339, 166);
                    this.listViewTools.Sorting = Common.Forms.ListView.ESortOrder.None;
                    this.listViewTools.TabIndex = 0;
                    this.listViewTools.UseCompatibleStateImageBehavior = false;
                    this.listViewTools.LabelEditor = Common.Forms.ListView.ELabelEditor.Label;
                    this.listViewTools.View = System.Windows.Forms.View.Details;
                    this.listViewTools.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.listViewTools_MouseDoubleClick);
                    this.listViewTools.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.listViewTools_ItemSelectionChanged);
                    // 
                    // columnHeaderPosition
                    // 
                    this.columnHeaderPosition.Text = "Position";
                    // 
                    // columnHeaderDescription
                    // 
                    this.columnHeaderDescription.Text = "Description";
                    // 
                    // buttonOK
                    // 
                    this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                    this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
                    this.buttonOK.Location = new System.Drawing.Point(276, 191);
                    this.buttonOK.Name = "buttonOK";
                    this.buttonOK.Size = new System.Drawing.Size(75, 23);
                    this.buttonOK.TabIndex = 5;
                    this.buttonOK.Text = "OK";
                    this.buttonOK.UseVisualStyleBackColor = true;
                    this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
                    // 
                    // buttonDown
                    // 
                    this.buttonDown.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
                    this.buttonDown.Location = new System.Drawing.Point(357, 63);
                    this.buttonDown.Name = "buttonDown";
                    this.buttonDown.Size = new System.Drawing.Size(75, 23);
                    this.buttonDown.TabIndex = 2;
                    this.buttonDown.Text = "&Down";
                    this.buttonDown.UseVisualStyleBackColor = true;
                    this.buttonDown.Click += new System.EventHandler(this.buttonDown_Click);
                    // 
                    // buttonUp
                    // 
                    this.buttonUp.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
                    this.buttonUp.Location = new System.Drawing.Point(357, 34);
                    this.buttonUp.Name = "buttonUp";
                    this.buttonUp.Size = new System.Drawing.Size(75, 23);
                    this.buttonUp.TabIndex = 1;
                    this.buttonUp.Text = "&Up";
                    this.buttonUp.UseVisualStyleBackColor = true;
                    this.buttonUp.Click += new System.EventHandler(this.buttonUp_Click);
                    // 
                    // buttonEdit
                    // 
                    this.buttonEdit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
                    this.buttonEdit.Location = new System.Drawing.Point(357, 106);
                    this.buttonEdit.Name = "buttonEdit";
                    this.buttonEdit.Size = new System.Drawing.Size(75, 23);
                    this.buttonEdit.TabIndex = 3;
                    this.buttonEdit.Text = "&Edit";
                    this.buttonEdit.UseVisualStyleBackColor = true;
                    this.buttonEdit.Click += new System.EventHandler(this.buttonEdit_Click);
                    // 
                    // buttonClear
                    // 
                    this.buttonClear.Location = new System.Drawing.Point(357, 135);
                    this.buttonClear.Name = "buttonClear";
                    this.buttonClear.Size = new System.Drawing.Size(75, 23);
                    this.buttonClear.TabIndex = 4;
                    this.buttonClear.Text = "&Clear";
                    this.buttonClear.UseVisualStyleBackColor = true;
                    this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
                    // 
                    // buttonCancel
                    // 
                    this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
                    this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
                    this.buttonCancel.Location = new System.Drawing.Point(357, 191);
                    this.buttonCancel.Name = "buttonCancel";
                    this.buttonCancel.Size = new System.Drawing.Size(75, 23);
                    this.buttonCancel.TabIndex = 6;
                    this.buttonCancel.Text = "Cancel";
                    this.buttonCancel.UseVisualStyleBackColor = true;
                    // 
                    // ListToolsForm
                    // 
                    this.AcceptButton = this.buttonOK;
                    this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
                    this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                    this.CancelButton = this.buttonOK;
                    this.ClientSize = new System.Drawing.Size(444, 226);
                    this.Controls.Add(this.buttonCancel);
                    this.Controls.Add(this.buttonClear);
                    this.Controls.Add(this.buttonEdit);
                    this.Controls.Add(this.buttonUp);
                    this.Controls.Add(this.buttonDown);
                    this.Controls.Add(this.buttonOK);
                    this.Controls.Add(this.listViewTools);
                    this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
                    this.MaximizeBox = false;
                    this.MinimizeBox = false;
                    this.Name = "ListToolsForm";
                    this.ShowIcon = false;
                    this.ShowInTaskbar = false;
                    this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
                    this.Text = "Configure";
                    this.Load += new System.EventHandler(this.ListToolsForm_Load);
                    this.ResumeLayout(false);

                }

                #endregion

                private System.Windows.Forms.Button buttonOK;
                private System.Windows.Forms.Button buttonDown;
                private System.Windows.Forms.Button buttonUp;
                private System.Windows.Forms.ColumnHeader columnHeaderPosition;
                private System.Windows.Forms.ColumnHeader columnHeaderDescription;
                private Common.Forms.ListView listViewTools;
                private System.Windows.Forms.Button buttonEdit;
                private System.Windows.Forms.Button buttonClear;
                private System.Windows.Forms.Button buttonCancel;
            }
        }
    }
}