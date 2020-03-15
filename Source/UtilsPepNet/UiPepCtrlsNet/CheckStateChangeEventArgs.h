/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
		/// Summary for CheckStateChangeEventArgs
		/// </summary>
		public ref class CheckStateChangedEventArgs sealed :
			public System::EventArgs
		{
		public:
			/// <summary>
			/// Index of the item.
			/// </summary>

			property System::Int32 Index
			{
				System::Int32 get()
				{
					return m_nIndex;
				}
			}

			/// <summary>
			/// New check state of the item.
			/// </summary>
			property Pep::Forms::ECheckState CheckState
			{
				Pep::Forms::ECheckState get() 
				{
					return m_CheckState;
                }
			}

		internal:
			CheckStateChangedEventArgs(System::Int32 nIndex, Pep::Forms::ECheckState CheckState);

		private:
			System::Int32 m_nIndex;
			Pep::Forms::ECheckState m_CheckState;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
