/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
		/// Summary for CheckedListBoxItemCollectionEditor
		/// </summary>
		public ref class CheckedListBoxItemCollectionEditor sealed :
			public System::ComponentModel::Design::CollectionEditor
		{
		public:
			CheckedListBoxItemCollectionEditor();
			CheckedListBoxItemCollectionEditor(System::Type^ type);

		protected:
			virtual System::Object^ CreateInstance(System::Type^ ItemType) override;
			virtual System::ComponentModel::Design::CollectionEditor::CollectionForm^ CreateCollectionForm() override;

		private:
			void CollectionForm_FormClosed(System::Object^ sender, System::Windows::Forms::FormClosedEventArgs^ e);

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~CheckedListBoxItemCollectionEditor();

		private:
			System::ComponentModel::Design::CollectionEditor::CollectionForm^ m_CollectionForm;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
