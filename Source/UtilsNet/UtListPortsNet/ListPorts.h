/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace ListPorts
    {
        public ref class Parallel sealed :
            System::Collections::Generic::IEnumerable<Common::ListPorts::Port^>
        {
        public:
            // System::Collections::Generic::IEnumerable<>
            virtual System::Collections::Generic::IEnumerator<Common::ListPorts::Port^>^ GetEnumerator() = System::Collections::Generic::IEnumerable<Common::ListPorts::Port^>::GetEnumerator;
            virtual System::Collections::IEnumerator^ GetEnumeratorBase() = System::Collections::IEnumerable::GetEnumerator;

        public:
            Parallel();
            ~Parallel();
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
