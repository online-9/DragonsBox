using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace DragonsBox
{
    public partial class DotNetInfo : Form
    {
        public DotNetInfo(int pid)
        {
            InitializeComponent();
            label1.Invoke(new MainForm.Invoky(() => this.label1.Text += pid.ToString()));
        }
    }
}
