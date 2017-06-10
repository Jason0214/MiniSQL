using System.Collections.Generic;

namespace MiniSQL.SQLAnalyzer.Structures
{
    public class AttrType : Statement
    {
        public string TypeName { get; private set; }

        public int Param { get; private set; }

        public AttrType(string typeName, int param)
        {
            TypeName = typeName;
            Param = param;
        }
    }

    public class AttrDefinition : Statement
    {
        public string AttrName { get; private set; }

        public AttrType Type { get; private set; }

        public bool IsNotNull { get; private set; }

        public bool IsUnique { get; private set; }

        public bool IsPrimaryKey { get; private set; }

        public AttrDefinition(string name, AttrType type, IEnumerable<string> restricts)
        {
            AttrName = name;
            Type = type;

            foreach (string restrict in restricts)
            {
                switch (restrict)
                {
                    case "not null": IsNotNull = true; break;
                    case "unique": IsUnique = true; break;
                    case "primary key": IsPrimaryKey = true; break;
                }
            }
        }
    }

    public class CreateTable : Statement
    {
        public string TableName { get; private set; }

        public ListNode<AttrDefinition> DefinitionsHeaderNode { get; private set; }

        public ListNode<string> PrimaryKeysHeaderNode { get; private set; }

        public List<AttrDefinition> Definitions { get; private set; }

        public List<string> PrimaryKeys { get; private set; }

        public CreateTable(string name, ListNode<AttrDefinition> definitionsHeaderNode, ListNode<string> primaryKeysHeaderNode)
        {
            TableName = name;
            DefinitionsHeaderNode = definitionsHeaderNode;
            PrimaryKeysHeaderNode = primaryKeysHeaderNode;

            Definitions = DefinitionsHeaderNode.ToList();
            PrimaryKeys = PrimaryKeysHeaderNode.ToList();
        }
    }

    public class DropTable : Statement
    {
        public string TableName { get; private set; }

        public DropTable(string tableName)
        {
            TableName = tableName;
        }
    }

    public class CreateIndex : Statement
    {
        public string IndexName { get; private set; }

        public string TableName { get; private set; }

        public string AttrName { get; private set; }

        public CreateIndex(string indexName, string tableName, string attrName)
        {
            IndexName = indexName;
            TableName = tableName;
            AttrName = attrName;
        }
    }

    public class DropIndex : Statement
    {
        public string IndexName { get; private set; }

        public string TableName { get; private set; }

        public DropIndex(string indexName, string tableName)
        {
            IndexName = indexName;
            TableName = tableName;
        }
    }

    public class NewValue : Statement
    {
        public string TypeName { get; private set; }

        public string Value { get; private set; }

        public NewValue(string typeName, string value)
        {
            TypeName = typeName;
            Value = value;
        }
    }

    public class Insert : Statement
    {
        public string TableName { get; private set; }

        public ListNode<NewValue> ValuesHeaderNode { get; private set; }

        public List<NewValue> ValuesList { get; private set; }

        public Insert(string tableName, ListNode<NewValue> valuesHeaderNode)
        {
            TableName = tableName;
            ValuesHeaderNode = valuesHeaderNode;
            ValuesList = ValuesHeaderNode.ToList();
        }
    }

    public class Update : Statement
    {
        public string TableName { get; private set; }

        public string AttrName { get; private set; }

        public NewValue Value { get; private set; }

        public Update(string tableName, string attrName, NewValue value)
        {
            TableName = tableName;
            AttrName = attrName;
            Value = value;
        }
    }

    public class Delete : Statement
    {
        public string TableName { get; private set; }

        public WhereClause Where { get; private set; }

        public Delete(string tableName, WhereClause where)
        {
            TableName = tableName;
            Where = where;
        }
    }
}
