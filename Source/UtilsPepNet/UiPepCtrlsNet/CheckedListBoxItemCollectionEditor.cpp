/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "ICheckedListBoxItemChange.h"

#include "CheckedListBoxEnums.h"
#include "CheckStateChangeEventArgs.h"
#include "CheckedListBoxItemCollectionEditor.h"
#include "CheckedListBoxItem.h"
#include "ICheckedListBoxList.h"
#include "CheckedListBoxItemCollection.h"

#include "CheckedListBoxItemCollectionEditor.h"

Pep::Forms::CheckedListBoxItemCollectionEditor::CheckedListBoxItemCollectionEditor() :
	CollectionEditor(CheckedListBoxItem::typeid)
{
}

Pep::Forms::CheckedListBoxItemCollectionEditor::CheckedListBoxItemCollectionEditor(
  System::Type^ type) : 
	CollectionEditor(type)
{
}

Pep::Forms::CheckedListBoxItemCollectionEditor::~CheckedListBoxItemCollectionEditor()
{
}

System::ComponentModel::Design::CollectionEditor::CollectionForm^ Pep::Forms::CheckedListBoxItemCollectionEditor::CreateCollectionForm()
{
	System::ComponentModel::Design::CollectionEditor::CollectionForm^ Form = System::ComponentModel::Design::CollectionEditor::CreateCollectionForm();
	System::Type^ Type = Form->GetType();
	System::Reflection::PropertyInfo^ PropInfo = Type->GetProperty("CollectionEditable", System::Reflection::BindingFlags::Instance | System::Reflection::BindingFlags::NonPublic);

	PropInfo->SetValue(Form, true, nullptr);

	return Form;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
