/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class SelectDeviceForm : System.Windows.Forms.Form
        {
            #region "Delegates"
            private delegate void DelegateRefreshDevices();
            private DelegateRefreshDevices m_DelegateRefreshDevices;
            #endregion

            #region Constants
            private static System.String CAllFilterType = "All";

            private static System.String CDevicePinsText = "{0} Pins";
            #endregion

            #region Member Variables
            private Pep.Programmer.Device m_PepDevice = null;

            private System.String[] m_Filters = null;

            private System.Boolean m_bIgnoreItemCheck = false;
            #endregion

            #region Properties
            public Pep.Programmer.Device PepDevice
            {
                get
                {
                    return m_PepDevice;
                }

                set
                {
                    m_PepDevice = value;
                }
            }
            #endregion

            #region Constructor
            public SelectDeviceForm()
            {
                m_DelegateRefreshDevices = new DelegateRefreshDevices(OnRefreshDevices);

                InitializeComponent();
            }
            #endregion

            #region SelectDeviceForm Event Handlers
            private void SelectDevice2Form_Load(object sender, EventArgs e)
            {
                System.Collections.Generic.SortedSet<System.String> FilterSet = new System.Collections.Generic.SortedSet<System.String>();

                foreach (Pep.Programmer.Device Device in Pep.Programmer.Devices.DevicesList)
                {
                    FilterSet.Add(Device.DeviceType);
                }

                m_Filters = new System.String[FilterSet.Count];

                FilterSet.CopyTo(m_Filters);

                checkedListBoxFilter.Items.Add(CAllFilterType);
                checkedListBoxFilter.Items.AddRange(m_Filters);
                checkedListBoxFilter.SetItemChecked(0, true);

                RefreshDevices();
            }

            private void treeViewDevice_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
            {
                Pep.Programmer.Device PepDevice = (Pep.Programmer.Device)treeViewDevice.SelectedNode.Tag;

                if (PepDevice != null)
                {
                    textBoxPinDiagram.Text = PepDevice.PinNamesDiagram;

                    buttonOK.Enabled = true;
                }
                else
                {
                    textBoxPinDiagram.Text = "";

                    buttonOK.Enabled = false;
                }
            }

            private void treeViewDevice_MouseDoubleClick(object sender, System.Windows.Forms.MouseEventArgs e)
            {
                if (buttonOK.Enabled)
                {
                    buttonOK.PerformClick();
                }
            }

            private void textBoxSearch_KeyPressTimerExpired(object sender, EventArgs e)
            {
                RefreshDevices();
            }

            private void checkedListBoxFilter_ItemCheck(object sender, System.Windows.Forms.ItemCheckEventArgs e)
            {
                if (m_bIgnoreItemCheck)
                {
                    return;
                }

                if (e.Index == 0)
                {
                    if (e.NewValue == System.Windows.Forms.CheckState.Checked)
                    {
                        m_bIgnoreItemCheck = true;

                        for (System.Int32 nIndex = 1; nIndex < checkedListBoxFilter.Items.Count; ++nIndex)
                        {
                            checkedListBoxFilter.SetItemChecked(nIndex, true);
                        }

                        m_bIgnoreItemCheck = false;

                        BeginInvoke(m_DelegateRefreshDevices, new object[] { });
                    }
                }
                else
                {
                    if (e.NewValue == System.Windows.Forms.CheckState.Unchecked)
                    {
                        m_bIgnoreItemCheck = true;

                        checkedListBoxFilter.SetItemChecked(0, false);

                        m_bIgnoreItemCheck = false;
                    }

                    BeginInvoke(m_DelegateRefreshDevices, new object[] {});
                }
            }

            private void buttonOK_Click(object sender, EventArgs e)
            {
                m_PepDevice = (Pep.Programmer.Device)treeViewDevice.SelectedNode.Tag;
            }

            private void buttonCancel_Click(object sender, EventArgs e)
            {
            }
            #endregion

            #region Internal Helpers
            private void RefreshDevices()
            {
                System.Collections.Generic.List<Pep.Programmer.Device> DeviceList = new System.Collections.Generic.List<Pep.Programmer.Device>();

                foreach (Pep.Programmer.Device Device in Pep.Programmer.Devices.DevicesList)
                {
                    if (IsDeviceAllowed(Device))
                    {
                        DeviceList.Add(Device);
                    }
                }

                UpdateDevices(DeviceList);
            }

            private void UpdateDevices(
                System.Collections.Generic.List<Pep.Programmer.Device> DeviceList)
            {
                System.Windows.Forms.TreeNode NameTreeNode = null;
                System.Windows.Forms.TreeNode DeviceTypeTreeNode, PinCountTreeNode;
                System.String sPinCountName;

                treeViewDevice.BeginUpdate();

                treeViewDevice.Nodes.Clear();

                foreach (Pep.Programmer.Device PepDevice in DeviceList)
                {
                    DeviceTypeTreeNode = Common.Forms.TreeView.FindTreeNode(treeViewDevice.Nodes,
                                                                            PepDevice.DeviceType,
                                                                            true);

                    if (DeviceTypeTreeNode == null)
                    {
                        DeviceTypeTreeNode = treeViewDevice.Nodes.Add(PepDevice.DeviceType);
                    }

                    sPinCountName = FormatDevicePinCount(PepDevice.PinCount);

                    PinCountTreeNode = Common.Forms.TreeView.FindTreeNode(DeviceTypeTreeNode.Nodes,
                                                                          sPinCountName,
                                                                          true);

                    if (PinCountTreeNode == null)
                    {
                        PinCountTreeNode = DeviceTypeTreeNode.Nodes.Add(sPinCountName);
                    }

                    NameTreeNode = PinCountTreeNode.Nodes.Add(PepDevice.Name);

                    NameTreeNode.Tag = PepDevice;
                }

                if (!String.IsNullOrWhiteSpace(textBoxSearch.Text))
                {
                    ExpandTreeNodes();
                }

                treeViewDevice.Sort();
                treeViewDevice.EndUpdate();

                NameTreeNode = null;

                if (m_PepDevice != null)
                {
                    DeviceTypeTreeNode = Common.Forms.TreeView.FindTreeNode(treeViewDevice.Nodes,
                                                                            m_PepDevice.DeviceType,
                                                                            true);

                    if (DeviceTypeTreeNode != null)
                    {
                        sPinCountName = FormatDevicePinCount(m_PepDevice.PinCount);

                        PinCountTreeNode = Common.Forms.TreeView.FindTreeNode(DeviceTypeTreeNode.Nodes,
                                                                              sPinCountName,
                                                                              true);

                        if (PinCountTreeNode != null)
                        {
                            NameTreeNode = Common.Forms.TreeView.FindTreeNode(PinCountTreeNode.Nodes,
                                                                              m_PepDevice.Name,
                                                                              true);
                        }
                    }
                }

                if (NameTreeNode != null)
                {
                    treeViewDevice.SelectedNode = NameTreeNode;

                    treeViewDevice.SelectedNode.EnsureVisible();

                    buttonOK.Enabled = true;
                }
                else
                {
                    textBoxPinDiagram.Text = "";

                    buttonOK.Enabled = false;
                }
            }

            private void ExpandTreeNodes()
            {
                foreach (System.Windows.Forms.TreeNode DeviceTypeTreeNode in treeViewDevice.Nodes)
                {
                    DeviceTypeTreeNode.Expand();

                    foreach (System.Windows.Forms.TreeNode PinCountTreeNode in DeviceTypeTreeNode.Nodes)
                    {
                        PinCountTreeNode.Expand();
                    }
                }
            }

            private System.Boolean IsDeviceAllowed(
                Pep.Programmer.Device Device)
            {
                if (!String.IsNullOrWhiteSpace(textBoxSearch.Text))
                {
                    if (!Device.Name.ToLower().Contains(textBoxSearch.Text.ToLower()))
                    {
                        return false;
                    }
                }

                for (System.Int32 nIndex = 0; nIndex < m_Filters.Length; ++nIndex)
                {
                    if (System.String.Compare(Device.DeviceType, m_Filters[nIndex]) == 0)
                    {
                        return checkedListBoxFilter.GetItemChecked(nIndex + 1);
                    }
                }

                return false;
            }

            private void OnRefreshDevices()
            {
                RefreshDevices();
            }

            private static System.String FormatDevicePinCount(
              System.UInt32 nPinCount)
            {
                return System.String.Format(CDevicePinsText, nPinCount);
            }
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
