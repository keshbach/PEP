/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for TreeNodeDescendingSorter.
        /// </summary>

        private ref class TreeNodeDescendingSorter :
            System::Collections::IComparer
        {
        public:
            virtual System::Int32 Compare(System::Object^ Object1, System::Object^ Object2);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
