/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class ListToolsForm : System.Windows.Forms.Form
        {
            #region "Member Variables"
            private System.Collections.Generic.List<TToolData> m_ToolDataList = new System.Collections.Generic.List<TToolData>();
            #endregion

            #region "Structures"
            public struct TToolData
            {
                public System.String sDescription;
                public System.String sApplication;
                public System.String sArguments;
            }
            #endregion

            #region "Properties"
            public System.Collections.Generic.List<TToolData> ToolDataList
            {
                get
                {
                    return m_ToolDataList;
                }

                set
                {
                    m_ToolDataList = value;
                }
            }
            #endregion

            #region "Constructor"
            public ListToolsForm()
            {
                InitializeComponent();
            }
            #endregion

            #region "Event Handlers"
            private void ListToolsForm_Load(object sender, EventArgs e)
            {
                System.Int32 nIndex = 1;
                System.Windows.Forms.ListViewItem ListItem;

                listViewTools.BeginUpdate();

                foreach (TToolData ToolData in m_ToolDataList)
                {
                    ListItem = listViewTools.Items.Add(nIndex.ToString());

                    ListItem.SubItems.Add(ToolData.sDescription);

                    ListItem.Tag = ToolData;

                    ++nIndex;
                }

                listViewTools.Items[0].Selected = true;

                listViewTools.AutosizeColumns();
                listViewTools.EndUpdate();
            }

            private void listViewTools_ItemSelectionChanged(object sender, System.Windows.Forms.ListViewItemSelectionChangedEventArgs e)
            {
                if (e.ItemIndex > 0)
                {
                    buttonUp.Enabled = true;
                }
                else
                {
                    buttonUp.Enabled = false;
                }

                if (e.ItemIndex < m_ToolDataList.Count - 1)
                {
                    buttonDown.Enabled = true;
                }
                else
                {
                    buttonDown.Enabled = false;
                }
            }

            private void listViewTools_MouseDoubleClick(object sender, System.Windows.Forms.MouseEventArgs e)
            {
                System.Windows.Forms.ListViewHitTestInfo HitTestInfo = listViewTools.HitTest(e.Location);

                EditItem(HitTestInfo.Item);
            }

            private void buttonUp_Click(object sender, EventArgs e)
            {
                System.Int32 nIndex = listViewTools.FocusedItem.Index;
                System.Windows.Forms.ListViewItem ListItem1, ListItem2;
                System.String sItemText;
                System.Object ItemTag;

                ListItem1 = listViewTools.Items[nIndex - 1];
                ListItem2 = listViewTools.Items[nIndex];

                sItemText = ListItem1.SubItems[1].Text;
                ItemTag = ListItem1.Tag;

                listViewTools.BeginUpdate();

                ListItem1.SubItems[1].Text = ListItem2.SubItems[1].Text;
                ListItem1.Tag = ListItem2.Tag;

                ListItem2.SubItems[1].Text = sItemText;
                ListItem2.Tag = ItemTag;

                ListItem1.Selected = true;
                ListItem1.Focused = true;

                listViewTools.EndUpdate();
            }

            private void buttonDown_Click(object sender, EventArgs e)
            {
                System.Int32 nIndex = listViewTools.FocusedItem.Index;
                System.Windows.Forms.ListViewItem ListItem1, ListItem2;
                System.String sItemText;
                System.Object ItemTag;

                ListItem1 = listViewTools.Items[nIndex];
                ListItem2 = listViewTools.Items[nIndex + 1];

                sItemText = ListItem1.SubItems[1].Text;
                ItemTag = ListItem1.Tag;

                listViewTools.BeginUpdate();

                ListItem1.SubItems[1].Text = ListItem2.SubItems[1].Text;
                ListItem1.Tag = ListItem2.Tag;

                ListItem2.SubItems[1].Text = sItemText;
                ListItem2.Tag = ItemTag;

                ListItem2.Selected = true;
                ListItem2.Focused = true;

                listViewTools.EndUpdate();
            }

            private void buttonEdit_Click(object sender, EventArgs e)
            {
                System.Int32 nIndex = listViewTools.FocusedItem.Index;
                System.Windows.Forms.ListViewItem ListItem = listViewTools.Items[nIndex];

                EditItem(ListItem);
            }

            private void buttonClear_Click(object sender, EventArgs e)
            {
                System.Int32 nIndex = listViewTools.FocusedItem.Index;
                System.Windows.Forms.ListViewItem ListItem = listViewTools.Items[nIndex];
                TToolData ToolData = (TToolData)ListItem.Tag;

                ToolData.sDescription = "";
                ToolData.sApplication = "";
                ToolData.sArguments = "";

                listViewTools.BeginUpdate();

                listViewTools.Items[nIndex].SubItems[1].Text = ToolData.sDescription;

                listViewTools.EndUpdate();

                ListItem.Tag = ToolData;
            }

            private void buttonOK_Click(object sender, EventArgs e)
            {
                m_ToolDataList.Clear();

                foreach (System.Windows.Forms.ListViewItem ListViewItem in listViewTools.Items)
                {
                    m_ToolDataList.Add((TToolData)ListViewItem.Tag);
                }
            }
            #endregion

            #region "Other Helpers"
            private void EditItem(
                System.Windows.Forms.ListViewItem ListItem)
            {
                Pep.Forms.ConfigToolForm ConfigTool = new Pep.Forms.ConfigToolForm();
                TToolData ToolData = (TToolData)ListItem.Tag;

                ConfigTool.Description = ToolData.sDescription;
                ConfigTool.Application = ToolData.sApplication;
                ConfigTool.Arguments = ToolData.sArguments;

                if (ConfigTool.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    ToolData.sDescription = ConfigTool.Description;
                    ToolData.sApplication = ConfigTool.Application;
                    ToolData.sArguments = ConfigTool.Arguments;

                    listViewTools.BeginUpdate();

                    ListItem.SubItems[1].Text = ConfigTool.Description;

                    listViewTools.AutosizeColumns();
                    listViewTools.EndUpdate();

                    ListItem.Tag = ToolData;
                }
            }
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2010-2019 Kevin Eshbach                                  */
/***************************************************************************/
