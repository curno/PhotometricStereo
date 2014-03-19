using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Drawing;

namespace ImagePreprocess
{
    class Program
    {
        static void Main(string[] args)
        {
            string input_folder_name = @"F:\Data";
            DirectoryInfo di = new DirectoryInfo(input_folder_name);
            foreach (DirectoryInfo sub_di in di.GetDirectories())
            {
                string new_sub_di = Path.Combine(di.FullName, "new_" + sub_di.Name);
                Transform(sub_di.FullName, new_sub_di);
            }
            return;
        }

        private static void Transform(string input_folder_name, string output_folder_name)
        {
            Console.WriteLine("transforming files from" + input_folder_name + " to" + output_folder_name);
            int width = 600;
            DirectoryInfo input_di = new DirectoryInfo(input_folder_name);
            if (!input_di.Exists)
                return;
            DirectoryInfo output_di = new DirectoryInfo(output_folder_name);
            if (!output_di.Exists)
                output_di.Create();
            foreach (FileInfo fi in input_di.GetFiles("*.JPG", SearchOption.AllDirectories))
            {
                Console.WriteLine("resizing " + fi.FullName);
                Bitmap bmp = new Bitmap(fi.FullName);
                Bitmap new_bmp = new Bitmap(bmp, new Size(width, width * bmp.Size.Height / bmp.Size.Width));
                new_bmp.Save(Path.Combine(output_di.FullName, fi.Name));
                bmp.Dispose();
                new_bmp.Dispose();
            }
        }
    }
}
