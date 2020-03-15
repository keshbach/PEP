/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
		/// Summary for CheckedListBoxItem
		/// </summary>
		public ref class CheckedListBoxItem sealed
		{
		public:
			/// <summary>
			/// The name of the item.
			/// </summary>
			[System::ComponentModel::DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Visible),
				System::ComponentModel::DefaultValueAttribute("")]
			property System::String^ Name
			{
				System::String^ get()
				{
					return m_sName;
				}

				void set(System::String^ value)
				{
					m_sName = value;

					if (m_CheckedListBoxItemChange != nullptr)
					{
						m_CheckedListBoxItemChange->OnNameChange(this);
					}
				}
			}

			/// <summary>
			/// The check state of the item.
			/// </summary>
			[System::ComponentModel::DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Visible)]
			property ECheckState CheckState
			{
				ECheckState get()
				{
					return m_CheckState;
				}

				void set(ECheckState value)
				{
					m_CheckState = value;

					if (m_CheckedListBoxItemChange != nullptr)
					{
						m_CheckedListBoxItemChange->OnCheckStateChange(this);
					}
				}
			}

		internal:
			property ICheckedListBoxItemChange^ CheckedListBoxItemChange
			{
				void set(ICheckedListBoxItemChange^ value)
				{
					m_CheckedListBoxItemChange = value;
				}
			}

		private:
			System::String^ m_sName;
			ECheckState m_CheckState;
			ICheckedListBoxItemChange^ m_CheckedListBoxItemChange;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
