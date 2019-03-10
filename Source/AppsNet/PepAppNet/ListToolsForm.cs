/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Application
    {
        namespace Forms
        {
            internal partial class ListToolsForm : System.Windows.Forms.Form
            {
                #region "Member Variables"
                private System.Collections.ArrayList m_ToolDataCollection = new System.Collections.ArrayList();
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
                public System.Collections.ArrayList ToolDataCollection
                {
                    get
                    {
                        return m_ToolDataCollection;
                    }

                    set
                    {
                        m_ToolDataCollection = value;
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
                    System.Collections.IEnumerator Enum = m_ToolDataCollection.GetEnumerator();
                    System.Int32 nIndex = 1;
                    System.Windows.Forms.ListViewItem ListItem;
                    TToolData ToolData;

                    listViewTools.BeginUpdate();

                    while (Enum.MoveNext())
                    {
                        ToolData = (TToolData)Enum.Current;

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

                    if (e.ItemIndex < m_ToolDataCollection.Count - 1)
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
                    System.Windows.Forms.ListViewHitTestInfo HitTestInfo;

                    HitTestInfo = listViewTools.HitTest(e.Location);

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
                    System.Windows.Forms.ListViewItem ListItem;

                    ListItem = (System.Windows.Forms.ListViewItem)listViewTools.Items[nIndex];

                    EditItem(ListItem);
                }

                private void buttonClear_Click(object sender, EventArgs e)
                {
                    System.Int32 nIndex = listViewTools.FocusedItem.Index;
                    System.Windows.Forms.ListViewItem ListItem;
                    TToolData ToolData;

                    ListItem = (System.Windows.Forms.ListViewItem)listViewTools.Items[nIndex];

                    ToolData = (TToolData)ListItem.Tag;

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
                    System.Collections.IEnumerator Enum;
                    System.Windows.Forms.ListViewItem ListItem;

                    m_ToolDataCollection.Clear();

                    Enum = listViewTools.Items.GetEnumerator();

                    while (Enum.MoveNext())
                    {
                        ListItem = (System.Windows.Forms.ListViewItem)Enum.Current;

                        m_ToolDataCollection.Add(ListItem.Tag);
                    }
                }
                #endregion

                #region "Other Helpers"
                private void EditItem(
                    System.Windows.Forms.ListViewItem ListItem)
                {
                    Pep.Application.Forms.ConfigToolForm ConfigTool = new Pep.Application.Forms.ConfigToolForm();
                    TToolData ToolData;

                    ToolData = (TToolData)ListItem.Tag;

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
}

/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/
