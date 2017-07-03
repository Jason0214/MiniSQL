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
                Attr attr = operand.OperandEntity as Attr;
                return typeInfo + "\n " + attr.AttributeName + "\n " + attr.TableName;
            }
            else
            {
                return typeInfo + "\n " + operand.OperandEntity as string + "\n NULL";
            }
        }

        public static string ToCommandString(this CompareExpression cmp)
        {
            return string.Format("{0}\n {1}\n {2}\n", cmp.Operand1.ToCommandString(), cmp.Operation, cmp.Operand2.ToCommandString());
        }

        public static string ToCommandString(this WithAlias<Attr> attr)
        {
            return string.Format("{0} {1}\n", attr.Entity.AttributeName, 
                attr.Alias == "__Default_Alias" ? attr.Entity.AttributeName : attr.Alias);
        }

        public static string ToCommandString(this List<CompareExpression> attrs)
        {
            string result = "";

            result += attrs.Count;

            foreach (var attr in attrs)
            {
                result += "\n " + attr.ToCommandString();
            }

            return result;
        }

        public static int ToZeroOne(this bool b)
        {
            return b ? 1 : 0;
        }

        public static List<CompareExpression> ToCompareList(this WhereClause whereClause)
        {
            var list =
            (
                from exp in (whereClause.Exp as ConjunctionExpression).ExpressionList
                select (CompareExpression)exp   
            ).ToList();

            list.Sort((cmp1, cmp2) =>
            {
                return string.Compare(cmp1.Operand1.OperandEntity.ToString(), cmp2.Operand1.OperandEntity.ToString());
            });

            return list;
        }
    }
}
