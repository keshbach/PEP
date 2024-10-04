/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep.Forms
{
    public partial class ViewIntelHexForm : Common.Forms.Form
    {
        #region "Member Variables"
        private string m_sFormLocationsRegistryKey;

        private string m_sFileName = null;
        #endregion

        #region "Properties"
        public string FormLocationsRegistryKey
        {
            set
            {
                m_sFormLocationsRegistryKey = value;
            }
        }

        public string FileName
        {
            get
            {
                return m_sFileName;
            }
            set
            {
                m_sFileName = value;
            }
        }
        #endregion

        #region "Constructor"
        public ViewIntelHexForm()
        {
            InitializeComponent();
        }
        #endregion

        #region "Event Handlers"
        private void ViewIntelHexForm_Load(object sender, EventArgs e)
        {
            Pep.Programmer.IntelHexData IntelHexData = null;
            string sErrorMessage = null;
            System.Windows.Forms.ListViewItem ListItem;

            Common.Debug.Thread.IsUIThread();

            if (m_sFileName != null)
            {
                if (Pep.Programmer.IntelHex.LoadIntelHexFile(m_sFileName, ref IntelHexData, ref sErrorMessage))
                {
                    listViewPages.BeginUpdate();

                    for (uint nPage = 0; nPage < IntelHexData.TotalPages; ++nPage)
                    {
                        ListItem = new System.Windows.Forms.ListViewItem(string.Format("{0}", nPage + 1));

                        ListItem.Tag = IntelHexData.PageData(nPage);

                        listViewPages.Items.Add(ListItem);

                        ListItem.SubItems.Add(String.Format("0x{0:X8} - 0x{1:X8}",
                                                            IntelHexData.PageAddress(nPage),
                                                            IntelHexData.PageAddress(nPage) + (IntelHexData.PageData(nPage).Length - 1)));
                    }

                    listViewPages.AutosizeColumns();
                    listViewPages.EndUpdate();

                    buttonView.Enabled = false;
                }
                else
                {
                    buttonView.Enabled = false;
                    listViewPages.Enabled = false;

                    RunOnUIThreadNoWait(() =>
                    {
                        Common.Forms.MessageBox.Show(sErrorMessage,
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Information);
                    });
                }
            }
            else
            {
            }
        }

        private void listViewPages_ItemSelectionChanged(object sender, System.Windows.Forms.ListViewItemSelectionChangedEventArgs e)
        {
            Common.Debug.Thread.IsUIThread();

            buttonView.Enabled = true;
        }

        private void listViewPages_MouseDoubleClick(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            System.Windows.Forms.ListViewHitTestInfo HitTestInfo = listViewPages.HitTest(e.Location);

            Common.Debug.Thread.IsUIThread();

            ViewItem(HitTestInfo.Item);
        }

        private void buttonView_Click(object sender, EventArgs e)
        {
            System.Int32 nIndex = listViewPages.FocusedItem.Index;
            System.Windows.Forms.ListViewItem ListItem = listViewPages.Items[nIndex];

            Common.Debug.Thread.IsUIThread();

            ViewItem(ListItem);
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            Common.Debug.Thread.IsUIThread();

            this.Close();
        }
        #endregion

        #region "Other Helpers"
        private void ViewItem(
            System.Windows.Forms.ListViewItem ListItem)
        {
            Pep.Forms.ViewBufferForm ViewBuffer = new ViewBufferForm();
            byte[] byPageData = (byte[])ListItem.Tag;

            Common.Debug.Thread.IsUIThread();

            new Common.Forms.FormLocation(ViewBuffer, m_sFormLocationsRegistryKey);

            ViewBuffer.Buffer = byPageData;

            ViewBuffer.ShowDialog(this);

            ViewBuffer.Dispose();
        }
        #endregion
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
