using System;
using MiniSQL.Errors;

namespace MiniSQL.Executor.Interface
{
    public interface ICatalog
    {
        bool TableExist(string name);

        bool AttrExist(string tableName, string attrName);

        Schema GetSchemaInfo(string name);
    }

    public class CatalogInterface : DatabaseInterface, ICatalog
    {
        public static CatalogInterface Instance = new CatalogInterface();

        public bool TableExist(string name)
        {
            CheckConnection();

            In.WriteLines("table_exist_check", name);
            return Out.ReadLine() == "true";
        }

        public bool AttrExist(string tableName, string attrName)
        {
            CheckConnection();

            In.WriteLines("attr_exist_check", tableName, attrName);
            return Out.ReadLine() == "true";
        }
        
        public Schema GetSchemaInfo(string name)
        {
            CheckConnection();
            
            In.WriteLines("schema_info", name);

            Schema schema = new Schema(name);

            try
            {
                int nCol = Convert.ToInt32(Out.ReadLine());

                for (int i = 0; i < nCol; i++)
                {
                    string attrName = Out.ReadLine();
                    string attrType = Out.ReadLine();
                    int param = 0;

                    switch (attrType)
                    {
                        case "int": break;
                        case "char": param = Convert.ToInt32(Out.ReadLine()); break;
                        case "varchar": param = Convert.ToInt32(Out.ReadLine()); break;
                        default: throw new InterfaceError("Schema information inconsistent [Unknown Type]");
                    }

                    schema += new AttrInfo(attrName, new TypeInfo(attrType, param));
                }
            }
            catch (Exception ex)
            {
                throw new InterfaceError(ex.Message);
            }

            return schema;
        }
    }
}
