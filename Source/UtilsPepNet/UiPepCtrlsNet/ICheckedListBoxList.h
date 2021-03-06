/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
		/// Summary for ICheckedListBoxList
		/// </summary>
		private interface class ICheckedListBoxList
		{
		public:
			property array<CheckedListBoxItem^>^ Array
			{
				virtual array<CheckedListBoxItem^>^ get();
			}

			virtual void Add(CheckedListBoxItem^ CheckedListBoxItem);
			virtual void Clear();
			virtual void CopyTo(array<CheckedListBoxItem^>^ CheckedListBoxItemArray, int nIndex);
			virtual bool Remove(int nIndex);
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
