/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Application.h"

#include "ImageManager.h"

#include "MyThreadContext.h"

#include "IProcessMessage.h"

#include "Includes/UtTemplates.h"

static void lHandleProcessMessage(
  Common::Forms::IProcessMessage^ ProcessMessage,
  LPMSG pMsg)
{
	switch (pMsg->message)
	{
		case WM_KEYDOWN:
			ProcessMessage->ProcessKeyDown(System::Windows::Forms::Control::FromHandle(System::IntPtr::IntPtr(pMsg->hwnd)),
                                           (int)pMsg->wParam, (int)pMsg->lParam);
			break;
		case WM_KEYUP:
			ProcessMessage->ProcessKeyUp(System::Windows::Forms::Control::FromHandle(System::IntPtr::IntPtr(pMsg->hwnd)),
				                         (int)pMsg->wParam, (int)pMsg->lParam);
			break;
		case WM_MOUSEMOVE:
			ProcessMessage->ProcessMouseMove(System::Windows::Forms::Control::FromHandle(System::IntPtr::IntPtr(pMsg->hwnd)),
				                             GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam));
			break;
	}
}

System::Boolean Common::Forms::Application::Run(Common::Forms::MainForm^ MainForm)
{
	MyThreadContext^ ThreadContext = gcnew MyThreadContext();
	BOOL bQuit = FALSE;
	BOOL bResult;
	MSG Msg;
	System::Windows::Forms::Control^ Control;
	System::Windows::Forms::Message^ Message;
	System::Windows::Forms::IMessageFilter^ MessageFilter;

	if (!Initialize(MainForm))
    {
        return false;
    }

	s_ApplicationContext->ThreadExit += gcnew System::EventHandler(ThreadContext, &MyThreadContext::OnThreadExit);

	MainForm->Visible = true;

	while (!bQuit)
	{
		bResult = ::GetMessage(&Msg, NULL, 0, 0);

		if (bResult == TRUE)
		{
			if (Msg.message == WM_KEYDOWN &&
				IsInstance<System::Windows::Forms::IMessageFilter^>(MainForm))
			{
				MessageFilter = (System::Windows::Forms::IMessageFilter^)MainForm;

				Message = System::Windows::Forms::Message::Create(System::IntPtr::IntPtr(Msg.hwnd),
					                                              Msg.message,
					                                              System::IntPtr::IntPtr((LPVOID)Msg.wParam),
					                                              System::IntPtr::IntPtr(Msg.lParam));

				if (MessageFilter->PreFilterMessage(*Message))
				{
					continue;
				}
			}

			lHandleProcessMessage(MainForm, &Msg);

			Control = System::Windows::Forms::Control::FromHandle(System::IntPtr::IntPtr(Msg.hwnd));

			if (Control != nullptr)
			{
				Message = System::Windows::Forms::Message::Create(System::IntPtr::IntPtr(Msg.hwnd),
					                                              Msg.message,
					                                              System::IntPtr::IntPtr((LPVOID)Msg.wParam),
					                                              System::IntPtr::IntPtr(Msg.lParam));

				if (Control->PreProcessMessage(*Message))
				{
					continue;
				}
			}

			::TranslateMessage(&Msg);
			::DispatchMessage(&Msg);
		}
		else if (bResult == FALSE)
		{
			bQuit = TRUE;
		}
		else if (bResult == -1)
		{
			// Error returned
		}
	}

	s_ApplicationContext->ThreadExit -= gcnew System::EventHandler(ThreadContext, &MyThreadContext::OnThreadExit);

    Uninitialize();

    return true;
}

#pragma region "Internal Helpers"

System::Boolean Common::Forms::Application::Initialize(Common::Forms::MainForm^ MainForm)
{
    if (Common::Forms::ImageManager::Initialize())
    {
        if (s_ApplicationContext == nullptr)
        {
            s_ApplicationContext = gcnew Common::Forms::MyApplicationContext(MainForm);
        }

        return true;
    }

    return false;
}

System::Boolean Common::Forms::Application::Uninitialize()
{
    delete s_ApplicationContext;

    return Common::Forms::ImageManager::Uninitialize();
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
