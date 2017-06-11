using MiniSQL.SQLAnalyzer.Structures;

namespace MiniSQL.Executor.Optimizer
{
    internal static class OptimizerTools
    {
        private static int IndexGenerator = 0;

        public static string GetOptTmpName()
        {
            return "_opt_tmp" + IndexGenerator++;
        }

        public static bool GetConstantExpResult(this CompareExpression cmp)
        {
            Comparand op1 = cmp.Operand1;
            Comparand op2 = cmp.Operand2;

            string entity1 = op1.OperandEntity.ToString();
            string entity2 = op2.OperandEntity.ToString();

            switch (cmp.Operation)
            {
                case "<": return string.Compare(entity1, entity2) < 0;
                case "<=": return string.Compare(entity1, entity2) <= 0;
                case "=": return string.Compare(entity1, entity2) == 0;
                case "<>": return string.Compare(entity1, entity2) != 0;
                case ">=": return string.Compare(entity1, entity2) >= 0;
                case ">": return string.Compare(entity1, entity2) > 0;
                default: return false;
            }
        }

        public static bool IsNonTableInvolvedExp(this CompareExpression cmp)
        {
            Comparand op1 = cmp.Operand1;
            Comparand op2 = cmp.Operand2;

            bool op1IsAttr = op1.OperandEntity is Attr;
            bool op2IsAttr = op2.OperandEntity is Attr;

            return !(op1IsAttr || op2IsAttr);
        }

        public static bool IsSingleTableInvolvedExp(this CompareExpression cmp)
        {
            Comparand op1 = cmp.Operand1;
            Comparand op2 = cmp.Operand2;

            bool op1IsAttr = op1.OperandEntity is Attr;
            bool op2IsAttr = op2.OperandEntity is Attr;

            if (op1IsAttr && op2IsAttr)
            {
                Attr attr1 = op1.OperandEntity as Attr;
                Attr attr2 = op2.OperandEntity as Attr;

                return BelongToSameTable(attr1, attr2);
            }

            return op1IsAttr || op2IsAttr;
        }

        public static bool IsDoubleTablesInvolvedExp(this CompareExpression cmp)
        {
            Comparand op1 = cmp.Operand1;
            Comparand op2 = cmp.Operand2;

            bool op1IsAttr = op1.OperandEntity is Attr;
            bool op2IsAttr = op2.OperandEntity is Attr;

            if (op1IsAttr && op2IsAttr)
            {
                Attr attr1 = op1.OperandEntity as Attr;
                Attr attr2 = op2.OperandEntity as Attr;

                return !BelongToSameTable(attr1, attr2);
            }

            return false;
        }

        public static bool BelongToSameTable(Attr attr1, Attr attr2)
        {
            return attr1.TableName == attr2.TableName;
        }
    }
}
