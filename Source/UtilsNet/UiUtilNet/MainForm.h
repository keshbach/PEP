/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MenuItemHelp.h"
#include "MenuItemHelpList.h"
#include "IProcessMessage.h"

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for MainForm.
        /// </summary>

        public ref class MainForm : public System::Windows::Forms::Form,
                                    public Common::Forms::IProcessMessage
        {
        public:
            /// <summary>
            /// Associates help text with a menu item.
            /// </summary>

            [System::ComponentModel::Description("Associates help text with a menu item."),
                System::ComponentModel::Category("Behavior"),
                System::ComponentModel::DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Content)]
            property System::Collections::Generic::List<MenuItemHelp^>^ MenuStripHelp
            {
                System::Collections::Generic::List<MenuItemHelp^>^ get()
                {
                    return m_MenuItemHelpList;
                }

                void set(System::Collections::Generic::List<MenuItemHelp^>^ value)
                {
                    m_MenuItemHelpList = value;
                }
            }

            [System::ComponentModel::Description(""),
                System::ComponentModel::Category("Behavior"),
                System::ComponentModel::DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Content)]
            property Common::Forms::MenuItemHelpList^ MenuItemHelpList2
            {
                Common::Forms::MenuItemHelpList^ get()
                {
                    return m_MenuItemHelpList2;
                }
                void set(Common::Forms::MenuItemHelpList^ value)
                {
                    m_MenuItemHelpList2 = value;
                }
            }

        private:
            Common::Forms::MenuItemHelpList^ m_MenuItemHelpList2;



        internal:
            static property MainForm^ MainFormDesignMode
            {
                MainForm^ get()
                {
                    return s_MainFormDesignMode;
                }
            }

        public:
            MainForm();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~MainForm();

        public:
            /// <summary>
            /// Turns on the taskbar list progress bar on the main form.
            /// <param name="nTotal">
            /// Progress bar length
            /// </param>
            /// </summary>

            System::Boolean BeginTaskbarListProgressBar(System::UInt64 nTotal);

            /// <summary>
            /// Updates the taskbar list progress bar's current location.
            /// <param name="nValue">
            /// Progress bar current location
            /// </param>
            /// </summary>

            System::Boolean UpdateTaskbarListProgressBar(System::UInt64 nValue);

            /// <summary>
            /// Turns off the taskbar list progress bar on the main form.
            /// </summary>

            System::Boolean EndTaskbarListProgressBar();

            // Designer Support for the MenuStripHelp Property
            System::Boolean ShouldSerializeMenuStripHelp()
            {
                return m_MenuItemHelpList->Count > 0;
            }

            // Designer Support for the MenuStripHelp Property
            void ResetMenuStripHelp()
            {
                m_MenuItemHelpList->Clear();
            }

        public:
            // IProcessMessage overrides
            virtual void ProcessKeyDown(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
            virtual void ProcessSysKeyDown(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
            virtual void ProcessLeftButtonDown(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);
            virtual void ProcessLeftButtonUp(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);
            virtual void ProcessMouseMove(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);

        protected:
            virtual void OnHandleCreated(System::EventArgs^ e) override;

        private:
            void CreateMenuStatusStrip();
            void DestroyMenuStatusStrip();
            void ShowStatusLabelHelp(System::String^ text);
            System::Windows::Forms::StatusStrip^ FindStatusStrip();
            System::String^ FindToolStripItemHelpText(System::Windows::Forms::ToolStripItem^ ToolStripItem);

        private:
            ITaskbarList3* m_pTaskbarList3;
            ULONGLONG m_nProgressBarTotal;
            System::Collections::Generic::List<MenuItemHelp^>^ m_MenuItemHelpList;
            System::Windows::Forms::Control^ m_SelectedControl;
            System::Windows::Forms::StatusStrip^ m_StatusStrip;
            System::Windows::Forms::ToolStripStatusLabel^ m_ToolStripStatusLabelHelp;
            System::Boolean m_bFindStatusStrip;
            System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Boolean>^ m_ToolStripItemVisibleDict;

            static MainForm^ s_MainFormDesignMode;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
