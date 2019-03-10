/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for WaitCursor.
        /// </summary>

        public ref class WaitCursor sealed
        {
        public:
            WaitCursor(System::Windows::Forms::Control^ pControl);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~WaitCursor();

        private:
            System::Windows::Forms::Control^ m_pControl;
            System::Windows::Forms::Cursor^ m_pPreviousCursor;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
