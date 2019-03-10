/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
	public ref class Collections sealed
	{
    public:
        /// <summary>
        /// Generic string dictionary where the string uses a case-insensitive comparer.
        /// </summary>

        generic<class TValue>
        ref class StringDictionary :
            System::Collections::Generic::Dictionary<System::String^, TValue>
        {
        public:
            StringDictionary() :
                System::Collections::Generic::Dictionary<System::String^, TValue>(System::StringComparer::CurrentCultureIgnoreCase)
            {
            }
        };

        /// <summary>
        /// Generic string sorted list where the string uses a case-insensitive comparer.
        /// </summary>

        generic<class TValue>
        ref class StringSortedList :
            System::Collections::Generic::SortedList<System::String^, TValue>
        {
        public:
            StringSortedList() :
                System::Collections::Generic::SortedList<System::String^, TValue>(System::StringComparer::CurrentCultureIgnoreCase)
            {
            }

            /// <summary>
            /// Creates a new String Sorted List.
            /// </summary>

            StringSortedList<TValue>^ MakeCopy()
            {
                StringSortedList<TValue>^ NewSortedList = gcnew StringSortedList<TValue>();
                System::Collections::Generic::IEnumerator<System::Collections::Generic::KeyValuePair<System::String^, TValue>>^ Enum = GetEnumerator();

                while (Enum->MoveNext())
                {
                    NewSortedList->Add(Enum->Current.Key, Enum->Current.Value);
                }

                return NewSortedList;
            }

            /// <summary>
            /// Creates a List of strings from the keys.
            /// </summary>

            System::Collections::Generic::List<System::String^>^ GetKeyList()
            {
                System::Collections::Generic::List<System::String^>^ KeyList;

                KeyList = gcnew System::Collections::Generic::List<System::String^>(Count);

                for each(System::String^ Key in Keys)
                {
                    KeyList->Add(Key);
                }

                return KeyList;
            }

            /// <summary>
            /// Creates an ArrayList of strings from the keys.
            /// </summary>

            System::Collections::ArrayList^ GetKeyArrayList()
            {
                System::Collections::ArrayList^ KeyArrayList;

                KeyArrayList = gcnew System::Collections::ArrayList(Count);

                for each(System::String^ Key in Keys)
                {
                    KeyArrayList->Add(Key);
                }

                return KeyArrayList;
            }

            /// <summary>
            /// Creates a List from the values.
            /// </summary>

            System::Collections::Generic::List<TValue>^ GetValueList()
            {
                System::Collections::Generic::List<TValue>^ ValueList;

                ValueList = gcnew System::Collections::Generic::List<TValue>(Count);

                for each (TValue Value in Values)
                {
                    ValueList->Add(Value);
                }

                return ValueList;
            }

            /// <summary>
            /// Creates an ArrayList from the values.
            /// </summary>

            System::Collections::ArrayList^ GetValueArrayList()
            {
                System::Collections::ArrayList^ ValueArrayList;

                ValueArrayList = gcnew System::Collections::ArrayList(Count);

                for each(System::String^ Value in Values)
                {
                    ValueArrayList->Add(Value);
                }

                return ValueArrayList;
            }
        };

        /// <summary>
        /// Extension of the system String Collection where a case-insensitive comparer is used.
        /// </summary>

        ref class StringCollection :
            System::Collections::Specialized::StringCollection
        {
        public:
            /// <summary>
            /// Checks for any duplicate strings using a case-insensitive comparer.
            /// </summary>

            property System::Boolean HasDuplicates
            {
                System::Boolean get()
                {
                    Common::Collections::StringCollection TmpColl;

                    for each (System::String^ sValue in this)
                    {
                        if (TmpColl.Contains(sValue))
                        {
                            return true;
                        }

                        TmpColl.Add(sValue);
                    }

                    return false;
                }
            }

        public:
            StringCollection();

            /// <summary>
            /// Performs a search for a value using a case-insensitive comparer.
            /// </summary>

            System::Boolean Contains(System::String^ sValue) new;

            /// <summary>
            /// Creates a new string collection.
            /// </summary>

            Common::Collections::StringCollection^ MakeCopy();

            /// <summary>
            /// Merges an existing string collections into a new string collection
            /// disallowing duplicates.
            /// </summary>

            Common::Collections::StringCollection^ Merge(Common::Collections::StringCollection^ ExistingColl);

            /// <summary>
            /// Creates a string collection from a string array.
            /// </summary>

            static Common::Collections::StringCollection^ FromArray(array<System::String^>^ StringArray);

        protected:
            System::Collections::Generic::IComparer<System::String^>^ m_Comparer;
        };
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
