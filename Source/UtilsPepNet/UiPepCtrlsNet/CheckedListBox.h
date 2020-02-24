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
		public ref class CheckedListBox : public System::Windows::Forms::Control
		{
		public:
			[System::ComponentModel::Category("Behavior")]
			event System::EventHandler<Pep::Forms::CheckStateChangedEventArgs^>^ CheckStateChange;

		public:
			/// <summary>
			/// The items in the checked list box.
			/// </summary>

			[System::ComponentModel::Description("The checked list box items."),
				System::ComponentModel::Category("Data")]
			property array<System::String^>^ Items
			{
				array<System::String^>^ get()
				{
					return m_ItemList->ToArray();
				}

				void set(array<System::String^>^ value)
				{
					m_ItemList->Clear();

					for (System::Int32 nIndex = 0; nIndex < value->Length; ++nIndex)
					{
						m_ItemList->Add(value[nIndex]);
					}

    				UpdateItems();
				}
			}

		public:
			CheckedListBox();
			CheckedListBox(System::ComponentModel::IContainer ^container);

			void BeginUpdate(void);
			void EndUpdate(void);

			ECheckState GetCheckState(System::Int32 nIndex);
			void SetCheckState(System::Int32 nIndex, ECheckState CheckState);

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
				// BufferViewer
				// 
				this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
				this->Name = L"CheckedListBox";
				this->ResumeLayout(false);
			}
#pragma endregion

		private:
			System::Collections::Generic::List<System::String^>^ m_ItemList = gcnew System::Collections::Generic::List<System::String^>();

			HWND m_hCheckedListBoxCtrl;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
