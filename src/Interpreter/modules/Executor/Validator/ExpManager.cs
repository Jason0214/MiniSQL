using System.Collections.Generic;
using MiniSQL.Errors;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Interface;

namespace MiniSQL.Executor.Validator
{
    public class ExpManager
    {
        private Expression Exp;
        private List<SQLError> Errors;
        private TableManager tableManager;

        public ExpManager(Expression exp, List<SQLError> errorReportList, TableManager tableManager)
        {
            Exp = exp;
            Errors = errorReportList;
            this.tableManager = tableManager;
        }

        public void OnlyConjunctionSupported()
        {
            if (!(Exp is ConjunctionExpression)) goto Error;

            ConjunctionExpression conj = Exp as ConjunctionExpression;
            foreach (Expression subExp in conj.ExpressionList)
            {
                if (!(subExp is CompareExpression))
                {
                    goto Error;
                }
            }

            return;

        Error:
            Errors.Add(new SQLError("Validator: Only Conjunction Expression Supported"));
        }

        public string GetOperandType(Comparand op)
        {
            if (op.OperandEntity is Attr)
            {
                AttrInfo? attrInfo = tableManager.GetAttrInfo(op.OperandEntity as Attr);
                return attrInfo.Value.AttrType.TypeName;
            }
            else
            {
                return op.OperandType;
            }
        }

        public void OperandTypeMatchCheck(Comparand op1, Comparand op2)
        {
            string type1 = TableManager.TypeNormalize(GetOperandType(op1));
            string type2 = TableManager.TypeNormalize(GetOperandType(op2));

            if (type1 != type2)
            {
                Errors.Add(new SQLError(string.Format("{0} and {1}: Type mismatch", type1, type2)));
            }
        }

        public void ScanExpression()
        {
            ScanExpression(Exp);
        }

        private void ScanExpression(Expression p)
        {
            if (p is DisjunctionExpression)
            {
                DisjunctionExpression disj = p as DisjunctionExpression;
                foreach (Expression subExp in disj.ExpressionList)
                {
                    ScanExpression(subExp);
                }
            }
            else if (p is ConjunctionExpression)
            {
                ConjunctionExpression conj = p as ConjunctionExpression;
                foreach (Expression subExp in conj.ExpressionList)
                {
                    ScanExpression(subExp);
                }
            }
            else if (p is CompareExpression)
            {
                CompareExpression cmp = p as CompareExpression;

                if (cmp.Operand1.OperandEntity is Attr)
                {
                    Attr attr = cmp.Operand1.OperandEntity as Attr;
                    tableManager.CheckAttr(attr);
                }

                if (cmp.Operand2.OperandEntity is Attr)
                {
                    Attr attr = cmp.Operand2.OperandEntity as Attr;
                    tableManager.CheckAttr(attr);
                }

                OperandTypeMatchCheck(cmp.Operand1, cmp.Operand2);
            }
            else
            {
                Errors.Add(new SQLError("Validator: Invalid Expression"));
            }
        }
    }
}
