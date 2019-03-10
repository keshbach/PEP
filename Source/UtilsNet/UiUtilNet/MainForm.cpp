/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MainForm.h"
#include "Application.h"

#include "Includes/UtTemplates.h"

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

Common::Forms::MainForm::MainForm()
{
    m_pTaskbarList3 = NULL;
    m_nProgressBarTotal = 0;
    m_MenuItemHelpList = gcnew System::Collections::Generic::List<MenuItemHelp^>();
    m_SelectedControl = nullptr;
    m_StatusStrip = nullptr;
    m_ToolStripStatusLabelHelp = nullptr;
    m_bFindStatusStrip = true;
    m_ToolStripItemVisibleDict = nullptr;


    m_MenuItemHelpList2 = gcnew Common::Forms::MenuItemHelpList();



    s_MainFormDesignMode = nullptr;

    Common::Forms::Application::ProcessMessageList->Add(this);
}

Common::Forms::MainForm::~MainForm()
{
    DestroyMenuStatusStrip();

    Common::Forms::Application::ProcessMessageList->Remove(this);

    delete m_MenuItemHelpList;

    m_MenuItemHelpList = nullptr;
    m_StatusStrip = nullptr;
    m_SelectedControl = nullptr;

    s_MainFormDesignMode = nullptr;
}

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

// IProcessMessage overrides

void Common::Forms::MainForm::ProcessKeyDown(
  System::Windows::Forms::Control^ control,
  System::Int32 nVirtKey,
  System::Int32 nData)
{
    control;
    nVirtKey;
    nData;

    CreateMenuStatusStrip();
}

void Common::Forms::MainForm::ProcessSysKeyDown(
  System::Windows::Forms::Control^ control,
  System::Int32 nVirtKey,
  System::Int32 nData)
{
    control;
    nVirtKey;
    nData;

    CreateMenuStatusStrip();
}

void Common::Forms::MainForm::ProcessLeftButtonDown(
  System::Windows::Forms::Control^ control,
  System::Int32 nXPos,
  System::Int32 nYPos)
{
    control;
    nXPos;
    nYPos;

    CreateMenuStatusStrip();
}

void Common::Forms::MainForm::ProcessLeftButtonUp(
  System::Windows::Forms::Control^ control,
  System::Int32 nXPos,
  System::Int32 nYPos)
{
    control;
    nXPos;
    nYPos;

    CreateMenuStatusStrip();
}

void Common::Forms::MainForm::ProcessMouseMove(
    System::Windows::Forms::Control^ control,
    System::Int32 nXPos,
    System::Int32 nYPos)
{
    System::Windows::Forms::ToolStrip^ ToolStrip = nullptr;
    System::Windows::Forms::ToolStripItem^ ToolStripItem;

    nXPos;
    nYPos;

    CreateMenuStatusStrip();

    if (IsInstance<System::Windows::Forms::ToolStrip^>(control))
    {
        m_SelectedControl = control;

        if (IsInstance<System::Windows::Forms::MenuStrip^>(control) ||
            IsInstance<System::Windows::Forms::ToolStripDropDownMenu^>(control))
        {
            ToolStrip = dynamic_cast<System::Windows::Forms::ToolStrip^>(control);

            ToolStripItem = ToolStrip->GetItemAt(nXPos, nYPos);

            if (ToolStripItem != nullptr)
            {
                ShowStatusLabelHelp(FindToolStripItemHelpText(ToolStripItem));
            }
            else
            {
                ShowStatusLabelHelp("");
            }
        }
        else
        {
            // Unknown control type

            ShowStatusLabelHelp("");
        }
    }
    else
    {
        DestroyMenuStatusStrip();

        m_SelectedControl = nullptr;
    }
}

void Common::Forms::MainForm::OnHandleCreated(
  System::EventArgs^ e)
{
    if (this->DesignMode)
    {
        s_MainFormDesignMode = this;
    }

    System::Windows::Forms::Form::OnHandleCreated(e);
}

void Common::Forms::MainForm::CreateMenuStatusStrip()
{
    if (m_bFindStatusStrip)
    {
        m_bFindStatusStrip = false;

        m_StatusStrip = FindStatusStrip();
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
  System::String^ text)
{
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

    m_ToolStripStatusLabelHelp->Text = text;
}

System::Windows::Forms::StatusStrip^ Common::Forms::MainForm::FindStatusStrip()
{
    for each (System::Windows::Forms::Control^ control in this->Controls)
    {
        if (IsInstance<System::Windows::Forms::StatusStrip^>(control))
        {
            return dynamic_cast<System::Windows::Forms::StatusStrip^>(control);
        }
    }

    return nullptr;
}

System::String^ Common::Forms::MainForm::FindToolStripItemHelpText(
  System::Windows::Forms::ToolStripItem^ ToolStripItem)
{
    for each (Common::Forms::MenuItemHelp^ menuItemHelp in Common::Forms::Application::MainForm->MenuStripHelp)
    {
        if (System::Object::ReferenceEquals(menuItemHelp->MenuItem, ToolStripItem))
        {
            return menuItemHelp->HelpText;
        }
    }

    return L"";
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
