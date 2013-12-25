using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Reflection;

namespace TestApp
{
    public partial class Form1 : Form
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

        public Form1()
        {
            CheckForIllegalCrossThreadCalls = false;
            Application.ThreadException += new ThreadExceptionEventHandler((object o, ThreadExceptionEventArgs e) =>
            {
                System.Diagnostics.StackTrace trace = new System.Diagnostics.StackTrace(e.Exception, true);
                string ErrorLog = "[Error] " + "Function: " + trace.GetFrame(0).GetMethod().Name + "\r\n" +
                                  "Line: " + trace.GetFrame(0).GetFileLineNumber() + "\r\n" +
                                  "Column: " + trace.GetFrame(0).GetFileColumnNumber() + "\r\n" +
                                  "Message: " + e.Exception.Message + "\r\n" +
                                  "Stacktrace: " + e.Exception.StackTrace;
                MessageBox.Show(ErrorLog, "");
            });
            AppDomain.CurrentDomain.UnhandledException += (object sender, UnhandledExceptionEventArgs e) =>
            {
                System.Diagnostics.StackTrace trace = new System.Diagnostics.StackTrace((Exception)e.ExceptionObject, true);
                string ErrorLog = "[Error] " + "Function: " + trace.GetFrame(0).GetMethod().Name + "\r\n" +
                                  "Line: " + trace.GetFrame(0).GetFileLineNumber() + "\r\n" +
                                  "Column: " + trace.GetFrame(0).GetFileColumnNumber() + "\r\n" +
                                  "Message: " + ((Exception)e.ExceptionObject).Message + "\r\n" +
                                  "Stacktrace: " + ((Exception)e.ExceptionObject).StackTrace;
                MessageBox.Show(ErrorLog, "");
            };
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            keylogger keylog = new keylogger();
            
        }

        private void button2_Click(object sender, EventArgs e)
        {
            BufferedGraphics bg = new BufferedGraphicsContext().Allocate(this.pictureBox1.CreateGraphics(), this.pictureBox1.ClientRectangle);
            //ThreadPool.QueueUserWorkItem((object obj) =>
            {
                int FPS = 0;
                int counter = 0;
                Stopwatch sw = Stopwatch.StartNew();

                //while (true)
                {
                    Bitmap bmpScreenshot = new Bitmap(Screen.PrimaryScreen.Bounds.Width, Screen.PrimaryScreen.Bounds.Height, PixelFormat.Format32bppArgb);
                    Graphics gfxScreenshot = Graphics.FromImage(bmpScreenshot);
                    gfxScreenshot.CopyFromScreen(Screen.PrimaryScreen.Bounds.X, Screen.PrimaryScreen.Bounds.Y, 0, 0, new Size(Screen.PrimaryScreen.Bounds.Width, Screen.PrimaryScreen.Bounds.Height), CopyPixelOperation.SourceCopy);
                    gfxScreenshot.Dispose();
                    Bitmap bmpScreenshot_resized = (Bitmap)((System.Drawing.Image)bmpScreenshot).GetThumbnailImage(412, 328, null, IntPtr.Zero);
                    gfxScreenshot.Dispose();
                    bmpScreenshot.Dispose();

                    bg.Graphics.Clear(Color.Black);
                    bg.Graphics.DrawImage(bmpScreenshot_resized, new Point(0, 0));
                    bg.Render();
                    bmpScreenshot_resized.Dispose();
                    counter++;

                    if (sw.ElapsedMilliseconds >= 1000)
                    {
                        FPS = counter;
                        counter = 0;
                        sw = Stopwatch.StartNew();
                        this.Text = "FPS: " + FPS;
                    }
                }
            }/*);*/
        }

        private void button3_Click(object sender, EventArgs e)
        {
            File.WriteAllBytes("C:\\test.txt", new byte[] { 1, 3, 3, 7 });
        }

        byte[] Data = new byte[] { 0x4D, 0x5A, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20, 0x6D, 0x6F, 0x64, 0x65 };
        private void button4_Click(object sender, EventArgs e)
        {
            IntPtr alloc = Marshal.AllocHGlobal(Data.Length);
            UIntPtr bytesWritten = UIntPtr.Zero;
            WriteProcessMemory(Process.GetCurrentProcess().Handle, alloc, Data, (uint)Data.Length, out bytesWritten);
            Marshal.FreeHGlobal(alloc);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            try
            {
            Assembly.Load(Data);
            }catch{}
        }
    }
}