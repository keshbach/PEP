/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    private ref class CollectionComparers sealed
    {
    internal:
        ref class StringSortedListComparer :
            System::Collections::Generic::IComparer<System::String^>
        {
        private:
            System::Boolean m_bCaseSensitive;

        internal:
            property System::Boolean CaseSensitive
            {
                System::Boolean get()
                {
                    return m_bCaseSensitive;
                }
            }

        internal:
            StringSortedListComparer(System::Boolean bCaseSensitive);

        public:
            virtual int Compare(System::String^ x, System::String^ y);
        };
    };
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
