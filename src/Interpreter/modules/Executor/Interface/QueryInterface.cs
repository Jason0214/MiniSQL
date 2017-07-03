using System.Collections.Generic;
using System.IO;
using MiniSQL.SQLAnalyzer.Structures;

namespace MiniSQL.Executor.Interface
{
    public interface IQuery : IDB
    {
        void TableInfo(List<WithAlias<string>> nameAlias);

        void CopyResultTo(string tableName, TextWriter writer);
    }

    public class QueryInterface : DatabaseInterface, IQuery
    {
        public static QueryInterface Instance = new QueryInterface();

        public void TableInfo(List<WithAlias<string>> nameAlias)
        {
            In.WriteLines("table_info", nameAlias.Count);

            foreach (var namePair in nameAlias)
            {
                In.WriteLine((namePair.Alias == "__Default_Alias" ? namePair.Entity : namePair.Alias)
                    + " " + namePair.Entity);
            }
        }

        public void CopyResultTo(string tableName, TextWriter writer)
        {
            In.WriteLines("get_result", tableName);
            AcceptOutput(writer);
        }
    }
}
