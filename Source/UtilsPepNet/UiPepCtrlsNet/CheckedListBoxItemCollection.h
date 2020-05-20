/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
        /// Summary for CheckedListBoxItemCollection
        /// </summary>
		public ref class CheckedListBoxItemCollection sealed : public System::Collections::IList,
			                                                   public System::Collections::Generic::ICollection<CheckedListBoxItem^>,
			                                                   public System::Collections::Generic::IList<CheckedListBoxItem^>
		{
		internal:
			CheckedListBoxItemCollection(ICheckedListBoxList^ CheckedListBoxList);

		public:
			CheckedListBoxItemCollection();

		internal:
			void Close();

        // System::Collections::IList
		public:
			property bool IsSynchronized
			{
				virtual bool get()
				{
					return false;
				}
			}

			property System::Object^ SyncRoot
			{
				virtual System::Object^ get()
				{
					return m_SyncObject;
				}
			}

			property bool IsFixedSize
			{
				virtual bool get()
				{
					return false;
				}
			}

			property System::Object^ ItemsNonGeneric[int]
			{
				virtual System::Object^ get(int nIndex) = System::Collections::IList::default::get
				{
					return GetItem(nIndex);
				}

				virtual void set(int nIndex, System::Object^ CheckedListBoxItem) = System::Collections::IList::default::set
				{
					SetItem(nIndex, (Pep::Forms::CheckedListBoxItem^)CheckedListBoxItem);
    			}
			}

			virtual void CopyTo(System::Array^ Array, int nIndex);
			virtual int Add(System::Object^ Obj);
			virtual bool Contains(System::Object^ Obj);
			virtual int IndexOf(System::Object^ Obj);
			virtual	void InsertNonGeneric(int nIndex, System::Object^ Obj) = System::Collections::IList::Insert;
			virtual void Remove(System::Object^ Obj);
			virtual void RemoveAtNonGeneric(int nIndex) = System::Collections::IList::RemoveAt;

		// System.Collections.Generic.ICollection
		public:
			property int Count
			{
				virtual int get()
				{
					if (m_CheckedListBoxList != nullptr)
					{
						return m_CheckedListBoxList->Array->Length;
					}

					return 0;
				}
			}

			property bool IsReadOnly
			{
				virtual bool get()
				{
					return false;
				}
			}

			virtual void Add(CheckedListBoxItem^ CheckedListBoxItem);
			virtual void Clear();
			virtual bool Contains(CheckedListBoxItem^ CheckedListBoxItem);
			virtual void CopyTo(array<CheckedListBoxItem^>^ CheckedListBoxItemArray, int nIndex);
			virtual bool Remove(CheckedListBoxItem^ CheckedListBoxItem);

		// System.Collections.Generic.IEnumerable
		public:
			virtual System::Collections::IEnumerator^ GetEnumerator();
			virtual System::Collections::Generic::IEnumerator<CheckedListBoxItem^>^ GetEnumeratorBase() = System::Collections::Generic::IEnumerable<CheckedListBoxItem^>::GetEnumerator;

		// System::Collections::Generic::IList
		public:
			property CheckedListBoxItem^ default[int]
			{
				virtual CheckedListBoxItem^ get(int nIndex)
				{
    				return GetItem(nIndex);
				}

				virtual void set(int nIndex, CheckedListBoxItem^ CheckedListBoxItem)
				{
					SetItem(nIndex, (Pep::Forms::CheckedListBoxItem^)CheckedListBoxItem);
				}
			}

			virtual int IndexOf(CheckedListBoxItem^ CheckedListBoxItem);
			virtual void Insert(int nIndex, CheckedListBoxItem^ CheckedListBoxItem);
			virtual void RemoveAt(int nIndex);

		// Other
		public:
			void AddRange(array<System::Object^>^ ObjectArray);

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~CheckedListBoxItemCollection();

		private:
			CheckedListBoxItem^ GetItem(int nIndex);
			void SetItem(int nIndex, CheckedListBoxItem^ CheckedListBoxItem);

		private:
			ICheckedListBoxList^ m_CheckedListBoxList;

			System::Object^ m_SyncObject;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
