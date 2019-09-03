/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

extern "C"
{

HRESULT __stdcall PepAppHostNetExecuteInAppDomain(
  void* cookie)
{
    System::Reflection::Assembly^ assembly;
    array<System::Type^>^ typesArray;
    System::Reflection::MethodInfo^ MethodInfo;

    ::OutputDebugString(L"PepAppHostNetExecuteInAppDomain entering\n");

    try
    {
        assembly = System::Reflection::Assembly::LoadFrom("C:\\git\\PEP\\Source\\bin\\Debug\\x86\\PepAppNet.exe");

        typesArray = assembly->GetTypes();

        for (int nIndex = 0; nIndex < typesArray->GetLength(0); ++nIndex)
        {
            System::Type^ Type = (System::Type^)typesArray->GetValue(nIndex);
            pin_ptr<const wchar_t> pszName = PtrToStringChars(Type->Name);

            if (Type->IsClass && 0 == System::String::Compare(Type->Name, L"Startup"))
            {
                ::OutputDebugString(L"found the class \n");

                MethodInfo = Type->GetMethod("Test");

                if (MethodInfo != nullptr)
                {
                    ::OutputDebugString(L"Found the method\n");

                    array<System::String^>^ StringArray = gcnew array<System::String^>(1);

                    StringArray->SetValue(L"", 0);


                    MethodInfo->Invoke(nullptr, StringArray);

                }
            }
        }
    }
    catch (System::Exception^ Exception)
    {
        pin_ptr<const wchar_t> pszValue = PtrToStringChars(Exception->ToString());

        ::OutputDebugString(L"Exception: ");
        ::OutputDebugString(pszValue);
        ::OutputDebugString(L"\n");
    }

    ::OutputDebugString(L"PepAppHostNetExecuteInAppDomain leaving\n");


    return S_OK;
}

}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
