/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class PipeProcess sealed
		{
		private:
			PipeProcess();

		public:
			PipeProcess(IPipeProcess^ PipeProcess);
			~PipeProcess();
			!PipeProcess();

		public:
			System::Boolean CaptureProcessOutput(
				System::String^ sApplication,
				System::String^ sArguments,
				System::String^% sErrorMessage);

			System::Boolean StopCapture(System::Boolean bForce);

			void Close();

		private:
			System::IntPtr m_PipeProcessData;
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
