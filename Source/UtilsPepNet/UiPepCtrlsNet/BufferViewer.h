/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
		/// Summary for BufferViewer
		/// </summary>
		[System::Drawing::ToolboxBitmapAttribute(Pep::Forms::BufferViewer::typeid, "IDB_BUFFERVIEWER")]
		[System::ComponentModel::DefaultPropertyAttribute("Buffer")]
		public ref class BufferViewer sealed : public System::Windows::Forms::Control
		{
		public:
			enum class EDataOrganization
			{
				Ascii,
				Byte,
				WordBigEndian,
				WordLittleEndian
			};

		public:
            /// <summary>
            /// The byte data the control uses.
            /// </summary>

            [System::ComponentModel::Description("The byte data the control uses."),
                System::ComponentModel::Category("Data")]
            property array<System::Byte>^ Buffer
			{
				array<System::Byte>^ get()
				{
					return m_byBuffer;
				}

				void set(array<System::Byte>^ value)
				{
					m_byBuffer = value;

					if (m_byBuffer != nullptr)
					{
						UpdateBuffer();
					}
				}
			}

            /// <summary>
            /// Indicates the display format that the control uses.
            /// </summary>

            [System::ComponentModel::Description("Indicates the display format that the control uses."),
                System::ComponentModel::Category("Behavior")]
            property EDataOrganization DataOrganization
			{
				EDataOrganization get()
				{
					return m_DataOrganization;
				}

				void set(EDataOrganization value)
				{
					m_DataOrganization = value;

					UpdateDataOrganization();
				}
			}

            /// <summary>
            /// Indicates the point size of the font the control uses.
            /// </summary>

            [System::ComponentModel::Description("Indicates the point size of the font the control uses."),
                System::ComponentModel::Category("Appearance")]
            property System::UInt32 FontPtSize
			{
				System::UInt32 get()
				{
					return m_nFontPtSize;
				}

				void set(System::UInt32 value)
				{
					m_nFontPtSize = value;

					UpdateFontPtSize();
				}
			}

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Never)]
            virtual property System::Drawing::Font^ Font
			{
				System::Drawing::Font^ get() override
				{
					return System::Windows::Forms::Control::Font;
				}

				void set(System::Drawing::Font^ value) override
				{
					System::Windows::Forms::Control::Font = value;
				}
			}

		public:
			BufferViewer();
			BufferViewer(System::ComponentModel::IContainer ^container);

			void BeginUpdate(void);
			void EndUpdate(void);

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~BufferViewer();

		private:
			/// <summary>
			/// Required designer variable.
			/// </summary>
			System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
			/// <summary>
			/// Required method for Designer support - do not modify
			/// the contents of this method with the code editor.
			/// </summary>
			void InitializeComponent(void)
			{
				this->SuspendLayout();
				// 
				// BufferViewer
				// 
				this->ResumeLayout(false);
			}
#pragma endregion

		protected:
			void OnHandleCreated(System::EventArgs^ e) override;
			void OnHandleDestroyed(System::EventArgs^ e) override;
			void OnEnabledChanged(System::EventArgs^ e) override;
			void OnVisibleChanged(System::EventArgs^ e) override;
			void OnGotFocus(System::EventArgs^ e) override;
			void OnPaint(System::Windows::Forms::PaintEventArgs^ e) override;
			void OnResize(System::EventArgs^ e) override;

		private:
			System::Boolean UpdateBuffer(void);
			System::Boolean UpdateDataOrganization(void);
			System::Boolean UpdateFontPtSize(void);

		private:
			array<System::Byte>^ m_byBuffer;
			EDataOrganization m_DataOrganization;
			System::UInt32 m_nFontPtSize;

			HWND m_hBufferViewerCtrl;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
