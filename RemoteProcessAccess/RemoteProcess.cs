using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Diagnostics;
using DragonsBox;

namespace RemoteProcessAccess
{
    public class RemoteProcess
    {
        public enum API
        {
            BitBlt = 0,
            MessageboxA = 1,
            MessageboxW = 2,
            SetWindowsHookEx = 3,
            WriteFile = 4,
            WriteProcessMemory = 5,
            DotNet_AssemblyLoad = 6,
            CreateFile = 7,
            CreateProcess = 8,
            DotNet_MethodCalls = 9,
        };

        public enum Permission { Allow = 1, Block = 0 };
        [DllImport("RPA.dll")]
        public static extern bool RuntimeInject(string dllName, int ProcessID);
        [DllImport("RPA.dll", EntryPoint = "CreateAndInjectFromDisk")]
        public static extern int CreateAndInjectFromDisk(string ExE, string DLL);

        [DllImport("RPA.dll", EntryPoint = "InjectFromDisk")]
        public static extern int InjectFromDisk(string DLL);
        [DllImport("RPA.dll", EntryPoint = "StartProcess")]
        public static extern int StartProcess(string ExE);
        
        public MemoryPipe stream;
        public Process process;
        public bool Connected { get; private set; }
        public enum RememberOption { DontKnow = 0, Block = 1, Allow = 2 };
        public SortedList<API, RememberOption> ProcessSettings;
        public SortedList<int, string> FileHandles;
        public DotNetInfo ProcessInfoWindow;

        public RemoteProcess(uint PID)
        {
            ProcessSettings = new SortedList<API, RememberOption>();
            Array values = Enum.GetValues(typeof(API));

            for (int i = 0; i < values.Length; i++)
                ProcessSettings.Add((API)Enum.Parse(typeof(API), values.GetValue(i).ToString()), RememberOption.DontKnow);
            FileHandles = new SortedList<int, string>();

            this.stream = new MemoryPipe("RemoteProcessAccessTunnel" + PID, 1024);
            bool listenRet = this.stream.Listen();
            bool AcceptRet = this.stream.AcceptClient();
            process = Process.GetProcessById((int)PID);
            Connected = true;
            this.ProcessInfoWindow = new DotNetInfo((int)PID);
        }

        public byte[] Receive()
        {
            byte[] buffer = stream.Receive();

            if (buffer.Length == 0)
            {
                Connected = false;
                //throw new Exception("Remote process has been closed");
            }
            return buffer;
        }
    }
}