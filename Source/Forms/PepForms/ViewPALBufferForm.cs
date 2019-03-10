/***************************************************************************/
/*  Copyright (C) 2011-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class ViewPALBufferForm : System.Windows.Forms.Form
        {
            private System.String m_sBuffer = null;

            public System.String Buffer
            {
                get
                {
                    return m_sBuffer;
                }
                set
                {
                    m_sBuffer = value;
                }
            }

            public ViewPALBufferForm()
            {
                InitializeComponent();
            }

            private void ViewPALBufferForm_Load(object sender, EventArgs e)
            {
                textBoxBuffer.Text = m_sBuffer;
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2011-2014 Kevin Eshbach                                  */
/***************************************************************************/
