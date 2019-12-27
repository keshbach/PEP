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

            private System.Byte m_byMaxValue = System.Byte.MaxValue;
            #endregion

            #region "Properties"
            public System.Byte Value
            {
                get
                {
                    return m_byValue;
                }
            }

            public System.Byte MaxValue
            {
                get
                {
                    return m_byMaxValue;
                }
                set
                {
                    m_byMaxValue = value;
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
                labelInstructions.Text = System.String.Format("(Enter a value from 0 to {0}.)", m_byMaxValue);

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

                    if (nValue <= (System.Int16)m_byMaxValue)
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
