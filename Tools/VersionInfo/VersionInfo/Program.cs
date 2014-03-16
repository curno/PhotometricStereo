using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace VersionInfo
{
    class Program
    {
        static readonly string BUILD = "BUILD";
        static readonly string BUILD_TRY = "BUILD-TRY";
        static readonly string EXCUTE = "EXCUTE";
        static readonly char Seperator_in_file = ':';
        static readonly char Seperator = '-';
        static readonly string TITLE = "BUILD LOG";
        static readonly string [] Status = { BUILD, BUILD_TRY, EXCUTE };
        static void Main(string[] args)
        {
            DirectoryInfo di = new DirectoryInfo(Path.Combine(args.Length > 2 ? args[2] : ".", "Data"));
            if (!di.Exists)
                di.Create();
            FileInfo fi = new FileInfo(Path.Combine(di.FullName, "Build.dat"));
            if (!fi.Exists)
                fi.CreateText().Close();
            string[] contents = File.ReadAllLines(fi.FullName);

            int [] counts = new int[Status.Length];

            int target = -1;
            if (args.Length > 0)
                for (int i = 0; i < Status.Length; ++i)
                    if (args[0].ToLower() == Status[i].ToLower())
                    {
                        target = i;
                        break;
                    }
            if (target < 0)
                return;

            int delta = 1;
            if (args.Length > 1)
            {
                try
                {
                    delta = Convert.ToInt32(args[1]);
                }
                catch (System.Exception) { }
            }

            foreach (string line in contents)
            {
                for (int i = 0; i < Status.Length; ++i)
                {
                    if (line.ToLower().StartsWith(Status[i].ToLower() + Seperator_in_file))
                        counts[i] = Convert.ToInt32(line.Substring(Status[i].Length + 1));
                }
            }

            counts[target] += delta;

            string[] new_content = new string[Status.Length];

            string title = TITLE;
            for (int i = 0; i < Status.Length; ++i)
            {
                new_content[i] = Status[i] + Seperator_in_file + Convert.ToString(counts[i]);
                title += Seperator + new_content[i];
            }
            File.WriteAllLines(fi.FullName, new_content);

            fi = new FileInfo(Path.Combine(di.FullName, "Seq.dat"));
            if (!fi.Exists)
                fi.OpenText().Close();
            File.WriteAllText(fi.FullName, title);
        }
    }
}
