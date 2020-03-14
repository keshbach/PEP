/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		ref class CheckedListBoxItemListEnumerator sealed :
			System::Collections::Generic::IEnumerator<Pep::Forms::CheckedListBoxItem^>
		{
		public:
			property Pep::Forms::CheckedListBoxItem^ Current
			{
				virtual Pep::Forms::CheckedListBoxItem^ get() = System::Collections::Generic::IEnumerator<Pep::Forms::CheckedListBoxItem^>::Current::get
				{
					if (m_bAtEndOfList)
					{
						throw gcnew System::Exception(L"Checked list box item enumerator has been closed.");
					}

					return m_CheckedListBoxItemList[m_nCurrentCheckedListBoxItemIndex];
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
			CheckedListBoxItemListEnumerator(System::Collections::Generic::List<Pep::Forms::CheckedListBoxItem^>^ CheckedListBoxItemList);

		private:
			CheckedListBoxItemListEnumerator();
			~CheckedListBoxItemListEnumerator();

		private:
			System::Collections::Generic::List<Pep::Forms::CheckedListBoxItem^>^ m_CheckedListBoxItemList;
			System::Int32 m_nCurrentCheckedListBoxItemIndex;
			System::Boolean m_bAtEndOfList;
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
