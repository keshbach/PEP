/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description of ListViewComboBoxEditEventArgs
        /// </summary>

        public ref class ListViewComboBoxEditEventArgs : System::EventArgs
        {
        public:
            /// <summary>
            /// Gets or sets a value indicating whether the edit has been canceled.
            /// </summary>

            property System::Boolean CancelEdit
            {
                System::Boolean get()
                {
                    return m_bCancelEdit;
                }

                void set(System::Boolean value)
                {
                    m_bCancelEdit = value;
                }
            }

            /// <summary>
            /// Gets a value indicating the row being edited.
            /// </summary>

            property System::Int32 Row
            {
                System::Int32 get()
                {
                    return m_Row;
                }
            }

            /// <summary>
            /// Gets a value indicating the column being edited.
            /// </summary>

            property System::Int32 Column
            {
                System::Int32 get()
                {
                    return m_Column;
                }
            }

            /// <summary>
            /// Gets a value indicating the text being edited.
            /// </summary>

            property System::String^ Text
            {
                System::String^ get()
                {
                    return m_sText;
                }
            }

        public:
            ListViewComboBoxEditEventArgs(System::Int32 nRow,
                                          System::Int32 nColumn,
                                          System::String^ sText);

        private:
            System::Boolean m_bCancelEdit;
            System::Int32 m_Row;
            System::Int32 m_Column;
            System::String^ m_sText;

        private:
            ListViewComboBoxEditEventArgs();
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
