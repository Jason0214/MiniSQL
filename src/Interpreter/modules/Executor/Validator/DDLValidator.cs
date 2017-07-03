using System.Collections.Generic;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Interface;
using MiniSQL.Errors;

namespace MiniSQL.Executor.Validator
{
    public class DropTableValidator : SQLValidator
    {
        private DropTable drop;
        private TableManager tableManager;

        public DropTableValidator(DropTable drop, ICatalog catalogInterface = null)
        {
            this.drop = drop;
            tableManager = new TableManager(this.Errors, catalogInterface);
        }

        public override List<SQLError> Validate()
        {
            tableManager.AddInfo(drop.TableName);
            return Errors;
        }
    }

    public class DropIndexValidator : SQLValidator
    {
        private IIndex indexInterface = IndexInterface.Instance;
        private DropIndex drop;
        private TableManager tableManager;

        public DropIndexValidator(DropIndex drop, IIndex indexInterface = null, ICatalog catalogInterface = null)
        {
            this.drop = drop;
            tableManager = new TableManager(this.Errors, catalogInterface);

            if (indexInterface != null)
            {
                this.indexInterface = indexInterface;
            }
        }

        public override List<SQLError> Validate()
        {
          /*  tableManager.AddInfo(drop.TableName);

            if (indexInterface.IndexExist(drop.IndexName) == false)
            {
                Errors.Add(new SQLError("Validator: Not Exist Index: " + drop.IndexName));
            }

            return Errors;*/
            return Errors;
        }
    }

    public class CreateIndexValidator : SQLValidator
    {
        private IIndex indexInterface = IndexInterface.Instance;
        private CreateIndex create;
        private TableManager tableManager;

        public CreateIndexValidator(CreateIndex create, IIndex indexInterface = null, ICatalog catalogInterface = null)
        {
            this.create = create;
            tableManager = new TableManager(this.Errors, catalogInterface);

            if (indexInterface != null)
            {
                this.indexInterface = indexInterface;
            }
        }

        public override List<SQLError> Validate()
        {
            if (indexInterface.IndexExist(create.IndexName) == true)
            {
                Errors.Add(new SQLError("Validator: Already Exist Index: " + create.IndexName));
                return Errors;
            }

            tableManager.AddInfo(create.TableName);

            if (Errors.Count > 0)
            {
                return Errors;
            }

            AttrInfo? attr = tableManager.GetAttrInfo(create.AttrName);

            if (attr == null)
            {
                Errors.Add(new SQLError("Validator: Attribute does not exist: " + create.AttrName));
                return Errors;
            }

            return Errors;
        }
    }

    public class CreateTableValidator : SQLValidator
    {
        private CreateTable create;
        private ICatalog catalogInterface = CatalogInterface.Instance;

        public CreateTableValidator(CreateTable create, ICatalog catalogInterface = null)
        {
            this.create = create;

            if (catalogInterface != null)
            {
                this.catalogInterface = catalogInterface;
            }
        }

        public override List<SQLError> Validate()
        {
            if (catalogInterface.TableExist(create.TableName) == true)
            {
                Errors.Add(new SQLError("Validator: Table Already Exist: " + create.TableName));
                return Errors;
            }

            HashSet<string> nameSet = new HashSet<string>();
            HashSet<string> primaryKeys = new HashSet<string>();

            foreach (AttrDefinition def in create.Definitions)
            {
                if (nameSet.Add(def.AttrName) == false)
                {
                    Errors.Add(new SQLError("Validator: Duplicate Attr Name: " + def.AttrName));
                    return Errors;
                }

                if (def.IsPrimaryKey)
                {
                    if (primaryKeys.Add(def.AttrName) == false)
                    {
                        Errors.Add(new SQLError("Validator: Duplicate Primary Keys: " + def.AttrName));
                        return Errors;
                    }
                }
            }

            foreach (string attr in create.PrimaryKeys)
            {
                if (nameSet.Contains(attr) == false)
                {
                    Errors.Add(new SQLError("Validator: Undefined Attr Name: " + attr));
                    return Errors;
                }

                if (primaryKeys.Add(attr) == false)
                {
                    Errors.Add(new SQLError("Validator: Duplicate Primary Keys: " + attr));
                    return Errors;
                }
            }

            if (primaryKeys.Count > 1)
            {
                Errors.Add(new SQLError("Validator: Only one primary key supported"));
            }

            return Errors;
        }
    }
}
