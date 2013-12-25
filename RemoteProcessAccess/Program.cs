using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;
using System.Drawing;
using DragonsBox;
using Microsoft.Win32.SafeHandles;
using System.IO;
using System.Reflection;

namespace RemoteProcessAccess
{
    static class Program
    {
        [STAThread()]
        static void Main()
        {
            bool bCreatedNew;

            Mutex mutex = new Mutex(true, "Global\\DragonBox", out bCreatedNew);
            if (bCreatedNew)
                mutex.ReleaseMutex();
            if (!bCreatedNew == true)
                return;
            Application.Run(new MainForm());
        }
    }
}