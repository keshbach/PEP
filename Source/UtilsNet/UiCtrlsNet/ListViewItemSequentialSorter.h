/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ListViewItemSequentialSorter.
        /// </summary>

        private ref class ListViewItemSequentialSorter :
            System::Collections::IComparer
        {
        public:
            ListViewItemSequentialSorter();
            ListViewItemSequentialSorter(System::Int32 nColumn);

            virtual System::Int32 Compare(System::Object^ Object1, System::Object^ Object2);

        private:
            System::Int32 m_nColumn;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
