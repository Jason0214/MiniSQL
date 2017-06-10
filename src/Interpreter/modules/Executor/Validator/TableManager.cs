using System;
using System.Linq;
using System.Collections.Generic;
using MiniSQL.Errors;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Interface;

namespace MiniSQL.Executor.Validator
{
    public class TableManager
    {
        private ICatalog catalogInterface = CatalogInterface.Instance;
        private List<SQLError> Errors;
        private Dictionary<string, string> AliasTableNameMatcher = new Dictionary<string, string>();
        private Dictionary<string, Schema> TableNameSchemaMatcher = new Dictionary<string, Schema>();

        public TableManager(List<SQLError> errorReportList, ICatalog catalogInterface = null)
        {
            Errors = errorReportList;
            if (catalogInterface != null) this.catalogInterface = catalogInterface;
        }

        public void AddInfo(string tableName)
        {
            AddInfo(new WithAlias<string>(tableName, "__Default_Alias"));
        }

        public void AddInfo(WithAlias<string> nameAlias)
        {
            string alias = nameAlias.Alias == "__Default_Alias" ? nameAlias.Entity : nameAlias.Alias;

            if (catalogInterface.TableExist(nameAlias.Entity) == false)
            {
                Errors.Add(new SQLError("Validator: Unexist Table: " + nameAlias.Entity));
                return;
            }

            if (TableNameSchemaMatcher.ContainsKey(nameAlias.Entity) == false)
            {
                TableNameSchemaMatcher.Add(nameAlias.Entity, catalogInterface.GetSchemaInfo(nameAlias.Entity));
            }

            try
            {
                AliasTableNameMatcher.Add(alias, nameAlias.Entity);
            }
            catch (Exception)
            {
                Errors.Add(new SQLError("Validator: Duplicate Table " + alias));
            }
        }

        public Schema? GetSchema(string alias)
        {
            if (AliasTableNameMatcher.ContainsKey(alias) == false)
            {
                return null;
            }

            string tableName = AliasTableNameMatcher[alias];

            if (TableNameSchemaMatcher.ContainsKey(tableName) == false)
            {
                return null;
            }

            return TableNameSchemaMatcher[tableName];
        }

        public List<string> GetNormalizedAttrTypes(string alias)
        {
            Schema? schema = GetSchema(alias);

            if (schema == null)
            {
                return null;
            }

            List<string> result = new List<string>();

            foreach (AttrInfo attr in schema.Value.AttrList)
            {
                result.Add(TypeNormalize(attr.AttrType.TypeName));
            }

            return result;
        }

        public AttrInfo? GetAttrInfo(string attrName)
        {
            foreach (Schema schema in TableNameSchemaMatcher.Values)
            {
                AttrInfo? result = schema.GetAttr(attrName);

                if (result != null)
                {
                    return result;
                }
            }

            return null;
        }

        public AttrInfo? GetAttrInfo(Attr attr)
        {
            return GetAttrInfo(attr.AttributeName);
        }

        public void CheckAttr(Attr attr)
        {
            if (attr.TableName == "__Default_Table")
            {
                AttrAmbiguityCheck(attr);
                return;
            }

            if (AliasTableNameMatcher.ContainsKey(attr.TableName) == false)
            {
                Errors.Add(new SQLError("Validator: Unspecified Table: " + attr.TableName));
                return;
            }

            string actualTableName = AliasTableNameMatcher[attr.TableName];
            if (TableNameSchemaMatcher.ContainsKey(actualTableName) == false)
            {
                Errors.Add(new SQLError("Validator: Inner Error "));
                return;
            }

            Schema schema = TableNameSchemaMatcher[actualTableName];
            AttrInfo? attrInfo = schema.GetAttr(attr.AttributeName);

            if (attrInfo == null)
            {
                Errors.Add(new SQLError("Validator: Attribute does not exist: " + attr.AttributeName));
                return;
            }
        }

        public void AttrAmbiguityCheck(Attr attr)
        {
            int count = 0;

            foreach (Schema schema in TableNameSchemaMatcher.Values)
            {
                if (schema.GetAttr(attr.AttributeName) != null)
                {
                    count++;
                    attr.TableName = schema.TableName;
                }
            }

            if (count == 0)
            {
                Errors.Add(new SQLError("Validator: Unexist Attribute: " + attr.AttributeName));
            }
            else if (count > 1)
            {
                Errors.Add(new SQLError("Validator: Attribute ambiguity: " + attr.AttributeName));
            }
        }

        public List<string> CommonAttributeNames(string tableName1, string tableName2)
        {
            if (TableNameSchemaMatcher.ContainsKey(tableName1) == false || TableNameSchemaMatcher.ContainsKey(tableName2) == false)
            {
                throw new InterfaceError("Common Attr: Invalid table name");
            }

            return
            (
                from info1 in TableNameSchemaMatcher[tableName1].AttrList
                from info2 in TableNameSchemaMatcher[tableName2].AttrList
                where info1.AttrName == info2.AttrName
                select info1.AttrName
            ).ToList();
        }

        public static string TypeNormalize(string type)
        {
            if (type == "single" || type == "char" || type == "varchar")
            {
                return "string";
            }

            return type;
        }
    }
}
