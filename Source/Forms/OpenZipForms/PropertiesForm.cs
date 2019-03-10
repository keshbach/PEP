/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace OpenZip
{
    namespace Forms
    {
        internal partial class PropertiesForm : System.Windows.Forms.Form
        {
            public enum EPropertiesMode
            {
                Zip,
                File
            };

            private EPropertiesMode m_PropertiesMode = EPropertiesMode.File;
            private System.String m_sFileName = "";
            private System.String m_sComment = "";
            private System.Boolean m_bReadOnlyFile = false;

            public EPropertiesMode PropertiesMode
            {
                get
                {
                    return m_PropertiesMode;
                }
                set
                {
                    m_PropertiesMode = value;
                }
            }

            public System.String FileName
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

            public System.String Comment
            {
                get
                {
                    return m_sComment;
                }
                set
                {
                    m_sComment = value;
                }
            }

            public System.Boolean ReadOnlyFile
            {
                get
                {
                    return m_bReadOnlyFile;
                }
                set
                {
                    m_bReadOnlyFile = value;
                }
            }

            public PropertiesForm()
            {
                InitializeComponent();
            }

            private void PropertiesForm_Load(object sender, EventArgs e)
            {
                switch (m_PropertiesMode)
                {
                    case EPropertiesMode.Zip:
                        this.Text = "Zip Properties";
                        break;
                    case EPropertiesMode.File:
                        this.Text = m_sFileName + " Properties";
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false);
                        break;
                }

                textBoxComment.Text = m_sComment;
            }

            private void buttonOK_Click(object sender, EventArgs e)
            {
                m_sComment = textBoxComment.Text;
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/
