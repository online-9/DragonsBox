using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using DragonsBox;

namespace RemoteProcessAccess
{
    public partial class Warning : Form
    {
        private byte[] ContentData;
        public Warning(byte[] ContentData)
        {
            InitializeComponent();
            this.ContentData = ContentData;
            ghostButton3.Enabled = ContentData != null;
            ghostButton3.Visible = ContentData != null;
        }

        private void ghostButton1_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Close();
        }

        private void ghostButton2_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.No;
            this.Close();
        }

        private void ghostButton3_Click(object sender, EventArgs e)
        {
            new WarningContent(ContentData).ShowDialog();
        }
    }
}
