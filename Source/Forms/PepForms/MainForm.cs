/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class MainForm : Common.Forms.MainForm,
                                        Pep.Programmer.IDeviceIO,
                                        Pep.Programmer.IDeviceChange,
                                        Pep.Programmer.IPipeProcess,
                                        System.Windows.Forms.IMessageFilter
        {
            #region "Constants"
            private const string CRegExternalToolsKey = "External Tools";

            private const string CRegDescriptionValue = "Description";
            private const string CRegApplicationValue = "Application";
            private const string CRegArgumentsValue = "Arguments";

            private const string CRegSelectedDeviceKey = "Selected Device";

            private const string CRegDeviceNameValue = "Name";
            private const string CRegDeviceTypeValue = "Type";

            private const string CFileOpenDialogAllFilesClientGuid = "{BD9A494B-A6A4-436B-9E25-920EE6A0D92C}";
            private const string CFileSaveDialogAllFilesClientGuid = "{7C2B340B-C853-4E82-9220-DB1ACFA9F319}";
            private const string CFileSaveDialogJedFilesClientGuid = "{7C3D3622-2113-4AF0-B739-2FCBD24FFF7A}";
            private const string CFileSaveDialogLogFileClientGuid = "{75D6FA91-DDC6-43A5-B80E-CA572044BF4E}";

            private const string CEndOfLine = "\r\n";
            private const string CAddressFormat = "X8";
            private const string CByteDataFormat = "X2";
            private const string CWordDataFormat = "X4";
            private const int CTotalExternalTools = 10;

            private const char CCarriageReturnChar = '\r';
            private const char CLinefeedChar = '\n';

            private const int CForceStopPipeProcessSecs = 5;

            // Status Group Names

            private const string CInformationGroupName = "Information";
            private const string CDeviceGroupName = "Device";
            private const string CRunToolGroupName = "Run Tool";

            // Windows constants

            private const int WM_KEYDOWN = 0x0100;
            private const int VK_ESCAPE = 0x001B;

            // Device type constants

            private const string CEPROMDeviceType = "EPROM";
            private const string CEPROMWithIODeviceType = "EPROM With IO";
            private const string CPROMDeviceType = "PROM";
            private const string CSRAMDeviceType = "SRAM";
            private const string CPALDeviceType = "PAL";

            // Zip mode

            private const string CZipModeEnabled = "Zip Mode";
            private const string CZipModeDisabled = "Non-Zip Mode";

            // Device status

            private const string CDeviceConnected = "Connected";
            private const string CDeviceDisconnected = "Disconnected";
            #endregion

            #region "Enumerations"
            private enum EDeviceMode
            {
                Read,
                Write,
                Program,
                Verify
            };

            public enum EEnableControlsMode
            {
                Device,
                RunApp
            };
            #endregion

            #region "Member Variables"
            private System.String m_sRegistryKey;
            private System.String m_sFormLocationsRegistryKey;

            private System.Collections.Generic.List<System.String> m_InfoMessageList = new System.Collections.Generic.List<System.String>();
            private System.Collections.Generic.List<System.String> m_ErrorMessageList = new System.Collections.Generic.List<System.String>();

            private byte[] m_byDeviceBuffer = new byte[1];
            private Pep.Forms.CfgDevicePin[] m_CfgDevicePinsArray = null;
            private System.Threading.Thread m_Thread = null;
            private TThreadDeviceData m_ThreadDeviceData;
            private bool m_bAllowClose = true;
            private bool m_bCancelBtnPressed = false;
            private bool m_bExternalToolsInited = false;
            private System.Threading.Mutex m_CancelBtnMutex = new System.Threading.Mutex();
            private System.DateTime m_DeviceStartDateTime;
            private System.String m_sZipFile = null;
            private System.String m_sZipCurrentPath = "";
            private System.Collections.Generic.List<Pep.Forms.ListToolsForm.TToolData> m_ToolDataList = new System.Collections.Generic.List<ListToolsForm.TToolData>();

            private System.UInt32 m_nCurrentProgressBarValue = 0;

            private System.Collections.Generic.List<System.String> m_MessageCacheList = new System.Collections.Generic.List<System.String>();
            private System.Threading.Mutex m_MessageCacheMutex = new System.Threading.Mutex();

            private TPipeProcessData m_PipeProcessData = new TPipeProcessData();

            private static System.Boolean s_bInitializeImages = true;
            #endregion

            #region "Structures"
            private struct TToolStripData
            {
                public TToolStripData(
                    Common.Forms.ToolStripMenuItem ToolStripMenuItem,
                    System.Windows.Forms.ToolStrip ToolStrip)
                {
                    this.ToolStripMenuItem = ToolStripMenuItem;
                    this.ToolStrip = ToolStrip;
                }

                public Common.Forms.ToolStripMenuItem ToolStripMenuItem;
                public System.Windows.Forms.ToolStrip ToolStrip;
            };

            private struct TThreadDeviceData
            {
                public EDeviceMode DeviceMode;
                public byte[] byDeviceBuffer;
                public Pep.Programmer.IDeviceIO DeviceIO;
                public Pep.Programmer.Device Device;
                public Pep.Programmer.PinConfig[] PinConfigArray;
            }

            private struct TPipeProcessData
            {
                public Pep.Programmer.PipeProcess PipeProcess;
                public System.DateTime PipeProcessCancelDateTime;
                public System.Threading.Thread PipeProcessCloseThread;
                public System.Text.StringBuilder OutputStringBuilder;
                public System.Text.StringBuilder ErrorStringBuilder;
            }
            #endregion

            #region "Constructor"
            private MainForm()
            {
            }

            public MainForm(
                System.String sRegistryKey,
                System.String sFormLocationsRegistryKey)
            {
                m_sRegistryKey = sRegistryKey;
                m_sFormLocationsRegistryKey = sFormLocationsRegistryKey;

                InitializeComponent();
            }
            #endregion

            #region "Device Helpers"
            private static void RunROMDeviceFunc(
                EDeviceMode DeviceMode,
                Pep.Programmer.ROMData RomData,
                byte[] byDeviceBuffer,
                Pep.Programmer.IDeviceIO DeviceIO)
            {
                Common.Debug.Thread.IsWorkerThread();

                try
                {
                    switch (DeviceMode)
                    {
                        case EDeviceMode.Read:
                            RomData.ReadDeviceFunc(DeviceIO, byDeviceBuffer);
                            break;
                        case EDeviceMode.Program:
                            RomData.ProgramDeviceFunc(DeviceIO, byDeviceBuffer);
                            break;
                        case EDeviceMode.Verify:
                            RomData.VerifyDeviceFunc(DeviceIO, byDeviceBuffer);
                            break;
                        default:
                            System.Diagnostics.Debug.Assert(false, "Unsupported Device Mode");
                            break;
                    }
                }

                catch (System.Exception)
                {
                }
            }

            private static void RunRAMDeviceFunc(
                EDeviceMode DeviceMode,
                Pep.Programmer.RAMData RamData,
                byte[] byDeviceBuffer,
                Pep.Programmer.IDeviceIO DeviceIO)
            {
                Common.Debug.Thread.IsWorkerThread();

                try
                {
                    switch (DeviceMode)
                    {
                        case EDeviceMode.Write:
                            RamData.WriteDeviceFunc(DeviceIO, byDeviceBuffer);
                            break;
                        case EDeviceMode.Verify:
                            RamData.VerifyDeviceFunc(DeviceIO, byDeviceBuffer);
                            break;
                        default:
                            System.Diagnostics.Debug.Assert(false, "Unsupported Device Mode");
                            break;
                    }
                }

                catch (System.Exception)
                {
                }
            }

            private static void RunPALDeviceFunc(
                EDeviceMode DeviceMode,
                Pep.Programmer.PALData PalData,
                Pep.Programmer.PinConfig[] PinConfigArray,
                byte[] byDeviceBuffer,
                Pep.Programmer.IDeviceIO DeviceIO)
            {
                Common.Debug.Thread.IsWorkerThread();

                try
                {
                    switch (DeviceMode)
                    {
                        case EDeviceMode.Read:
                            PalData.ReadDeviceFunc(DeviceIO, byDeviceBuffer, PinConfigArray);
                            break;
                        default:
                            System.Diagnostics.Debug.Assert(false, "Unsupported Device Mode");
                            break;
                    }
                }

                catch (System.Exception)
                {
                }
            }

            private static void RunDeviceFunc(
                TThreadDeviceData ThreadDeviceData)
            {
                Pep.Programmer.ROMData RomData;
                Pep.Programmer.RAMData RamData;
                Pep.Programmer.PALData PalData;

                Common.Debug.Thread.IsWorkerThread();

                switch (ThreadDeviceData.Device.DeviceType)
                {
                    case CEPROMDeviceType:
                    case CEPROMWithIODeviceType:
                    case CPROMDeviceType:
                        RomData = (Pep.Programmer.ROMData)ThreadDeviceData.Device.DeviceData;

                        RunROMDeviceFunc(ThreadDeviceData.DeviceMode, RomData,
                                            ThreadDeviceData.byDeviceBuffer,
                                            ThreadDeviceData.DeviceIO);
                        break;
                    case CSRAMDeviceType:
                        RamData = (Pep.Programmer.RAMData)ThreadDeviceData.Device.DeviceData;

                        RunRAMDeviceFunc(ThreadDeviceData.DeviceMode, RamData,
                                            ThreadDeviceData.byDeviceBuffer,
                                            ThreadDeviceData.DeviceIO);
                        break;
                    case CPALDeviceType:
                        PalData = (Pep.Programmer.PALData)ThreadDeviceData.Device.DeviceData;

                        RunPALDeviceFunc(ThreadDeviceData.DeviceMode, PalData,
                                            ThreadDeviceData.PinConfigArray,
                                            ThreadDeviceData.byDeviceBuffer,
                                            ThreadDeviceData.DeviceIO);
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown Device Type");
                        break;
                }
            }
            #endregion

            #region "Pep.IUtDeviceIO"
            public void BeginDeviceIO(
                System.UInt32 nTotalAddresses,
                Pep.Programmer.IDeviceIO.EDeviceOperation DeviceOperation)
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();

                Common.Debug.Thread.IsWorkerThread();

                m_DeviceStartDateTime = System.DateTime.Now;

                sb.Append("Starting to ");

                switch (DeviceOperation)
                {
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Read:
                        sb.Append("read");
                        break;
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Write:
                        sb.Append("write");
                        break;
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Program:
                        sb.Append("program");
                        break;
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Verify:
                        sb.Append("verify");
                        break;
                }

                sb.Append(" the device");

                RunOnUIThreadNoWait(() =>
                {
                    EnableControls(EEnableControlsMode.Device, false);

                    BeginProgressBar(nTotalAddresses);
                });

                AddCachedMessage(sb.ToString());

                RunOnUIThreadNoWait(() =>
                {
                    BeginUpdateTimer();
                    BeginElapsedTimeTimer();
                });
            }

            public void EndDeviceIO(
                System.Boolean bErrorOccurred,
                Pep.Programmer.IDeviceIO.EDeviceOperation DeviceOperation)
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.DateTime DeviceEndDateTime = System.DateTime.Now;
                System.TimeSpan ts;

                Common.Debug.Thread.IsWorkerThread();

                RunOnUIThreadNoWait(() =>
                {
                    EndUpdateTimer();

                    EndElapsedTimeTimer();
                });

                if (!bErrorOccurred)
                {
                    sb.Append("Finished ");
                }
                else
                {
                    sb.Append("Error ");
                }

                switch (DeviceOperation)
                {
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Read:
                        sb.Append("reading");
                        break;
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Write:
                        sb.Append("writing");
                        break;
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Program:
                        sb.Append("program");
                        break;
                    case Pep.Programmer.IDeviceIO.EDeviceOperation.Verify:
                        sb.Append("verifying");
                        break;
                }

                sb.Append(" the device");

                AddCachedMessage(sb.ToString());

                sb = new System.Text.StringBuilder();

                ts = DeviceEndDateTime.Subtract(m_DeviceStartDateTime);

                sb.Append("Operation took ");

                if (ts.Minutes > 0)
                {
                    sb.Append(ts.Minutes.ToString());
                    sb.Append(" minute(s), ");
                }

                sb.Append(ts.Seconds.ToString());
                sb.Append(" second(s)");

                AddCachedMessage(sb.ToString());

                RunOnUIThreadNoWait(() =>
                {
                    EndProgressBar();

                    EnableControls(EEnableControlsMode.Device, true);
                });

                if (Pep.Programmer.Config.Reset() == false)
                {
                    AddCachedMessage("Could not reset the Pep device.");
                }
            }

            public void ProgressDeviceIO(
                System.UInt32 nAddress)
            {
                Common.Debug.Thread.IsWorkerThread();

                m_nCurrentProgressBarValue = nAddress;
            }

            public void VerifyByteErrorDeviceIO(
                System.UInt32 nAddress,
                System.Byte byExpected,
                System.Byte byFound)
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();

                Common.Debug.Thread.IsWorkerThread();

                sb.Append("Address: ");
                sb.Append(nAddress.ToString(CAddressFormat));
                sb.Append(", Expected: ");
                sb.Append(byExpected.ToString(CByteDataFormat));
                sb.Append(", Found: ");
                sb.Append(byFound.ToString(CByteDataFormat));

                AddCachedMessage(sb.ToString());
            }

            public void VerifyWordErrorDeviceIO(
                System.UInt32 nAddress,
                System.UInt16 wExpected,
                System.UInt16 wFound)
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();

                Common.Debug.Thread.IsWorkerThread();

                sb.Append("Address: ");
                sb.Append(nAddress.ToString(CAddressFormat));
                sb.Append(", Expected: ");
                sb.Append(wExpected.ToString(CWordDataFormat));
                sb.Append(", Found: ");
                sb.Append(wFound.ToString(CWordDataFormat));

                AddCachedMessage(sb.ToString());
            }

            public System.Boolean ContinueDeviceIO()
            {
                System.Boolean bResult = true;

                Common.Debug.Thread.IsWorkerThread();

                if (m_CancelBtnMutex.WaitOne())
                {
                    bResult = !m_bCancelBtnPressed;

                    m_CancelBtnMutex.ReleaseMutex();
                }

                return bResult;
            }

            public void ShowMessageDeviceIO(
                System.String sMessage)
            {
                Common.Debug.Thread.IsWorkerThread();

                RunOnUIThreadWait(() =>
                {
                    Common.Forms.MessageBox.Show(sMessage,
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Information);
                });
            }

            public void LogMessageDeviceIO(
                System.String sMessage)
            {
                Common.Debug.Thread.IsWorkerThread();

                RunOnUIThreadWait(() =>
                {
                    LogMessage(sMessage);
                });
            }
            #endregion

            #region "Pep.IUtDeviceChange"
            public void DeviceChange(
                Pep.Programmer.IDeviceChange.ENotification Notification)
            {
                Common.Debug.Thread.IsWorkerThread();

                RunOnUIThreadNoWait(() =>
                {
                    switch (Notification)
                    {
                        case Pep.Programmer.IDeviceChange.ENotification.Arrived:
                            DeviceArrived();
                            break;
                        case Pep.Programmer.IDeviceChange.ENotification.Removed:
                            DeviceRemoved();
                            break;
                        default:
                            System.Diagnostics.Debug.Assert(false);
                            break;
                    }
                });
            }
            #endregion

            #region "Pep.Programmer.IPipeProcess"
            public void BeginPipeProcess()
            {
                Common.Debug.Thread.IsWorkerThread();

                m_DeviceStartDateTime = System.DateTime.Now;

                RunOnUIThreadNoWait(() =>
                {
                    EnableControls(EEnableControlsMode.RunApp, false);

                    BeginUpdateTimer();
                    BeginElapsedTimeTimer();
                });
            }

            public void EndPipeProcess()
            {
                Pep.Programmer.PipeProcess PipeProcess = m_PipeProcessData.PipeProcess;

                Common.Debug.Thread.IsWorkerThread();

                if (m_PipeProcessData.OutputStringBuilder.Length > 0)
                {
                    AddCachedMessage(m_PipeProcessData.OutputStringBuilder.ToString());
                }

                if (m_PipeProcessData.ErrorStringBuilder.Length > 0)
                {
                    AddCachedMessage(m_PipeProcessData.ErrorStringBuilder.ToString());
                }

                RunOnUIThreadNoWait(() =>
                {
                    EndUpdateTimer();
                    EndElapsedTimeTimer();

                    EnableControls(EEnableControlsMode.RunApp, true);

                    m_PipeProcessData.PipeProcess = null;
                    m_PipeProcessData.OutputStringBuilder = null;
                    m_PipeProcessData.ErrorStringBuilder = null;
                });

                m_PipeProcessData.PipeProcessCloseThread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    PipeProcess.Close();
                }, "Close Pipe Process Thread");
            }

            public void OutputDataPipeProcess(
                byte[] Data)
            {
                Common.Debug.Thread.IsWorkerThread();

                ProcessPipeData(Data, m_PipeProcessData.OutputStringBuilder);
            }

            public void ErrorDataPipeProcess(
                byte[] Data)
            {
                Common.Debug.Thread.IsWorkerThread();

                ProcessPipeData(Data, m_PipeProcessData.ErrorStringBuilder);
            }
            #endregion

            #region "System.Windows.Forms.IMessageFilter"
            public bool PreFilterMessage(
                ref System.Windows.Forms.Message m)
            {
                Common.Debug.Thread.IsUIThread();

                if (m.Msg == WM_KEYDOWN)
                {
                    switch (m.WParam.ToInt32())
                    {
                        case VK_ESCAPE:
                            if (m_bAllowClose == false)
                            {
                                ExecuteCancelCommand();

                                return true;
                            }
                            break;
                    }
                }

                return false;
            }
            #endregion

            #region "Internal Functions"
            private void EnableControls(
                EEnableControlsMode EnableControlsMode,
                System.Boolean bEnable)
            {
                System.Windows.Forms.ToolStripItem[] ExcludedMenuItems = {
                    menuItemDeviceCancel};

                Common.Debug.Thread.IsUIThread();

                switch (EnableControlsMode)
                {
                    case EEnableControlsMode.Device:
                        if (bEnable)
                        {
                            menuAppStrip.RestoreItemsEnableState();
                            statusStrip.ActiveGroup = CInformationGroupName;
                        }
                        else
                        {
                            menuAppStrip.SaveItemsEnableState();
                            menuAppStrip.DisableAllItems(ExcludedMenuItems);

                            menuItemDeviceCancel.Enabled = true;

                            statusStrip.ActiveGroup = CDeviceGroupName;
                        }
                        break;
                    case EEnableControlsMode.RunApp:
                        if (bEnable)
                        {
                            menuAppStrip.RestoreItemsEnableState();
                            statusStrip.ActiveGroup = CInformationGroupName;
                        }
                        else
                        {
                            menuAppStrip.SaveItemsEnableState();
                            menuAppStrip.DisableAllItems(ExcludedMenuItems);

                            menuItemDeviceCancel.Enabled = true;

                            statusStrip.ActiveGroup = CRunToolGroupName;
                        }
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false);
                        break;
                }

                m_bAllowClose = bEnable;
            }

            private void AddTextBoxMessage(
                System.String sMessage)
            {
                Common.Debug.Thread.IsUIThread();

                textBoxMessages.AppendText(sMessage);
                textBoxMessages.AppendText(CEndOfLine);
            }

            private void BeginUpdateTimer()
            {
                Common.Debug.Thread.IsUIThread();

                timerUpdater.Start();
            }

            private void EndUpdateTimer()
            {
                Common.Debug.Thread.IsUIThread();

                timerUpdater.Stop();

                FlushMessages();
            }

            private void BeginElapsedTimeTimer()
            {
                Common.Debug.Thread.IsUIThread();

                toolStripTimeStatusLabel.Text = "00:00:00";

                timerElapsedTime.Start();
            }

            private void EndElapsedTimeTimer()
            {
                Common.Debug.Thread.IsUIThread();

                timerElapsedTime.Stop();
            }

            private void BeginProgressBar(
                System.UInt32 nMaxValue)
            {
                Common.Debug.Thread.IsUIThread();

                m_nCurrentProgressBarValue = 0;

                toolStripProgressBar.Minimum = 0;
                toolStripProgressBar.Maximum = (System.Int32)nMaxValue;
                toolStripProgressBar.Step = 1;
                toolStripProgressBar.Value = (int)m_nCurrentProgressBarValue;

                BeginTaskbarListProgressBar(nMaxValue);
            }

            private void EndProgressBar()
            {
                Common.Debug.Thread.IsUIThread();

                SetProgressBarValue(m_nCurrentProgressBarValue);

                EndTaskbarListProgressBar();
            }

            private void SetProgressBarValue(
                System.UInt32 nValue)
            {
                Common.Debug.Thread.IsUIThread();

                toolStripProgressBar.Value = (System.Int32)nValue;

                UpdateTaskbarListProgressBar(nValue);
            }

            private void LogMessage(
                System.String sMessage)
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();

                Common.Debug.Thread.IsUIThread();

                for (System.Int32 nIndex = 0; nIndex < sMessage.Length; ++nIndex)
                {
                    if (sMessage[nIndex] != '\n')
                    {
                        sb.Append(sMessage[nIndex]);
                    }
                    else
                    {
                        sb.Append(CEndOfLine);
                    }
                }

                sb.Append(CEndOfLine);

                textBoxMessages.AppendText(sb.ToString());
            }

            private void DeviceArrived()
            {
                Common.Debug.Thread.IsUIThread();

                toolStripConnectedStatusLabel.Text = CDeviceConnected;

                textBoxMessages.AppendText("Device has arrived.");
                textBoxMessages.AppendText(CEndOfLine);

                if (Pep.Programmer.Config.Reset() == false)
                {
                    textBoxMessages.AppendText("Could not reset the Pep device.");
                    textBoxMessages.AppendText(CEndOfLine);
                }

                if (m_ThreadDeviceData.Device != null)
                {
                    EnableMenuItems(m_ThreadDeviceData.Device);
                }

                UpdateDemoMode(false);
            }

            private void DeviceRemoved()
            {
                Common.Debug.Thread.IsUIThread();

                toolStripConnectedStatusLabel.Text = CDeviceDisconnected;

                textBoxMessages.AppendText("Device was removed.");
                textBoxMessages.AppendText(CEndOfLine);

                if (m_ThreadDeviceData.Device != null)
                {
                    EnableMenuItems(m_ThreadDeviceData.Device);
                }

                UpdateDemoMode(true);
            }

            private void AddCachedMessage(
                System.String sMessage)
            {
                Common.Debug.Thread.IsWorkerThread();

                m_MessageCacheMutex.WaitOne();

                m_MessageCacheList.Add(sMessage);

                m_MessageCacheMutex.ReleaseMutex();
            }

            private void ExecuteCancelCommand()
            {
                Common.Debug.Thread.IsUIThread();

                switch (statusStrip.ActiveGroup)
                {
                    case CDeviceGroupName:
                        CancelDeviceAction();
                        break;
                    case CRunToolGroupName:
                        CancelRunToolAction();
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false);
                        break;
                }
            }

            private void CloseDeviceThread()
            {
                Common.Debug.Thread.IsUIThread();

                if (m_Thread != null)
                {
                    m_Thread.Join();

                    m_Thread = null;
                }
            }

            private void ClosePipeProcessThread()
            {
                Common.Debug.Thread.IsUIThread();

                if (m_PipeProcessData.PipeProcessCloseThread != null)
                {
                    m_PipeProcessData.PipeProcessCloseThread.Join();

                    m_PipeProcessData.PipeProcessCloseThread = null;
                }
            }

            private void CancelDeviceAction()
            {
                Common.Debug.Thread.IsUIThread();

                if (m_CancelBtnMutex.WaitOne())
                {
                    m_bCancelBtnPressed = true;

                    m_CancelBtnMutex.ReleaseMutex();

                    AddTextBoxMessage("Cancelling the device operation");
                }
            }

            private void CancelRunToolAction()
            {
                System.TimeSpan ts;

                Common.Debug.Thread.IsUIThread();

                if (m_PipeProcessData.PipeProcess == null)
                {
                    return;
                }

                if (m_PipeProcessData.PipeProcessCancelDateTime == System.DateTime.MinValue)
                {
                    m_PipeProcessData.PipeProcessCancelDateTime = System.DateTime.Now;
                }

                ts = System.DateTime.Now.Subtract(m_PipeProcessData.PipeProcessCancelDateTime);

                if (ts.TotalSeconds < CForceStopPipeProcessSecs)
                {
                    if (m_PipeProcessData.PipeProcess.StopCapture(false))
                    {
                        AddTextBoxMessage("Cancelling the executing tool");
                    }
                    else
                    {
                        AddTextBoxMessage("Could not cancel the executing tool");
                    }
                }
                else
                {
                    if (m_PipeProcessData.PipeProcess.StopCapture(true))
                    {
                        AddTextBoxMessage("Force cancelling the executing tool");
                    }
                    else
                    {
                        AddTextBoxMessage("Could not force cancel the executing tool");
                    }
                }
            }

            private void OpenFile()
            {
                Common.Forms.FileOpenDialog OpenFileDlg = new Common.Forms.FileOpenDialog();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.IO.FileStream fs = null;
                System.IO.BinaryReader Reader = null;
                System.Collections.Generic.List<Common.Forms.FileTypeItem> FileTypeList = new System.Collections.Generic.List<Common.Forms.FileTypeItem>();
                Common.Forms.FileTypeItem FileTypeItem;

                Common.Debug.Thread.IsUIThread();

                FileTypeItem = new Common.Forms.FileTypeItem("All Files", "*.*");

                FileTypeList.Add(FileTypeItem);

                OpenFileDlg.AddToRecentList = false;
                OpenFileDlg.AllowReadOnly = false;
                OpenFileDlg.FileTypes = FileTypeList;
                OpenFileDlg.PickFolders = false;
                OpenFileDlg.SelectedFileType = 1;
                OpenFileDlg.SelectMultipleItems = false;
                OpenFileDlg.ShowHidden = false;
                OpenFileDlg.ClientGuid = System.Guid.ParseExact(CFileOpenDialogAllFilesClientGuid, "B");

                if (OpenFileDlg.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        fs = new System.IO.FileStream(OpenFileDlg.FileName,
                                                      System.IO.FileMode.Open,
                                                      System.IO.FileAccess.Read,
                                                      System.IO.FileShare.Read);

                        if (fs.Length != m_byDeviceBuffer.Length)
                        {
                            throw new System.IO.IOException("The file length does not match that of the selected device.");
                        }

                        Reader = new System.IO.BinaryReader(fs);
                        fs = null;

                        using (new Common.Forms.WaitCursor(this))
                        {
                            Reader.Read(m_byDeviceBuffer, 0, m_byDeviceBuffer.Length);

                            sb.AppendFormat("Reading the file \"{0}\" into the buffer.", OpenFileDlg.FileName);

                            AddTextBoxMessage(sb.ToString());
                        }
                    }
                    catch (System.IO.IOException exception)
                    {
                        sb.AppendFormat("Could not open the file \"{0}\".", OpenFileDlg.FileName);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }

                    if (Reader != null)
                    {
                        Reader.Close();
                    }

                    if (fs != null)
                    {
                        fs.Close();
                    }
                }
            }

            private void OpenFileFromZip()
            {
                OpenZip.Forms.OpenZipForm OpenZipForm = new OpenZip.Forms.OpenZipForm();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                Common.Zip.File ZipFile = null;
                Common.Zip.Item ZipItem;

                Common.Debug.Thread.IsUIThread();

                OpenZipForm.OpenMode = OpenZip.Forms.OpenZipForm.EOpenMode.Open;
                OpenZipForm.ZipFile = m_sZipFile;
                OpenZipForm.CurrentPath = m_sZipCurrentPath;

                new Common.Forms.FormLocation(OpenZipForm, m_sFormLocationsRegistryKey);

                if (OpenZipForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    m_sZipCurrentPath = OpenZipForm.CurrentPath;

                    try
                    {
                        ZipFile = Common.Zip.File.Open(m_sZipFile, Common.Zip.File.EOpenMode.Open);

                        ZipItem = ZipFile.FindItem(OpenZipForm.Selection);

                        if (m_byDeviceBuffer.Length != (System.Int32)ZipItem.UncompressedSize)
                        {
                            AddTextBoxMessage("Warning: Device buffer being resized.");
                        }

                        ZipFile.ExtractToBuffer(OpenZipForm.Selection, ref m_byDeviceBuffer);

                        sb.AppendFormat("Read the file \"{0}\" from the zip file.", OpenZipForm.Selection);

                        AddTextBoxMessage(sb.ToString());
                    }

                    catch (Exception exception)
                    {
                        sb.AppendFormat("The file \"{0}\" could not be read from the zip file.", OpenZipForm.Selection);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }

                    if (ZipFile != null)
                    {
                        ZipFile.Close();
                    }
                }
            }

            private void SaveFile()
            {
                Common.Forms.FileSaveDialog SaveFileDlg = new Common.Forms.FileSaveDialog();
                System.Collections.Generic.List<Common.Forms.FileTypeItem> FileTypeList = new System.Collections.Generic.List<Common.Forms.FileTypeItem>();
                Common.Forms.FileTypeItem FileTypeItem;

                Common.Debug.Thread.IsUIThread();

                SaveFileDlg.AddToRecentList = false;
                SaveFileDlg.AllowReadOnly = false;
                SaveFileDlg.CreatePrompt = false;
                SaveFileDlg.DefaultExtension = "";
                SaveFileDlg.OverwritePrompt = false;
                SaveFileDlg.SelectedFileType = 1;
                SaveFileDlg.ShowHidden = false;
                SaveFileDlg.StrictFileType = true;

                if (m_ThreadDeviceData.Device.DeviceType != CPALDeviceType)
                {
                    FileTypeItem = new Common.Forms.FileTypeItem("All Files", "*.*");

                    SaveFileDlg.ClientGuid = System.Guid.ParseExact(CFileSaveDialogAllFilesClientGuid, "B");
                }
                else
                {
                    FileTypeItem = new Common.Forms.FileTypeItem("JED Files", "*.jed");

                    SaveFileDlg.DefaultExtension = "jed";

                    SaveFileDlg.ClientGuid = System.Guid.ParseExact(CFileSaveDialogJedFilesClientGuid, "B");
                }

                FileTypeList.Add(FileTypeItem);

                SaveFileDlg.FileTypes = FileTypeList;

                if (SaveFileDlg.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    SaveBufferToFile(SaveFileDlg.FileName);
                }
            }

            private void SaveFileToZip()
            {
                OpenZip.Forms.OpenZipForm OpenZipForm = new OpenZip.Forms.OpenZipForm();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                Common.Zip.File ZipFile = null;
                System.String sFileName, sJEDText;
                Pep.Programmer.PALData PalData;

                Common.Debug.Thread.IsUIThread();

                OpenZipForm.OpenMode = OpenZip.Forms.OpenZipForm.EOpenMode.SaveAs;
                OpenZipForm.ZipFile = m_sZipFile;
                OpenZipForm.CurrentPath = m_sZipCurrentPath;
                OpenZipForm.AllowOverwrite = false;

                new Common.Forms.FormLocation(OpenZipForm, m_sFormLocationsRegistryKey);

                if (OpenZipForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    m_sZipCurrentPath = OpenZipForm.CurrentPath;

                    sFileName = OpenZipForm.Selection;

                    if (m_ThreadDeviceData.Device.DeviceType == CPALDeviceType)
                    {
                        if (System.IO.Path.GetExtension(sFileName) != "jed")
                        {
                            sFileName += ".jed";
                        }
                    }

                    try
                    {
                        ZipFile = Common.Zip.File.Open(m_sZipFile, Common.Zip.File.EOpenMode.Open);

                        if (m_ThreadDeviceData.Device.DeviceType != CPALDeviceType)
                        {
                            ZipFile.AddBuffer(sFileName, "", m_byDeviceBuffer,
                                                Common.Zip.File.ECompressionMethod.Deflated);
                        }
                        else
                        {
                            PalData = (Pep.Programmer.PALData)m_ThreadDeviceData.Device.DeviceData;

                            sJEDText = "";

                            if (PalData.WriteJEDText(m_byDeviceBuffer, ref sJEDText))
                            {
                                ZipFile.AddString(sFileName, "", sJEDText,
                                                    Common.Zip.File.ECompressionMethod.Deflated);
                            }
                            else
                            {
                                AddTextBoxMessage("Could not generate JED text from the buffer.");
                            }
                        }

                        sb.AppendFormat("Wrote the buffer to the file \"{0}\" in the zip file.", OpenZipForm.Selection);

                        AddTextBoxMessage(sb.ToString());
                    }

                    catch (Exception exception)
                    {
                        sb.AppendFormat("Could not add the file \"{0}\" to the zip file \"{1}\".",
                                        OpenZipForm.Selection, m_sZipFile);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }

                    if (ZipFile != null)
                    {
                        ZipFile.Close();
                    }
                }
            }

            private void OpenIntelHexFile()
            {
                Common.Forms.FileOpenDialog OpenFileDlg = new Common.Forms.FileOpenDialog();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.Collections.Generic.List<Common.Forms.FileTypeItem> FileTypeList = new System.Collections.Generic.List<Common.Forms.FileTypeItem>();
                Common.Forms.FileTypeItem FileTypeItem;

                Common.Debug.Thread.IsUIThread();

                FileTypeItem = new Common.Forms.FileTypeItem("Intel Hex Files", "*.hex");

                FileTypeList.Add(FileTypeItem);

                FileTypeItem = new Common.Forms.FileTypeItem("All Files", "*.*");

                FileTypeList.Add(FileTypeItem);

                OpenFileDlg.AddToRecentList = false;
                OpenFileDlg.AllowReadOnly = false;
                OpenFileDlg.FileTypes = FileTypeList;
                OpenFileDlg.PickFolders = false;
                OpenFileDlg.SelectedFileType = 1;
                OpenFileDlg.SelectMultipleItems = false;
                OpenFileDlg.ShowHidden = false;
                OpenFileDlg.ClientGuid = System.Guid.ParseExact(CFileOpenDialogAllFilesClientGuid, "B");

                if (OpenFileDlg.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    sb.AppendFormat("Reading the Intel hex file \"{0}\".", OpenFileDlg.FileName);

                    AddTextBoxMessage(sb.ToString());

                    RunOnUIThreadNoWait(() =>
                    {
                        DisplayViewIntelHexForm(OpenFileDlg.FileName);
                    });
                }
            }

            private void OpenIntelHexFileFromZip()
            {
                OpenZip.Forms.OpenZipForm OpenZipForm = new OpenZip.Forms.OpenZipForm();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                byte[] byData = null;
                Common.Zip.File ZipFile = null;
                Common.Zip.Item ZipItem;

                Common.Debug.Thread.IsUIThread();

                OpenZipForm.OpenMode = OpenZip.Forms.OpenZipForm.EOpenMode.Open;
                OpenZipForm.ZipFile = m_sZipFile;
                OpenZipForm.CurrentPath = m_sZipCurrentPath;

                new Common.Forms.FormLocation(OpenZipForm, m_sFormLocationsRegistryKey);

                if (OpenZipForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    m_sZipCurrentPath = OpenZipForm.CurrentPath;

                    try
                    {
                        ZipFile = Common.Zip.File.Open(m_sZipFile, Common.Zip.File.EOpenMode.Open);

                        ZipItem = ZipFile.FindItem(OpenZipForm.Selection);

                        ZipFile.ExtractToBuffer(OpenZipForm.Selection, ref byData);

                        sb.AppendFormat("Read the Intel hex file \"{0}\" from the zip file.", OpenZipForm.Selection);

                        AddTextBoxMessage(sb.ToString());

                        RunOnUIThreadNoWait(() =>
                        {
                            DisplayViewIntelHexForm(byData);
                        });
                    }

                    catch (Exception exception)
                    {
                        sb.AppendFormat("The file \"{0}\" could not be read from the zip file.", OpenZipForm.Selection);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }

                    if (ZipFile != null)
                    {
                        ZipFile.Close();
                    }
                }
            }

            private void SaveLogFile()
            {
                Common.Forms.FileSaveDialog SaveFileDlg = new Common.Forms.FileSaveDialog();
                System.Collections.Generic.List<Common.Forms.FileTypeItem> FileTypeList = new System.Collections.Generic.List<Common.Forms.FileTypeItem>();
                Common.Forms.FileTypeItem FileTypeItem;

                Common.Debug.Thread.IsUIThread();

                FileTypeItem = new Common.Forms.FileTypeItem("Text Files", "*.txt");

                FileTypeList.Add(FileTypeItem);

                SaveFileDlg.AddToRecentList = false;
                SaveFileDlg.AllowReadOnly = false;
                SaveFileDlg.CreatePrompt = false;
                SaveFileDlg.DefaultExtension = "txt";
                SaveFileDlg.OverwritePrompt = false;
                SaveFileDlg.FileTypes = FileTypeList;
                SaveFileDlg.SelectedFileType = 1;
                SaveFileDlg.ShowHidden = false;
                SaveFileDlg.StrictFileType = true;
                SaveFileDlg.ClientGuid = System.Guid.ParseExact(CFileSaveDialogLogFileClientGuid, "B");

                if (SaveFileDlg.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    SaveLogToFile(SaveFileDlg.FileName);
                }
            }

            private void SaveLogFileToZip()
            {
                OpenZip.Forms.OpenZipForm OpenZipForm = new OpenZip.Forms.OpenZipForm();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                Common.Zip.File ZipFile = null;
                System.String sFileName;

                Common.Debug.Thread.IsUIThread();

                OpenZipForm.OpenMode = OpenZip.Forms.OpenZipForm.EOpenMode.SaveAs;
                OpenZipForm.ZipFile = m_sZipFile;
                OpenZipForm.CurrentPath = m_sZipCurrentPath;
                OpenZipForm.AllowOverwrite = false;

                new Common.Forms.FormLocation(OpenZipForm, m_sFormLocationsRegistryKey);

                if (OpenZipForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    m_sZipCurrentPath = OpenZipForm.CurrentPath;

                    sFileName = OpenZipForm.Selection;

                    try
                    {
                        ZipFile = Common.Zip.File.Open(m_sZipFile, Common.Zip.File.EOpenMode.Open);

                        ZipFile.AddBuffer(sFileName, "",
                                          System.Text.Encoding.ASCII.GetBytes(this.textBoxMessages.Text),
                                          Common.Zip.File.ECompressionMethod.Deflated);

                        sb.AppendFormat("Wrote the messages to the file \"{0}\" in the zip file.", OpenZipForm.Selection);

                        AddTextBoxMessage(sb.ToString());
                    }

                    catch (Exception exception)
                    {
                        sb.AppendFormat("Could not add the file \"{0}\" to the zip file\"{1}\".",
                                        OpenZipForm.Selection, m_sZipFile);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }

                    if (ZipFile != null)
                    {
                        ZipFile.Close();
                    }
                }
            }

            private System.Boolean SaveBufferToFile(
                System.String sFile)
            {
                System.Boolean bResult = false;
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.IO.FileStream fs = null;
                System.IO.BinaryWriter Writer = null;
                Pep.Programmer.PALData PalData;

                Common.Debug.Thread.IsUIThread();

                if (m_ThreadDeviceData.Device.DeviceType != CPALDeviceType)
                {
                    try
                    {
                        fs = new System.IO.FileStream(sFile,
                                                      System.IO.FileMode.OpenOrCreate,
                                                      System.IO.FileAccess.Write,
                                                      System.IO.FileShare.Read);

                        Writer = new System.IO.BinaryWriter(fs);
                        fs = null;

                        using (new Common.Forms.WaitCursor(this))
                        {
                            Writer.Write(m_byDeviceBuffer);

                            sb.AppendFormat("Wrote the buffer to the file \"{0}\".", sFile);

                            AddTextBoxMessage(sb.ToString());

                            bResult = true;
                        }
                    }
                    catch (System.IO.IOException exception)
                    {
                        sb.AppendFormat("Could not save to the file \"{0}\".", sFile);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }

                    if (Writer != null)
                    {
                        Writer.Close();
                    }

                    if (fs != null)
                    {
                        fs.Close();
                    }
                }
                else
                {
                    PalData = (Pep.Programmer.PALData)m_ThreadDeviceData.Device.DeviceData;

                    if (PalData.WriteJEDFile(sFile, m_byDeviceBuffer))
                    {
                        sb.AppendFormat("Wrote the buffer to the file \"{0}\".", sFile);

                        AddTextBoxMessage(sb.ToString());

                        bResult = true;
                    }
                    else
                    {
                        sb.AppendFormat("Could not save to the file \"{0}\".", sFile);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }
                }

                return bResult;
            }

            private System.Boolean SaveLogToFile(
                System.String sFile)
            {
                System.Boolean bResult = false;
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.IO.FileStream fs = null;
                System.IO.StreamWriter Writer = null;

                Common.Debug.Thread.IsUIThread();

                try
                {
                    fs = new System.IO.FileStream(sFile,
                                                  System.IO.FileMode.OpenOrCreate,
                                                  System.IO.FileAccess.Write,
                                                  System.IO.FileShare.Read);

                    Writer = new System.IO.StreamWriter(fs);
                    fs = null;

                    using (new Common.Forms.WaitCursor(this))
                    {
                        Writer.Write(this.textBoxMessages.Text);

                        sb.AppendFormat("Wrote the log to the file \"{0}\".", sFile);

                        AddTextBoxMessage(sb.ToString());

                        bResult = true;
                    }
                }
                catch (System.IO.IOException exception)
                {
                    sb.AppendFormat("Could not save the log to the file \"{0}\".", sFile);
                    sb.AppendLine();
                    sb.AppendLine();
                    sb.AppendLine(exception.Message);

                    AddTextBoxMessage(sb.ToString());

                    Common.Forms.MessageBox.Show(sb.ToString(),
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Error);
                }

                if (Writer != null)
                {
                    Writer.Close();
                }

                if (fs != null)
                {
                    fs.Close();
                }

                return bResult;
            }

            private void EnableMenuItems(
                Pep.Programmer.Device Device)
            {
                System.Boolean bDevicePresent = Pep.Programmer.Config.DevicePresent;
                System.Windows.Forms.ToolStripMenuItem[] ToolMenuItems = {
                    menuItemTools0,
                    menuItemTools1,
                    menuItemTools2,
                    menuItemTools3,
                    menuItemTools4,
                    menuItemTools5,
                    menuItemTools6,
                    menuItemTools7,
                    menuItemTools8,
                    menuItemTools9};
                System.Windows.Forms.ToolStripMenuItem[] OtherMenuItems = {
                    menuItemDeviceRead,
                    menuItemDeviceProgram,
                    menuItemDeviceVerify,
                    menuItemDeviceConfigureDevice,
                    menuItemBufferView,
                    menuItemBufferFill,
                    menuItemBufferTestEmpty,
                    menuItemBufferTestStuckBits,
                    menuItemBufferTestOverdump,
                    menuItemBufferTestAll,
                    menuItemBufferTestZip};
                Pep.Programmer.ROMData RomData;
                Pep.Programmer.RAMData RamData;
                Pep.Programmer.PALData PalData;

                Common.Debug.Thread.IsUIThread();

                for (System.Int32 nIndex = 0; nIndex < ToolMenuItems.Length; ++nIndex)
                {
                    ToolMenuItems[nIndex].Enabled = (Device != null) ? true : false;
                }

                foreach (System.Windows.Forms.ToolStripMenuItem MenuItem in OtherMenuItems)
                {
                    MenuItem.Enabled = false;
                }

                switch (Device.DeviceType)
                {
                    case CEPROMDeviceType:
                    case CEPROMWithIODeviceType:
                    case CPROMDeviceType:
                        RomData = (Pep.Programmer.ROMData)Device.DeviceData;

                        if (bDevicePresent)
                        {
                            menuItemDeviceRead.Enabled = (RomData.ReadDeviceFunc != null) ? true : false;
                            menuItemDeviceProgram.Enabled = (RomData.ProgramDeviceFunc != null) ? true : false;
                            menuItemDeviceVerify.Enabled = (RomData.VerifyDeviceFunc != null) ? true : false;
                        }

                        menuItemBufferView.Enabled = true;
                        menuItemBufferFill.Enabled = true;
                        menuItemBufferTestEmpty.Enabled = true;
                        menuItemBufferTestStuckBits.Enabled = true;
                        menuItemBufferTestOverdump.Enabled = true;
                        menuItemBufferTestAll.Enabled = true;
                        menuItemBufferTestZip.Enabled = (m_sZipFile != null);
                        break;
                    case CSRAMDeviceType:
                        RamData = (Pep.Programmer.RAMData)Device.DeviceData;

                        if (bDevicePresent &&
                            RamData.WriteDeviceFunc != null &&
                            RamData.VerifyDeviceFunc != null)
                        {
                            menuItemDeviceTest.Enabled = true;
                        }

                        menuItemBufferView.Enabled = true;
                        menuItemBufferFill.Enabled = true;
                        menuItemBufferTestEmpty.Enabled = true;
                        menuItemBufferTestStuckBits.Enabled = true;
                        menuItemBufferTestOverdump.Enabled = true;
                        menuItemBufferTestAll.Enabled = true;
                        menuItemBufferTestZip.Enabled = (m_sZipFile != null);
                        break;
                    case CPALDeviceType:
                        PalData = (Pep.Programmer.PALData)Device.DeviceData;

                        if (bDevicePresent)
                        {
                            menuItemDeviceRead.Enabled = (m_ThreadDeviceData.PinConfigArray != null) ? true : false;
                        }

                        menuItemBufferView.Enabled = (m_ThreadDeviceData.PinConfigArray != null) ? true : false;
                        menuItemBufferFill.Enabled = (m_ThreadDeviceData.PinConfigArray != null) ? true : false;
                        menuItemBufferTestEmpty.Enabled = false;
                        menuItemBufferTestStuckBits.Enabled = false;
                        menuItemBufferTestOverdump.Enabled = false;
                        menuItemBufferTestAll.Enabled = false;
                        menuItemBufferTestZip.Enabled = false;
                        menuItemDeviceConfigureDevice.Enabled = true;
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown Device Type");
                        break;
                }
            }

            private static Pep.Forms.CfgDevicePin[] InitCfgDevicePins(
                Pep.Programmer.PinConfigValues[] PinConfigValues)
            {
                Pep.Forms.CfgDevicePin[] CfgDevicePinsArray;

                Common.Debug.Thread.IsUIThread();

                CfgDevicePinsArray = new Pep.Forms.CfgDevicePin[PinConfigValues.Length];

                for (System.Int32 nIndex = 0; nIndex < PinConfigValues.Length; ++nIndex)
                {
                    CfgDevicePinsArray[nIndex] = new Pep.Forms.CfgDevicePin(PinConfigValues[nIndex].Pin,
                                                                            PinConfigValues[nIndex].Types,
                                                                            PinConfigValues[nIndex].Polarities);

                    if (CfgDevicePinsArray[nIndex].Types.Length == 1)
                    {
                        CfgDevicePinsArray[nIndex].SelectedType = CfgDevicePinsArray[nIndex].Types[0];
                    }
                }

                return CfgDevicePinsArray;
            }

            private static Pep.Programmer.PinConfig[] InitCfgPinData(
                Pep.Forms.CfgDevicePin[] CfgDevicePins)
            {
                Pep.Programmer.PinConfig[] PinConfigArray;

                Common.Debug.Thread.IsUIThread();

                PinConfigArray = new Pep.Programmer.PinConfig[CfgDevicePins.Length];

                for (System.Int32 nIndex = 0; nIndex < CfgDevicePins.Length; ++nIndex)
                {
                    PinConfigArray[nIndex] = new Pep.Programmer.PinConfig(CfgDevicePins[nIndex].PinNumber,
                                                                          CfgDevicePins[nIndex].SelectedType,
                                                                          CfgDevicePins[nIndex].SelectedPolarity);
                }

                return PinConfigArray;
            }

            private System.Boolean ReadExternalTools(
                Microsoft.Win32.RegistryKey RegKey)
            {
                System.Int32 nIndex = 0;
                Microsoft.Win32.RegistryKey ExtToolsRegKey;
                Pep.Forms.ListToolsForm.TToolData ToolData;

                Common.Debug.Thread.IsUIThread();

                m_bExternalToolsInited = true;

                m_ToolDataList.Clear();

                ExtToolsRegKey = Common.Registry.OpenRegKey(RegKey, CRegExternalToolsKey, false);

                if (ExtToolsRegKey == null)
                {
                    return true;
                }

                while (nIndex < CTotalExternalTools)
                {
                    ToolData = new Pep.Forms.ListToolsForm.TToolData();

                    ToolData.sDescription = (System.String)ExtToolsRegKey.GetValue(
                                                CRegDescriptionValue + nIndex.ToString());
                    ToolData.sApplication = (System.String)ExtToolsRegKey.GetValue(
                                                CRegApplicationValue + nIndex.ToString());
                    ToolData.sArguments = (System.String)ExtToolsRegKey.GetValue(
                                                CRegArgumentsValue + nIndex.ToString());

                    if (ToolData.sDescription == null)
                    {
                        ToolData.sDescription = "";
                    }

                    if (ToolData.sApplication == null)
                    {
                        ToolData.sApplication = "";
                    }

                    if (ToolData.sArguments == null)
                    {
                        ToolData.sArguments = "";
                    }

                    m_ToolDataList.Add(ToolData);

                    ++nIndex;
                }

                ExtToolsRegKey.Close();

                return true;
            }

            private System.Boolean WriteExternalTools(
                Microsoft.Win32.RegistryKey RegKey)
            {
                System.Int32 nIndex = 0;
                Microsoft.Win32.RegistryKey ExtToolsRegKey;
                Pep.Forms.ListToolsForm.TToolData ToolData;

                Common.Debug.Thread.IsUIThread();

                if (!m_bExternalToolsInited)
                {
                    return false;
                }

                ExtToolsRegKey = Common.Registry.CreateRegKey(RegKey, CRegExternalToolsKey);

                while (nIndex < m_ToolDataList.Count)
                {
                    ToolData = m_ToolDataList[nIndex];

                    ExtToolsRegKey.SetValue(CRegDescriptionValue + nIndex.ToString(),
                                            ToolData.sDescription);
                    ExtToolsRegKey.SetValue(CRegApplicationValue + nIndex.ToString(),
                                            ToolData.sApplication);
                    ExtToolsRegKey.SetValue(CRegArgumentsValue + nIndex.ToString(),
                                            ToolData.sArguments);

                    ++nIndex;
                }

                ExtToolsRegKey.Close();

                return true;
            }

            private void UpdateExternalTools()
            {
                System.Windows.Forms.ToolStripMenuItem[] MenuItems = {
                    menuItemTools0,
                    menuItemTools1,
                    menuItemTools2,
                    menuItemTools3,
                    menuItemTools4,
                    menuItemTools5,
                    menuItemTools6,
                    menuItemTools7,
                    menuItemTools8,
                    menuItemTools9};
                System.Collections.Generic.List<Pep.Forms.ListToolsForm.TToolData> ToolDataList;
                Pep.Forms.ListToolsForm.TToolData NewToolData;
                System.Int32 nMenuItemIndex, nVisibleMenuItems;
                System.Boolean bVisible;
                System.String sMenuItemText;

                Common.Debug.Thread.IsUIThread();

                ToolDataList = new System.Collections.Generic.List<Pep.Forms.ListToolsForm.TToolData>();

                foreach (Pep.Forms.ListToolsForm.TToolData ToolData in m_ToolDataList)
                {
                    if (!System.String.IsNullOrWhiteSpace(ToolData.sDescription))
                    {
                        ToolDataList.Add(ToolData);
                    }
                }

                while (ToolDataList.Count < CTotalExternalTools)
                {
                    NewToolData = new Pep.Forms.ListToolsForm.TToolData();

                    NewToolData.sDescription = "";
                    NewToolData.sApplication = "";
                    NewToolData.sArguments = "";

                    ToolDataList.Add(NewToolData);
                }

                m_ToolDataList = ToolDataList;

                nMenuItemIndex = 0;
                nVisibleMenuItems = 0;

                foreach (Pep.Forms.ListToolsForm.TToolData ToolData in m_ToolDataList)
                {
                    if (ToolData.sDescription.Length > 0)
                    {
                        if (Common.Forms.ImageManager.AddFileSmallImage(ToolData.sApplication,
                                                                        ToolData.sApplication) &&
                            Common.Forms.ImageManager.CopyFileSmallImageToToolbar(ToolData.sApplication))
                        {
                            MenuItems[nMenuItemIndex].ImageKey = ToolData.sApplication;
                        }

                        sMenuItemText = ToolData.sDescription;

                        sMenuItemText = sMenuItemText.Replace("&", "&&");
                        sMenuItemText = sMenuItemText.Insert(0, " ");
                        sMenuItemText = sMenuItemText.Insert(0, nVisibleMenuItems.ToString());
                        sMenuItemText = sMenuItemText.Insert(0, "&");

                        MenuItems[nMenuItemIndex].Text = sMenuItemText;

                        bVisible = true;

                        ++nVisibleMenuItems;
                    }
                    else
                    {
                        bVisible = false;
                    }

                    MenuItems[nMenuItemIndex].Visible = bVisible;

                    ++nMenuItemIndex;
                }

                menuItemToolsSeparator.Visible = (nVisibleMenuItems != 0) ? true : false;

                UninitToolStripItems(toolStripTools);

                InitToolStripItems(menuItemTools, toolStripTools);
            }

            private System.Boolean RunExternalTool(
                System.Int32 nToolIndex)
            {
                System.Boolean bResult = false;
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.String sErrorMsg = null;
                System.String sFile = null;
                System.String sArguments, sFileArg;
                Pep.Forms.ListToolsForm.TToolData ToolData;

                Common.Debug.Thread.IsUIThread();

                ToolData = m_ToolDataList[nToolIndex];

                if (m_sZipFile == null)
                {
                    if (!Common.IO.TempFileManager.CreateTempFile(".bin", ref sFile))
                    {
                        sb.Append("A temporary file could not be created.");

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Information);

                        AddTextBoxMessage(sb.ToString());

                        return false;
                    }

                    AddTextBoxMessage("Writing the buffer to a temporary file.");

                    if (false == SaveBufferToFile(sFile))
                    {
                        sb.Append("Could not save the buffer to the temporary file \"");
                        sb.Append(sFile);
                        sb.Append("\".");

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);

                        AddTextBoxMessage(sb.ToString());

                        return false;
                    }

                    sFileArg = "\"" + sFile + "\"";
                }
                else
                {
                    sFileArg = "\"" + m_sZipFile + "\"";
                }

                sArguments = ToolData.sArguments.Replace("%1", sFileArg);

                ClosePipeProcessThread();

                m_PipeProcessData.PipeProcess = new Pep.Programmer.PipeProcess(this);

                m_PipeProcessData.PipeProcessCancelDateTime = System.DateTime.MinValue;
                m_PipeProcessData.OutputStringBuilder = new System.Text.StringBuilder();
                m_PipeProcessData.ErrorStringBuilder = new System.Text.StringBuilder();

                if (m_PipeProcessData.PipeProcess.CaptureProcessOutput(ToolData.sApplication,
                                                                       sArguments,
                                                                       ref sErrorMsg))
                {
                    bResult = true;
                }
                else
                {
                    m_PipeProcessData.PipeProcess.Close();

                    m_PipeProcessData.PipeProcess = null;
                    m_PipeProcessData.OutputStringBuilder = null;
                    m_PipeProcessData.ErrorStringBuilder = null;

                    sb.Append("The external tool could not be launched.  (");
                    sb.Append(sErrorMsg);
                    sb.Append(")");
                    sb.AppendLine();
                    sb.AppendLine();
                    sb.Append("Application: ");
                    sb.Append(ToolData.sApplication);
                    sb.AppendLine();
                    sb.AppendLine("Arguments: ");
                    sb.AppendLine(sArguments);
                    sb.AppendLine();

                    Common.Forms.MessageBox.Show(sb.ToString(),
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Error);

                    AddTextBoxMessage(sb.ToString());
                }

                return bResult;
            }

            private void FlushMessages()
            {
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.Int32 nLength = 0;

                Common.Debug.Thread.IsUIThread();

                m_MessageCacheMutex.WaitOne();

                foreach (System.String sValue in m_MessageCacheList)
                {
                    nLength += sValue.Length;
                    nLength += CEndOfLine.Length;
                }

                sb.EnsureCapacity(nLength);

                foreach (System.String sValue in m_MessageCacheList)
                {
                    sb.Append(sValue);
                    sb.Append(CEndOfLine);
                }

                m_MessageCacheList.Clear();

                textBoxMessages.AppendText(sb.ToString());

                m_MessageCacheMutex.ReleaseMutex();
            }

            private void UpdateDemoMode(System.Boolean bEnableDemoMode)
            {
                Common.Debug.Thread.IsUIThread();

                if (bEnableDemoMode)
                {
                    this.Text = System.String.Format("{0} ({1})", Pep.Forms.Resources.Resources.MainFormText, Pep.Forms.Resources.Resources.DemoText);
                }
                else
                {
                    this.Text = Pep.Forms.Resources.Resources.MainFormText;
                }
            }

            private void ChangeDevice(
                Pep.Programmer.Device Device)
            {
                uint nSize = 0;
                Pep.Programmer.ROMData RomData;
                Pep.Programmer.RAMData RamData;
                Pep.Programmer.PALData PalData;
                Microsoft.Win32.RegistryKey RegKey;

                Common.Debug.Thread.IsUIThread();

                RegKey = Common.Registry.CreateCurrentUserRegKey(m_sRegistryKey);

                if (RegKey != null)
                {
                    WriteSelectedDevice(RegKey, Device);

                    RegKey.Close();
                }

                deviceInfo.BeginUpdate();

                deviceInfo.DeviceVpp = "----";
                deviceInfo.DeviceBits = "----";
                deviceInfo.DeviceChipEnable = FormatChipDelay(Device.ChipEnableNanoseconds);
                deviceInfo.DeviceOutputEnable = FormatChipDelay(Device.OutputEnableNanoseconds);

                m_CfgDevicePinsArray = null;

                switch (Device.DeviceType)
                {
                    case CEPROMDeviceType:
                    case CEPROMWithIODeviceType:
                    case CPROMDeviceType:
                        RomData = (Pep.Programmer.ROMData)Device.DeviceData;

                        nSize = RomData.Size;

                        deviceInfo.DeviceVpp = RomData.DeviceVpp;
                        deviceInfo.DeviceBits = RomData.BitsPerValue.ToString();
                        break;
                    case CSRAMDeviceType:
                        RamData = (Pep.Programmer.RAMData)Device.DeviceData;

                        nSize = RamData.Size;

                        deviceInfo.DeviceBits = RamData.BitsPerValue.ToString();
                        break;
                    case CPALDeviceType:
                        PalData = (Pep.Programmer.PALData)Device.DeviceData;

                        nSize = PalData.FuseMapSize;

                        m_CfgDevicePinsArray = InitCfgDevicePins(PalData.PinConfigValues);
                        break;
                    default:
                        System.Diagnostics.Debug.Assert(false, "Unknown Device Type");
                        break;
                }

                m_byDeviceBuffer = new byte[nSize];

                switch (Device.DeviceType)
                {
                    case CPALDeviceType:
                        Pep.Programmer.Devices.InitializePALFuseMap(m_byDeviceBuffer);
                        break;
                    default:
                        for (int nIndex = 0; nIndex < m_byDeviceBuffer.Length; ++nIndex)
                        {
                            m_byDeviceBuffer[nIndex] = 0xFF;
                        }
                        break;
                }

                m_ThreadDeviceData.byDeviceBuffer = m_byDeviceBuffer;
                m_ThreadDeviceData.DeviceIO = this;
                m_ThreadDeviceData.Device = Device;
                m_ThreadDeviceData.PinConfigArray = null;

                --nSize;

                deviceInfo.DeviceName = Device.Name;
                deviceInfo.DeviceAdapter = Device.Adapter;
                deviceInfo.DeviceDipSwitches = Device.DipSwitches;
                deviceInfo.DeviceSize = "0 - " + nSize.ToString("X") + " (Bytes)";

                deviceInfo.EndUpdate();

                EnableMenuItems(Device);

                menuItemFileNewZip.Enabled = true;
                menuItemFileOpenZip.Enabled = true;
                menuItemFileOpen.Enabled = true;
                menuItemFileSaveAs.Enabled = true;
                menuItemFileOpenIntelHex.Enabled = true;
                menuItemFileSaveLogAs.Enabled = true;

                if (Device.Message != null)
                {
                    if (m_InfoMessageList == null)
                    {
                        Common.Forms.MessageBox.Show(Device.Message,
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Information);
                    }
                    else
                    {
                        m_InfoMessageList.Add(Device.Message);
                    }

                    AddTextBoxMessage(Device.Message);
                }

                if (Pep.Programmer.Config.DevicePresent &&
                    Pep.Programmer.Config.Reset() == false)
                {
                    AddTextBoxMessage("Could not reset the Pep device.");
                }
            }

            private void TestBufferIfEmpty(
                byte[] buffer)
            {
                System.Boolean bIsEmpty = false;

                Common.Debug.Thread.IsUIThread();

                if (Pep.Programmer.Buffer.IsEmpty(buffer, ref bIsEmpty))
                {
                    AddTextBoxMessage(bIsEmpty ? "The buffer is empty." : "The buffer is not empty.");
                }
                else
                {
                    AddTextBoxMessage("Could not test if the buffer is empty.");
                }
            }

            private void TestBufferStuckBits(
                byte[] buffer)
            {
                System.Collections.Generic.List<Pep.Programmer.Buffer.EBitStuck> BitsStuckList = new System.Collections.Generic.List<Programmer.Buffer.EBitStuck>();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                Pep.Programmer.Buffer.EBitStuck[] HighBits = {
                    Pep.Programmer.Buffer.EBitStuck.High7,
                    Pep.Programmer.Buffer.EBitStuck.High6,
                    Pep.Programmer.Buffer.EBitStuck.High5,
                    Pep.Programmer.Buffer.EBitStuck.High4,
                    Pep.Programmer.Buffer.EBitStuck.High3,
                    Pep.Programmer.Buffer.EBitStuck.High2,
                    Pep.Programmer.Buffer.EBitStuck.High1,
                    Pep.Programmer.Buffer.EBitStuck.High0};
                Pep.Programmer.Buffer.EBitStuck[] LowBits = {
                    Pep.Programmer.Buffer.EBitStuck.Low7,
                    Pep.Programmer.Buffer.EBitStuck.Low6,
                    Pep.Programmer.Buffer.EBitStuck.Low5,
                    Pep.Programmer.Buffer.EBitStuck.Low4,
                    Pep.Programmer.Buffer.EBitStuck.Low3,
                    Pep.Programmer.Buffer.EBitStuck.Low2,
                    Pep.Programmer.Buffer.EBitStuck.Low1,
                    Pep.Programmer.Buffer.EBitStuck.Low0};

                Common.Debug.Thread.IsUIThread();

                if (Pep.Programmer.Buffer.CheckStuckBits(buffer, ref BitsStuckList))
                {
                    if (BitsStuckList.Count > 0)
                    {
                        AddTextBoxMessage("Buffer contains stuck bits.");
                        AddTextBoxMessage("");

                        for (System.Int32 nIndex = 0; nIndex < HighBits.Length; ++nIndex)
                        {
                            sb.AppendFormat("Bit {0} - ", 7 - nIndex);

                            if (BitsStuckList.Contains(HighBits[nIndex]))
                            {
                                sb.Append("High");
                            }
                            else if (BitsStuckList.Contains(LowBits[nIndex]))
                            {
                                sb.Append("Low");
                            }
                            else
                            {
                                sb.Append("x");
                            }

                            AddTextBoxMessage(sb.ToString());

                            sb.Clear();
                        }
                    }
                    else
                    {
                        AddTextBoxMessage("Buffer does not contain any stuck bits.");
                    }
                }
                else
                {
                    AddTextBoxMessage("Could not test if the buffer contains any stuck bits.");
                }
            }

            private void TestBufferOverdump(
                byte[] buffer)
            {
                System.Boolean bIsOverdump = false;

                Common.Debug.Thread.IsUIThread();

                if (Pep.Programmer.Buffer.IsOverdump(buffer, ref bIsOverdump))
                {
                    AddTextBoxMessage(bIsOverdump ? "The buffer contains an overdump." : "The buffer does not contain an overdump.");
                }
                else
                {
                    AddTextBoxMessage("Could not test if the buffer contains an overdump.");
                }
            }

            private static System.Boolean ReadSelectedDevice(
                Microsoft.Win32.RegistryKey RegKey,
                out Pep.Programmer.Device Device)
            {
                Microsoft.Win32.RegistryKey SelectedDeviceRegKey;
                System.String sDeviceName, sDeviceType;

                Common.Debug.Thread.IsUIThread();

                Device = null;

                SelectedDeviceRegKey = Common.Registry.OpenRegKey(RegKey, CRegSelectedDeviceKey, false);

                if (SelectedDeviceRegKey == null)
                {
                    return false;
                }

                sDeviceName = (System.String)SelectedDeviceRegKey.GetValue(CRegDeviceNameValue);
                sDeviceType = (System.String)SelectedDeviceRegKey.GetValue(CRegDeviceTypeValue);

                SelectedDeviceRegKey.Close();

                foreach (Pep.Programmer.Device TmpDevice in Pep.Programmer.Devices.DevicesList)
                {
                    if (TmpDevice.Name == sDeviceName && TmpDevice.DeviceType == sDeviceType)
                    {
                        Device = TmpDevice;

                        return true;
                    }
                }

                return false;
            }

            private static System.Boolean WriteSelectedDevice(
                Microsoft.Win32.RegistryKey RegKey,
                Pep.Programmer.Device Device)
            {
                Microsoft.Win32.RegistryKey SelectedDeviceRegKey;

                Common.Debug.Thread.IsUIThread();

                SelectedDeviceRegKey = Common.Registry.CreateRegKey(RegKey, CRegSelectedDeviceKey);

                SelectedDeviceRegKey.SetValue(CRegDeviceNameValue, Device.Name);
                SelectedDeviceRegKey.SetValue(CRegDeviceTypeValue, Device.DeviceType);

                SelectedDeviceRegKey.Close();

                return true;
            }

            private static System.String FormatChipDelay(
                System.UInt32 nDelay)
            {
                Common.Debug.Thread.IsUIThread();

                if (nDelay > 0)
                {
                    return string.Format("{0} nanoseconds", nDelay);
                }

                return "None";
            }

            private void EnableZipMode(
                System.String sFile)
            {
                Common.Debug.Thread.IsUIThread();

                menuItemFileOpen.HelpText = "Fill the buffer with the contents of a file contained in the active zip file.";
                menuItemFileSaveAs.HelpText = "Save the contents of the buffer to a file in the active zip file.";
                menuItemFileSaveLogAs.HelpText = "Save the current messages to a file in the active zip file.";

                m_sZipFile = sFile;
                m_sZipCurrentPath = "";

                menuItemFileCloseZip.Enabled = true;
                menuItemBufferTestZip.Enabled = true;

                toolStripZipStatusLabel.Text = CZipModeEnabled;
            }

            private void DisableZipMode()
            {
                Common.Debug.Thread.IsUIThread();

                menuItemFileOpen.HelpText = "Fill the buffer with the contents of a file.";
                menuItemFileSaveAs.HelpText = "Save the contents of the buffer to a file.";
                menuItemFileSaveLogAs.HelpText = "Save the current messages to a file.";

                m_sZipFile = null;
                m_sZipCurrentPath = "";

                menuItemFileCloseZip.Enabled = false;
                menuItemBufferTestZip.Enabled = false;

                toolStripZipStatusLabel.Text = CZipModeDisabled;
            }

            private void InitImageLists()
            {
                System.Windows.Forms.ToolStrip[] ToolStrips = {
                    menuAppStrip,
                    contextMenuMessageWindowStrip,
                    toolStripFile,
                    toolStripEdit,
                    toolStripDevice,
                    toolStripBuffer,
                    toolStripTools};

                Common.Debug.Thread.IsUIThread();

                if (s_bInitializeImages)
                {
                    Common.Forms.ImageManager.AddToolbarSmallImages(Pep.Forms.Resources.Resources.ResourceManager);

                    s_bInitializeImages = false;
                }

                foreach (System.Windows.Forms.ToolStrip ToolStrip in ToolStrips)
                {
                    ToolStrip.ImageList = Common.Forms.ImageManager.ToolbarSmallImageList;
                }

                menuAppStrip.RefreshToolStripItemsImageList();
            }

            private void UninitImageLists()
            {
                System.Windows.Forms.ToolStrip[] ToolStrips = {
                    menuAppStrip,
                    contextMenuMessageWindowStrip,
                    toolStripFile,
                    toolStripEdit,
                    toolStripDevice,
                    toolStripBuffer,
                    toolStripTools};

                Common.Debug.Thread.IsUIThread();

                foreach (System.Windows.Forms.ToolStrip ToolStrip in ToolStrips)
                {
                    ToolStrip.ImageList = null;
                }

                menuAppStrip.RefreshToolStripItemsImageList();
            }

            private void InitImageKeys()
            {
                Common.Debug.Thread.IsUIThread();

                menuItemFileNewZip.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "NewProject_16x");
                menuItemFileSaveAs.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "SaveAs_16x");
                menuItemFileSaveLogAs.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "SaveLogAs_16x");
                menuItemFileOpen.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "OpenFile_16x");
                menuItemFileOpenZip.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "OpenProject_16x");
                menuItemFileCloseZip.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "CloseProject_16x");

                menuItemDeviceCancel.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "Cancel_16x");
                menuItemDeviceSelectDevice.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "Device_16x");
                menuItemDeviceRead.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "ReadDevice_16x");
                menuItemDeviceProgram.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "ProgramDevice_16x");
                menuItemDeviceVerify.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "VerifyDevice_16x");

                menuItemBufferView.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "Buffer_16x");

                menuItemToolsConfigure.ImageKey = Common.Forms.ImageManager.GenerateToolbarImageKey(Pep.Forms.Resources.Resources.ResourceManager, "Settings_16x");

                menuItemEditCopy.ImageKey = Common.Forms.ToolbarImageKey.Copy;
                menuItemEditDelete.ImageKey = Common.Forms.ToolbarImageKey.Delete;
                menuItemEditSelectAll.ImageKey = Common.Forms.ToolbarImageKey.Select;
            }

            private void InitToolStripItems()
            {
                TToolStripData[] ToolStripDataArray = {
                    new TToolStripData(menuItemFile, toolStripFile),
                    new TToolStripData(menuItemEdit, toolStripEdit),
                    new TToolStripData(menuItemDevice, toolStripDevice),
                    new TToolStripData(menuItemBuffer, toolStripBuffer),
                    new TToolStripData(menuItemTools, toolStripTools)
                };

                Common.Debug.Thread.IsUIThread();

                foreach (TToolStripData ToolStripData in ToolStripDataArray)
                {
                    InitToolStripItems(ToolStripData.ToolStripMenuItem,
                                       ToolStripData.ToolStrip);
                }
            }

            private void UninitToolStripItems()
            {
                System.Windows.Forms.ToolStrip[] ToolStrips = {
                    toolStripFile,
                    toolStripEdit,
                    toolStripDevice,
                    toolStripBuffer,
                    toolStripTools};

                Common.Debug.Thread.IsUIThread();

                foreach (System.Windows.Forms.ToolStrip ToolStrip in ToolStrips)
                {
                    UninitToolStripItems(ToolStrip);
                }
            }

            private void InitContextMenuStripItems()
            {
                Common.Debug.Thread.IsUIThread();

                InitContextMenuItems(menuItemEdit, contextMenuMessageWindowStrip);
            }

            private void UninitContextMenuStripItems()
            {
                Common.Debug.Thread.IsUIThread();

                UninitContextMenuItems(contextMenuMessageWindowStrip);
            }

            private void DisplayViewIntelHexForm(
                string sFileName)
            {
                ViewIntelHexForm ViewIntelHex = new ViewIntelHexForm();

                Common.Debug.Thread.IsUIThread();

                new Common.Forms.FormLocation(ViewIntelHex, m_sFormLocationsRegistryKey);

                ViewIntelHex.FormLocationsRegistryKey = m_sFormLocationsRegistryKey;
                ViewIntelHex.FileName = sFileName;

                ViewIntelHex.ShowDialog(this);

                ViewIntelHex.Dispose();
            }

            private void DisplayViewIntelHexForm(
                byte[] byData)
            {
                ViewIntelHexForm ViewIntelHex = new ViewIntelHexForm();

                Common.Debug.Thread.IsUIThread();

                new Common.Forms.FormLocation(ViewIntelHex, m_sFormLocationsRegistryKey);

                ViewIntelHex.FormLocationsRegistryKey = m_sFormLocationsRegistryKey;
                ViewIntelHex.Data = byData;

                ViewIntelHex.ShowDialog(this);

                ViewIntelHex.Dispose();
            }

            private void ProcessPipeData(
                byte[] Data,
                System.Text.StringBuilder sb)
            {
                System.String sText = System.Text.UTF8Encoding.UTF8.GetString(Data);
                int nIndex = 0;

                Common.Debug.Thread.IsWorkerThread();

                while (nIndex < sText.Length)
                {
                    switch (sText[nIndex])
                    {
                        case CCarriageReturnChar:
                            ++nIndex;

                            if (nIndex < sText.Length && sText[nIndex] == CLinefeedChar)
                            {
                                ++nIndex;
                            }

                            AddCachedMessage(sb.ToString());

                            sb.Clear();
                            break;
                        case CLinefeedChar:
                            ++nIndex;

                            AddCachedMessage(sb.ToString());

                            sb.Clear();
                            break;
                        default:
                            sb.Append(sText[nIndex]);

                            ++nIndex;
                            break;
                    }
                }
            }
            #endregion

            #region "MainForm Event Handlers"
            private void MainForm_Load(object sender, EventArgs e)
            {
                System.Windows.Forms.ToolStripItem[] ExcludedItemsArray = {
                    menuItemFileExit,
                    menuItemHelpAbout,
                    menuItemDeviceSelectDevice,
                    menuItemDeviceTest,
                    menuItemToolsConfigure,
                    menuItemToolsOptions};
                System.Windows.Forms.ToolStripItem[] InfoGroupItems = {
                    toolStripConnectedStatusLabel,
                    toolStripSeparatorStatusBar1,
                    toolStripZipStatusLabel,
                    toolStripSeparatorStatusBar2};
                System.Windows.Forms.ToolStripItem[] DeviceGroupItems = {
                    toolStripProgressBar,
                    toolStripSeparatorStatusBar3,
                    toolStripTimeStatusLabel,
                    toolStripSeparatorStatusBar4,
                    toolStripCancelButton};
                System.Windows.Forms.ToolStripItem[] RunToolGroupItems = {
                    toolStripTimeStatusLabel,
                    toolStripSeparatorStatusBar4,
                    toolStripCancelButton};
                System.Drawing.Rectangle deviceInfoMinRect = deviceInfo.MinRect;
                int nAdjustHeight = deviceInfoMinRect.Height - deviceInfo.ClientSize.Height;
                Microsoft.Win32.RegistryKey RegKey;
                System.String sErrorMessage, sPortDeviceName;
                Pep.Programmer.Device Device;

                statusStrip.Hide();

                deviceInfo.ClientSize = new System.Drawing.Size(deviceInfo.ClientSize.Width, deviceInfoMinRect.Height);

                textBoxMessages.Bounds = new System.Drawing.Rectangle(textBoxMessages.Bounds.X,
                                                                      textBoxMessages.Bounds.Y + nAdjustHeight,
                                                                      textBoxMessages.Bounds.Width,
                                                                      textBoxMessages.Bounds.Height + -nAdjustHeight);

                statusStrip.Show();

                menuAppStrip.DisableAllItems(ExcludedItemsArray);

                statusStrip.CreateGroup(CInformationGroupName, InfoGroupItems);
                statusStrip.CreateGroup(CDeviceGroupName, DeviceGroupItems);
                statusStrip.CreateGroup(CRunToolGroupName, RunToolGroupItems);

                statusStrip.ActiveGroup = CInformationGroupName;

                toolStripConnectedStatusLabel.Text = "Not Found";

                InitImageLists();
                InitImageKeys();

                InitToolStripItems();

                InitContextMenuStripItems();

                DisableZipMode();

                sErrorMessage = "";

                if (Pep.Programmer.Config.Initialized)
                {
                    Pep.Programmer.Config.DeviceChangeNotification = true;

                    if (Pep.Programmer.Config.DevicePresent)
                    {
                        toolStripConnectedStatusLabel.Text = CDeviceConnected;

                        UpdateDemoMode(false);
                    }
                    else
                    {
                        toolStripConnectedStatusLabel.Text = CDeviceDisconnected;

                        UpdateDemoMode(true);
                    }

                    switch (Pep.Programmer.Config.DeviceType)
                    {
                        case Programmer.Config.EDeviceType.ParallelPort:
                            if (Pep.Programmer.Config.GetDeviceName(out sPortDeviceName))
                            {
                                AddTextBoxMessage(string.Format("The \"{0}\" parallel port is being used.",
                                                                sPortDeviceName));
                            }
                            else
                            {
                                AddTextBoxMessage("The parallel port device name could not be retrieved.");
                            }
                            break;
                        case Programmer.Config.EDeviceType.USB:
                            menuItemToolsSeparator1.Visible = false;
                            menuItemToolsOptions.Visible = false;

                            AddTextBoxMessage("USB device type has been chosen.");
                            break;
                        default:
                            menuItemToolsSeparator1.Visible = false;
                            menuItemToolsOptions.Visible = false;

                            AddTextBoxMessage("No device type has been chosen.");
                            break;
                    }
                }
                else
                {
                    UpdateDemoMode(true);

                    sErrorMessage = "Could not initialize the Pep Control driver.";

                    m_ErrorMessageList.Add(sErrorMessage);

                    AddTextBoxMessage(sErrorMessage);

                    menuItemToolsOptions.Enabled = false;
                }

                RegKey = Common.Registry.CreateCurrentUserRegKey(m_sRegistryKey);

                if (RegKey != null)
                {
                    ReadExternalTools(RegKey);
                    ReadSelectedDevice(RegKey, out Device);

                    RegKey.Close();
                }
                else
                {
                    Device = null;

                    sErrorMessage = "The user settings could not be read.";

                    m_ErrorMessageList.Add(sErrorMessage);

                    AddTextBoxMessage(sErrorMessage);
                }

                UpdateExternalTools();

                if (Device != null)
                {
                    ChangeDevice(Device);
                }
            }

            private void PepMainForm_Shown(object sender, EventArgs e)
            {
                Common.Threading.Thread.RunWorkerThread(() =>
                {
                    RunOnUIThreadWait(() =>
                    {
                        foreach (System.String sErrorMessage in m_ErrorMessageList)
                        {
                            Common.Forms.MessageBox.Show(sErrorMessage,
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Error);
                        }

                        foreach (System.String sInfoMessage in m_InfoMessageList)
                        {
                            Common.Forms.MessageBox.Show(sInfoMessage,
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Information);
                        }

                        m_ErrorMessageList.Clear();
                        m_InfoMessageList.Clear();

                        m_ErrorMessageList = null;
                        m_InfoMessageList = null;
                    });
                }, "Main Form Initialize Thread");
            }

            private void MainForm_FormClosing(object sender, System.Windows.Forms.FormClosingEventArgs e)
            {
                Microsoft.Win32.RegistryKey RegKey;

                e.Cancel = !m_bAllowClose;

                if (m_bAllowClose)
                {
                    RegKey = Common.Registry.CreateCurrentUserRegKey(m_sRegistryKey);

                    if (RegKey != null)
                    {
                        WriteExternalTools(RegKey);

                        RegKey.Close();
                    }

                    CloseDeviceThread();

                    ClosePipeProcessThread();

                    m_CancelBtnMutex.Close();
                    m_MessageCacheMutex.Close();

                    m_CancelBtnMutex = null;
                    m_MessageCacheMutex = null;

                    Pep.Programmer.Config.DeviceChangeNotification = false;
                }
            }

            private void MainForm_FormClosed(object sender, System.Windows.Forms.FormClosedEventArgs e)
            {
                UninitImageLists();

                UninitToolStripItems();

                UninitContextMenuStripItems();
            }

            private void MainForm_SystemColorsChanged(object sender, EventArgs e)
            {
                deviceInfo.UpdateSystemColors();
            }
            #endregion

            #region "ToolStrip Event Handlers"
            private void toolStripCancelButton_Click(object sender, EventArgs e)
            {
                ExecuteCancelCommand();
            }
            #endregion

            #region "MenuItem Event Handlers"
            private void FileNewZipMenuItem_Click(object sender, EventArgs e)
            {
                Common.Forms.FileSaveDialog SaveFileDlg = new Common.Forms.FileSaveDialog();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.Collections.Generic.List<Common.Forms.FileTypeItem> FileTypeList = new System.Collections.Generic.List<Common.Forms.FileTypeItem>();
                Common.Zip.File ZipFile;
                Common.Forms.FileTypeItem FileTypeItem;

                FileTypeItem = new Common.Forms.FileTypeItem("Zip Files", "*.zip");

                FileTypeList.Add(FileTypeItem);

                SaveFileDlg.AddToRecentList = false;
                SaveFileDlg.AllowReadOnly = false;
                SaveFileDlg.CreatePrompt = false;
                SaveFileDlg.DefaultExtension = "zip";
                SaveFileDlg.OverwritePrompt = false;
                SaveFileDlg.FileTypes = FileTypeList;
                SaveFileDlg.SelectedFileType = 1;
                SaveFileDlg.ShowHidden = false;
                SaveFileDlg.StrictFileType = true;

                if (SaveFileDlg.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        ZipFile = Common.Zip.File.Open(SaveFileDlg.FileName,
                                                       Common.Zip.File.EOpenMode.CreateAlways);

                        ZipFile.Close();

                        EnableZipMode(SaveFileDlg.FileName);

                        sb.AppendFormat("Created the zip file \"{0}\".", SaveFileDlg.FileName);

                        AddTextBoxMessage(sb.ToString());
                    }

                    catch (Exception exception)
                    {
                        sb.AppendFormat("Could not create the zip file \"{0}\".", SaveFileDlg.FileName);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }
                }

                SaveFileDlg.Dispose();
            }

            private void FileOpenZipMenuItem_Click(object sender, EventArgs e)
            {
                Common.Forms.FileOpenDialog OpenFileDlg = new Common.Forms.FileOpenDialog();
                System.Text.StringBuilder sb = new System.Text.StringBuilder();
                System.Collections.Generic.List<Common.Forms.FileTypeItem> FileTypeList = new System.Collections.Generic.List<Common.Forms.FileTypeItem>();
                Common.Zip.File ZipFile = null;
                Common.Forms.FileTypeItem FileTypeItem;

                FileTypeItem = new Common.Forms.FileTypeItem("Zip Files", "*.zip");

                FileTypeList.Add(FileTypeItem);

                OpenFileDlg.AddToRecentList = false;
                OpenFileDlg.AllowReadOnly = false;
                OpenFileDlg.FileTypes = FileTypeList;
                OpenFileDlg.PickFolders = false;
                OpenFileDlg.SelectedFileType = 1;
                OpenFileDlg.SelectMultipleItems = false;
                OpenFileDlg.ShowHidden = false;

                if (OpenFileDlg.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        ZipFile = Common.Zip.File.Open(OpenFileDlg.FileName,
                                                       Common.Zip.File.EOpenMode.Open);
                    }

                    catch (Exception exception)
                    {
                        sb.AppendFormat("The zip file \"{0}\" could not be opened.", OpenFileDlg.FileName);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        OpenFileDlg.Dispose();

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);

                        return;
                    }

                    try
                    {
                        if (ZipFile.IsValidArchive())
                        {
                            EnableZipMode(OpenFileDlg.FileName);

                            sb.AppendFormat("Opened the zip file \"{0}\".", OpenFileDlg.FileName);

                            AddTextBoxMessage(sb.ToString());
                        }
                        else
                        {
                            sb.AppendFormat("The zip file \"{0}\" is corrupt.", OpenFileDlg.FileName);

                            AddTextBoxMessage(sb.ToString());

                            Common.Forms.MessageBox.Show(sb.ToString(),
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Error);
                        }
                    }

                    catch (Exception exception)
                    {
                        sb.AppendFormat("The validity of the zip file \"{0}\" could not be determined.", OpenFileDlg.FileName);
                        sb.AppendLine();
                        sb.AppendLine();
                        sb.AppendLine(exception.Message);

                        AddTextBoxMessage(sb.ToString());

                        Common.Forms.MessageBox.Show(sb.ToString(),
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error);
                    }

                    if (ZipFile != null)
                    {
                        ZipFile.Close();
                    }
                }

                OpenFileDlg.Dispose();
            }

            private void FileCloseZipMenuItem_Click(object sender, EventArgs e)
            {
                DisableZipMode();

                AddTextBoxMessage("The zip file has been closed.");
            }

            private void FileOpenMenuItem_Click(object sender, EventArgs e)
            {
                if (m_sZipFile == null)
                {
                    OpenFile();
                }
                else
                {
                    OpenFileFromZip();
                }
            }

            private void FileSaveAsMenuItem_Click(object sender, EventArgs e)
            {
                if (m_sZipFile == null)
                {
                    SaveFile();
                }
                else
                {
                    SaveFileToZip();
                }
            }

            private void FileOpenIntelHexMenuItem_Click(object sender, EventArgs e)
            {
                if (m_sZipFile == null)
                {
                    OpenIntelHexFile();
                }
                else
                {
                    OpenIntelHexFileFromZip();
                }
            }

            private void FileSaveLogAsMenuItem_Click(object sender, EventArgs e)
            {
                if (m_sZipFile == null)
                {
                    SaveLogFile();
                }
                else
                {
                    SaveLogFileToZip();
                }
            }

            private void FileExitMenuItem_Click(object sender, EventArgs e)
            {
                Close();
            }

            private void EditCopyMenuItem_Click(object sender, EventArgs e)
            {
                Common.Debug.Thread.IsUIThread();

                System.Windows.Forms.Clipboard.SetText(textBoxMessages.SelectedText);
            }

            private void EditDeleteMenuItem_Click(object sender, EventArgs e)
            {
                Common.Debug.Thread.IsUIThread();

                textBoxMessages.Text = "";
            }

            private void EditSelectAllMenuItem_Click(object sender, EventArgs e)
            {
                Common.Debug.Thread.IsUIThread();

                textBoxMessages.SelectionStart = 0;
                textBoxMessages.SelectionLength = textBoxMessages.TextLength;
            }

            private void DeviceCancelMenuItem_Click(object sender, EventArgs e)
            {
                ExecuteCancelCommand();
            }

            private void DeviceReadMenuItem_Click(object sender, EventArgs e)
            {
                CloseDeviceThread();

                m_ThreadDeviceData.DeviceMode = EDeviceMode.Read;
                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Read Thread");
            }

            private void DeviceWriteMenuItem_Click(object sender, EventArgs e)
            {
                CloseDeviceThread();

                m_ThreadDeviceData.DeviceMode = EDeviceMode.Write;
                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Write Thread");
            }

            private void DeviceProgramMenuItem_Click(object sender, EventArgs e)
            {
                CloseDeviceThread();

                m_ThreadDeviceData.DeviceMode = EDeviceMode.Program;
                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Program Thread");
            }

            private void DeviceVerifyMenuItem_Click(object sender, EventArgs e)
            {
                CloseDeviceThread();

                m_ThreadDeviceData.DeviceMode = EDeviceMode.Verify;
                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Verify Thread");
            }

            private void DeviceTestBufferMenuItem_Click(object sender, EventArgs e)
            {
                CloseDeviceThread();

                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Write;

                    RunDeviceFunc(m_ThreadDeviceData);

                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Verify;

                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Test Read/Write Thread");
            }

            private void DeviceTestZeroesMenuItem_Click(object sender, EventArgs e)
            {
                for (int nIndex = 0; nIndex < m_byDeviceBuffer.Length; ++nIndex)
                {
                    m_byDeviceBuffer[nIndex] = 0x00;
                }

                CloseDeviceThread();

                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Write;

                    RunDeviceFunc(m_ThreadDeviceData);

                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Verify;

                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Test Read/Write Thread");
            }

            private void DeviceTestOnesMenuItem_Click(object sender, EventArgs e)
            {
                for (int nIndex = 0; nIndex < m_byDeviceBuffer.Length; ++nIndex)
                {
                    m_byDeviceBuffer[nIndex] = 0xFF;
                }

                CloseDeviceThread();

                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Write;

                    RunDeviceFunc(m_ThreadDeviceData);

                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Verify;

                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Test Read/Write Thread");
            }

            private void DeviceTestRandomMenuItem_Click(object sender, EventArgs e)
            {
                System.Random rnd = new System.Random();

                rnd.NextBytes(m_byDeviceBuffer);

                CloseDeviceThread();

                m_bCancelBtnPressed = false;

                m_Thread = Common.Threading.Thread.RunWorkerThread(() =>
                {
                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Write;

                    RunDeviceFunc(m_ThreadDeviceData);

                    m_ThreadDeviceData.DeviceMode = EDeviceMode.Verify;

                    RunDeviceFunc(m_ThreadDeviceData);
                }, "Device Test Read/Write Thread");
            }

            private void DeviceSelectDeviceMenuItem_Click(object sender, EventArgs e)
            {
                Pep.Forms.SelectDeviceForm SelectDevice = new Pep.Forms.SelectDeviceForm();

                new Common.Forms.FormLocation(SelectDevice, m_sFormLocationsRegistryKey);

                SelectDevice.PepDevice = m_ThreadDeviceData.Device;
                SelectDevice.RegistryKey = m_sRegistryKey;

                if (SelectDevice.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    ChangeDevice(SelectDevice.PepDevice);
                }

                SelectDevice.Dispose();
            }

            private void DeviceConfigureDeviceMenuItem_Click(object sender, EventArgs e)
            {
                Pep.Forms.CfgDeviceForm CfgDevice = new Pep.Forms.CfgDeviceForm();

                new Common.Forms.FormLocation(CfgDevice, m_sFormLocationsRegistryKey);

                CfgDevice.CfgDevicePins = m_CfgDevicePinsArray;

                if (CfgDevice.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    m_ThreadDeviceData.PinConfigArray = InitCfgPinData(m_CfgDevicePinsArray);

                    EnableMenuItems(m_ThreadDeviceData.Device);
                }

                CfgDevice.Dispose();
            }

            private void BufferViewMenuItem_Click(object sender, EventArgs e)
            {
                if (m_ThreadDeviceData.Device.DeviceType != CPALDeviceType)
                {
                    Pep.Forms.ViewBufferForm ViewBuffer = new Pep.Forms.ViewBufferForm();

                    ViewBuffer.Buffer = m_byDeviceBuffer;

                    new Common.Forms.FormLocation(ViewBuffer, m_sFormLocationsRegistryKey);

                    if (ViewBuffer.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                    {
                        m_byDeviceBuffer = ViewBuffer.Buffer;

                        m_ThreadDeviceData.byDeviceBuffer = m_byDeviceBuffer;
                    }

                    ViewBuffer.Dispose();
                }
                else
                {
                    Pep.Forms.ViewPALBufferForm ViewPALBuffer = new Pep.Forms.ViewPALBufferForm();
                    System.String sText = "";
                    Pep.Programmer.PALData PalData;

                    PalData = (Pep.Programmer.PALData)m_ThreadDeviceData.Device.DeviceData;

                    if (PalData.WriteJEDText(m_byDeviceBuffer, ref sText))
                    {
                        ViewPALBuffer.Buffer = sText;

                        new Common.Forms.FormLocation(ViewPALBuffer, m_sFormLocationsRegistryKey);

                        ViewPALBuffer.ShowDialog(this);

                        ViewPALBuffer.Dispose();
                    }
                    else
                    {
                        Common.Forms.MessageBox.Show(this, "The JED text could not be generated.",
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Warning);
                    }
                }
            }

            private void BufferFillMenuItem_Click(object sender, EventArgs e)
            {
                Pep.Forms.FillBufferForm FillBuffer = new Pep.Forms.FillBufferForm();
                System.Byte byValue;

                if (m_ThreadDeviceData.Device.DeviceType == CPALDeviceType)
                {
                    FillBuffer.MaxValue = 1;
                }

                new Common.Forms.FormLocation(FillBuffer, m_sFormLocationsRegistryKey);

                if (FillBuffer.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    byValue = FillBuffer.Value;

                    if (m_ThreadDeviceData.Device.DeviceType == CPALDeviceType)
                    {
                        byValue += 0x30;
                    }

                    for (System.Int32 nIndex = 0; nIndex < m_byDeviceBuffer.Length; ++nIndex)
                    {
                        m_byDeviceBuffer[nIndex] = byValue;
                    }
                }

                FillBuffer.Dispose();
            }

            private void BufferTestEmptyMenuItem_Click(object sender, EventArgs e)
            {
                TestBufferIfEmpty(m_byDeviceBuffer);
            }

            private void BufferTestStuckBitsMenuItem_Click(object sender, EventArgs e)
            {
                TestBufferStuckBits(m_byDeviceBuffer);
            }

            private void BufferTestOverdumpMenuItem_Click(object sender, EventArgs e)
            {
                TestBufferOverdump(m_byDeviceBuffer);
            }

            private void BufferTestAllMenuItem_Click(object sender, EventArgs e)
            {
                BufferTestEmptyMenuItem_Click(menuItemBufferTestEmpty, EventArgs.Empty);

                AddTextBoxMessage("");

                BufferTestStuckBitsMenuItem_Click(menuItemBufferTestStuckBits, EventArgs.Empty);

                AddTextBoxMessage("");

                BufferTestOverdumpMenuItem_Click(menuItemBufferTestOverdump, EventArgs.Empty);
            }

            private void BufferTestZipMenuItem_Click(object sender, EventArgs e)
            {
                Common.Zip.File ZipFile;
                byte[] buffer = null;

                try
                {
                    ZipFile = Common.Zip.File.Open(m_sZipFile, Common.Zip.File.EOpenMode.Open);
                }
                catch (Exception exception)
                {
                    AddTextBoxMessage("The zip file could not be opened.");
                    AddTextBoxMessage("");
                    AddTextBoxMessage(exception.Message);

                    return;
                }

                try
                {
                    using (System.Collections.Generic.IEnumerator<Common.Zip.Item> Enum = ZipFile.GetEnumerator())
                    {
                        while (Enum.MoveNext())
                        {
                            if (buffer == null || (ulong)buffer.Length != Enum.Current.UncompressedSize)
                            {
                                buffer = new byte[Enum.Current.UncompressedSize];
                            }

                            ZipFile.ExtractToBuffer(Enum.Current.SrcFileName, ref buffer);

                            AddTextBoxMessage($"Testing the file \"{Enum.Current.SrcFileName}\".");
                            AddTextBoxMessage("");

                            TestBufferIfEmpty(buffer);

                            AddTextBoxMessage("");

                            TestBufferStuckBits(buffer);

                            AddTextBoxMessage("");

                            TestBufferOverdump(buffer);

                            AddTextBoxMessage("");
                        }
                    }
                }
                catch (Exception exception)
                {
                    AddTextBoxMessage("The files contained within the zip file could not be retrieved.");
                    AddTextBoxMessage("");
                    AddTextBoxMessage(exception.Message);
                }

                ZipFile.Close();
            }

            private void Tools0MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(0);
            }

            private void Tools1MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(1);
            }

            private void Tools2MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(2);
            }

            private void Tools3MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(3);
            }

            private void Tools4MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(4);
            }

            private void Tools5MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(5);
            }

            private void Tools6MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(6);
            }

            private void Tools7MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(7);
            }

            private void Tools8MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(8);
            }

            private void Tools9MenuItem_Click(object sender, EventArgs e)
            {
                RunExternalTool(9);
            }

            private void ToolsConfigureMenuItem_Click(object sender, EventArgs e)
            {
                Pep.Forms.ListToolsForm ListTools = new Pep.Forms.ListToolsForm();

                ListTools.FormLocationsRegistryKey = m_sFormLocationsRegistryKey;
                ListTools.ToolDataList = m_ToolDataList;

                ListTools.ShowDialog(this);

                ListTools.Dispose();

                m_ToolDataList = ListTools.ToolDataList;

                UpdateExternalTools();
            }

            private void ToolsOptionsMenuItem_Click(object sender, EventArgs e)
            {
                Pep.Forms.SelectPortForm SelectPortForm = new Pep.Forms.SelectPortForm();
                System.String sPortDeviceName;

                if (Pep.Programmer.Config.GetDeviceName(out sPortDeviceName) == false)
                {
                    AddTextBoxMessage("The existing parallel port settings could not be retrieved.");

                    Common.Forms.MessageBox.Show(this, "The existing parallel port settings could not be retrieved.",
                                                 System.Windows.Forms.MessageBoxButtons.OK,
                                                 System.Windows.Forms.MessageBoxIcon.Warning);
                }

                SelectPortForm.PortDeviceObjectName = sPortDeviceName;

                new Common.Forms.FormLocation(SelectPortForm, m_sFormLocationsRegistryKey);

                if (SelectPortForm.ShowDialog(this) == System.Windows.Forms.DialogResult.OK)
                {
                    if (Pep.Programmer.Config.SetDeviceName(SelectPortForm.PortDeviceObjectName))
                    {
                        AddTextBoxMessage(string.Format("The \"{0}\" parallel port is now being used.",
                                                        SelectPortForm.PortDeviceObjectName));
                    }
                    else
                    {
                        AddTextBoxMessage("The existing parallel port settings could not be changed.");

                        Common.Forms.MessageBox.Show(this, "The existing parallel port settings could not be changed.",
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Warning);
                    }
                }

                SelectPortForm.Dispose();
            }

            private void HelpAboutMenuItem_Click(object sender, EventArgs e)
            {
                Pep.Forms.AboutForm About = new Pep.Forms.AboutForm();

                About.ShowDialog(this);

                About.Dispose();
            }
            #endregion

            #region "Timer Event Handlers"
            private void timerUpdater_Tick(object sender, EventArgs e)
            {
                Common.Debug.Thread.IsUIThread();

                SetProgressBarValue(m_nCurrentProgressBarValue);

                FlushMessages();
            }

            private void timerElapsedTime_Tick(object sender, EventArgs e)
            {
                System.DateTime DeviceEndDateTime = System.DateTime.Now;
                System.TimeSpan ts;

                ts = DeviceEndDateTime.Subtract(m_DeviceStartDateTime);

                toolStripTimeStatusLabel.Text = System.String.Format("{0:00}:{1:00}:{2:00}", ts.Hours, ts.Minutes, ts.Seconds);
            }
            #endregion

            #region "Text Box Messages Event Handlers"
            private void textBoxMessages_TextChanged(object sender, EventArgs e)
            {
                if (textBoxMessages.Text.Length > 0)
                {
                    menuItemEditDelete.Enabled = true;
                    menuItemEditSelectAll.Enabled = true;
                }
                else
                {
                    menuItemEditCopy.Enabled = false;
                    menuItemEditDelete.Enabled = false;
                    menuItemEditSelectAll.Enabled = false;
                }
            }

            private void textBoxMessages_TextSelected(object sender, Common.Forms.TextBoxMessagesTextSelectedEventArgs e)
            {
                menuItemEditCopy.Enabled = e.TextSelected;
            }
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/
