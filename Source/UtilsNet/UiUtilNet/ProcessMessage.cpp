/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProcessMessage.h"

Common::Forms::ProcessMessage::ProcessMessage()
{
    m_ProcessMessageList = gcnew System::Collections::Generic::List<IProcessMessage^>();
}

Common::Forms::ProcessMessage::~ProcessMessage()
{
    this->!ProcessMessage();
}

Common::Forms::ProcessMessage::!ProcessMessage()
{
    delete m_ProcessMessageList;

    m_ProcessMessageList = nullptr;
}

void Common::Forms::ProcessMessage::GetMessageCallback(
  System::Windows::Forms::Message^ msg)
{
    System::Windows::Forms::Control^ control;

    control = System::Windows::Forms::Control::FromHandle(msg->HWnd);

    if (control == nullptr)
    {
        return;
    }

    for each (IProcessMessage^ ProcessMessage in m_ProcessMessageList)
    {
        switch (msg->Msg)
        {
            case WM_KEYDOWN:
                ProcessMessage->ProcessKeyDown(control, PtrToInt(msg->WParam.ToPointer()),
                                               PtrToInt(msg->LParam.ToPointer()));
                break;
            case WM_SYSKEYDOWN:
                ProcessMessage->ProcessSysKeyDown(control, PtrToInt(msg->WParam.ToPointer()),
                                                  PtrToInt(msg->LParam.ToPointer()));
                break;
            case WM_LBUTTONDOWN:
                ProcessMessage->ProcessLeftButtonDown(control, GET_X_LPARAM(PtrToInt(msg->LParam.ToPointer())),
                                                      GET_Y_LPARAM(PtrToInt(msg->LParam.ToPointer())));
                break;
            case WM_LBUTTONUP:
                ProcessMessage->ProcessLeftButtonUp(control, GET_X_LPARAM(PtrToInt(msg->LParam.ToPointer())),
                                                    GET_Y_LPARAM(PtrToInt(msg->LParam.ToPointer())));
                break;
            case WM_MOUSEMOVE:
                ProcessMessage->ProcessMouseMove(control, GET_X_LPARAM(PtrToInt(msg->LParam.ToPointer())),
                                                 GET_Y_LPARAM(PtrToInt(msg->LParam.ToPointer())));
                break;
            default:
                System::Diagnostics::Debug::Assert(false, "Unsupported windows message.");
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
