/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ITextBoxClipboard.h"
#include "ContextMenuStrip.h"
#include "EditContextMenuStrip.h"
#include "TextBox.h"
#include "TextBoxTimer.h"

Common::Forms::TextBoxTimer::TextBoxTimer() :
  m_dTimerDelay(1.0),
  m_bTimerStarted(false)
{
    m_Timer = gcnew System::Windows::Forms::Timer;

    m_Timer->Tag = this;
    m_Timer->Interval = 1000;

    m_Timer->Tick += gcnew System::EventHandler(TimerEventProcessor);

    m_Timer->Stop();

	InitializeComponent();
}

Common::Forms::TextBoxTimer::~TextBoxTimer()
{
    if (m_Timer != nullptr)
    {
        m_Timer->Stop();

        m_Timer->Tag = nullptr;

		m_Timer->Tick -= gcnew System::EventHandler(TimerEventProcessor);

        delete m_Timer;

        m_Timer = nullptr;
    }

	if (components)
	{
		delete components;
	}
}

void Common::Forms::TextBoxTimer::CancelKeyPressTimer()
{
    m_Timer->Stop();

	m_bTimerStarted = false;
}

void Common::Forms::TextBoxTimer::StartKeyPressTimer()
{
    m_Timer->Stop();
    m_Timer->Start();

    m_bTimerStarted = true;
}

void Common::Forms::TextBoxTimer::WndProc(
  System::Windows::Forms::Message% Message)
{
    switch (Message.Msg)
    {
        case WM_CHAR:
            HandleChar(Message);
            break;
        case WM_KEYDOWN:
            HandleKeyDown(Message);
            break;
        default:
            Common::Forms::TextBox::WndProc(Message);
            break;
    }
}

void Common::Forms::TextBoxTimer::HandleChar(
  System::Windows::Forms::Message% Message)
{
    m_Timer->Stop();

    Common::Forms::TextBox::WndProc(Message);

    m_Timer->Start();

    m_bTimerStarted = true;
}

void Common::Forms::TextBoxTimer::HandleKeyDown(
  System::Windows::Forms::Message% Message)
{
    WPARAM wParam = (WPARAM)Message.WParam.ToPointer();

    m_Timer->Stop();

    Common::Forms::TextBox::WndProc(Message);

    switch (wParam)
    {
        case VK_DELETE:
        case VK_BACK:
            m_Timer->Start();

            m_bTimerStarted = true;
            break;
        default:
            if (m_bTimerStarted)
            {
                m_Timer->Start();
            }
            break;
    }
}

void Common::Forms::TextBoxTimer::TimerEventProcessor(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    System::Windows::Forms::Timer^ Timer = (System::Windows::Forms::Timer^)Object;
    Common::Forms::TextBoxTimer^ TextBoxTimer = (Common::Forms::TextBoxTimer^)Timer->Tag;
    System::EventArgs^ EventArg = gcnew System::EventArgs();

    EventArgs;

	TextBoxTimer->CancelKeyPressTimer();

    TextBoxTimer->OnKeyPressTimerExpired(EventArg);
}

void Common::Forms::TextBoxTimer::OnKeyPressTimerExpired(
  System::EventArgs^ e)
{
    KeyPressTimerExpired(this, e);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
