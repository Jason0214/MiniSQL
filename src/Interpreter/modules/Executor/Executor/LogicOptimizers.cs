using System.Collections.Generic;
using System.Linq;
using MiniSQL.SQLAnalyzer.Structures;

namespace MiniSQL.Executor.Optimizer
{
    internal static class LogicOptimizers
    {
        public static bool RemoveConstantExp_IsEmptySet(this Query query)
        {
            List<CompareExpression> cmpList = query.whereClause.ToCompareList();

            for (int i = 0; i < cmpList.Count; i++)
            {
                CompareExpression cmp = cmpList[i];

                if (cmp.IsNonTableInvolvedExp())
                {
                    if (cmp.GetConstantExpResult() == false)
                    {
                        return true;
                    }
                    else
                    {
                        (query.whereClause.Exp as ConjunctionExpression).ExpressionList.RemoveAt(i);
                    }
                }
            }

            return false;
        }

        public static List<List<string>> CartesianToJoin(this Query query)
        {
            DisjointSet<string> disjSet = new DisjointSet<string>();

            foreach (var joinedList in query.fromClause.TableList)
            {
                disjSet.AddGroup
                (
                    from nameAlias in joinedList.JoinedTablesList
                    select nameAlias.Alias == "__Default_Alias" ? nameAlias.Entity : nameAlias.Alias
                );
            }

            foreach (var cmp in query.whereClause.ToCompareList())
            {
                if (cmp.IsDoubleTablesInvolvedExp() && cmp.Operation == "=")
                {
                    string tableName1 = (cmp.Operand1.OperandEntity as Attr).TableName;
                    string tableName2 = (cmp.Operand2.OperandEntity as Attr).TableName;

                    disjSet.UnionRoot(tableName1, tableName2);

                    (query.whereClause.Exp as ConjunctionExpression).ExpressionList.Remove(cmp);
                }
            }

            return disjSet.GetSets();
        }

        public static Dictionary<string, List<CompareExpression>> PushSelectDown(this Query query)
        {
            Dictionary<string, List<CompareExpression>> tablesNeedSelect = new Dictionary<string, List<CompareExpression>>();
            List<CompareExpression> cmpList = query.whereClause.ToCompareList();

            for (int i = 0; i < cmpList.Count; i++)
            {
                CompareExpression cmp = cmpList[i];

                if (cmp.IsSingleTableInvolvedExp())
                {
                    string tableName = "";

                    if (cmp.Operand1.OperandEntity is Attr)
                    {
                        tableName = (cmp.Operand1.OperandEntity as Attr).TableName;
                    }
                    else
                    {
                        tableName = (cmp.Operand2.OperandEntity as Attr).TableName;
                    }

                    if (tablesNeedSelect.ContainsKey(tableName) == false)
                    {
                        tablesNeedSelect.Add(tableName, new List<CompareExpression>());
                    }

                    tablesNeedSelect[tableName].Add(cmp);

                    (query.whereClause.Exp as ConjunctionExpression).ExpressionList.Remove(cmp);
                }
            }

            return tablesNeedSelect;
        }
    }
}
