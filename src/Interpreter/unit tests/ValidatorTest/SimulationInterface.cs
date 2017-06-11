using MiniSQL.Executor.Interface;

namespace ValidatorTest
{
    internal class SimulationInterface : ICatalog
    {
        public bool TableExist(string name)
        {
            if (name == "t1" || name == "t11" || name == "t2" || name == "t3")
            {
                return true;
            }

            return false;
        }

        public bool AttrExist(string tableName, string attrName)
        {
            return false;
        }

        public Schema GetSchemaInfo(string name)
        {
            if (name == "t1")
            {
                Schema schema = new Schema("t1");

                schema += new AttrInfo("a1", TypeInfo.Int());
                schema += new AttrInfo("b1", TypeInfo.Char(5));
                schema += new AttrInfo("c1", TypeInfo.Varchar(20));

                return schema;
            }
            else if (name == "t11")
            {
                Schema schema = new Schema("t11");

                schema += new AttrInfo("a1", TypeInfo.Int());
                schema += new AttrInfo("b1", TypeInfo.Char(5));
                schema += new AttrInfo("c1", TypeInfo.Varchar(20));

                return schema;
            }
            else if (name == "t2")
            {
                Schema schema = new Schema("t2");

                schema += new AttrInfo("a2", TypeInfo.Int());
                schema += new AttrInfo("b2", TypeInfo.Char(5));
                schema += new AttrInfo("c2", TypeInfo.Varchar(20));

                return schema;
            }
            else
            {
                Schema schema = new Schema("t3");

                schema += new AttrInfo("a3", TypeInfo.Int());
                schema += new AttrInfo("b3", TypeInfo.Char(5));
                schema += new AttrInfo("c3", TypeInfo.Varchar(20));

                return schema;
            }
        }
    }
}
