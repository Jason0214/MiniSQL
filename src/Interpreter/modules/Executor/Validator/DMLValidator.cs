using System.Collections.Generic;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Interface;
using MiniSQL.Errors;

namespace MiniSQL.Executor.Validator
{
    public class InsertValidator : SQLValidator
    {
        private Insert insert;
        private TableManager tableManager;

        public InsertValidator(Insert insert, ICatalog catalogInterface = null)
        {
            this.insert = insert;
            tableManager = new TableManager(this.Errors, catalogInterface);
        }

        public override List<SQLError> Validate()
        {
            tableManager.AddInfo(insert.TableName);

            if (Errors.Count > 0)
            {
                return Errors;
            }

            List<string> typeList = tableManager.GetNormalizedAttrTypes(insert.TableName);

            if (typeList.Count != insert.ValuesList.Count)
            {
                Errors.Add(new SQLError("Validator: Too Many / Too Few Values"));
                return Errors;
            }

            for (int i = 0; i < typeList.Count; i++)
            {
                string normalizedType = TableManager.TypeNormalize(insert.ValuesList[i].TypeName);

                if (typeList[i] != normalizedType)
                {
                    Errors.Add(new SQLError(string.Format("Validator: {0} and {1}, Type Mismatch", typeList[i], normalizedType)));
                }
            }

            return Errors;
        }
    }

    public class DeleteValidator : SQLValidator
    {
        private Delete delete;
        private TableManager tableManager;
        private ExpManager expManager;

        public DeleteValidator(Delete delete, ICatalog catalogInterface = null)
        {
            this.delete = delete;
            tableManager = new TableManager(this.Errors, catalogInterface);
            expManager = new ExpManager(delete.Where.Exp, Errors, tableManager);
        }

        public override List<SQLError> Validate()
        {
            tableManager.AddInfo(delete.TableName);

            if (Errors.Count > 0)
            {
                return Errors;
            }

            expManager.OnlyConjunctionSupported();
            expManager.ScanExpression();

            return Errors;
        }
    }

    public class UpdateValidator : SQLValidator
    {
        private Update update;
        private TableManager tableManager;

        public UpdateValidator(Update update, ICatalog catalogInterface = null)
        {
            this.update = update;
            tableManager = new TableManager(this.Errors, catalogInterface);
        }

        public override List<SQLError> Validate()
        {
            tableManager.AddInfo(update.TableName);

            if (Errors.Count > 0)
            {
                return Errors;
            }

            AttrInfo? attr = tableManager.GetAttrInfo(update.AttrName);

            if (attr == null)
            {
                Errors.Add(new SQLError("Validator: Attribute does not exist: " + update.AttrName));
                return Errors;
            }

            string type1 = TableManager.TypeNormalize(attr.Value.AttrType.TypeName);
            string type2 = TableManager.TypeNormalize(update.Value.TypeName);

            if (type1 != type2)
            {
                Errors.Add(new SQLError(string.Format("Validator: {0} and {1}, Type Mismatch", type1, type2)));
            }

            return Errors;
        }
    }
}
