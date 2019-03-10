/***************************************************************************/
/*  Copyright (C) 2009-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class CfgDeviceForm : System.Windows.Forms.Form
        {
            private delegate void DelegateListViewChanged();
            private DelegateListViewChanged m_DelegateListViewChanged;

            private System.String COutputType = "Output";
            private System.String CRegisteredType = "Registered";
            private System.String CNoPolarity = "----";
            private Pep.Forms.CfgDevicePin[] m_CfgDevicePinsArray = null;

            public Pep.Forms.CfgDevicePin[] CfgDevicePins
            {
                get
                {
                    return m_CfgDevicePinsArray;
                }

                set
                {
                    m_CfgDevicePinsArray = value;
                }
            }

            public CfgDeviceForm()
            {
                m_DelegateListViewChanged = new DelegateListViewChanged(OnListViewChanged);

                InitializeComponent();
            }

            private void ListViewChanged()
            {
                BeginInvoke(m_DelegateListViewChanged);
            }

            private void OnListViewChanged()
            {
                listViewPins.BeginUpdate();
                listViewPins.AutosizeColumns();
                listViewPins.EndUpdate();

                VerifyPinValues();
            }

            private void CfgDeviceForm_Load(
                object sender,
                EventArgs e)
            {
                CfgDevicePin CfgDevicePin;
                System.Windows.Forms.ListViewItem ListViewItem;

                listViewPins.BeginUpdate();

                if (m_CfgDevicePinsArray != null)
                {
                    for (System.Int32 nIndex = 0; nIndex < m_CfgDevicePinsArray.Length;
                         ++nIndex)
                    {
                        CfgDevicePin = m_CfgDevicePinsArray[nIndex];

                        ListViewItem = listViewPins.Items.Add(CfgDevicePin.PinNumber.ToString());

                        ListViewItem.SubItems.Add(CfgDevicePin.SelectedType);

                        if (CfgDevicePin.Polarities != null &&
                            CfgDevicePin.SelectedType == COutputType)
                        {
                            ListViewItem.SubItems.Add(CfgDevicePin.SelectedPolarity);
                        }
                        else
                        {
                            ListViewItem.SubItems.Add(CNoPolarity);
                        }
                    }
                }

                listViewPins.AutosizeColumns();
                listViewPins.EndUpdate();

                VerifyPinValues();
            }

            private void listViewPins_BeforeComboBoxEdit(
                object sender,
                Common.Forms.ListViewComboBoxEditEventArgs e)
            {
                Pep.Forms.CfgDevicePin CfgDevicePin = m_CfgDevicePinsArray[e.Row - 1];
                Common.Collections.StringCollection StringColl;

                switch (e.Column)
                {
                    case 1:
                        e.CancelEdit = true;
                        break;
                    case 2:
                        StringColl = Common.Collections.StringCollection.FromArray(CfgDevicePin.Types);

                        listViewPins.ComboBoxItems = StringColl;
                        break;
                    case 3:
                        if (CfgDevicePin.Polarities != null)
                        {
                            if (listViewPins.Items[e.Row - 1].SubItems[1].Text == COutputType ||
                                listViewPins.Items[e.Row - 1].SubItems[1].Text == CRegisteredType)
                            {
                                StringColl = Common.Collections.StringCollection.FromArray(CfgDevicePin.Polarities);

                                listViewPins.ComboBoxItems = StringColl;
                            }
                            else
                            {
                                e.CancelEdit = true;
                            }
                        }
                        else
                        {
                            e.CancelEdit = true;
                        }
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false);
                        break;
                }
            }

            private void listViewPins_AfterComboBoxEdit(
                object sender,
                Common.Forms.ListViewComboBoxEditEventArgs e)
            {
                Pep.Forms.CfgDevicePin CfgDevicePin = m_CfgDevicePinsArray[e.Row - 1];

                if (e.CancelEdit == false && e.Text != null)
                {
                    switch (e.Column)
                    {
                        case 2:
                            if (e.Text == COutputType || e.Text == CRegisteredType)
                            {
                                if (listViewPins.Items[e.Row - 1].SubItems[2].Text.Length == 0 ||
                                    listViewPins.Items[e.Row - 1].SubItems[2].Text == CNoPolarity)
                                {
                                    if (CfgDevicePin.Polarities.Length == 1)
                                    {
                                        listViewPins.Items[e.Row - 1].SubItems[2].Text = CfgDevicePin.Polarities[0];
                                    }
                                    else
                                    {
                                        listViewPins.Items[e.Row - 1].SubItems[2].Text = "";
                                    }
                                }
                            }
                            else
                            {
                                listViewPins.Items[e.Row - 1].SubItems[2].Text = CNoPolarity;
                            }
                            break;
                        case 3:
                            break;
                        default:
                            System.Diagnostics.Debug.Assert(false);
                            break;
                    }

                    ListViewChanged();
                }
            }

            private void buttonOK_Click(object sender, EventArgs e)
            {
                for (System.Int32 nIndex = 0; nIndex < m_CfgDevicePinsArray.Length;
                     ++nIndex)
                {
                    m_CfgDevicePinsArray[nIndex].SelectedType = listViewPins.Items[nIndex].SubItems[1].Text;

                    if (m_CfgDevicePinsArray[nIndex].Polarities != null &&
                        listViewPins.Items[nIndex].SubItems[2].Text != CNoPolarity)
                    {
                        m_CfgDevicePinsArray[nIndex].SelectedPolarity = listViewPins.Items[nIndex].SubItems[2].Text;
                    }
                    else
                    {
                        m_CfgDevicePinsArray[nIndex].SelectedPolarity = "";
                    }
                }
            }

            private void buttonCancel_Click(object sender, EventArgs e)
            {
            }

            private void VerifyPinValues()
            {
                System.Boolean bValid = true;

                for (System.Int32 nIndex = 0; nIndex < m_CfgDevicePinsArray.Length;
                     ++nIndex)
                {
                    if (listViewPins.Items[nIndex].SubItems[1].Text.Length == 0 ||
                        listViewPins.Items[nIndex].SubItems[2].Text.Length == 0)
                    {
                        bValid = false;
                    }

                    if (m_CfgDevicePinsArray[nIndex].Polarities != null)
                    {
                        if (listViewPins.Items[nIndex].SubItems[2].Text == COutputType ||
                            listViewPins.Items[nIndex].SubItems[2].Text == CRegisteredType)
                        {
                            if (listViewPins.Items[nIndex].SubItems[3].Text.Length == 0)
                            {
                                bValid = false;
                            }
                        }
                    }
                }

                buttonOK.Enabled = bValid;
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2009-2014 Kevin Eshbach                                  */
/***************************************************************************/
