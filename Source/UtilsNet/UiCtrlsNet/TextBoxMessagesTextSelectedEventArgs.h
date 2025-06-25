/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description of TextBoxMessagesTextSelectedEventArgs
        /// </summary>

        public ref class TextBoxMessagesTextSelectedEventArgs : System::EventArgs
        {
        public:
            /// <summary>
            /// Gets or sets a value indicating if any text is selected.
            /// </summary>

            property System::Boolean TextSelected
            {
                System::Boolean get()
                {
                    return m_bTextSelected;
                }
            }

        public:
            TextBoxMessagesTextSelectedEventArgs(System::Boolean bSelected);

        private:
            TextBoxMessagesTextSelectedEventArgs();

        private:
            System::Boolean m_bTextSelected;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
