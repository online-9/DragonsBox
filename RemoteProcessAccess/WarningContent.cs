using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Be.Windows.Forms;
using System.IO;

namespace DragonsBox
{
    public partial class WarningContent : Form
    {
        public WarningContent(byte[] data)
        {
            InitializeComponent();
            this.hexBox1.ByteProvider = new DynamicByteProvider(data);

        }

        private void dumpToHarddiskToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog dialog = new SaveFileDialog())
            {
                dialog.CreatePrompt = true;
                dialog.OverwritePrompt = true;
                dialog.Title = "Select the file to dump the data";

                if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        File.WriteAllBytes(dialog.FileName, ((DynamicByteProvider)hexBox1.ByteProvider).Bytes.ToArray());
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.Message, "DragonBox Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }
        }

        private void CalcWidth()
        {
            if ((this.hexBox1.Width - 100) > 0)
            {
                int length = (this.hexBox1.Width - 100) / 35;
                hexBox1.BytesPerLine = length;
            }
        }

        private void WarningContent_Resize(object sender, EventArgs e)
        {
            CalcWidth();
        }
    }
}