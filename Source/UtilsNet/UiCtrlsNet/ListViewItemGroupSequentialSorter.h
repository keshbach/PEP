/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ListViewItemGroupSequentialSorter.
        /// </summary>

        private ref class ListViewItemGroupSequentialSorter :
            System::Collections::IComparer
        {
        public:
            ListViewItemGroupSequentialSorter();
            ListViewItemGroupSequentialSorter(System::Int32 nColumn);

            virtual System::Int32 Compare(System::Object^ Object1, System::Object^ Object2);

            System::Int32 FindMatchingChars(System::String^ String1, System::String^ String2);
            bool CompareNumbers(System::String^ String1, System::String^ String2, System::Int32* pnResult);

        private:
            System::Int32 m_nColumn;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
