using System.IO;

namespace MiniSQL.Executor.Interface
{
    public interface IQuery
    {
        void ExeCommand(string command, string sourceTableName, string resultTableName, string Param = null);

        void ExeCommand(string command, string sourceTableName1, string sourceTableName2,
            string resultTableName, string Param = null);

        void CopyResultTo(string tableName, TextWriter writer);
    }

    public class QueryInterface : DatabaseInterface, IQuery
    {
        public static QueryInterface Instance = new QueryInterface();

        public void ExeCommand(string command, string sourceTableName, string resultTableName, string Param = null)
        {
            In.WriteLines(command, sourceTableName, resultTableName);

            if (Param != null)
            {
                In.WriteLine(Param);
            }
        }

        public void ExeCommand(string command, string sourceTableName1, string sourceTableName2,
            string resultTableName, string Param = null)
        {
            In.WriteLines(command, sourceTableName1, sourceTableName2, resultTableName);

            if (Param != null)
            {
                In.WriteLine(Param);
            }
        }

        public void CopyResultTo(string tableName, TextWriter writer)
        {
            In.WriteLines("get result", tableName);

            string resultLine = Out.ReadLine();

            while (resultLine != "end result")
            {
                writer.WriteLine(resultLine); 
                resultLine = Out.ReadLine();
            }
        }
    }
}
