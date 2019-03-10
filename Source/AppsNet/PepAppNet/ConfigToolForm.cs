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
            internal partial class ConfigToolForm : System.Windows.Forms.Form
            {
                #region "Member Variables"
                private System.String m_sDescription = "";
                private System.String m_sApplication = "";
                private System.String m_sArguments = "";
                #endregion

                #region "Properties"
                public System.String Description
                {
                    get
                    {
                        return m_sDescription;
                    }

                    set
                    {
                        m_sDescription = value;
                    }
                }

                public System.String Application
                {
                    get
                    {
                        return m_sApplication;
                    }

                    set
                    {
                        m_sApplication = value;
                    }
                }

                public System.String Arguments
                {
                    get
                    {
                        return m_sArguments;
                    }

                    set
                    {
                        m_sArguments = value;
                    }
                }
                #endregion

                #region "Constructor"
                public ConfigToolForm()
                {
                    InitializeComponent();
                }
                #endregion

                #region "Event Handlers"
                private void ConfigToolForm_Load(object sender, EventArgs e)
                {
                    textBoxDescription.Text = m_sDescription;
                    textBoxApplication.Text = m_sApplication;
                    textBoxArguments.Text = m_sArguments;

                    UpdateOKBtn();
                }

                private void textBoxDescription_TextChanged(object sender, EventArgs e)
                {
                    UpdateOKBtn();
                }

                private void textBoxArguments_TextChanged(object sender, EventArgs e)
                {
                    UpdateOKBtn();
                }

                private void buttonApplicationEllipse_Click(object sender, EventArgs e)
                {
                    Common.Forms.FileOpenDialog OpenFileDialog = new Common.Forms.FileOpenDialog();
                    System.Collections.Generic.List<Common.Forms.FileTypeItem> FileTypeList = new System.Collections.Generic.List<Common.Forms.FileTypeItem>();
                    Common.Forms.FileTypeItem FileTypeItem;

                    FileTypeItem = new Common.Forms.FileTypeItem("Executable Files", "*.exe");

                    FileTypeList.Add(FileTypeItem);

                    OpenFileDialog.AddToRecentList = false;
                    OpenFileDialog.AllowReadOnly = false;
                    OpenFileDialog.FileTypes = FileTypeList;
                    OpenFileDialog.PickFolders = false;
                    OpenFileDialog.SelectedFileType = 1;
                    OpenFileDialog.SelectMultipleItems = false;
                    OpenFileDialog.ShowHidden = false;

                    if (OpenFileDialog.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                    {
                        textBoxApplication.Text = OpenFileDialog.FileName;
                    }

                    UpdateOKBtn();
                }

                private void buttonOK_Click(object sender, EventArgs e)
                {
                    m_sDescription = textBoxDescription.Text;
                    m_sApplication = textBoxApplication.Text;
                    m_sArguments = textBoxArguments.Text;
                }
                #endregion

                #region "Other Helpers"
                private void UpdateOKBtn()
                {
                    System.Boolean bEnable = false;

                    if (textBoxDescription.Text.Length > 0 &&
                        textBoxApplication.Text.Length > 0 &&
                        textBoxArguments.Text.Length > 0)
                    {
                        bEnable = true;
                    }

                    buttonOK.Enabled = bEnable;
                }
                #endregion
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2010-2014 Kevin Eshbach                                  */
/***************************************************************************/
