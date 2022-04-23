/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description of TreeViewLabelEditPasteEventArgs
        /// </summary>

        public ref class TreeViewLabelEditPasteEventArgs : System::EventArgs
        {
        public:
            /// <summary>
            /// Gets or sets a value indicating the text to paste.
            /// </summary>

            property System::String^ TextPaste
            {
                System::String^ get()
                {
                    return m_sTextPaste;
                }

                void set(System::String^ value)
                {
                    m_sTextPaste = value;
                }
            }

        public:
            TreeViewLabelEditPasteEventArgs();

        private:
            System::String^ m_sTextPaste;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
