using System.Collections.Generic;

namespace MiniSQL.SQLAnalyzer.Structures
{
    public class Attr : Statement
    {
        public string AttributeName { get; protected set; }

        public string TableName { get; protected set; }

        public Attr(string attr, string table)
        {
            AttributeName = attr;
            TableName = table;
        }
    }

    public class Table : Statement
    {
        public ListNode<WithAlias<string>> JoinedTablesHeaderNode { get; protected set; }

        public List<WithAlias<string>> JoinedTablesList { get; protected set; }

        public Table(ListNode<WithAlias<string>> joinedTablesList)
        {
            JoinedTablesHeaderNode = joinedTablesList;
            JoinedTablesList = JoinedTablesHeaderNode.ToList();
        }
    }

    public class CompareOperand : Statement
    {
        public string OperandType { get; protected set; }

        public object OperandEntity { get; protected set; }

        public CompareOperand(string type, string entity)
        {
            OperandType = type;
            OperandEntity = entity;
        }

        public CompareOperand(string type, Attr entity)
        {
            OperandType = type;
            OperandEntity = entity;
        }
    }

    public class ComparePredict : Predict
    {
        public CompareOperand Operand1 { get; protected set; }

        public string Operation { get; protected set; }

        public CompareOperand Operand2 { get; protected set; }

        public ComparePredict(CompareOperand operand1, string operation, CompareOperand operand2)
        {
            Operand1 = operand1;
            Operation = operation;
            Operand2 = operand2;
        }
    }

    public class ConjunctionPredict : Predict
    {
        public ListNode<Predict> Predicts { get; protected set; }

        public List<Predict> PredictList { get; protected set; }

        public ConjunctionPredict(ListNode<Predict> predicts)
        {
            Predicts = predicts;
            PredictList = Predicts.ToList();
        }
    }

    public class DisjunctionPredict : Predict
    {
        public ListNode<Predict> Predicts { get; protected set; }

        public List<Predict> PredictList { get; protected set; }

        public DisjunctionPredict(ListNode<Predict> predicts)
        {
            Predicts = predicts;
            PredictList = Predicts.ToList();
        }
    }

    public class SelectClause : Statement
    {
        public ListNode<WithAlias<Attr>> AttributesHeaderNode { get; protected set; }

        public List<WithAlias<Attr>> AttributesList { get; protected set; }

        public SelectClause(ListNode<WithAlias<Attr>> attributeList)
        {
            AttributesHeaderNode = attributeList;
            AttributesList = AttributesHeaderNode.ToList();
        }
    }

    public class FromClause : Statement
    {
        public ListNode<Table> TablesHeaderNode { get; protected set; }

        public List<Table> TableList { get; protected set; }

        public FromClause(ListNode<Table> tableList)
        {
            TablesHeaderNode = tableList;
            TableList = TablesHeaderNode.ToList();
        }
    }

    public class WhereClause : Statement
    {
        public static WhereClause nullWhere = new WhereClause(null);

        public Predict P { get; protected set; }

        public WhereClause(Predict p)
        {
            P = p;
        }
    }

    public class Query : Statement
    {
        public SelectClause selectClause { get; protected set; }

        public FromClause fromClause { get; protected set; }

        public WhereClause whereClause { get; protected set; }

        public Query(SelectClause select, FromClause from, WhereClause where)
        {
            selectClause = select;
            fromClause = from;
            whereClause = where;
        }
    }
}
