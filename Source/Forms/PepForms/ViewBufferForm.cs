/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class ViewBufferForm : System.Windows.Forms.Form
        {
            private struct TOrganizationData
            {
                public System.String sName;
                public Pep.Forms.BufferViewer.EDataOrganization DataOrganization;

                public TOrganizationData(
                    System.String sNameVal,
                    Pep.Forms.BufferViewer.EDataOrganization DataOrganizationVal)
                {
                    sName = sNameVal;
                    DataOrganization = DataOrganizationVal;
                }
            }

            private TOrganizationData[] m_DataOrganization = {
            new TOrganizationData("Ascii",              Pep.Forms.BufferViewer.EDataOrganization.Ascii),
            new TOrganizationData("Byte",               Pep.Forms.BufferViewer.EDataOrganization.Byte),
            new TOrganizationData("Word Big Endian",    Pep.Forms.BufferViewer.EDataOrganization.WordBigEndian),
            new TOrganizationData("Word Little Endian", Pep.Forms.BufferViewer.EDataOrganization.WordLittleEndian)};
            private byte[] m_byBuffer = null;

            public byte[] Buffer
            {
                get
                {
                    return m_byBuffer;
                }
                set
                {
                    m_byBuffer = new byte[value.Length];

                    for (System.Int32 nIndex = 0; nIndex < m_byBuffer.Length; ++nIndex)
                    {
                        m_byBuffer[nIndex] = value[nIndex];
                    }
                }
            }

            public ViewBufferForm()
            {
                InitializeComponent();
            }

            private void ViewBufferForm_Load(object sender, EventArgs e)
            {
                for (System.Int32 nIndex = 0; nIndex < m_DataOrganization.Length; ++nIndex)
                {
                    comboBoxOrganization.Items.Add(m_DataOrganization[nIndex].sName);
                }

                comboBoxOrganization.AutosizeDropDown();

                bufferViewer.BeginUpdate();

                comboBoxOrganization.SelectedIndex = 1;

                bufferViewer.Buffer = m_byBuffer;

                bufferViewer.EndUpdate();
            }

            private void comboBoxOrganization_SelectedIndexChanged(object sender, EventArgs e)
            {
                bufferViewer.DataOrganization = m_DataOrganization[comboBoxOrganization.SelectedIndex].DataOrganization;
            }

            private void buttonFill_Click(object sender, EventArgs e)
            {
                Pep.Forms.FillBufferForm FillBuffer = new Pep.Forms.FillBufferForm();

                if (FillBuffer.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    for (System.Int32 nIndex = 0; nIndex < m_byBuffer.Length; ++nIndex)
                    {
                        m_byBuffer[nIndex] = FillBuffer.Value;
                    }

                    bufferViewer.BeginUpdate();

                    bufferViewer.Buffer = m_byBuffer;

                    bufferViewer.EndUpdate();
                }
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/
