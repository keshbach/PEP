/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description of MenuStatusStripEventArgs
        /// </summary>

        public ref class MenuStatusStripEventArgs : System::EventArgs
        {
        public:
            property System::Windows::Forms::StatusStrip^ StatusStrip
            {
                System::Windows::Forms::StatusStrip^ get()
                {
                    return m_StatusStrip;
                }
            }

        public:
            MenuStatusStripEventArgs(System::Windows::Forms::StatusStrip^ StatusStrip);

        private:
            MenuStatusStripEventArgs();

        private:
            System::Windows::Forms::StatusStrip^ m_StatusStrip;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
