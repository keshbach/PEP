/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace OpenZip.Forms
{
    internal partial class FileConfirmationForm : System.Windows.Forms.Form
    {
        public enum EConfirmationMode
        {
            Copy,
            Move
        };

        public enum EConfirmationChoice
        {
            CopyAndReplace,
            DontCopy,
            CopyButKeepBothFiles,
            MoveAndReplace,
            DontMove,
            MoveButKeepBothFiles,
            None
        };

        private EConfirmationMode m_ConfirmationMode = EConfirmationMode.Copy;
        private EConfirmationChoice m_ConfirmationChoice = EConfirmationChoice.None;
        private System.String m_sNewFileName = "";

        public EConfirmationMode ConfirmationMode
        {
            get
            {
                return m_ConfirmationMode;
            }
            set
            {
                m_ConfirmationMode = value;
            }
        }

        public EConfirmationChoice ConfirmationChoice
        {
            get
            {
                return m_ConfirmationChoice;
            }
        }

        public System.String NewFileName
        {
            set
            {
                m_sNewFileName = value;
            }
        }

        public FileConfirmationForm()
        {
            InitializeComponent();
        }

        private void FileConfirmationForm_Load(
            object sender,
            System.EventArgs e)
        {
            System.Text.StringBuilder sb = new System.Text.StringBuilder();

            sb.Append("(The file being ");

            if (m_ConfirmationMode == EConfirmationMode.Copy)
            {
                // Use defaults from designer mode

                sb.Append("copied");
            }
            else if (m_ConfirmationMode == EConfirmationMode.Move)
            {
                sb.Append("moved");

                Text = "Move File";

                radioButtonCopyAndReplace.Text = "Move and Replace";
                radioButtonCopyButKeepBothFiles.Text = "Move, but keep both files.";

                labelCopyAndReplace.Text = "(Replace the file in the destination folder with the file you are moving.)";
            }
            else
            {
                System.Diagnostics.Debug.Assert(false, "Unknown confirmation mode.");
            }

            sb.Append(" will be renamed \"");
            sb.Append(m_sNewFileName);
            sb.Append("\".)");

            labelCopyButKeepBothFiles.Text = sb.ToString();
        }

        private void buttonOK_Click(
            object sender,
            System.EventArgs e)
        {
            if (m_ConfirmationMode == EConfirmationMode.Copy)
            {
                if (radioButtonCopyAndReplace.Checked)
                {
                    m_ConfirmationChoice = EConfirmationChoice.CopyAndReplace;
                }
                else if (radioButtonCopyButKeepBothFiles.Checked)
                {
                    m_ConfirmationChoice = EConfirmationChoice.CopyButKeepBothFiles;
                }
                else
                {
                    System.Diagnostics.Debug.Assert(false, "Unknown copy radio button confirmation choice.");
                }
            }
            else if (m_ConfirmationMode == EConfirmationMode.Move)
            {
                if (radioButtonCopyAndReplace.Checked)
                {
                    m_ConfirmationChoice = EConfirmationChoice.MoveAndReplace;
                }
                else if (radioButtonCopyButKeepBothFiles.Checked)
                {
                    m_ConfirmationChoice = EConfirmationChoice.MoveButKeepBothFiles;
                }
                else
                {
                    System.Diagnostics.Debug.Assert(false, "Unknown move radio button confirmation choice.");
                }
            }
            else
            {
                System.Diagnostics.Debug.Assert(false, "Unknown confirmation mode.");
            }
        }

        private void buttonCancel_Click(
            object sender,
            System.EventArgs e)
        {
            if (m_ConfirmationMode == EConfirmationMode.Copy)
            {
                m_ConfirmationChoice = EConfirmationChoice.DontCopy;
            }
            else if (m_ConfirmationMode == EConfirmationMode.Move)
            {
                m_ConfirmationChoice = EConfirmationChoice.DontMove;
            }
            else
            {
                System.Diagnostics.Debug.Assert(false, "Unknown confirmation mode.");
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/
