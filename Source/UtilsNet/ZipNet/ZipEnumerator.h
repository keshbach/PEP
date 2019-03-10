/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Zip
    {
        public ref class Enumerator sealed :
            System::Collections::Generic::IEnumerator<Common::Zip::Item^>
        {
        public:
            property Common::Zip::Item^ Current
            {
                virtual Common::Zip::Item^ get() = System::Collections::Generic::IEnumerator<Common::Zip::Item^>::Current::get
                {
                    if (m_bAtEndOfList)
                    {
                        throw gcnew System::Exception(L"Zip item enumerator has been closed.");
                    }

                    return m_ZipItemList[m_nCurrentZipItemIndex];
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
            Enumerator(System::Collections::Generic::List<Common::Zip::Item^>^ ZipItemList);

        private:
            Enumerator();
            ~Enumerator();

        private:
            System::Collections::Generic::List<Common::Zip::Item^>^ m_ZipItemList;
            System::Int32 m_nCurrentZipItemIndex;
            System::Boolean m_bAtEndOfList;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
