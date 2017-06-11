using System.IO;

namespace MiniSQL.Executor.Interface
{
    public interface IDirectExe
    {
        StreamWriter In { get; }

        StreamReader Out { get; }
    }

    public class DirectExeInterface : DatabaseInterface, IDirectExe
    {
        public static DirectExeInterface Instance = new DirectExeInterface();

        public new StreamWriter In { get { return base.In; } }

        public new StreamReader Out { get { return base.Out; } }
    }
}
