
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Registry.h"

#pragma unmanaged

static HKEY lUnmanagedCreateRegKey(
	HKEY hRootKey,
	LPCWSTR pszKeyName)
{
	HKEY hKey;
	DWORD dwKeyDisposition;

	if (ERROR_SUCCESS == ::RegCreateKeyEx(hRootKey, pszKeyName, 0,
  										  NULL, REG_OPTION_NON_VOLATILE,
  										  KEY_ALL_ACCESS | KEY_WOW64_64KEY,
 										  NULL, &hKey, &dwKeyDisposition))
	{
		return hKey;
	}

	return NULL;
}

static HKEY lUnmanagedOpenRegKey(
	HKEY hRootKey,
	LPCWSTR pszKeyName,
	BOOL bWritable)
{
	HKEY hKey;
	REGSAM SamDesired;
	DWORD dwKeyDisposition;

	SamDesired = KEY_READ | KEY_WOW64_64KEY;
	SamDesired |= (bWritable ? KEY_WRITE : 0);

	if (ERROR_SUCCESS == ::RegCreateKeyEx(hRootKey, pszKeyName, 0,
                                          NULL, REG_OPTION_NON_VOLATILE,
                                          SamDesired, NULL, &hKey,
                                          &dwKeyDisposition))
	{
		return hKey;
	}

	return NULL;
}

#pragma managed

static Microsoft::Win32::RegistryKey^ lCreateRegKey(
	HKEY hRootKey,
	System::String^ sKeyName)
{
	pin_ptr<const wchar_t> pszKeyName = PtrToStringChars(sKeyName);
	HKEY hKey = lUnmanagedCreateRegKey(hRootKey, pszKeyName);
	Microsoft::Win32::SafeHandles::SafeRegistryHandle^ SafeRegHandle;

	if (hKey)
	{
		SafeRegHandle = gcnew Microsoft::Win32::SafeHandles::SafeRegistryHandle(
			(System::IntPtr)hKey, true);

		return Microsoft::Win32::RegistryKey::FromHandle(SafeRegHandle);
	}

	return nullptr;
}

static Microsoft::Win32::RegistryKey^ lOpenRegKey(
	HKEY hRootKey,
	System::String^ sKeyName,
	System::Boolean bWritable)
{
	pin_ptr<const wchar_t> pszKeyName = PtrToStringChars(sKeyName);
	HKEY hKey;
	Microsoft::Win32::SafeHandles::SafeRegistryHandle^ SafeRegHandle;

	hKey = lUnmanagedOpenRegKey(hRootKey, pszKeyName, bWritable ? TRUE : FALSE);

	if (hKey)
	{
		SafeRegHandle = gcnew Microsoft::Win32::SafeHandles::SafeRegistryHandle(
			(System::IntPtr)hKey, true);

		return Microsoft::Win32::RegistryKey::FromHandle(SafeRegHandle);
	}

	return nullptr;
}

Microsoft::Win32::RegistryKey^ Common::Registry::CreateCurrentUserRegKey(
	System::String^ sKeyName)
{
	return lCreateRegKey(HKEY_CURRENT_USER, sKeyName);
}

Microsoft::Win32::RegistryKey^ Common::Registry::OpenCurrentUserRegKey(
	System::String^ sKeyName,
	System::Boolean bWritable)
{
	return lOpenRegKey(HKEY_CURRENT_USER, sKeyName, bWritable);
}

Microsoft::Win32::RegistryKey^ Common::Registry::CreateLocalMachineRegKey(
	System::String^ sKeyName)
{
	return lCreateRegKey(HKEY_LOCAL_MACHINE, sKeyName);
}

Microsoft::Win32::RegistryKey^ Common::Registry::OpenLocalMachineRegKey(
	System::String^ sKeyName,
	System::Boolean bWritable)
{
	return lOpenRegKey(HKEY_LOCAL_MACHINE, sKeyName, bWritable);
}

Microsoft::Win32::RegistryKey^ Common::Registry::CreateRegKey(
	Microsoft::Win32::RegistryKey^ ParentKey,
	System::String^ sKeyName)
{
	System::IntPtr pKey = ParentKey->Handle->DangerousGetHandle();

	return lCreateRegKey((HKEY)pKey.ToPointer(), sKeyName);
}

Microsoft::Win32::RegistryKey^ Common::Registry::OpenRegKey(
	Microsoft::Win32::RegistryKey^ ParentKey,
	System::String^ sKeyName,
	System::Boolean bWritable)
{
	System::IntPtr pKey = ParentKey->Handle->DangerousGetHandle();

	return lOpenRegKey((HKEY)pKey.ToPointer(), sKeyName, bWritable);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
