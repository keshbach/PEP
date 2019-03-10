/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MyApplicationContext.h"

Common::Forms::MyApplicationContext::MyApplicationContext(Common::Forms::MainForm^ MainForm) :
    System::Windows::Forms::ApplicationContext(MainForm)
{
    MainForm->Closed += gcnew System::EventHandler(this, &Common::Forms::MyApplicationContext::OnFormClosed);
}

Common::Forms::MyApplicationContext::MyApplicationContext() :
    System::Windows::Forms::ApplicationContext()
{
}

Common::Forms::MyApplicationContext::MyApplicationContext(System::Windows::Forms::Form^ form) :
    System::Windows::Forms::ApplicationContext(form)
{
}

void Common::Forms::MyApplicationContext::OnFormClosed(System::Object^ sender, System::EventArgs^ e)
{
    sender;
    e;

    MainForm->Closed -= gcnew System::EventHandler(this, &Common::Forms::MyApplicationContext::OnFormClosed);

    ExitThread();
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2017 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
