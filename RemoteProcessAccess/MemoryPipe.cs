using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.ComponentModel;

namespace RemoteProcessAccess
{
    public class MemoryPipe
    {
        [DllImport("kernel32.dll", EntryPoint = "CreateFile", SetLastError = true)]
        private static extern IntPtr CreateFile(String lpFileName,
            UInt32 dwDesiredAccess, UInt32 dwShareMode,
            IntPtr lpSecurityAttributes, UInt32 dwCreationDisposition,
            UInt32 dwFlagsAndAttributes,
            IntPtr hTemplateFile);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr CreateNamedPipeA(String lpName,
                                                      uint dwOpenMode,
                                                      uint dwPipeMode,
                                                      uint nMaxInstances,
                                                      uint nOutBufferSize,
                                                      uint nInBufferSize,
                                                      uint nDefaultTimeOut,
                                                      IntPtr pipeSecurityDescriptor);
            
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool DisconnectNamedPipe(IntPtr hHandle);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool ConnectNamedPipe(IntPtr hHandle, IntPtr lpOverlapped);
        [DllImport("kernel32.dll", EntryPoint = "PeekNamedPipe", SetLastError = true)]
        private static extern bool PeekNamedPipe(IntPtr handle,
            byte[] buffer, uint nBufferSize, ref uint bytesRead,
            ref uint bytesAvail, ref uint BytesLeftThisMessage);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool ReadFile(IntPtr handle,
            byte[] buffer, uint toRead, ref uint read, IntPtr lpOverLapped);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteFile(IntPtr handle,
            byte[] buffer, uint count, ref uint written, IntPtr lpOverlapped);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr handle);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool FlushFileBuffers(IntPtr handle);

        //Constants for dwDesiredAccess:
        private const UInt32 GENERIC_READ = 0x80000000;
        private const UInt32 GENERIC_WRITE = 0x40000000;

        //Constants for return value:
        private const Int32 INVALID_HANDLE_VALUE = -1;

        //Constants for dwFlagsAndAttributes:
        private const UInt32 FILE_FLAG_OVERLAPPED = 0x40000000;
        private const UInt32 FILE_FLAG_NO_BUFFERING = 0x20000000;

        //Constants for dwCreationDisposition:
        private const UInt32 OPEN_EXISTING = 3;
        private const uint PIPE_ACCESS_OUTBOUND = 0x00000002;
        private const uint PIPE_ACCESS_DUPLEX = 0x00000003;
        private const uint PIPE_ACCESS_INBOUND = 0x00000001;
        private const uint PIPE_WAIT = 0x00000000;
        private const uint PIPE_NOWAIT = 0x00000001;
        private const uint PIPE_READMODE_BYTE = 0x00000000;
        private const uint PIPE_READMODE_MESSAGE = 0x00000002;
        private const uint PIPE_TYPE_BYTE = 0x00000000;
        private const uint PIPE_TYPE_MESSAGE = 0x00000004;
        private const uint PIPE_CLIENT_END = 0x00000000;
        private const uint PIPE_SERVER_END = 0x00000001;
        private const uint PIPE_UNLIMITED_INSTANCES = 255;
        private const uint NMPWAIT_WAIT_FOREVER = 0xffffffff;
        private const uint NMPWAIT_NOWAIT = 0x00000001;
        private const uint NMPWAIT_USE_DEFAULT_WAIT = 0x00000000;

        private const int ERROR_PIPE_CONNECTED = 535;

        public IntPtr Handle { get; private set; }
        public string pipeName { get; private set; }
        public uint BufferSize { get; private set; }

        public MemoryPipe(string pipe, uint bufferSize)
        {
            this.pipeName = "\\\\.\\pipe\\" + pipe;
            this.BufferSize = bufferSize;
        }

        public bool Connect()
        {
            Handle = CreateFile(pipeName, GENERIC_READ | GENERIC_WRITE, 0, IntPtr.Zero, OPEN_EXISTING, 0, IntPtr.Zero);
            if (Handle.ToInt32() == INVALID_HANDLE_VALUE)
                return false;
            return true;
        }

        public bool Listen()
        {
            Handle = CreateNamedPipeA(pipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                     PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                     PIPE_UNLIMITED_INSTANCES,
                                     BufferSize,
                                     BufferSize,
                                     0,
                                     IntPtr.Zero);
            return ((Handle.ToInt32() > 0) ? true : (Marshal.GetLastWin32Error() == ERROR_PIPE_CONNECTED));
        }

        public bool AcceptClient()
        {
            return ConnectNamedPipe(Handle, IntPtr.Zero) ? true : (Marshal.GetLastWin32Error() == ERROR_PIPE_CONNECTED);
        }

        public void Close()
        {
            DisconnectNamedPipe(Handle);
        }

        public uint Receive(ref string buff)
        {
            byte[] buffer = Receive();
            buff = ASCIIEncoding.Unicode.GetString(buffer);
            return (uint)buff.Length;
        }

        public byte[] Receive()
        {
            uint readed = 0;
            byte[] sizeBuffer = new byte[4];
            bool success = ReadFile(Handle, sizeBuffer, (uint)sizeBuffer.Length, ref readed, IntPtr.Zero);

            if (success)
            {
                int PacketSize = BitConverter.ToInt32(sizeBuffer, 0);
                byte[] buffer = new byte[PacketSize];
                success = ReadFile(Handle, buffer, (uint)buffer.Length, ref readed, IntPtr.Zero);
                return buffer;
            }
            return new byte[0];
        }

        public bool Send(byte[] buffer)
        {
            uint written = 0;
            FlushFileBuffers(Handle);

            WriteFile(Handle, BitConverter.GetBytes(buffer.Length), 4, ref written, IntPtr.Zero);
            return WriteFile(Handle, buffer, (uint)buffer.Length, ref written, IntPtr.Zero);
        }
    }
}