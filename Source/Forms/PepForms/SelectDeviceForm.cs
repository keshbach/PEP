/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class SelectDeviceForm : System.Windows.Forms.Form
        {
            private Pep.Programmer.Device m_PepDevice = null;

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

            public SelectDeviceForm()
            {
                InitializeComponent();
            }

            private System.String FormatDevicePinCount(
              System.UInt32 nPinCount)
            {
                return nPinCount.ToString() + " Pins";
            }

            private void SelectDeviceForm_Load(object sender, EventArgs e)
            {
                System.Collections.Generic.List<Pep.Programmer.Device> DeviceList = Pep.Programmer.Devices.DevicesList;
                System.Windows.Forms.TreeNode DeviceTypeTreeNode, PinCountTreeNode;
                System.Windows.Forms.TreeNode NameTreeNode;
                System.String sPinCountName;

                treeViewDevice.BeginUpdate();

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

                treeViewDevice.Sort();
                treeViewDevice.EndUpdate();

                if (m_PepDevice != null)
                {
                    DeviceTypeTreeNode = Common.Forms.TreeView.FindTreeNode(treeViewDevice.Nodes,
                                                                            m_PepDevice.DeviceType,
                                                                            true);

                    sPinCountName = FormatDevicePinCount(m_PepDevice.PinCount);

                    PinCountTreeNode = Common.Forms.TreeView.FindTreeNode(DeviceTypeTreeNode.Nodes,
                                                                          sPinCountName,
                                                                          true);

                    NameTreeNode = Common.Forms.TreeView.FindTreeNode(PinCountTreeNode.Nodes,
                                                                      m_PepDevice.Name,
                                                                      true);

                    treeViewDevice.SelectedNode = NameTreeNode;

                    buttonOK.Enabled = true;
                }
                else
                {
                    buttonOK.Enabled = false;
                }
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

            private void buttonOK_Click(object sender, EventArgs e)
            {
                m_PepDevice = (Pep.Programmer.Device)treeViewDevice.SelectedNode.Tag;
            }

            private void buttonCancel_Click(object sender, EventArgs e)
            {
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/
