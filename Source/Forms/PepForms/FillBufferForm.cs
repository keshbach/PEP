/***************************************************************************/
/*  Copyright (C) 2013-2019 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class FillBufferForm : System.Windows.Forms.Form
        {
            #region "Member Variables"
            private System.Byte m_byValue = 0;
            #endregion

            #region "Properties"
            public System.Byte Value
            {
                get
                {
                    return m_byValue;
                }
            }
            #endregion

            #region "Constructor"
            public FillBufferForm()
            {
                InitializeComponent();
            }
            #endregion

            #region "Event Handlers"
            private void FillBufferForm_Load(object sender, EventArgs e)
            {
                VerifyValue();
            }

            private void maskedTextBoxValue_TextChanged(object sender, EventArgs e)
            {
                VerifyValue();
            }

            private void buttonOK_Click(object sender, EventArgs e)
            {
                m_byValue = System.Convert.ToByte(maskedTextBoxValue.Text);
            }

            private void buttonCancel_Click(object sender, EventArgs e)
            {
            }
            #endregion

            #region "Internal Helpers"
            private void VerifyValue()
            {
                System.Boolean bEnable = false;
                System.Int16 nValue;

                if (maskedTextBoxValue.Text.Length > 0)
                {
                    nValue = System.Convert.ToInt16(maskedTextBoxValue.Text);

                    if (nValue <= (System.Int16)System.Byte.MaxValue)
                    {
                        bEnable = true;
                    }
                }

                buttonOK.Enabled = bEnable;
            }
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2013-2019 Kevin Eshbach                                  */
/***************************************************************************/
