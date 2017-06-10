using System.Collections.Generic;
using System.Linq;

namespace MiniSQL.SQLAnalyzer.Structures
{
    public static class StructureExtensions
    {
        public static string ToCommandString(this Comparand operand)
        {
            string typeInfo = operand.OperandType + " ";

            if (operand.OperandEntity is Attr)
            {
                return typeInfo + (operand.OperandEntity as Attr).AttributeName;
            }
            else
            {
                return typeInfo + operand.OperandEntity.ToString();
            }
        }

        public static string ToCommandString(this CompareExpression cmp)
        {
            return string.Format("{0} {1} {2}\n", cmp.Operand1.ToCommandString(), cmp.Operation, cmp.Operand2.ToCommandString());
        }

        public static string ToCommandString(this WithAlias<Attr> attr)
        {
            return string.Format("{0} {1}", attr.Entity.AttributeName, attr.Alias);
        }

        public static List<CompareExpression> ToCompareList(this WhereClause whereClause)
        {
            return
            (
                from exp in (whereClause.Exp as ConjunctionExpression).ExpressionList
                select (CompareExpression)exp
            ).ToList();
        }
    }
}
