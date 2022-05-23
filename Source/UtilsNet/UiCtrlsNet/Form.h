/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        public ref class Form : System::Windows::Forms::Form,
                                Common::Forms::IFormLocation
        {
        public:
            Form();

        protected:
            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            virtual property array<System::Windows::Forms::Control^>^ ControlLocationSettings
            {
                array<System::Windows::Forms::Control^>^ get()
                {
                    return nullptr;
                }
            }

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            virtual property array<array<System::Windows::Forms::RadioButton^>^>^ RadioButtonLocationSettings
            {
                array<array<System::Windows::Forms::RadioButton^>^>^ get()
                {
                    return nullptr;
                }
            }

        // Common::Forms::IFormLocation
        public:
            virtual void OnFormLocationSaved(Microsoft::Win32::RegistryKey^ RegKey);
            virtual void OnFormLocationRestored(Microsoft::Win32::RegistryKey^ RegKey);

        protected:
            virtual Common::Forms::ListView::ESortOrder GetListViewDefaultSortOrderFormLocationSetting(Common::Forms::ListView^ ListView);

            virtual System::Int32 GetComboBoxDefaultSelectedIndexSetting(System::Windows::Forms::ComboBox^ ComboBox);

            virtual System::Int32 GetRadioButtonDefaultSelectedIndexSetting(array<System::Windows::Forms::RadioButton^>^ RadioButtons);

            virtual System::Boolean GetCheckBoxDefaultCheckedSetting(System::Windows::Forms::CheckBox^ CheckBox);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~Form();

        private:
            /// <summary>
            /// Required designer variable.
            /// </summary>
            System::ComponentModel::Container^ components;

            /// <summary>
            /// Required method for Designer support - do not modify
            /// the contents of this method with the code editor.
            /// </summary>		
            void InitializeComponent(void)
            {
            }

        private:
            void ReadFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, Common::Forms::ListView^ ListView);
            void ReadFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, System::Windows::Forms::ComboBox^ ComboBox);
            void ReadFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, array<System::Windows::Forms::RadioButton^>^ RadioButtons);
            void ReadFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, System::Windows::Forms::CheckBox^ CheckBox);
            void ReadFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, System::Windows::Forms::SplitContainer^ SplitContainer);

            void WriteFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, Common::Forms::ListView^ ListView);
            void WriteFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, System::Windows::Forms::ComboBox^ ComboBox);
            void WriteFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, array<System::Windows::Forms::RadioButton^>^ RadioButtons);
            void WriteFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, System::Windows::Forms::CheckBox^ CheckBox);
            void WriteFormLocationSettings(Microsoft::Win32::RegistryKey^ RegKey, System::Windows::Forms::SplitContainer^ SplitContainer);
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
