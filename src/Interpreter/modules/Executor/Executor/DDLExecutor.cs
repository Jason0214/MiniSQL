using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Interface;

namespace MiniSQL.Executor
{
    public class DropIndexExecutor : SQLExecutor
    {
        private IDirectExe exeInterface = DirectExeInterface.Instance;
        private DropIndex drop;

        public DropIndexExecutor(DropIndex drop, IDirectExe exeInterface = null)
        {
            this.drop = drop;

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            exeInterface.In.WriteLines("begin_drop_index", drop.IndexName);
            exeInterface.AcceptOutput(outStream);
        }
    }

    public class DropTableExecutor : SQLExecutor
    {
        private IDirectExe exeInterface = DirectExeInterface.Instance;
        private DropTable drop;

        public DropTableExecutor(DropTable drop, IDirectExe exeInterface = null)
        {
            this.drop = drop;

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            exeInterface.In.WriteLines("begin_drop_table", drop.TableName);
            exeInterface.AcceptOutput(outStream);
        }
    }

    public class CreateIndexExecutor : SQLExecutor
    {
        private IDirectExe exeInterface = DirectExeInterface.Instance;
        private CreateIndex create;

        public CreateIndexExecutor(CreateIndex create, IDirectExe exeInterface = null)
        {
            this.create = create;

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            exeInterface.In.WriteLines("begin_create_index", create.TableName, create.AttrName, create.IndexName);
            exeInterface.AcceptOutput(outStream);
        }
    }

    public class CreateTableExecutor : SQLExecutor
    {
        private IDirectExe exeInterface = DirectExeInterface.Instance;
        private CreateTable create;

        public CreateTableExecutor(CreateTable create, IDirectExe exeInterface = null)
        {
            this.create = create;

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            exeInterface.In.WriteLines("begin_create_table", create.TableName, create.Definitions.Count);

            foreach (var attr in create.Definitions)
            {
                exeInterface.In.WriteLine(string.Format("{0} {1} {2} {3} {4} {5}", 
                    attr.AttrName, attr.Type.TypeName, attr.Type.Param,
                    attr.IsPrimaryKey.ToZeroOne(), attr.IsUnique.ToZeroOne(), attr.IsNotNull.ToZeroOne()));
            }

            exeInterface.AcceptOutput(outStream);
        }
    }
}
