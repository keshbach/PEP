/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
// UiListViewColumnHeader.h

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ListViewColumnHeader
        /// </summary>

        public ref class ListViewColumnHeader : System::Windows::Forms::ColumnHeader
        {
        public:
            enum class EControlType
            {
                Label,
                ComboBox
            };

        public:
            property EControlType ControlType
            {
                EControlType get()
                {
                    return m_ControlType;
                }

                void set(EControlType value)
                {
                    m_ControlType = value;
                }
            }

        public:
            ListViewColumnHeader();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ListViewColumnHeader();

        private:
            EControlType m_ControlType;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
