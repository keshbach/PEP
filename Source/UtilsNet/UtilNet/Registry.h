/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
	public ref class Registry sealed
	{
	public:
		/// <summary>
		/// Creates a registry key under the current user ignoring registry redirection.
		/// </summary>

		static Microsoft::Win32::RegistryKey^ CreateCurrentUserRegKey(
			System::String^ sKeyName);

		/// <summary>
		/// Opens a registry key under the current user ignoring registry redirection.
		/// </summary>

		static Microsoft::Win32::RegistryKey^ OpenCurrentUserRegKey(
			System::String^ sKeyName,
			System::Boolean bWritable);

		/// <summary>
		/// Creates a registry key under the local machine ignoring registry redirection.
		/// </summary>

		static Microsoft::Win32::RegistryKey^ CreateLocalMachineRegKey(
			System::String^ sKeyName);

		/// <summary>
		/// Opens a registry key under the local machine ignoring registry redirection.
		/// </summary>

		static Microsoft::Win32::RegistryKey^ OpenLocalMachineRegKey(
			System::String^ sKeyName,
			System::Boolean bWritable);

		/// <summary>
		/// Creates a registry key under an existing registry key.
		/// </summary>

		static Microsoft::Win32::RegistryKey^ CreateRegKey(
			Microsoft::Win32::RegistryKey^ ParentKey,
			System::String^ sKeyName);

		/// <summary>
		/// Opens a registry key under an existing registry key.
		/// </summary>

		static Microsoft::Win32::RegistryKey^ OpenRegKey(
			Microsoft::Win32::RegistryKey^ ParentKey,
			System::String^ sKeyName,
			System::Boolean bWritable);
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
