/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2014 Kevin Eshbach
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
		public ref class BufferViewer : public System::Windows::Forms::UserControl
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
				System::Drawing::Font^ get() new
				{
					return nullptr;
				}

				void set(System::Drawing::Font^) new
				{
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
				this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
				this->AutoValidate = System::Windows::Forms::AutoValidate::Disable;
				this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
				this->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
				this->Font = (gcnew System::Drawing::Font(L"Courier New", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
					static_cast<System::Byte>(0)));
				this->Name = L"BufferViewer";
				this->Size = System::Drawing::Size(196, 196);
				this->Load += gcnew System::EventHandler(this, &BufferViewer::BufferViewer_Load);
				this->Resize += gcnew System::EventHandler(this, &BufferViewer::BufferViewer_Resize);
				this->ResumeLayout(false);

			}
#pragma endregion

		private:
			System::Void BufferViewer_Load(System::Object^ sender, System::EventArgs^ e);
			System::Void BufferViewer_Resize(System::Object^ sender, System::EventArgs^ e);

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
//  Copyright (C) 2010-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
