/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class ViewBufferForm : System.Windows.Forms.Form
        {
            #region "Structures"
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
            #endregion

            #region "Member Variables"
            private TOrganizationData[] m_DataOrganization = {
                new TOrganizationData("Ascii",              Pep.Forms.BufferViewer.EDataOrganization.Ascii),
                new TOrganizationData("Byte",               Pep.Forms.BufferViewer.EDataOrganization.Byte),
                new TOrganizationData("Word Big Endian",    Pep.Forms.BufferViewer.EDataOrganization.WordBigEndian),
                new TOrganizationData("Word Little Endian", Pep.Forms.BufferViewer.EDataOrganization.WordLittleEndian)};

            private byte[] m_byBuffer = null;
            private bool m_bReadOnlyMode = false;
            #endregion

            #region "Properties"
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

            public bool ReadOnlyMode
            {
                set
                {
                    m_bReadOnlyMode = value;
                }
            }
            #endregion

            #region "Constructor"
            public ViewBufferForm()
            {
                InitializeComponent();
            }
            #endregion

            #region "Event Handlers"
            private void ViewBufferForm_Load(object sender, EventArgs e)
            {
                Common.Debug.Thread.IsUIThread();

                for (System.Int32 nIndex = 0; nIndex < m_DataOrganization.Length; ++nIndex)
                {
                    comboBoxOrganization.Items.Add(m_DataOrganization[nIndex].sName);
                }

                comboBoxOrganization.AutosizeDropDown();

                bufferViewer.BeginUpdate();

                comboBoxOrganization.SelectedIndex = 1;

                bufferViewer.Buffer = m_byBuffer;

                bufferViewer.EndUpdate();

                if (m_bReadOnlyMode)
                {
                    buttonFill.Visible = false;
                    buttonOK.Visible = false;
                    buttonCancel.Text = "&Close";
                }

                buttonFill.ImageList = Common.Forms.ImageManager.ToolbarSmallImageList;
                buttonFill.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "FillBuffer_16x");
            }

            private void ViewBufferForm_FormClosed(object sender, System.Windows.Forms.FormClosedEventArgs e)
            {
                Common.Debug.Thread.IsUIThread();

                buttonFill.ImageList = null;
            }

            private void comboBoxOrganization_SelectedIndexChanged(object sender, EventArgs e)
            {
                Common.Debug.Thread.IsUIThread();

                bufferViewer.DataOrganization = m_DataOrganization[comboBoxOrganization.SelectedIndex].DataOrganization;
            }

            private void buttonFill_Click(object sender, EventArgs e)
            {
                Pep.Forms.FillBufferForm FillBuffer = new Pep.Forms.FillBufferForm();

                Common.Debug.Thread.IsUIThread();

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
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/
