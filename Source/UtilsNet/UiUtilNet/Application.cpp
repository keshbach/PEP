/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Application.h"
#include "ImageManager.h"

#if defined(NEW_MENU_CODE)
static HHOOK l_hHook = NULL;

static LRESULT CALLBACK lGetMsgProc(
  int nCode,
  WPARAM wParam,
  LPARAM lParam)
{
    LRESULT Result = ::CallNextHookEx(l_hHook, nCode, wParam, lParam);
    LPMSG pMsg = (LPMSG)lParam;
    System::Windows::Forms::Message^ message;
    System::Windows::Forms::Control^ control;

    if (nCode < 0)
    {
        return Result;
    }

    if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN ||
        pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP ||
        pMsg->message == WM_MOUSEMOVE)
    {
        control = System::Windows::Forms::Control::FromHandle(Common::Forms::Application::s_ApplicationContext->MainForm->Handle);

        if (control != nullptr)
        {
            message = System::Windows::Forms::Message::Create(System::IntPtr::IntPtr(pMsg->hwnd), pMsg->message, System::IntPtr::IntPtr((LPVOID)pMsg->wParam), System::IntPtr::IntPtr(pMsg->lParam));

            control->BeginInvoke(Common::Forms::Application::s_GetMessageCallback, message);
        }
    }

    return Result;
}
#endif

System::Boolean Common::Forms::Application::Run(Common::Forms::MainForm^ MainForm)
{
    if (!Initialize(MainForm))
    {
        return false;
    }

    System::Windows::Forms::Application::Run(s_ApplicationContext);

    Uninitialize();

    return true;
}

System::Boolean Common::Forms::Application::Initialize(Common::Forms::MainForm^ MainForm)
{
    if (Common::Forms::ImageManager::Initialize())
    {
        if (s_ProcessMessage == nullptr)
        {
            s_ProcessMessage = gcnew Common::Forms::ProcessMessage();
        }

        if (s_ApplicationContext == nullptr)
        {
            s_ApplicationContext = gcnew Common::Forms::MyApplicationContext(MainForm);
        }

#if defined(NEW_MENU_CODE)
        s_GetMessageCallback = gcnew Common::Forms::GetMessageCallback(s_ProcessMessage, &Common::Forms::ProcessMessage::GetMessageCallback);

        l_hHook = ::SetWindowsHookEx(WH_GETMESSAGE, lGetMsgProc, NULL, ::GetCurrentThreadId());
#endif

        return true;
    }

    return false;
}

System::Boolean Common::Forms::Application::Uninitialize()
{
#if defined(NEW_MENU_CODE)
	::UnhookWindowsHookEx(l_hHook);

    delete s_GetMessageCallback;
#endif
    delete s_ApplicationContext;
    delete s_ProcessMessage;

    return Common::Forms::ImageManager::Uninitialize();
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
