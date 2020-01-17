/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextBox.h"

Common::Forms::TextBox::TextBox() :
  m_dTimerDelay(1.0),
  m_bTimerStarted(false)
{
    m_Timer = gcnew Common::Forms::FormTimer;

    m_Timer->FormObject = this;
    m_Timer->Interval = 1000;

    m_Timer->Tick += gcnew System::EventHandler(TimerEventProcessor);

    m_Timer->Stop();

	InitializeComponent();
}

Common::Forms::TextBox::~TextBox()
{
    if (m_Timer != nullptr)
    {
        m_Timer->Stop();

        delete m_Timer;

        m_Timer = nullptr;
    }

	if (components)
	{
		delete components;
	}
}

void Common::Forms::TextBox::CancelKeyPressTimer()
{
    m_Timer->Stop();

	m_bTimerStarted = false;
}

void Common::Forms::TextBox::WndProc(
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
            System::Windows::Forms::TextBox::WndProc(Message);
            break;
    }
}

void Common::Forms::TextBox::HandleChar(
  System::Windows::Forms::Message% Message)
{
    m_Timer->Stop();

    System::Windows::Forms::TextBox::WndProc(Message);

    m_Timer->Start();

    m_bTimerStarted = true;
}

void Common::Forms::TextBox::HandleKeyDown(
  System::Windows::Forms::Message% Message)
{
    WPARAM wParam = (WPARAM)Message.WParam.ToPointer();

    m_Timer->Stop();

    System::Windows::Forms::TextBox::WndProc(Message);

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

void Common::Forms::TextBox::TimerEventProcessor(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Common::Forms::FormTimer^ FormTimer = (Common::Forms::FormTimer^)Object;
    Common::Forms::TextBox^ TextBox = (Common::Forms::TextBox^)FormTimer->FormObject;
    System::EventArgs^ EventArg = gcnew System::EventArgs();

    EventArgs;

	TextBox->CancelKeyPressTimer();

    TextBox->OnKeyPressTimerExpired(EventArg);
}

void Common::Forms::TextBox::OnKeyPressTimerExpired(
  System::EventArgs^ e)
{
    KeyPressTimerExpired(this, e);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
