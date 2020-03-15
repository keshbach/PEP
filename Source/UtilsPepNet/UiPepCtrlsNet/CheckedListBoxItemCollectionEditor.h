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
			virtual System::ComponentModel::Design::CollectionEditor::CollectionForm^ CreateCollectionForm() override;

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~CheckedListBoxItemCollectionEditor();
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
