/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IProcessMessage.h"
#include "IUpdateToolStripItems.h"
#include "IFormLocation.h"

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for MainForm.
        /// </summary>

        public ref class MainForm : public System::Windows::Forms::Form,
                                    public Common::Forms::IProcessMessage,
                                    public Common::Forms::IUpdateToolStripItems,
                                    public Common::Forms::IFormLocation
        {
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

			/// <summary>
			/// Refresh the help associated with menus.
			/// </summary>

			void RefreshMenuHelp();

        protected:
            void RunOnUIThreadWait(System::Action^ Action);
            void RunOnUIThreadNoWait(System::Action^ Action);

            void InitToolStripItems(System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem, System::Windows::Forms::ToolStrip^ ToolStrip);
            void UninitToolStripItems(System::Windows::Forms::ToolStrip^ ToolStrip);

            void InitContextMenuItems(System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem, System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip);
            void UninitContextMenuItems(System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip);

            System::Boolean ForceToolStripsIntoSingleRow();

        public:
            // IProcessMessage overrides
            virtual void ProcessKeyDown(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
			virtual void ProcessKeyUp(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
			virtual void ProcessMouseMove(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);

        public:
            // IUpdateToolStripItems overrides
            virtual void UpdateToolStripItems(Common::Forms::ToolStripMenuItem^ ToolStripMenuItem);

        public:
            // IFormLocation overrides
            virtual void OnFormLocationSaved(Microsoft::Win32::RegistryKey^ RegKey);
            virtual void OnFormLocationRestored(Microsoft::Win32::RegistryKey^ RegKey);

		protected:
			// Form overrides
			virtual void OnLoad(System::EventArgs^ e) override;

        private:
            void RefreshMenuHelp(System::Windows::Forms::Control^ parentControl);
            void CreateMenuStatusStrip();
            void DestroyMenuStatusStrip();
			void ShowStatusLabelHelp(System::Windows::Forms::ToolStripItem^ ToolStripItem);

            void EnumMenuStrip(System::Windows::Forms::MenuStrip^ MenuStrip);
			void EnumContextMenuStrip(System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip);
			void EnumToolStripDropDownItem(System::Windows::Forms::ToolStripDropDownItem^ ToolStripDropDownItem);
            void EnumToolStripContainer(System::Windows::Forms::ToolStripContainer^ ToolStripContainer);

			void ClearMenus();

			void IncreaseMenuOpenedCount();
			void DecreaseMenuOpenedCount();

			void DropDownOpened(System::Object^ sender, System::EventArgs^ e);
			void DropDownClosed(System::Object^ sender, System::EventArgs^ e);

			void ContextMenuOpened(System::Object^ sender, System::EventArgs^ e);
			void ContextMenuClosed(System::Object^ sender, System::Windows::Forms::ToolStripDropDownClosedEventArgs^ e);

			static void TimerTick(System::Object^ sender, System::EventArgs^ e);

        private:
            ITaskbarList3* m_pTaskbarList3;
            ULONGLONG m_nProgressBarTotal;
            System::Windows::Forms::Control^ m_SelectedControl;
            System::Windows::Forms::StatusStrip^ m_StatusStrip;
            System::Windows::Forms::ToolStripStatusLabel^ m_ToolStripStatusLabelHelp;
            System::Boolean m_bFindStatusStrip;
            System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Boolean>^ m_ToolStripItemVisibleDict;

			System::Collections::Generic::List<System::Windows::Forms::ToolStripDropDownItem^>^ m_ToolStripDrownDownItemList;
			System::Collections::Generic::List<System::Windows::Forms::ContextMenuStrip^>^ m_ContextMenuStripList;
	
			System::Collections::Generic::Stack<System::Windows::Forms::ToolStripDropDownItem^>^ m_OpenedToolStripDropDownItemStack;
			System::Windows::Forms::ContextMenuStrip^ m_OpenedContextMenuStrip;

			System::Int32 m_nMenuOpenedCount;

			System::Windows::Forms::Timer^ m_Timer;

			System::Drawing::Point^ m_LastMousePoint;

            System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>^ m_ToolStripItemDict;
            System::Collections::Generic::Dictionary<System::Windows::Forms::ToolStripItem^, System::Windows::Forms::ToolStripItem^>^ m_ContextMenuToolStripItemDict;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
