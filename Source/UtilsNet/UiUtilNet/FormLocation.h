/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for FormLocation.
        /// </summary>

		public ref class FormLocation sealed
		{
		private:
			FormLocation();

		public:
			FormLocation(System::Windows::Forms::Form^ form, System::String^ sRegistryPath);

		private:
			void OnFormLoad(System::Object^ sender, System::EventArgs^ e);
			void OnFormClosed(System::Object^ sender, System::EventArgs^ e);

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~FormLocation();

		protected:
			System::Windows::Forms::Form^ m_Form;
			System::String^ m_sRegistryPath;
			System::String^ m_sExtraRegistryPath;
		};
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
