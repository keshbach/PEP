/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for FileTypeItem.
        /// </summary>

        public ref class FileTypeItem
        {
        public:
            /// <summary>
            /// Name of the file type
            /// </summary>

            property System::String^ Name
            {
                System::String^ get()
                {
                    return m_sName;
                }
            }

            /// <summary>
            /// Filters for the file type
            /// </summary>

            property System::String^ Filters
            {
                System::String^ get()
                {
                    return m_sFilters;
                }
            }

        public:
            FileTypeItem(System::String^ sName,
                         System::String^ sFilters);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~FileTypeItem();

            FileTypeItem();

        private:
            System::String^ m_sName;
            System::String^ m_sFilters;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2013-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
