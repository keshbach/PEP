/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
		/// Summary for CheckedListBox
		/// </summary>
		[System::Drawing::ToolboxBitmapAttribute(Pep::Forms::CheckedListBox::typeid, "IDB_CHECKEDLISTBOX")]
	    [System::ComponentModel::DefaultPropertyAttribute("Items")]
		[System::ComponentModel::DefaultEventAttribute("CheckStateChange")]
		public ref class CheckedListBox sealed : public System::Windows::Forms::Control,
			                                     Pep::Forms::ICheckedListBoxList,
			                                     Pep::Forms::ICheckedListBoxItemChange
		{
		public:
			[System::ComponentModel::Category("Behavior")]
			event System::EventHandler<Pep::Forms::CheckStateChangedEventArgs^>^ CheckStateChange;

		public:
			/// <summary>
			/// The items in the checked list box.
			/// </summary>

			[System::ComponentModel::Description("The items in the checked list box."),
				System::ComponentModel::Category("Data"),
			    System::ComponentModel::DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Content),
				System::ComponentModel::EditorAttribute(CheckedListBoxItemCollectionEditor::typeid,
					                                    System::Drawing::Design::UITypeEditor::typeid)]
			property CheckedListBoxItemCollection^ Items
			{
				CheckedListBoxItemCollection^ get()
				{
                    return m_CheckedListBoxItemCollection;
				}
			}

			/// <summary>
			/// The minimum width in pixels that is required to fully display all items.
			/// </summary>

			[System::ComponentModel::Browsable(false),
				System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Never)]
			property System::Int32 MinWidth
			{
				System::Int32 get()
				{
					return GetMinWidth();
				}
			}

		public:
			CheckedListBox();
			CheckedListBox(System::ComponentModel::IContainer ^container);

			void BeginUpdate(void);
			void EndUpdate(void);

		// Pep::Forms::ICheckedListBoxList
		public:
			[System::ComponentModel::Browsable(false),
				System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Never)]
			property array<CheckedListBoxItem^>^ Array
			{
				virtual array<CheckedListBoxItem^>^ get()
				{
					return m_CheckedListBoxItemList->ToArray();
				}
			}

			virtual void Add(CheckedListBoxItem^ CheckedListBoxItem);
			virtual void Clear();
			virtual void CopyTo(array<CheckedListBoxItem^>^ CheckedListBoxItemArray, int nIndex);
			virtual bool Remove(int nIndex);

		// Pep::Forms::ICheckedListBoxItemChange
		public:
			virtual void OnNameChange(System::Object^ CheckedListBoxItem);
			virtual void OnCheckStateChange(System::Object^ CheckedListBoxItem);

		protected:
			void OnHandleCreated(System::EventArgs^ e) override;
			void OnHandleDestroyed(System::EventArgs^ e) override;
			void OnEnabledChanged(System::EventArgs^ e) override;
			void OnVisibleChanged(System::EventArgs^ e) override;
			void OnFontChanged(System::EventArgs^ e) override;
			void OnGotFocus(System::EventArgs^ e) override;
			void OnPaint(System::Windows::Forms::PaintEventArgs^ e) override;
			void OnResize(System::EventArgs^ e) override;
			void WndProc(System::Windows::Forms::Message% msg) override;

        public:
			bool PreProcessMessage(System::Windows::Forms::Message% msg) override;

		private:
			System::Boolean UpdateItems();

			void UpdateFont(System::Boolean bRedraw);

			System::Int32 GetMinWidth();

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~CheckedListBox();

		private:
			/// <summary>
			/// Required designer variable.
			/// </summary>
			System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
			/// <summary>
			/// Required method for Designer support - do not modify
			/// the contents of this method with the code editor.
			/// </summary>
			void InitializeComponent(void)
			{
				this->SuspendLayout();
				// 
				// CheckedListBox
				// 
				this->ResumeLayout(false);
			}
#pragma endregion

		private:
			CheckedListBoxItemCollection^ m_CheckedListBoxItemCollection = gcnew CheckedListBoxItemCollection(this);

			System::Collections::Generic::List<CheckedListBoxItem^>^ m_CheckedListBoxItemList = gcnew System::Collections::Generic::List<CheckedListBoxItem^>();;

			HWND m_hCheckedListBoxCtrl;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
