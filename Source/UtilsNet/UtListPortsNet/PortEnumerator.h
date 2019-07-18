/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace ListPorts
    {
        public ref class Enumerator sealed :
            System::Collections::Generic::IEnumerator<Common::ListPorts::Port^>
        {
        public:
            property Common::ListPorts::Port^ Current
            {
                virtual Common::ListPorts::Port^ get() = System::Collections::Generic::IEnumerator<Common::ListPorts::Port^>::Current::get
                {
                    if (m_bAtEndOfList)
                    {
                        throw gcnew System::Exception(L"Port enumerator has been closed.");
                    }

                    return m_PortList[m_nCurrentPortIndex];
                }
            }

            property System::Object^ CurrentBase
            {
                virtual System::Object^ get() = System::Collections::IEnumerator::Current::get
                {
                    return Current;
                }
            }

        public:
            virtual System::Boolean MoveNext(void);
            virtual void Reset(void);

        internal:
            Enumerator(System::Collections::Generic::List<Common::ListPorts::Port^>^ ZipItemList);

        private:
            Enumerator();
            ~Enumerator();

        private:
            System::Collections::Generic::List<Common::ListPorts::Port^>^ m_PortList;
            System::Int32 m_nCurrentPortIndex;
            System::Boolean m_bAtEndOfList;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
