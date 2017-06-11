using System.Collections.Generic;

namespace MiniSQL.SQLAnalyzer.Structures
{
    public class Attr : Statement
    {
        public string AttributeName { get; protected set; }

        public string TableName { get; set; }

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

    public class Comparand : Statement
    {
        public string OperandType { get; protected set; }

        public object OperandEntity { get; protected set; }

        public Comparand(string type, string entity)
        {
            OperandType = type;
            OperandEntity = entity;
        }

        public Comparand(string type, Attr entity)
        {
            OperandType = type;
            OperandEntity = entity;
        }
    }

    public class CompareExpression : Expression
    {
        public Comparand Operand1 { get; protected set; }

        public string Operation { get; protected set; }

        public Comparand Operand2 { get; protected set; }

        public CompareExpression(Comparand operand1, string operation, Comparand operand2)
        {
            Operand1 = operand1;
            Operation = operation;
            Operand2 = operand2;
        }
    }

    public class ConjunctionExpression : Expression
    {
        public ListNode<Expression> Expressions { get; protected set; }

        public List<Expression> ExpressionList { get; protected set; }

        public ConjunctionExpression(ListNode<Expression> expressions)
        {
            Expressions = expressions;
            ExpressionList = Expressions.ToList();
        }
    }

    public class DisjunctionExpression : Expression
    {
        public ListNode<Expression> Expressions { get; protected set; }

        public List<Expression> ExpressionList { get; protected set; }

        public DisjunctionExpression(ListNode<Expression> expressions)
        {
            Expressions = expressions;
            ExpressionList = Expressions.ToList();
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
        public static WhereClause nullWhere = new WhereClause(new ConjunctionExpression(ListNode<Expression>.NullNode));

        public Expression Exp { get; protected set; }

        public WhereClause(Expression exp)
        {
            Exp = exp;
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
