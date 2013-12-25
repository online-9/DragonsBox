using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using RemoteProcessAccess;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;

namespace DragonsBox
{
    public partial class MainForm : Form
    {
        [DllImport("user32.dll", SetLastError = false)]
        static extern IntPtr GetDesktopWindow();
        [DllImport("user32.dll", SetLastError = true)]
        static extern bool SetProp(IntPtr hWnd, string lpString, IntPtr hData);

        SortedList<int, RemoteProcess> SandboxedProcesses;
        public delegate void Invoky();
        public MainForm()
        {
            InitializeComponent();
            CheckForIllegalCrossThreadCalls = false;
            SandboxedProcesses = new SortedList<int, RemoteProcess>();
            ghostButton3_Click(null, null);
        }

        private void ghostButton1_Click(object sender, EventArgs e)
        {
            if (this.listView1.SelectedItems.Count == 0)
            {
                MessageBox.Show("Select a process to sandbox", "DragonBox - Select Process", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            for (int i = 0; i < this.listView1.SelectedItems.Count; i++)
            {
                try
                {
                    int pid = Convert.ToInt32(this.listView1.SelectedItems[i].SubItems[1].Text);

                    if (SandboxedProcesses.ContainsKey(pid))
                    {
                        MessageBox.Show("PID: " + pid + "\r\nProcess: " + listView1.SelectedItems[i].SubItems[0].Text + "\r\n\r\nIs already sandboxed", "DragonBox", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        continue;
                    }

                    SetProp(GetDesktopWindow(), "RemoteProcessAccessTunnel" + pid, (IntPtr)1);
                    RemoteProcess.RuntimeInject(Environment.CurrentDirectory + "\\RPA.dll", pid);

                    RemoteProcess proc = new RemoteProcess((uint)pid);
                    SandboxedProcesses.Add(pid, proc);
                    ThreadPool.QueueUserWorkItem(ProcessThread, new object[] { proc, listView1.SelectedItems[i] });
                    this.listView1.SelectedItems[i].BackColor = Color.Red;
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "DragonBox", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void ghostButton3_Click(object sender, EventArgs e)
        {
            listView1.Items.Clear();
            foreach (Process p in Process.GetProcesses())
            {
                try
                {
                    if (p.Id != Process.GetCurrentProcess().Id)
                    {
                        ListViewItem item = listView1.Items.Add(new ListViewItem(new string[] { p.ProcessName, p.Id.ToString(), p.MainModule.FileName }));
                        if (SandboxedProcesses.ContainsKey(p.Id))
                            item.BackColor = Color.Red;
                    }
                } catch { }
            }
        }

        void ProcessThread(object o)
        {
            RemoteProcess proc = (RemoteProcess)((object[])o)[0];
            ListViewItem ListItem = (ListViewItem)((object[])o)[1];

            ThreadPool.QueueUserWorkItem((object a) =>
            {
                proc.ProcessInfoWindow.ShowDialog();
            });

            while (proc.Connected)
            {
                byte[] data = proc.Receive();

                if (data.Length > 0)
                {
                    PayloadReader pr = new PayloadReader(data);
                    PayloadWriter pw = new PayloadWriter();
                    RemoteProcess.API API = (RemoteProcess.API)pr.ReadByte();

                    string DefaultString = "PID: " + proc.process.Id + "\r\nProcess name: " + proc.process.ProcessName + "\r\n\r\n";

                    switch (API)
                    {
                        case RemoteProcess.API.BitBlt:
                        {
                            ShowWarning(API, "BitBlt Detected", DefaultString + "Tried to screencapture the monitor be\r\naware that if you do not recognize this\r\nprogram block it to prevent spyware\r\nNote: BitBlt does not always copy from the screen\r\nBitBlt is also used in UI's in the .net Framework", proc);
                            break;
                        }
                        case RemoteProcess.API.MessageboxA:
                        case RemoteProcess.API.MessageboxW:
                        {
                            ShowWarning(API, "MessageBox [POPUP]", DefaultString + "Tried to show a MessageBox [Popup] on the screen\r\nTitle: " + pr.ReadString() + "\r\nMessage: " + pr.ReadString(), proc);
                            break;
                        }
                        case RemoteProcess.API.SetWindowsHookEx:
                        {
                            int IdHook = pr.ReadInteger();
                            int hModule = pr.ReadInteger();
                            int dwThreadId = pr.ReadInteger();

                            string IdHookStr = "UNKNOWN";
                            switch (IdHook)
                            {
                                case -1: IdHookStr = "WH_MSGFILTER: Window Message Hook"; break;
                                case 0: IdHookStr = "WH_JOURNALRECORD: Window Message Hook"; break;
                                case 1: IdHookStr = "WH_JOURNALPLAYBACK: Window Message Hook"; break;
                                case 2: IdHookStr = "WH_KEYBOARD: Hotkeys/Keylogger"; break;
                                case 3: IdHookStr = "WH_GETMESSAGE: Window Message Hook"; break;
                                case 4: IdHookStr = "WH_CALLWNDPROC: Window Message Hook"; break;
                                case 5: IdHookStr = "WH_CBT: Window Message Hook"; break;
                                case 6: IdHookStr = "WH_SYSMSGFILTER: Window Message Hook"; break;
                                case 7: IdHookStr = "WH_MOUSE: Mouse Hook"; break;
                                case 9: IdHookStr = "WH_DEBUG: Window Debug Message Hook"; break;
                                case 10: IdHookStr = "WH_SHELL: Window Notification Hook"; break;
                                case 11: IdHookStr = "WH_FOREGROUNDIDLE: Window Foreground Hook"; break;
                                case 12: IdHookStr = "WH_CALLWNDPROCRET: Window Message Hook"; break;
                                case 13: IdHookStr = "WH_KEYBOARD_LL: Hotkeys/Keylogger"; break;
                            }

                            ShowWarning(API, "Window Hook Detected", DefaultString + "Tried to Set a Window Hook\r\nHook Type: " + IdHookStr + "\r\nhModule: " + hModule + "\r\nThread Id: " + dwThreadId + ((dwThreadId == 0) ? "[GLOBAL HOOK]" : ""), proc);
                            break;
                        }
                        case RemoteProcess.API.WriteFile:
                        {
                            IntPtr FileHandle = new IntPtr(pr.ReadInteger());
                            byte[] WriteData = pr.ReadBytes(pr.ReadInteger());

                            string FilePath = "[UNKNOWN]";
                            if (proc.FileHandles.ContainsKey(FileHandle.ToInt32()))
                                FilePath = proc.FileHandles[FileHandle.ToInt32()];

                            ShowWarning(API, "Write Data To File", DefaultString + "Tried to write data to a file\r\nFile Handle: " + FileHandle.ToInt32() + "\r\nLength: " + WriteData.Length + "\r\nFile Path: " + FilePath, proc, WriteData);
                            break;
                        }
                        case RemoteProcess.API.WriteProcessMemory:
                        {
                            IntPtr ProcessHandle = new IntPtr(pr.ReadInteger());
                            byte[] WriteData = pr.ReadBytes(pr.ReadInteger());

                            byte[] PeMagic = new byte[] { 0x4D, 0x5A, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20, 0x6D, 0x6F, 0x64, 0x65 };

                            //if chance is higher then 50% it's a PE
                            int correct = 0;
                            for (int i = 0; i < PeMagic.Length && i < WriteData.Length; i++)
                            {
                                if (PeMagic[i] == WriteData[i])
                                    correct++;
                            }

                            double chance = ((float)correct / (float)PeMagic.Length) * 100F;
                            chance = Math.Round(chance, 2);
                            bool isPE = chance >= 50F;
                            ShowWarning(API, "Write Process Memory", DefaultString + "Tried to write data to a process memory" + (isPE ? "\r\nBe aware that this could be a virus infection\r\nThis process is trying to inject\r\nAnother Executable\r\n" : "") + "\r\nChance on being a Virus Infector(runPE Injector): " + chance + "%\r\nProcess Handle: " + ProcessHandle.ToInt32() + "\r\nLength: " + WriteData.Length, proc, WriteData);
                            break;
                        }
                        case RemoteProcess.API.DotNet_AssemblyLoad:
                        {
                            ShowWarning(API, "Assembly Load [.Net Framework]", DefaultString + "Tried to load a executable using the .Net Framework\r\nCode: Assembly.Load(...);", proc);
                            break;
                        }
                        case RemoteProcess.API.CreateFile:
                        {
                            string FilePath = pr.ReadString();
                            FileAccess DesiredAccess = (FileAccess)pr.ReadInteger();
                            int FileHandle = pr.ReadInteger();

                            if (!proc.FileHandles.ContainsKey(FileHandle))
                                proc.FileHandles.Add(FileHandle, FilePath);
                            else
                                proc.FileHandles[FileHandle] = FilePath;

                            ShowWarning(API, "File Access [Open]", DefaultString + "Tried to get access to a file\r\nFile Path: " + FilePath + "\r\nDesired Access: " + DesiredAccess + "\r\nFile Handle: " + FileHandle, proc);
                            break;
                        }
                        case RemoteProcess.API.CreateProcess:
                        {
                            string Path = pr.ReadString();
                            string CommandLine = pr.ReadString();
                            ShowWarning(API, "Create Process", DefaultString + "Tried to start a executable\r\nWhen allowed to start the process it will be\r\nautomatically sandboxed\r\n\r\nPath: " + Path + "\r\nCommand line arguments: " + CommandLine, proc);
                            break;
                        }
                        case RemoteProcess.API.DotNet_MethodCalls:
                        {
                            string ClassName = pr.ReadString();
                            string method = pr.ReadString();
                            proc.ProcessInfoWindow.listView1.BeginInvoke(new Invoky(() =>  proc.ProcessInfoWindow.listView1.Items.Add(new ListViewItem(new string[] { ClassName, method, "Allow", ".NET FrameWork" }))));
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }

            if (SandboxedProcesses.ContainsKey(Convert.ToInt32(ListItem.SubItems[1].Text)))
            {
                SandboxedProcesses.Remove(Convert.ToInt32(ListItem.SubItems[1].Text));
                listView1.Items.Remove(ListItem);
            }
        }

        void ShowWarning(RemoteProcess.API API, string Title, string Message, RemoteProcess proc, byte[] data = null)
        {
            Warning warning = new Warning(data);
            warning.StartPosition = FormStartPosition.Manual;
            warning.Location = new Point(Screen.PrimaryScreen.Bounds.Width - warning.Width, Screen.PrimaryScreen.Bounds.Height - warning.Height);
            warning.WarningBox.Text = Title;
            warning.WarningMessage.Text = Message;
            warning.TopMost = true;

            PayloadWriter pw = new PayloadWriter();
            if (proc.ProcessSettings[API] == RemoteProcess.RememberOption.DontKnow)
            {
                if (warning.ShowDialog() == DialogResult.OK)
                {
                    pw.WriteByte((byte)RemoteProcess.Permission.Allow);
                    pw.WriteByte(warning.AlwaysCheckbox.Checked ? (byte)1 : (byte)0);

                    if (warning.AlwaysCheckbox.Checked)
                    {
                        proc.ProcessSettings[API] = RemoteProcess.RememberOption.Allow;
                    }
                    proc.ProcessInfoWindow.listView1.BeginInvoke(new Invoky(() =>  proc.ProcessInfoWindow.listView1.Items.Add(new ListViewItem(new string[] { "", API.ToString(), "Allow", "Native" }))));
                    proc.stream.Send(pw.ToByteArray());
                }
                else
                {
                    pw.WriteByte((byte)RemoteProcess.Permission.Block);
                    pw.WriteByte(warning.AlwaysCheckbox.Checked ? (byte)1 : (byte)0);

                    if (warning.AlwaysCheckbox.Checked)
                    {
                        proc.ProcessSettings[API] = RemoteProcess.RememberOption.Block;
                    }
                    proc.ProcessInfoWindow.listView1.BeginInvoke(new Invoky(() =>  proc.ProcessInfoWindow.listView1.Items.Add(new ListViewItem(new string[] { "", API.ToString(), "Block", "Native" }))));
                    proc.stream.Send(pw.ToByteArray());
                }
            }
            else if (proc.ProcessSettings[API] == RemoteProcess.RememberOption.Allow)
            {
                pw.WriteByte((byte)RemoteProcess.Permission.Allow);
                pw.WriteByte(0);
                proc.stream.Send(pw.ToByteArray());
                proc.ProcessInfoWindow.listView1.BeginInvoke(new Invoky(() =>  proc.ProcessInfoWindow.listView1.Items.Add(new ListViewItem(new string[] { "", API.ToString(), "Allow", "Native" }))));
            }
            else if (proc.ProcessSettings[API] == RemoteProcess.RememberOption.Block)
            {
                pw.WriteByte((byte)RemoteProcess.Permission.Block);
                pw.WriteByte(0);
                proc.stream.Send(pw.ToByteArray());
                proc.ProcessInfoWindow.listView1.BeginInvoke(new Invoky(() =>  proc.ProcessInfoWindow.listView1.Items.Add(new ListViewItem(new string[] { "", API.ToString(), "Block", "Native" }))));
            }
        }

        private void ghostButton2_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog dialog = new OpenFileDialog())
            {
                dialog.Filter = "Portable Executable|*.exe";
                dialog.Multiselect = false;
                dialog.Title = "Select the file to sandbox";
                if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        int pid = RemoteProcess.StartProcess(dialog.FileName);
                        SetProp(GetDesktopWindow(), "RemoteProcessAccessTunnel" + pid, (IntPtr)1);

                        Process p = Process.GetProcessById(pid);
                        RemoteProcess proc = null;

                        /*ThreadPool.QueueUserWorkItem((object o) =>
                        {
                            proc = new RemoteProcess((uint)pid);
                        });*/

                        pid = RemoteProcess.InjectFromDisk(Environment.CurrentDirectory + "\\RPA.dll");
                        proc = new RemoteProcess((uint)pid);

                        /*if (proc == null)
                        {
                            MessageBox.Show("Took too long to start the process... killing process");
                            try
                            {
                                if (pid != 0)
                                {
                                    Process.GetProcessById(pid).Kill();
                                }
                            }
                            catch { }
                        }*/

                        if (pid == 0)
                        {
                            MessageBox.Show("Unable to start the process, 64-bit ?", "DragonBox - Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                        
                        SandboxedProcesses.Add(pid, proc);
                        ListViewItem item = listView1.Items.Add(new ListViewItem(new string[] { p.ProcessName, p.Id.ToString(), p.MainModule.FileName }));
                        item.BackColor = Color.Red;
                        ThreadPool.QueueUserWorkItem(ProcessThread, new object[] { proc, item });
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("Application was not even running for a second\r\n" + ex.Message, "DragonBox - Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }
        }

        private void ghostButton4_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Author: DragonHunter\r\n\r\nDragonsBox - The Sandbox for dragons\r\nTheme name: Ghost Theme\r\nTheme author: mavamaarten\r\n\r\nDetections:\r\nScreenCapture(BitBlt)\r\nMessageBox\r\nKeyboard Hook\r\nMouse Hook\r\n15 different Hook Message detections\r\nWriting data to a file\r\nDetects runPE (Runtime Portable Executable)\r\nWrite Process Memory\r\n\r\nCreated by a WhiteHat for a WhiteHat\r\n\r\nDoes not work yet for x64 processes", "About DragonsBox", MessageBoxButtons.OK, MessageBoxIcon.None);
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                for (int i = 0; i < SandboxedProcesses.Count; i++)
                {
                    try
                    {
                        SandboxedProcesses.Values[i].process.Kill();
                    }catch{}
                }
            }
            catch { }
        }
    }
}