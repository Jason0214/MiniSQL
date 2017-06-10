using System;
using System.IO;
using System.Diagnostics;
using MiniSQL.Errors;

namespace MiniSQL.Executor.Interface
{
    internal class APIProcess
    {
        public Process cmd;

        public void Start()
        {
            try
            {
                cmd = new Process();
                cmd.StartInfo.Arguments = "asd";
                cmd.StartInfo.FileName = @"E:\ZJU\dbs\miniSQL\ConsoleTest\Release\ConsoleTest.exe";
                cmd.StartInfo.UseShellExecute = false;
                cmd.StartInfo.RedirectStandardInput = true;
                cmd.StartInfo.RedirectStandardOutput = true;
                cmd.StartInfo.RedirectStandardError = true;
                cmd.StartInfo.CreateNoWindow = true;
                cmd.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
                cmd.Start();
            }
            catch (Exception)
            {
                throw new InterfaceError("Fail to start catalog manager");
            }
        }

        public APIProcess()
        {
            Start();
        }
    }

    public abstract class DatabaseInterface
    {
        internal static APIProcess Cmd = new APIProcess();

        internal StreamWriter In { get { return Cmd.cmd.StandardInput; } }

        internal StreamReader Out { get { return Cmd.cmd.StandardOutput; } }

        internal StreamReader Err { get { return Cmd.cmd.StandardError; } }

        internal void CheckConnection()
        {
            try
            {
                if (Cmd.cmd.HasExited)
                {
                    throw new InterfaceError("Connection to Catalog Manager Failed : 1");
                }
            }
            catch (Exception)
            {
                throw new InterfaceError("Connection to Catalog Manager Failed : 2");
            }
        }
    }
}
