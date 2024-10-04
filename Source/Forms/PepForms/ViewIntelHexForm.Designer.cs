
namespace Pep.Forms
{
    partial class ViewIntelHexForm
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
            this.listViewPages = new Common.Forms.ListView();
            this.columnHeaderPageNumber = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderPageAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.buttonView = new System.Windows.Forms.Button();
            this.buttonClose = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // listViewPages
            // 
            this.listViewPages.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewPages.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderPageNumber,
            this.columnHeaderPageAddress});
            this.listViewPages.ComboBoxItems = null;
            this.listViewPages.FullRowSelect = true;
            this.listViewPages.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listViewPages.HideSelection = false;
            this.listViewPages.LabelEdit = true;
            this.listViewPages.LabelEditor = Common.Forms.ListView.ELabelEditor.Label;
            this.listViewPages.LabelWrap = false;
            this.listViewPages.Location = new System.Drawing.Point(10, 10);
            this.listViewPages.Margin = new System.Windows.Forms.Padding(2);
            this.listViewPages.MultiSelect = false;
            this.listViewPages.Name = "listViewPages";
            this.listViewPages.ShowGroups = false;
            this.listViewPages.Size = new System.Drawing.Size(362, 200);
            this.listViewPages.SortArrow = Common.Forms.ListView.ESortArrow.None;
            this.listViewPages.Sorting = Common.Forms.ListView.ESortOrder.None;
            this.listViewPages.TabIndex = 0;
            this.listViewPages.UseCompatibleStateImageBehavior = false;
            this.listViewPages.View = System.Windows.Forms.View.Details;
            this.listViewPages.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.listViewPages_ItemSelectionChanged);
            this.listViewPages.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.listViewPages_MouseDoubleClick);
            // 
            // columnHeaderPageNumber
            // 
            this.columnHeaderPageNumber.Text = "Page Number";
            this.columnHeaderPageNumber.Width = 78;
            // 
            // columnHeaderPageAddress
            // 
            this.columnHeaderPageAddress.Text = "Page Address";
            this.columnHeaderPageAddress.Width = 160;
            // 
            // buttonView
            // 
            this.buttonView.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonView.Location = new System.Drawing.Point(216, 226);
            this.buttonView.Name = "buttonView";
            this.buttonView.Size = new System.Drawing.Size(75, 23);
            this.buttonView.TabIndex = 1;
            this.buttonView.Text = "&View...";
            this.buttonView.UseVisualStyleBackColor = true;
            this.buttonView.Click += new System.EventHandler(this.buttonView_Click);
            // 
            // buttonClose
            // 
            this.buttonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonClose.Location = new System.Drawing.Point(297, 226);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(75, 23);
            this.buttonClose.TabIndex = 2;
            this.buttonClose.Text = "&Close";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // ViewIntelHexForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonClose;
            this.ClientSize = new System.Drawing.Size(384, 261);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.buttonView);
            this.Controls.Add(this.listViewPages);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ViewIntelHexForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "View Intel Hex";
            this.Load += new System.EventHandler(this.ViewIntelHexForm_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private Common.Forms.ListView listViewPages;
        private System.Windows.Forms.Button buttonView;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.ColumnHeader columnHeaderPageNumber;
        private System.Windows.Forms.ColumnHeader columnHeaderPageAddress;
    }
}