/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Pep
{
    namespace Forms
    {
        public partial class SelectPortForm : Form
        {
            #region "Member Variables"
            private System.String m_sLocation;
            private System.String m_sPhysicalDeviceObjectName;

            private System.Collections.Generic.List<Common.ListPorts.Port> m_PortList = new System.Collections.Generic.List<Common.ListPorts.Port>();
            #endregion

            #region "Properties"
            public System.String PortLocation
            {
                get
                {
                    return m_sLocation;
                }
            }

            public System.String PortDeviceObjectName
            {
                get
                {
                    return m_sPhysicalDeviceObjectName;
                }
                set
                {
                    m_sPhysicalDeviceObjectName = value;
                }
            }
            #endregion

            #region "Constructor"
            public SelectPortForm()
            {
                InitializeComponent();
            }
            #endregion

            #region "Event Handlers"
            private void SelectPortForm_Load(object sender, EventArgs e)
            {
                RefreshPorts();
            }

            private void listViewPorts_ItemSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
            {
                buttonOK.Enabled = true;
            }

            private void buttonRefresh_Click(object sender, EventArgs e)
            {
                using (Common.Forms.WaitCursor waitCursor = new Common.Forms.WaitCursor(this))
                {
                    RefreshPorts();
                }
            }

            private void buttonOK_Click(object sender, EventArgs e)
            {
                Common.ListPorts.Port Port = m_PortList[listViewPorts.SelectedIndices[0]];

                m_sPhysicalDeviceObjectName = Port.PhysicalDeviceObjectName;
                m_sLocation = Port.Location;
            }

            private void buttonCancel_Click(object sender, EventArgs e)
            {
            }
            #endregion

            #region "Internal Helpers"
            private void RefreshPorts()
            {
                System.String sPhysicalDeviceObjectName = m_sPhysicalDeviceObjectName;

                if (listViewPorts.SelectedItems.Count > 0)
                {
                    sPhysicalDeviceObjectName = m_PortList[listViewPorts.SelectedIndices[0]].PhysicalDeviceObjectName;
                }

                m_PortList.Clear();

                listViewPorts.BeginUpdate();

                listViewPorts.Items.Clear();

                foreach (Common.ListPorts.Port Port in new Common.ListPorts.Parallel())
                {
                    m_PortList.Add(Port);

                    listViewPorts.Items.Add(Port.Location);
                }

                if (listViewPorts.Items.Count > 0)
                {
                    listViewPorts.Enabled = true;
                    buttonOK.Enabled = false;

                    listViewPorts.AutosizeColumns();

                    for (int nIndex = 0; nIndex < m_PortList.Count; ++nIndex)
                    {
                        if (m_PortList[nIndex].PhysicalDeviceObjectName == sPhysicalDeviceObjectName)
                        {
                            listViewPorts.Items[nIndex].Selected = true;

                            listViewPorts.Items[nIndex].EnsureVisible();

                            buttonOK.Enabled = true;

                            break;
                        }
                    }
                }
                else
                {
                    listViewPorts.Enabled = false;

                    buttonOK.Enabled = false;
                }

                listViewPorts.EndUpdate();
            }
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
