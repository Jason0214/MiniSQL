using System.Collections.Generic;

namespace Compiler.Parser
{
    public abstract class Statement
    {
    }

    public class LinkedListNode<T> : Statement
    {
        public static LinkedListNode<T> NullNode = new LinkedListNode<T>(default(T), null);

        public T Content { get; protected set; }

        public LinkedListNode<T> Next { get; protected set; }

        public LinkedListNode(T content, LinkedListNode<T> next)
           
        {
            Content = content;
            Next = next;
        }

        public List<T> ToList()
        {
            LinkedListNode<T> startNode = this;
            List<T> result = new List<T>();

            while (startNode != null && startNode != NullNode)
            {
                result.Add(startNode.Content);
                startNode = startNode.Next;
            }

            return result;
        }
    }

    public abstract class LogicComponent : Statement
    {
    }

    public abstract class Predict : LogicComponent
    {
    }

    public class LogicOperator : LogicComponent
    {
        public string Operator { get; protected set; }

        public LogicOperator(string oper)
        {
            Operator = oper;
        }
    }

    public class ComparePredict : Predict
    {
        public string OperandIdentifier { get; protected set; }

        public string Operator { get; protected set; }

        public string Operand2 { get; protected set; }

        public ComparePredict(string operandID, string operation, string operandNumOrStr)
        {
            OperandIdentifier = operandID;
            Operator = operation;
            Operand2 = operandNumOrStr;
        }
    }

    public class WhereClause : Statement
    {
        public LinkedListNode<LogicComponent> LogicHeaderNode { get; protected set; }

        public List<LogicComponent> LogicList { get; protected set; }

        public WhereClause(LinkedListNode<LogicComponent> logicList)
        {
            LogicHeaderNode = logicList;
            LogicList = LogicHeaderNode.ToList();
        }
    }

    public class SelectClause : Statement
    {
        public LinkedListNode<string> AttributesHeaderNode { get; protected set; }

        public List<string> AttributesList { get; protected set; }

        public SelectClause(LinkedListNode<string> attributeList)
        {
            AttributesHeaderNode = attributeList;
            AttributesList = AttributesHeaderNode.ToList();
        }
    }

    public abstract class Table : Statement
    {
    }

    public class TableID : Table
    {
        public string TableName { get; protected set; }

        public TableID(string tableName)
        {
            TableName = tableName;
        }
    }

    public class NaturalJoin : Table
    {
        public TableID table1 { get; protected set; }

        public Table table2 { get; protected set; }

        public NaturalJoin(TableID tb1, Table tb2)
        {
            table1 = tb1;
            table2 = tb2;
        }
    }

    public class FromClause : Statement
    {
        public LinkedListNode<Table> TablesHeaderNode { get; protected set; }

        public List<Table> TableList { get; protected set; }

        public FromClause(LinkedListNode<Table> tableList)
        {
            TablesHeaderNode = tableList;
            TableList = TablesHeaderNode.ToList();
        }
    }
}
