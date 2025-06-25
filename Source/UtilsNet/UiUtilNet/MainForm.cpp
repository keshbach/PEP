/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MainForm.h"

#include "Application.h"

#include "Includes/UtTemplates.h"

#pragma region "Local Functions"

#pragma unmanaged

static BOOL lCreateTaskbarList3(
  HWND hWnd,
  ITaskbarList3** ppTaskbarList3)
{
    BOOL bResult = FALSE;
    ITaskbarList* pTaskbarList = NULL;

    *ppTaskbarList3 = NULL;

    if (S_OK != ::CoCreateInstance(CLSID_TaskbarList, NULL,
                                   CLSCTX_INPROC_SERVER, IID_ITaskbarList,
                                   (LPVOID*)&pTaskbarList))
    {
        return FALSE;
    }

    if (S_OK == pTaskbarList->HrInit())
    {
        if (SUCCEEDED(pTaskbarList->QueryInterface(IID_ITaskbarList3,
                                                   (LPVOID*)ppTaskbarList3)))
        {
            if (S_OK == (*ppTaskbarList3)->SetProgressState(hWnd, TBPF_NORMAL))
            {
                bResult = TRUE;
            }
            else
            {
                (*ppTaskbarList3)->Release();

                *ppTaskbarList3 = NULL;
            }
        }
    }

    pTaskbarList->Release();

    return bResult;
}

static BOOL lDestroyTaskbarList3(
  HWND hWnd,
  ITaskbarList3** ppTaskbarList3)
{
    (*ppTaskbarList3)->SetProgressState(hWnd, TBPF_NOPROGRESS);
    (*ppTaskbarList3)->Release();

    *ppTaskbarList3 = NULL;

    return TRUE;
}

#pragma managed

static System::Windows::Forms::StatusStrip^ lFindStatusStrip(
  System::Windows::Forms::Control^ ParentControl)
{
	System::Windows::Forms::StatusStrip^ StatusStrip;
	System::Windows::Forms::ToolStripContainer^ ToolStripContainer;

	for each(System::Windows::Forms::Control^ control in ParentControl->Controls)
	{
		if (IsInstance<System::Windows::Forms::StatusStrip^>(control))
		{
			return dynamic_cast<System::Windows::Forms::StatusStrip^>(control);
		}
		else if (IsInstance<System::Windows::Forms::ToolStripContainer^>(control))
		{
			ToolStripContainer = (System::Windows::Forms::ToolStripContainer^)control;

			StatusStrip = lFindStatusStrip(ToolStripContainer);

			if (StatusStrip != nullptr)
			{
				return StatusStrip;
			}
		}
		else
		{
			StatusStrip = lFindStatusStrip(control);

			if (StatusStrip != nullptr)
			{
				return StatusStrip;
			}
		}
	}
	
	return nullptr;
}

static System::String^ lTranslateKey(
	System::Windows::Forms::Keys Keys)
{
	System::Int32 nKey = (System::Int32)Keys;

	nKey &= ~(System::Int32)System::Windows::Forms::Keys::Control;
	nKey &= ~(System::Int32)System::Windows::Forms::Keys::Alt;
	nKey &= ~(System::Int32)System::Windows::Forms::Keys::Shift;

	if (nKey >= L'A' && nKey <= L'Z')
	{
		return ((System::Windows::Forms::Keys)nKey).ToString();
	}

	switch ((System::Windows::Forms::Keys)nKey)
	{
		case System::Windows::Forms::Keys::Delete:
			return L"Del";
	}

	return ((System::Windows::Forms::Keys)nKey).ToString();
}

static System::String^ lGenerateToolTipText(
  System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem)
{
	System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();
	System::Int32 nIndex = 0;

	while (nIndex < ToolStripMenuItem->Text->Length)
	{
		if (ToolStripMenuItem->Text[nIndex] != L'&')
		{
			sb->Append(ToolStripMenuItem->Text[nIndex]);

			++nIndex;
		}
		else
		{
			if (nIndex + 1 < ToolStripMenuItem->Text->Length)
			{
				if (ToolStripMenuItem->Text[nIndex + 1] != L'&')
				{
					++nIndex;
				}
				else
				{
					sb->Append(ToolStripMenuItem->Text[nIndex]);

					nIndex += 2;
				}
			}
			else
			{
				sb->Append(ToolStripMenuItem->Text[nIndex]);

				++nIndex;
			}
		}
	}

	if (ToolStripMenuItem->ShowShortcutKeys)
	{
		if (ToolStripMenuItem->ShortcutKeys != System::Windows::Forms::Keys::None)
		{
			sb->Append(L" (");

			if ((System::Int32)ToolStripMenuItem->ShortcutKeys & (System::Int32)System::Windows::Forms::Keys::Control)
			{
				sb->Append(L"Ctrl+");
			}

			if ((System::Int32)ToolStripMenuItem->ShortcutKeys & (System::Int32)System::Windows::Forms::Keys::Shift)
			{
				sb->Append(L"Shift+");
			}

			if ((System::Int32)ToolStripMenuItem->ShortcutKeys & (System::Int32)System::Windows::Forms::Keys::Alt)
			{
				sb->Append(L"Alt+");
			}

			sb->Append(lTranslateKey(ToolStripMenuItem->ShortcutKeys));
			sb->Append(L")");
		}
		else if (!System::String::IsNullOrEmpty(ToolStripMenuItem->ShortcutKeyDisplayString))
		{
			sb->Append(L" (");
			sb->Append(ToolStripMenuItem->ShortcutKeyDisplayString);
			sb->Append(L")");
		}
	}

	return sb->ToString();
}

static void lRemoveExtraneousSeparators(
  System::Windows::Forms::ToolStrip^ ToolStrip)
{
	System::Boolean bQuit = false;
	System::Int32 nIndex = 1;

	// Delete beginning separators

	while (ToolStrip->Items->Count > 0 && !bQuit)
	{
		if (IsInstance<System::Windows::Forms::ToolStripSeparator^>(ToolStrip->Items[0]))
		{
			ToolStrip->Items->RemoveAt(0);
		}
		else
		{
			bQuit = true;
		}
	}

	// Delete ending separators

	bQuit = false;

	while (ToolStrip->Items->Count > 0 && !bQuit)
	{
		if (IsInstance<System::Windows::Forms::ToolStripSeparator^>(ToolStrip->Items[ToolStrip->Items->Count - 1]))
		{
			ToolStrip->Items->RemoveAt(ToolStrip->Items->Count - 1);
		}
		else
		{
			bQuit = true;
		}
	}

	// Delete duplicate separators

	while (nIndex < ToolStrip->Items->Count)
	{
		if (IsInstance<System::Windows::Forms::ToolStripSeparator^>(ToolStrip->Items[nIndex]))
		{
			if (nIndex + 1 < ToolStrip->Items->Count)
			{
				if (IsInstance<System::Windows::Forms::ToolStripSeparator^>(ToolStrip->Items[nIndex + 1]))
				{
					ToolStrip->Items->RemoveAt(nIndex);

					++nIndex;
				}
				else
				{
					++nIndex;
				}
			}
			else
			{
				++nIndex;
			}
		}
		else
		{
			++nIndex;
		}
	}
}

static System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>^ lRemoveToolStripItemFromToolStripItemDict(
  System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>^ ToolStripItemDict,
  System::Collections::Generic::List<System::Windows::Forms::ToolStripItem^>^ ToolStripItemList)
{
	System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>^ UpdatedToolStripItemDict = gcnew System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>();
	System::Collections::IDictionaryEnumerator^ Enumerator = ToolStripItemDict->GetEnumerator();

	while (Enumerator->MoveNext())
	{
		if (!ToolStripItemList->Contains((System::Windows::Forms::ToolStripItem^)Enumerator->Value))
		{
			UpdatedToolStripItemDict->Add((System::Windows::Forms::ToolStripItem^)Enumerator->Key,
                                          (System::Windows::Forms::ToolStripItem^)Enumerator->Value);
		}
	}

	return UpdatedToolStripItemDict;
}

static System::Reflection::FieldInfo^ lGetEventsField()
{
	System::Type^ ComponentType = System::ComponentModel::Component::typeid;

	return ComponentType->GetField("events",
                                   System::Reflection::BindingFlags::NonPublic |
                                   System::Reflection::BindingFlags::Instance |
                                   System::Reflection::BindingFlags::GetField);
}

static void lCopyEventHandlers(
  System::Windows::Forms::ToolStripItem^ SourceToolStripItem,
  System::Windows::Forms::ToolStripItem^ DestToolStripItem)
{
	System::Reflection::FieldInfo^ FieldInfo = lGetEventsField();
	System::ComponentModel::EventHandlerList^ EventHandlerList;

	EventHandlerList = (System::ComponentModel::EventHandlerList^)FieldInfo->GetValue(SourceToolStripItem);

	FieldInfo->SetValue(DestToolStripItem, EventHandlerList);
}

static void lClearEventHandlers(
  System::Windows::Forms::ToolStripItem^ ToolStripItem)
{
	System::Reflection::FieldInfo^ FieldInfo = lGetEventsField();

	FieldInfo->SetValue(ToolStripItem, nullptr);
}

#pragma endregion

#pragma region "Constructor"

Common::Forms::MainForm::MainForm()
{
    m_pTaskbarList3 = NULL;
    m_nProgressBarTotal = 0;
    m_SelectedControl = nullptr;
    m_StatusStrip = nullptr;
    m_ToolStripStatusLabelHelp = nullptr;
    m_bFindStatusStrip = true;
    m_ToolStripItemVisibleDict = nullptr;
	m_ContextMenuStripList = nullptr;

	m_ToolStripDrownDownItemList = gcnew System::Collections::Generic::List<System::Windows::Forms::ToolStripDropDownItem^>();
    m_ContextMenuStripList = gcnew System::Collections::Generic::List<System::Windows::Forms::ContextMenuStrip^>();

	m_OpenedToolStripDropDownItemStack = gcnew System::Collections::Generic::Stack<System::Windows::Forms::ToolStripDropDownItem^>();
	m_OpenedContextMenuStrip = nullptr;

	m_nMenuOpenedCount = 0;

	m_LastMousePoint = gcnew System::Drawing::Point();

	m_ToolStripItemDict = gcnew System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>();
	m_ContextMenuToolStripItemDict = gcnew System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>();

	m_Timer = gcnew System::Windows::Forms::Timer;

	m_Timer->Tag = this;
	m_Timer->Interval = 200;

	m_Timer->Tick += gcnew System::EventHandler(TimerTick);

	m_Timer->Stop();
}

#pragma endregion

#pragma region "Deconstructor"

Common::Forms::MainForm::~MainForm()
{
	ClearMenus();

    DestroyMenuStatusStrip();

    m_StatusStrip = nullptr;
    m_SelectedControl = nullptr;

	m_Timer->Tag = nullptr;

	m_Timer->Tick -= gcnew System::EventHandler(TimerTick);

	m_Timer = nullptr;
}

#pragma endregion

System::Boolean Common::Forms::MainForm::BeginTaskbarListProgressBar(
  System::UInt64 nTotal)
{
    pin_ptr<ITaskbarList3*> pTaskbarList3 = &m_pTaskbarList3;

    if (m_pTaskbarList3 != NULL)
    {
        return false;
    }

    if (FALSE == lCreateTaskbarList3((HWND)this->Handle.ToPointer(),
                                     pTaskbarList3))
    {
        return false;
    }

    m_nProgressBarTotal = nTotal;

    return true;
}

System::Boolean Common::Forms::MainForm::UpdateTaskbarListProgressBar(
  System::UInt64 nValue)
{
    if (m_pTaskbarList3 == NULL)
    {
        return false;
    }

    if (S_OK == m_pTaskbarList3->SetProgressValue((HWND)this->Handle.ToPointer(),
                                                  nValue, m_nProgressBarTotal))
    {
        return true;
    }

    return false;
}

System::Boolean Common::Forms::MainForm::EndTaskbarListProgressBar()
{
    pin_ptr<ITaskbarList3*> pTaskbarList3 = &m_pTaskbarList3;

    if (m_pTaskbarList3 == NULL)
    {
        return false;
    }

    lDestroyTaskbarList3((HWND)this->Handle.ToPointer(),
                         pTaskbarList3);

    return true;
}

void Common::Forms::MainForm::RefreshMenuHelp()
{
	ClearMenus();

	RefreshMenuHelp(this);
}

void Common::Forms::MainForm::RunOnUIThreadWait(
	System::Action^ Action)
{
	this->Invoke(Action);
}

void Common::Forms::MainForm::RunOnUIThreadNoWait(
	System::Action^ Action)
{
	this->BeginInvoke(Action);
}

void Common::Forms::MainForm::InitToolStripItems(
  System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem,
  System::Windows::Forms::ToolStrip^ ToolStrip)
{
	System::Windows::Forms::ToolStripButton^ ToolStripButton;
	System::Windows::Forms::ToolStripSeparator^ ToolStripSeparator;

	//Common::Debug::Thread::IsUIThread();

	for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in ToolStripMenuItem->DropDownItems)
	{
		if (IsInstance<System::Windows::Forms::ToolStripMenuItem^>(ToolStripItem))
		{
			if (!System::String::IsNullOrEmpty(ToolStripItem->ImageKey))
			{
				ToolStripMenuItem = (System::Windows::Forms::ToolStripMenuItem^)ToolStripItem;

				ToolStripButton = gcnew System::Windows::Forms::ToolStripButton();

				ToolStripButton->Name = ToolStripItem->Name->Replace("menuItem", "toolStripButton");
				ToolStripButton->ImageKey = ToolStripItem->ImageKey;
				ToolStripButton->Enabled = ToolStripItem->Enabled;
				ToolStripButton->ToolTipText = lGenerateToolTipText(ToolStripMenuItem);

				lCopyEventHandlers(ToolStripItem, ToolStripButton);

				ToolStrip->Items->Add(ToolStripButton);

				m_ToolStripItemDict->Add(ToolStripItem, ToolStripButton);
			}
		}
		else if (IsInstance<System::Windows::Forms::ToolStripSeparator^>(ToolStripItem))
		{
			ToolStripSeparator = gcnew System::Windows::Forms::ToolStripSeparator();

			ToolStripSeparator->Name = ToolStripItem->Name;

			ToolStrip->Items->Add(ToolStripSeparator);
		}
		else
		{
			System::Diagnostics::Debug::Assert(false, "Unsupported ToolStripItem");
		}
	}

	lRemoveExtraneousSeparators(ToolStrip);
}

void Common::Forms::MainForm::UninitToolStripItems(
  System::Windows::Forms::ToolStrip^ ToolStrip)
{
	System::Collections::Generic::List<System::Windows::Forms::ToolStripItem^>^ ToolStripItemList = gcnew System::Collections::Generic::List<System::Windows::Forms::ToolStripItem^>();
	System::Windows::Forms::ToolStripButton^ ToolStripButton;

	//Common::Debug::Thread::IsUIThread();

	for each(System::Windows::Forms::ToolStripItem^ ToolStripItem in ToolStrip->Items)
	{
		if (IsInstance<System::Windows::Forms::ToolStripButton^>(ToolStripItem))
		{
			ToolStripButton = (System::Windows::Forms::ToolStripButton^)ToolStripItem;

			lClearEventHandlers(ToolStripItem);

			ToolStripItemList->Add(ToolStripButton);
		}
	}

	m_ToolStripItemDict = lRemoveToolStripItemFromToolStripItemDict(m_ToolStripItemDict,
                                                                    ToolStripItemList);

	ToolStrip->Items->Clear();
}

void Common::Forms::MainForm::InitContextMenuItems(
  System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem,
  System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip)
{
	Common::Forms::ToolStripMenuItem^ ContextToolStripMenuItem;
	System::Windows::Forms::ToolStripSeparator^ ToolStripSeparator;

	//Common::Debug::Thread::IsUIThread();

	for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in ToolStripMenuItem->DropDownItems)
	{
		if (IsInstance<Common::Forms::ToolStripMenuItem^>(ToolStripItem))
		{
			ContextToolStripMenuItem = gcnew Common::Forms::ToolStripMenuItem();

			ContextToolStripMenuItem->Name = ToolStripItem->Name->Replace("menuItem", "contextMenuItem");
			ContextToolStripMenuItem->Text = ToolStripItem->Text;
			ContextToolStripMenuItem->ImageKey = ToolStripItem->ImageKey;
			ContextToolStripMenuItem->Enabled = ToolStripItem->Enabled;
			ContextToolStripMenuItem->HelpText = ((Common::Forms::ToolStripMenuItem^)ToolStripItem)->HelpText;

			lCopyEventHandlers(ToolStripItem, ContextToolStripMenuItem);

			ContextMenuStrip->Items->Add(ContextToolStripMenuItem);

			m_ContextMenuToolStripItemDict->Add(ToolStripItem,
				                                ContextToolStripMenuItem);
		}
		else if (IsInstance<System::Windows::Forms::ToolStripSeparator^>(ToolStripItem))
		{
			ToolStripSeparator = gcnew System::Windows::Forms::ToolStripSeparator();

			ToolStripSeparator->Name = ToolStripItem->Name;

			ContextMenuStrip->Items->Add(ToolStripSeparator);
		}
		else
		{
			System::Diagnostics::Debug::Assert(false, "Unsupported ToolStripItem");
		}
	}

	lRemoveExtraneousSeparators(ContextMenuStrip);
}

void Common::Forms::MainForm::UninitContextMenuItems(
  System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip)
{
	System::Collections::Generic::List<System::Windows::Forms::ToolStripItem^>^ ToolStripItemList = gcnew System::Collections::Generic::List<System::Windows::Forms::ToolStripItem^>();
	Common::Forms::ToolStripMenuItem^ ContextToolStripMenuItem;

	//Common::Debug::Thread::IsUIThread();

	for each(System::Windows::Forms::ToolStripItem^ ToolStripItem in ContextMenuStrip->Items)
	{
		if (IsInstance<Common::Forms::ToolStripMenuItem^>(ToolStripItem))
		{
			ContextToolStripMenuItem = (Common::Forms::ToolStripMenuItem^)ToolStripItem;

			lClearEventHandlers(ToolStripItem);

			ToolStripItemList->Add(ContextToolStripMenuItem);
		}
	}

	m_ToolStripItemDict = lRemoveToolStripItemFromToolStripItemDict(m_ContextMenuToolStripItemDict,
                                                                    ToolStripItemList);

	ContextMenuStrip->Items->Clear();
}

#pragma region "IProcessMessage"

void Common::Forms::MainForm::ProcessKeyDown(
  System::Windows::Forms::Control^ control,
  System::Int32 nVirtKey,
  System::Int32 nData)
{
    control;
    nVirtKey;
    nData;
}

void Common::Forms::MainForm::ProcessKeyUp(
	System::Windows::Forms::Control^ control,
	System::Int32 nVirtKey,
	System::Int32 nData)
{
	System::Windows::Forms::ToolStripItemCollection^ toolStripItemCollection;

	control;
	nVirtKey;
	nData;

	if (m_nMenuOpenedCount > 0)
	{
		if (m_OpenedToolStripDropDownItemStack->Count > 0)
		{
			toolStripItemCollection = m_OpenedToolStripDropDownItemStack->Peek()->DropDownItems;
		}
		else
		{
			toolStripItemCollection = m_OpenedContextMenuStrip->Items;
		}

		for each (System::Windows::Forms::ToolStripItem^ toolStripItem in toolStripItemCollection)
		{
			if (toolStripItem->Selected)
			{
				ShowStatusLabelHelp(toolStripItem);

				return;
			}
		}
	}
}

void Common::Forms::MainForm::ProcessMouseMove(
  System::Windows::Forms::Control^ control,
  System::Int32 nXPos,
  System::Int32 nYPos)
{
	System::Windows::Forms::ToolStrip^ ToolStrip;
	System::Windows::Forms::ToolStripItem^ ToolStripItem;
	POINT Point;

	if (m_nMenuOpenedCount > 0)
	{
		if (IsInstance<System::Windows::Forms::ToolStrip^>(control))
		{
			Point.x = nXPos;
			Point.y = nYPos;

			::ClientToScreen((HWND)control->Handle.ToPointer(), &Point);

			if (m_LastMousePoint->X != Point.x || m_LastMousePoint->Y != Point.y)
			{
				m_LastMousePoint->X = Point.x;
				m_LastMousePoint->Y = Point.y;

				ToolStrip = (System::Windows::Forms::ToolStrip^)control;

				ToolStripItem = ToolStrip->GetItemAt(nXPos, nYPos);

				ShowStatusLabelHelp(ToolStripItem);
			}
		}
		else
		{
			m_LastMousePoint->X = nXPos;
			m_LastMousePoint->Y = nYPos;

			ShowStatusLabelHelp(nullptr);
		}
	}
}

#pragma endregion

#pragma region "IUpdateToolStripItems"

void Common::Forms::MainForm::UpdateToolStripItems(
	Common::Forms::ToolStripMenuItem^ ToolStripMenuItem)
{
	System::Windows::Forms::ToolStripItem^ ToolStripItem;

	if (m_ToolStripItemDict->ContainsKey(ToolStripMenuItem))
	{
		ToolStripItem = m_ToolStripItemDict[ToolStripMenuItem];

		ToolStripItem->Enabled = ToolStripMenuItem->Enabled;
	}

	if (m_ContextMenuToolStripItemDict->ContainsKey(ToolStripMenuItem))
	{
		ToolStripItem = m_ContextMenuToolStripItemDict[ToolStripMenuItem];

		ToolStripItem->Enabled = ToolStripMenuItem->Enabled;
	}
}

#pragma endregion

#pragma region "Form Overrides"

void Common::Forms::MainForm::OnLoad(
  System::EventArgs^ e)
{
	System::Windows::Forms::Form::OnLoad(e);

	RefreshMenuHelp();
}

#pragma endregion

#pragma region "Internal Helpers"

void Common::Forms::MainForm::RefreshMenuHelp(
  System::Windows::Forms::Control^ parentControl)
{
	System::Windows::Forms::MenuStrip^ MenuStrip;
	System::Windows::Forms::TextBox^ TextBox;
	System::Windows::Forms::ToolStripContainer^ ToolStripContainer;

	for each (System::Windows::Forms::Control^ control in parentControl->Controls)
	{
		if (IsInstance<System::Windows::Forms::MenuStrip^>(control))
		{
			MenuStrip = (System::Windows::Forms::MenuStrip^)control;

			EnumMenuStrip(MenuStrip);
		}
		else if (IsInstance<System::Windows::Forms::TextBox^>(control))
		{
			TextBox = (System::Windows::Forms::TextBox^)control;

			if (TextBox->ContextMenuStrip != nullptr)
			{
				TextBox->ContextMenuStrip->Opened += gcnew System::EventHandler(this, &Common::Forms::MainForm::ContextMenuOpened);
				TextBox->ContextMenuStrip->Closed += gcnew System::Windows::Forms::ToolStripDropDownClosedEventHandler(this, &Common::Forms::MainForm::ContextMenuClosed);

				m_ContextMenuStripList->Add(TextBox->ContextMenuStrip);

				EnumContextMenuStrip(TextBox->ContextMenuStrip);
			}
		}
		else if (IsInstance<System::Windows::Forms::ToolStripContainer^>(control)) {
			ToolStripContainer = (System::Windows::Forms::ToolStripContainer^)control;

			EnumToolStripContainer(ToolStripContainer);
		}
	}
}

void Common::Forms::MainForm::CreateMenuStatusStrip()
{
    if (m_bFindStatusStrip)
    {
        m_bFindStatusStrip = false;

        m_StatusStrip = lFindStatusStrip(this);
    }

    if (m_StatusStrip == nullptr || m_ToolStripStatusLabelHelp != nullptr)
    {
        return;
    }

    m_ToolStripStatusLabelHelp = gcnew System::Windows::Forms::ToolStripStatusLabel();

    m_ToolStripStatusLabelHelp->Name = L"toolStripMenuStripStatusLabel";

    m_StatusStrip->Items->Add(m_ToolStripStatusLabelHelp);
}

void Common::Forms::MainForm::DestroyMenuStatusStrip()
{
    System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Boolean>::Enumerator^ ToolStripItemVisibleDictEnum;

    if (m_StatusStrip == nullptr)
    {
        return;
    }

    m_StatusStrip->Items->Remove(m_ToolStripStatusLabelHelp);

    m_ToolStripStatusLabelHelp = nullptr;

    if (m_ToolStripItemVisibleDict != nullptr)
    {
        ToolStripItemVisibleDictEnum = m_ToolStripItemVisibleDict->GetEnumerator();

        while (ToolStripItemVisibleDictEnum->MoveNext())
        {
            ToolStripItemVisibleDictEnum->Current.Key->Visible = ToolStripItemVisibleDictEnum->Current.Value;
        }

        delete m_ToolStripItemVisibleDict;

        m_ToolStripItemVisibleDict = nullptr;
    }
}

void Common::Forms::MainForm::ShowStatusLabelHelp(
  System::Windows::Forms::ToolStripItem^ ToolStripItem)
{
	Common::Forms::ToolStripMenuItem^ ToolStripMenuItem;
	System::String^ HelpText;

	if (IsInstance<Common::Forms::ToolStripMenuItem^>(ToolStripItem))
	{
		ToolStripMenuItem = (Common::Forms::ToolStripMenuItem^)ToolStripItem;

		HelpText = ToolStripMenuItem->HelpText;
	}
	else
	{
		HelpText = "";
	}

	if (m_ToolStripItemVisibleDict == nullptr)
    {
        m_ToolStripItemVisibleDict = gcnew System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Boolean>();

        for each (System::Windows::Forms::ToolStripItem^ toolStripItem in m_StatusStrip->Items)
        {
            m_ToolStripItemVisibleDict->Add(toolStripItem, toolStripItem->Visible);

            toolStripItem->Visible = false;
        }

        m_ToolStripStatusLabelHelp->Visible = true;
    }

    m_ToolStripStatusLabelHelp->Text = HelpText;
}

void Common::Forms::MainForm::EnumMenuStrip(
  System::Windows::Forms::MenuStrip^ MenuStrip)
{
	System::Windows::Forms::ToolStripDropDownItem^ ToolStripDropDownItem;

	for each (System::Windows::Forms::ToolStripItem^ toolStripItem in MenuStrip->Items)
	{
		if (IsInstance<System::Windows::Forms::ToolStripDropDownItem^>(toolStripItem))
		{
			ToolStripDropDownItem = (System::Windows::Forms::ToolStripDropDownItem^)toolStripItem;

			ToolStripDropDownItem->DropDownOpened += gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownOpened);
			ToolStripDropDownItem->DropDownClosed += gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownClosed);

			m_ToolStripDrownDownItemList->Add(ToolStripDropDownItem);

			EnumToolStripDropDownItem(ToolStripDropDownItem);
    	}
	}
}

void Common::Forms::MainForm::EnumContextMenuStrip(
  System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip)
{
	System::Windows::Forms::ToolStripDropDownItem^ ToolStripDropDownItem;

	for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in ContextMenuStrip->Items)
	{
		if (IsInstance<System::Windows::Forms::ToolStripDropDownItem^>(ToolStripItem))
		{
			ToolStripDropDownItem = (System::Windows::Forms::ToolStripDropDownItem^)ToolStripItem;

			if (ToolStripDropDownItem->HasDropDownItems)
			{
				ToolStripDropDownItem->DropDownOpened += gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownOpened);
				ToolStripDropDownItem->DropDownClosed += gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownClosed);

				m_ToolStripDrownDownItemList->Add(ToolStripDropDownItem);

				EnumToolStripDropDownItem(ToolStripDropDownItem);
			}
		}
	}
}

void Common::Forms::MainForm::EnumToolStripDropDownItem(
  System::Windows::Forms::ToolStripDropDownItem^ ToolStripDropDownItem)
{
	System::Windows::Forms::ToolStripDropDownItem^ ChildToolStripDropDownItem;

	for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in ToolStripDropDownItem->DropDownItems)
	{
		if (IsInstance<System::Windows::Forms::ToolStripDropDownItem^>(ToolStripItem))
		{
			ChildToolStripDropDownItem = (System::Windows::Forms::ToolStripDropDownItem^)ToolStripItem;

			if (ChildToolStripDropDownItem->DropDownItems->Count > 0)
			{
    			ChildToolStripDropDownItem->DropDownOpened += gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownOpened);
	    		ChildToolStripDropDownItem->DropDownClosed += gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownClosed);

		    	m_ToolStripDrownDownItemList->Add(ChildToolStripDropDownItem);

				EnumToolStripDropDownItem(ChildToolStripDropDownItem);
			}
		}
	}
}

void Common::Forms::MainForm::EnumToolStripContainer(
  System::Windows::Forms::ToolStripContainer^ ToolStripContainer)
{
	for each(System::Windows::Forms::Control^ Control in ToolStripContainer->Controls)
	{
		RefreshMenuHelp(Control);
	}
}

void Common::Forms::MainForm::ClearMenus()
{
	for each (System::Windows::Forms::ToolStripDropDownItem^ toolStripDropDownItem in m_ToolStripDrownDownItemList)
	{
		toolStripDropDownItem->DropDownOpened -= gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownOpened);
		toolStripDropDownItem->DropDownClosed -= gcnew System::EventHandler(this, &Common::Forms::MainForm::DropDownClosed);
	}
	
	m_ToolStripDrownDownItemList->Clear();

	for each (System::Windows::Forms::ContextMenuStrip^ contextMenuStrip in m_ContextMenuStripList)
	{
		contextMenuStrip->Opened -= gcnew System::EventHandler(this, &Common::Forms::MainForm::ContextMenuOpened);
		contextMenuStrip->Closed -= gcnew System::Windows::Forms::ToolStripDropDownClosedEventHandler(this, &Common::Forms::MainForm::ContextMenuClosed);
	}

	m_ContextMenuStripList->Clear();
}

void Common::Forms::MainForm::IncreaseMenuOpenedCount()
{
	++m_nMenuOpenedCount;

	if (m_nMenuOpenedCount == 1)
	{
		CreateMenuStatusStrip();

		m_LastMousePoint->X = GET_X_LPARAM(::GetMessagePos());
		m_LastMousePoint->Y = GET_Y_LPARAM(::GetMessagePos());

		m_Timer->Start();
	}

	ShowStatusLabelHelp(nullptr);
}

void Common::Forms::MainForm::DecreaseMenuOpenedCount()
{
	--m_nMenuOpenedCount;

	if (m_nMenuOpenedCount == 0)
	{
		m_Timer->Stop();

		DestroyMenuStatusStrip();
	}
	else
	{
		ShowStatusLabelHelp(nullptr);
	}
}

void Common::Forms::MainForm::DropDownOpened(
  System::Object^ sender,
  System::EventArgs^ e)
{
	sender;
	e;

	m_OpenedToolStripDropDownItemStack->Push((System::Windows::Forms::ToolStripDropDownItem^)sender);

	IncreaseMenuOpenedCount();
}

void Common::Forms::MainForm::DropDownClosed(
  System::Object^ sender,
  System::EventArgs^ e)
{
	sender;
	e;

	m_OpenedToolStripDropDownItemStack->Pop();

	DecreaseMenuOpenedCount();
}

void Common::Forms::MainForm::ContextMenuOpened(
  System::Object^ sender,
  System::EventArgs^ e)
{
	sender;
	e;

	m_OpenedContextMenuStrip = (System::Windows::Forms::ContextMenuStrip^)sender;

	IncreaseMenuOpenedCount();
}

void Common::Forms::MainForm::ContextMenuClosed(
  System::Object^ sender,
  System::Windows::Forms::ToolStripDropDownClosedEventArgs^ e)
{
	sender;
	e;

	m_OpenedContextMenuStrip = nullptr;

	DecreaseMenuOpenedCount();
}

void Common::Forms::MainForm::TimerTick(
  System::Object^ sender, 
  System::EventArgs^ e)
{
	System::Windows::Forms::Timer^ timer = (System::Windows::Forms::Timer^)sender;
	MainForm^ mainForm = (MainForm^)timer->Tag;
	Control^ control;
	POINT Point;
	HWND hWnd;

	e;

	Point.x = GET_X_LPARAM(::GetMessagePos());
	Point.y = GET_Y_LPARAM(::GetMessagePos());

	if (mainForm->m_LastMousePoint->X == Point.x && mainForm->m_LastMousePoint->Y == Point.y)
	{
		return;
    }

	mainForm->m_LastMousePoint->X = Point.x;
	mainForm->m_LastMousePoint->Y = Point.y;

	hWnd = ::WindowFromPoint(Point);

	control = System::Windows::Forms::Control::FromHandle(System::IntPtr::IntPtr(hWnd));

	if (IsInstance<System::Windows::Forms::ToolStrip^>(control))
	{
		::ScreenToClient(hWnd, &Point);
	}

	mainForm->ProcessMouseMove(control, Point.x, Point.y);
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
