/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "FormLocation.h"

#pragma region "Constants"

#define CFormWindowStateNormal 1
#define CFormWindowStateMinimized 2
#define CFormWindowStateMaximized 3

#define CWindowStateValueName L"WindowState"
#define CWindowXValueName L"X"
#define CWindowYValueName L"Y"
#define CWindowWidthValueName L"Width"
#define CWindowHeightValueName L"Height"

#pragma endregion

#pragma region "Constructor"

Common::Forms::FormLocation::FormLocation()
{
}

Common::Forms::FormLocation::FormLocation(
  System::Windows::Forms::Form^ form,
  System::String^ sRegistryPath)
{
	m_Form = form;
	m_sRegistryPath = System::String::Format(L"{0}\\{1}", sRegistryPath, form->GetType()->ToString());

	m_Form->Load += gcnew System::EventHandler(this, &Common::Forms::FormLocation::OnFormLoad);
	m_Form->Closed += gcnew System::EventHandler(this, &Common::Forms::FormLocation::OnFormClosed);
}

#pragma endregion

#pragma region "Deconstructor"

Common::Forms::FormLocation::~FormLocation()
{
}

#pragma endregion

#pragma region "Public Methods"

void Common::Forms::FormLocation::OnFormLoad(
  System::Object^ sender,
  System::EventArgs^ e)
{
	pin_ptr<const wchar_t> pszRegistryPath = PtrToStringChars(m_sRegistryPath);
	HKEY hKey;
	LSTATUS Status;
	DWORD dwValue, dwXValue, dwYValue, dwWidthValue, dwHeightValue, dwValueLen;
	HMONITOR hMonitor;
	RECT Rect;

	sender;
	e;

	m_Form->Load -= gcnew System::EventHandler(this, &Common::Forms::FormLocation::OnFormLoad);

	Status = ::RegCreateKeyEx(HKEY_CURRENT_USER, pszRegistryPath, 0, NULL, REG_OPTION_NON_VOLATILE,
		                      KEY_READ, NULL, &hKey, NULL);

	if (Status == ERROR_SUCCESS)
	{
		dwValueLen = sizeof(dwValue);

		Status = ::RegQueryValueEx(hKey, CWindowStateValueName, NULL, NULL,
			                       (LPBYTE)&dwValue, &dwValueLen);

		if (Status == ERROR_SUCCESS)
		{
			m_Form->StartPosition = System::Windows::Forms::FormStartPosition::Manual;

			switch (dwValue)
			{
			    case CFormWindowStateNormal:
					m_Form->WindowState = System::Windows::Forms::FormWindowState::Normal;
					break;
                case CFormWindowStateMinimized:
					m_Form->WindowState = System::Windows::Forms::FormWindowState::Minimized;
					break;
                case CFormWindowStateMaximized:
					m_Form->WindowState = System::Windows::Forms::FormWindowState::Maximized;
					break;
            }

			::RegQueryValueEx(hKey, CWindowXValueName, NULL, NULL, (LPBYTE)&dwXValue, &dwValueLen);
			::RegQueryValueEx(hKey, CWindowYValueName, NULL, NULL, (LPBYTE)&dwYValue, &dwValueLen);
			::RegQueryValueEx(hKey, CWindowWidthValueName, NULL, NULL, (LPBYTE)&dwWidthValue, &dwValueLen);
			::RegQueryValueEx(hKey, CWindowHeightValueName, NULL, NULL, (LPBYTE)&dwHeightValue, &dwValueLen);

			Rect.left = dwXValue;
			Rect.top = dwYValue;
			Rect.right = (dwXValue + dwWidthValue) - 1;
			Rect.bottom = (dwYValue + dwHeightValue) - 1;

			hMonitor = ::MonitorFromRect(&Rect, MONITOR_DEFAULTTONULL);

			if (hMonitor != NULL)
			{
				m_Form->SetDesktopBounds(dwXValue, dwYValue, dwWidthValue, dwHeightValue);
			}
			else 
			{
				m_Form->WindowState = System::Windows::Forms::FormWindowState::Normal;
				m_Form->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			}
		}

		::RegCloseKey(hKey);
	}
}

void Common::Forms::FormLocation::OnFormClosed(
  System::Object^ sender,
  System::EventArgs^ e)
{
	pin_ptr<const wchar_t> pszRegistryPath = PtrToStringChars(m_sRegistryPath);
	HKEY hKey;
	LSTATUS Status;
	DWORD dwValue;

	sender;
	e;

	m_Form->Closed -= gcnew System::EventHandler(this, &Common::Forms::FormLocation::OnFormClosed);

	Status = ::RegCreateKeyEx(HKEY_CURRENT_USER, pszRegistryPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_WRITE, NULL, &hKey, NULL);

	if (Status == ERROR_SUCCESS)
	{
		switch (m_Form->WindowState)
		{
			case System::Windows::Forms::FormWindowState::Normal:
				dwValue = CFormWindowStateNormal;
				break;
			case System::Windows::Forms::FormWindowState::Minimized:
				dwValue = CFormWindowStateMinimized;
				break;
			case System::Windows::Forms::FormWindowState::Maximized:
				dwValue = CFormWindowStateMaximized;
				break;
		}

		::RegSetKeyValue(hKey, NULL, CWindowStateValueName, REG_DWORD, (LPCVOID)&dwValue, sizeof(dwValue));

		dwValue = m_Form->DesktopLocation.X;

		::RegSetKeyValue(hKey, NULL, CWindowXValueName, REG_DWORD, (LPCVOID)&dwValue, sizeof(dwValue));

		dwValue = m_Form->DesktopLocation.Y;

		::RegSetKeyValue(hKey, NULL, CWindowYValueName, REG_DWORD, (LPCVOID)&dwValue, sizeof(dwValue));

		dwValue = m_Form->DesktopBounds.Width;

		::RegSetKeyValue(hKey, NULL, CWindowWidthValueName, REG_DWORD, (LPCVOID)&dwValue, sizeof(dwValue));

		dwValue = m_Form->DesktopBounds.Height;

		::RegSetKeyValue(hKey, NULL, CWindowHeightValueName, REG_DWORD, (LPCVOID)&dwValue, sizeof(dwValue));

		::RegCloseKey(hKey);
	}

	m_Form = nullptr;
	m_sRegistryPath = nullptr;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
