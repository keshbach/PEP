/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ListViewItemSequentialSorter.h"
#include "ListViewItemGroupSorter.h"
#include "ListViewItemGroupSequentialSorter.h"

#include "ListViewComboBoxEditEventArgs.h"
#include "ListViewLabelEditPasteEventArgs.h"

#include "ComboBox.h"

#include "ListViewComboBox.h"

#include "ContextMenuStrip.h"
#include "ITextBoxClipboard.h"
#include "EditContextMenuStrip.h"
#include "ITextBoxKeyPress.h"

#include "NativeEdit.h"

#include "ListView.h"

#include "Form.h"

#pragma region "Local Functions"

static System::String^ lFormatColumnWidthRegistryValueName(
  Common::Forms::ListView^ ListView,
  System::Int32 nColumn)
{
	return System::String::Format(L"{0}ColumnWidth{1}", ListView->Name, nColumn + 1);
}

static System::String^ lFormatSortOrderRegistryValueName(
  Common::Forms::ListView^ ListView)
{
	return System::String::Format(L"{0}SortOrder", ListView->Name);
}

static System::String^ lFormatSelectedIndexRegistryValueName(
  System::Windows::Forms::ComboBox^ ComboBox)
{
	return System::String::Format(L"{0}SelectedIndex", ComboBox->Name);
}

static System::String^ lFormatSelectedIndexRegistryValueName(
  array<System::Windows::Forms::RadioButton^>^ RadioButtons)
{
	System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();

	for each (System::Windows::Forms::RadioButton^ RadioButton in RadioButtons)
	{
		if (sb->Length > 0)
		{
			sb->Append("_");
		}

		sb->Append(RadioButton->Name);
	}

	return System::String::Format(L"{0}SelectedIndex", sb->ToString());
}

static System::String^ lFormatCheckBoxCheckedRegistryValueName(
  System::Windows::Forms::CheckBox^ CheckBox)
{
	return System::String::Format(L"{0}Checked", CheckBox->Name);
}
#pragma endregion

#pragma region "Constructor"

Common::Forms::Form::Form()
{
	InitializeComponent();
}

#pragma endregion

#pragma region "Destructor"

Common::Forms::Form::~Form()
{
	if (components)
	{
		delete components;
	}
}

#pragma endregion

#pragma region "Common::Forms::IFormLocation"

void Common::Forms::Form::OnFormLocationSaved(
  Microsoft::Win32::RegistryKey^ RegKey)
{
	array<System::Windows::Forms::Control^>^ SaveControls = ControlLocationSettings;
	array<array<System::Windows::Forms::RadioButton^>^>^ SaveRadioButtons = RadioButtonLocationSettings;

	if (SaveControls != nullptr)
	{
		for each (System::Windows::Forms::Control^ SaveControl in SaveControls)
		{
			if (SaveControl->GetType() == Common::Forms::ListView::typeid)
			{
				WriteFormLocationSettings(RegKey, (Common::Forms::ListView^)SaveControl);
			}
			else if (SaveControl->GetType() == Common::Forms::ComboBox::typeid ||
				     SaveControl->GetType() == System::Windows::Forms::ComboBox::typeid)
			{
				WriteFormLocationSettings(RegKey, (System::Windows::Forms::ComboBox^)SaveControl);
			}
			else if (SaveControl->GetType() == System::Windows::Forms::CheckBox::typeid)
			{
				WriteFormLocationSettings(RegKey, (System::Windows::Forms::CheckBox^)SaveControl);
			}
			else
			{
				System::Diagnostics::Debug::Assert(false, L"Unknown control type to save");
			}
		}
	}

	if (SaveRadioButtons != nullptr)
	{
		for each (array<System::Windows::Forms::RadioButton^>^ SaveRadioButtonControls in SaveRadioButtons)
		{
			WriteFormLocationSettings(RegKey, (array<System::Windows::Forms::RadioButton^>^)SaveRadioButtonControls);
		}
	}
}

void Common::Forms::Form::OnFormLocationRestored(
  Microsoft::Win32::RegistryKey^ RegKey)
{
	array<System::Windows::Forms::Control^>^ RestoreControls = ControlLocationSettings;
	array<array<System::Windows::Forms::RadioButton^>^>^ RestoreRadioButtons = RadioButtonLocationSettings;

	if (RestoreControls != nullptr)
	{
		for each (System::Windows::Forms::Control^ RestoreControl in RestoreControls)
		{
			if (RestoreControl->GetType() == Common::Forms::ListView::typeid)
			{
				ReadFormLocationSettings(RegKey, (Common::Forms::ListView^)RestoreControl);
			}
			else if (RestoreControl->GetType() == Common::Forms::ComboBox::typeid ||
				     RestoreControl->GetType() == System::Windows::Forms::ComboBox::typeid)
			{
				ReadFormLocationSettings(RegKey, (System::Windows::Forms::ComboBox^)RestoreControl);
			}
			else if (RestoreControl->GetType() == System::Windows::Forms::CheckBox::typeid)
			{
				ReadFormLocationSettings(RegKey, (System::Windows::Forms::CheckBox^)RestoreControl);
			}
			else
			{
				System::Diagnostics::Debug::Assert(false, L"Unknown control type to restore");
			}
		}
	}

	if (RestoreRadioButtons != nullptr)
	{
		for each (array<System::Windows::Forms::RadioButton^> ^ RestoreRadioButtonControls in RestoreRadioButtons)
		{
			ReadFormLocationSettings(RegKey, (array<System::Windows::Forms::RadioButton^>^)RestoreRadioButtonControls);
		}
	}
}

#pragma endregion

Common::Forms::ListView::ESortOrder Common::Forms::Form::GetListViewDefaultSortOrderFormLocationSetting(
	Common::Forms::ListView^ ListView)
{
	ListView;

	return Common::Forms::ListView::ESortOrder::None;
}

System::Int32 Common::Forms::Form::GetComboBoxDefaultSelectedIndexSetting(
  System::Windows::Forms::ComboBox^ ComboBox)
{
	ComboBox;

	return -1;
}

System::Int32 Common::Forms::Form::GetRadioButtonDefaultSelectedIndexSetting(
  array<System::Windows::Forms::RadioButton^>^ RadioButtons)
{
	RadioButtons;

	return 0;
}

System::Boolean Common::Forms::Form::GetCheckBoxDefaultCheckedSetting(
  System::Windows::Forms::CheckBox^ CheckBox)
{
	CheckBox;

	return false;
}

void Common::Forms::Form::ReadFormLocationSettings(
	Microsoft::Win32::RegistryKey^ RegKey,
	Common::Forms::ListView^ ListView)
{
	Common::Forms::ListView::ESortOrder SortOrder = GetListViewDefaultSortOrderFormLocationSetting(ListView);
	System::Object^ oColumnWidth;
	System::Object^ oSortOrder;

	ListView->BeginUpdate();

	for (System::Int32 nIndex = 0; nIndex < ListView->Columns->Count; ++nIndex)
	{
		oColumnWidth = RegKey->GetValue(lFormatColumnWidthRegistryValueName(ListView, nIndex));

		if (oColumnWidth != nullptr && oColumnWidth->GetType() == System::Int32::typeid)
		{
			ListView->Columns[nIndex]->Width = (System::Int32)oColumnWidth;
		}
		else
		{
			ListView->AutosizeColumn(nIndex);
		}
	}

	oSortOrder = RegKey->GetValue(lFormatSortOrderRegistryValueName(ListView));

	if (oSortOrder != nullptr && oSortOrder->GetType() == System::Int32::typeid)
	{
		SortOrder = (Common::Forms::ListView::ESortOrder)oSortOrder;
	}

	ListView->Sorting = SortOrder;

	ListView->EndUpdate();
}

void Common::Forms::Form::ReadFormLocationSettings(
  Microsoft::Win32::RegistryKey^ RegKey,
  System::Windows::Forms::ComboBox^ ComboBox)
{
	System::Int32 nSelectedIndex = GetComboBoxDefaultSelectedIndexSetting(ComboBox);
	System::Object^ oSelectedIndex = RegKey->GetValue(lFormatSelectedIndexRegistryValueName(ComboBox));

	if (oSelectedIndex != nullptr && oSelectedIndex->GetType() == System::Int32::typeid)
	{
		nSelectedIndex = (System::Int32)oSelectedIndex;
	}

	ComboBox->SelectedIndex = nSelectedIndex;
}

void Common::Forms::Form::ReadFormLocationSettings(
  Microsoft::Win32::RegistryKey^ RegKey,
  array<System::Windows::Forms::RadioButton^>^ RadioButtons)
{
	System::Int32 nSelectedIndex = GetRadioButtonDefaultSelectedIndexSetting(RadioButtons);
	System::Object^ oSelectedIndex = RegKey->GetValue(lFormatSelectedIndexRegistryValueName(RadioButtons));

	if (oSelectedIndex != nullptr && oSelectedIndex->GetType() == System::Int32::typeid)
	{
		nSelectedIndex = (System::Int32)oSelectedIndex;
	}

	if (nSelectedIndex > -1 && nSelectedIndex < RadioButtons->Length)
	{
		RadioButtons[nSelectedIndex]->Checked = true;
	}
}

void Common::Forms::Form::ReadFormLocationSettings(
  Microsoft::Win32::RegistryKey^ RegKey,
  System::Windows::Forms::CheckBox^ CheckBox)
{
	System::Boolean bChecked = GetCheckBoxDefaultCheckedSetting(CheckBox);
	System::Object^ oChecked = RegKey->GetValue(lFormatCheckBoxCheckedRegistryValueName(CheckBox));

	if (oChecked != nullptr && oChecked->GetType() == System::Int32::typeid)
	{
		bChecked = ((System::Int32)oChecked) > 0;
	}

	CheckBox->Checked = bChecked;
}

void Common::Forms::Form::WriteFormLocationSettings(
  Microsoft::Win32::RegistryKey^ RegKey, 
  Common::Forms::ListView^ ListView)
{
	for (System::Int32 nIndex = 0; nIndex < ListView->Columns->Count; ++nIndex)
	{
		RegKey->SetValue(lFormatColumnWidthRegistryValueName(ListView, nIndex), ListView->Columns[nIndex]->Width);
	}

	RegKey->SetValue(lFormatSortOrderRegistryValueName(ListView), (System::Int32)ListView->Sorting);
}

void Common::Forms::Form::WriteFormLocationSettings(
  Microsoft::Win32::RegistryKey^ RegKey,
  System::Windows::Forms::ComboBox^ ComboBox)
{
	RegKey->SetValue(lFormatSelectedIndexRegistryValueName(ComboBox), ComboBox->SelectedIndex);
}

void Common::Forms::Form::WriteFormLocationSettings(
  Microsoft::Win32::RegistryKey^ RegKey,
  array<System::Windows::Forms::RadioButton^>^ RadioButtons)
{
	System::Int32 nSelectedIndex = -1;

	for (System::Int32 nIndex = 0; nIndex < RadioButtons->Length && nSelectedIndex == -1;
		 ++nIndex)
	{
		if (RadioButtons[nIndex]->Checked)
		{
			nSelectedIndex = nIndex;
		}
	}

	RegKey->SetValue(lFormatSelectedIndexRegistryValueName(RadioButtons), nSelectedIndex);
}

void Common::Forms::Form::WriteFormLocationSettings(
  Microsoft::Win32::RegistryKey^ RegKey,
  System::Windows::Forms::CheckBox^ CheckBox)
{
	RegKey->SetValue(lFormatCheckBoxCheckedRegistryValueName(CheckBox), CheckBox->Checked);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
