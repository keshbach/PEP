/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MenuItemHelp.h"
#include "Application.h"

#include "Includes/UtTemplates.h"

#pragma region Constants
#define CSerializationMenuItemName L"MenuItemName"
#define CSerializationHelpTextName L"HelpText"
#pragma endregion

#pragma region Constructors

Common::Forms::MenuItemHelp::MenuItemHelp()
{
    m_sMenuItemName = System::String::Empty;
    m_sHelpText = System::String::Empty;
}

Common::Forms::MenuItemHelp::MenuItemHelp(
  System::Runtime::Serialization::SerializationInfo^ info,
  System::Runtime::Serialization::StreamingContext context)
{
    context;

    if (info == nullptr)
    {
        throw gcnew System::ArgumentNullException("info");
    }

    m_sMenuItemName = info->GetString(CSerializationMenuItemName);
    m_sHelpText = info->GetString(CSerializationHelpTextName);
}

#pragma endregion

#pragma region System::Runtime::Serialization::ISerializable

void Common::Forms::MenuItemHelp::GetObjectData(
  System::Runtime::Serialization::SerializationInfo^ info,
  System::Runtime::Serialization::StreamingContext context)
{
    context;

    if (info == nullptr)
    {
        throw gcnew System::ArgumentNullException("info");
    }

    info->AddValue(CSerializationMenuItemName, m_sMenuItemName);
    info->AddValue(CSerializationHelpTextName, m_sHelpText);
}

#pragma endregion

#pragma region System::IEquatable

bool Common::Forms::MenuItemHelp::Equals(
  MenuItemHelp^ other)
{
    return (m_sMenuItemName->CompareTo(other->m_sMenuItemName) == 0) ? true : false;
}

#pragma endregion

#pragma region System::IComparable

int Common::Forms::MenuItemHelp::CompareTo(MenuItemHelp^ other)
{
    return m_sMenuItemName->CompareTo(other->m_sMenuItemName);
}

#pragma endregion

#pragma region Deconstructor

Common::Forms::MenuItemHelp::~MenuItemHelp()
{
}

#pragma endregion

#pragma region Other Helpers

System::Windows::Forms::ToolStripMenuItem^ Common::Forms::MenuItemHelp::FindToolStripMenuItem(System::String^ name)
{
    System::Windows::Forms::ToolStrip^ toolStrip;
    System::Windows::Forms::ToolStripMenuItem^ toolStripMenuItem;
    Common::Forms::MainForm^ mainForm;
    System::Windows::Forms::TextBox^ textBox;

    if (Common::Forms::Application::MainForm != nullptr)
    {
        mainForm = Common::Forms::Application::MainForm;
    }
    else
    {
        mainForm = Common::Forms::MainForm::MainFormDesignMode;
    }

    for each (System::Windows::Forms::Control^ control in mainForm->Controls)
    {
        if (IsInstance<System::Windows::Forms::MenuStrip^>(control))
        {
            toolStrip = dynamic_cast<System::Windows::Forms::ToolStrip^>(control);

            toolStripMenuItem = FindToolStripMenuItem(toolStrip->Items, name);

            if (toolStripMenuItem != nullptr)
            {
                return toolStripMenuItem;
            }
        }
        else if (IsInstance<System::Windows::Forms::TextBox^>(control))
        {
            textBox = dynamic_cast<System::Windows::Forms::TextBox^>(control);

            if (textBox->ContextMenuStrip != nullptr)
            {
                toolStrip = dynamic_cast<System::Windows::Forms::ToolStrip^>(textBox->ContextMenuStrip);

                toolStripMenuItem = FindToolStripMenuItem(toolStrip->Items, name);

                if (toolStripMenuItem != nullptr)
                {
                    return toolStripMenuItem;
                }
            }
        }
    }

    return nullptr;
}

System::Windows::Forms::ToolStripMenuItem^ Common::Forms::MenuItemHelp::FindToolStripMenuItem(
  System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection,
  System::String^ name)
{
    System::Windows::Forms::ToolStripMenuItem^ toolStripMenuItem;

    for each (System::Windows::Forms::ToolStripItem^ toolStripItem in ToolStripItemCollection)
    {
        if (IsInstance<System::Windows::Forms::ToolStripMenuItem^>(toolStripItem))
        {
            toolStripMenuItem = dynamic_cast<System::Windows::Forms::ToolStripMenuItem^>(toolStripItem);

            if (System::String::Compare(toolStripMenuItem->Name, name) == 0)
            {
                return toolStripMenuItem;
            }

            toolStripMenuItem = FindToolStripMenuItem(toolStripMenuItem->DropDownItems, name);

            if (toolStripMenuItem != nullptr)
            {
                return toolStripMenuItem;
            }
        }
    }

    return nullptr;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
