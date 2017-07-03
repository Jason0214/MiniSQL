using System;
using System.IO;
using System.Diagnostics;
using MiniSQL.Errors;

namespace MiniSQL.Executor.Interface
{
    public interface IDB
    {
        void AcceptOutput(TextWriter writer);
    }

    internal class APIProcess
    {
        public Process cmd;

        public void Start()
        {
            try
            {
                cmd = new Process();
                cmd.StartInfo.Arguments = "asd";
                cmd.StartInfo.FileName = "API.exe";
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
                throw new InterfaceError("Fail to start api process");
            }
        }

        public APIProcess()
        {
            Start();
        }
    }

    public abstract class DatabaseInterface : IDB
    {
        internal static APIProcess Cmd = new APIProcess();

        internal StreamWriter In { get { CheckConnection(); return Cmd.cmd.StandardInput; } }

        internal StreamReader Out { get { CheckConnection(); return Cmd.cmd.StandardOutput; } }

        internal StreamReader Err { get { CheckConnection(); return Cmd.cmd.StandardError; } }

        internal void CheckConnection()
        {
            try
            {
                if (Cmd.cmd.HasExited)
                {
                    throw new InterfaceError("Connection to API Failed : 1");
                }
            }
            catch (Exception)
            {
                throw new InterfaceError("Connection to API Failed : 2");
            }
        }

        public void AcceptOutput(TextWriter writer)
        {
            string resultLine = Out.ReadLine();

            while (resultLine != "end_result")
            {
                writer.WriteLine(resultLine);
                resultLine = Out.ReadLine();
            }
        }
    }
}
