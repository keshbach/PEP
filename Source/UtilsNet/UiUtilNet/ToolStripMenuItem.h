/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ToolStripMenuItem.
        /// </summary>

        public ref class ToolStripMenuItem : public System::Windows::Forms::ToolStripMenuItem
        {
		public:
			[System::ComponentModel::Description("The help for the tool strip menu item."),
				System::ComponentModel::Category("Behavior"),
				System::ComponentModel::Browsable(true),
				System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Always)]
			property System::String^ HelpText
			{
				System::String^ get()
				{
					return m_sHelpText;
				}
				void set(System::String^ value)
				{
					m_sHelpText = value;
				}
			}

        public:
			ToolStripMenuItem();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ToolStripMenuItem();

		protected:
			virtual void OnEnabledChanged(System::EventArgs^ e) override;

		private:
			System::String^ m_sHelpText;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
