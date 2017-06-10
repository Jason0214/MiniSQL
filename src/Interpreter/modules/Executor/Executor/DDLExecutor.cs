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
            exeInterface.In.WriteLines(drop.TableName, drop.IndexName);
            outStream.WriteLine(exeInterface.Out.ReadLine());
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
            exeInterface.In.WriteLines(drop.TableName);
            outStream.WriteLine(exeInterface.Out.ReadLine());
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
            exeInterface.In.WriteLines(create.TableName, create.AttrName, create.IndexName);
            outStream.WriteLine(exeInterface.Out.ReadLine());
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
            exeInterface.In.WriteLines(create.TableName);

            foreach (var attr in create.Definitions)
            {
                exeInterface.In.WriteLine(string.Format("{0} {1} {2} {3} {4} {5}", 
                    attr.AttrName, attr.Type.TypeName, attr.Type.Param,
                    attr.IsPrimaryKey, attr.IsUnique, attr.IsNotNull));
            }

            outStream.WriteLine(exeInterface.Out.ReadLine());
        }
    }
}
