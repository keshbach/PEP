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

static array<System::Object^>^ lGetCheckedListBoxItemArray(
  System::Windows::Forms::Form^ Form)
{
	System::Reflection::PropertyInfo^ PropInfo = nullptr;
	System::Object^ Object;

	try
	{
		PropInfo = Form->GetType()->GetProperty("Items", System::Reflection::BindingFlags::Instance | System::Reflection::BindingFlags::NonPublic);
	}
	catch (System::Exception^)
	{
	}

	if (PropInfo != nullptr)
	{
		Object = PropInfo->GetValue(Form, nullptr);

		return (array<System::Object^>^)Object;
	}

	return nullptr;
}

Pep::Forms::CheckedListBoxItemCollectionEditor::CheckedListBoxItemCollectionEditor() :
	CollectionEditor(CheckedListBoxItem::typeid)
{
}

Pep::Forms::CheckedListBoxItemCollectionEditor::CheckedListBoxItemCollectionEditor(
   System::Type^ type) : CollectionEditor(type)
{
}

Pep::Forms::CheckedListBoxItemCollectionEditor::~CheckedListBoxItemCollectionEditor()
{
}

System::ComponentModel::Design::CollectionEditor::CollectionForm^ Pep::Forms::CheckedListBoxItemCollectionEditor::CreateCollectionForm()
{
	System::ComponentModel::Design::CollectionEditor::CollectionForm^ Form = System::ComponentModel::Design::CollectionEditor::CreateCollectionForm();
	System::Type^ Type = Form->GetType();
	System::Reflection::PropertyInfo^ PropInfo = nullptr;

	try
	{
		PropInfo = Type->GetProperty("CollectionEditable", System::Reflection::BindingFlags::Instance | System::Reflection::BindingFlags::NonPublic);
	}
	catch (System::Exception^)
	{
	}

	if (PropInfo != nullptr)
	{
		PropInfo->SetValue(Form, true, nullptr);
	}

	m_CollectionForm = Form;

	m_CollectionForm->FormClosed += gcnew System::Windows::Forms::FormClosedEventHandler(this, &Pep::Forms::CheckedListBoxItemCollectionEditor::CollectionForm_FormClosed);

	return Form;
}

System::Object^ Pep::Forms::CheckedListBoxItemCollectionEditor::CreateInstance(
  System::Type^ ItemType)
{
	if (ItemType == CheckedListBoxItem::typeid)
	{
		CheckedListBoxItem^ Item = gcnew CheckedListBoxItem();
		array<System::Object^>^ CheckedListBoxItemArray = lGetCheckedListBoxItemArray(m_CollectionForm);

		if (CheckedListBoxItemArray != nullptr)
		{
			Item->Name = System::String::Format("Item #{0}", CheckedListBoxItemArray->Length + 1);

			return Item;
		}
	}

	return System::ComponentModel::Design::CollectionEditor::CreateInstance(ItemType);
}

void Pep::Forms::CheckedListBoxItemCollectionEditor::CollectionForm_FormClosed(
  System::Object^ sender,
  System::Windows::Forms::FormClosedEventArgs^ e)
{
	m_CollectionForm->FormClosed -= gcnew System::Windows::Forms::FormClosedEventHandler(this, &Pep::Forms::CheckedListBoxItemCollectionEditor::CollectionForm_FormClosed);

	m_CollectionForm = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
