using System.Collections.Generic;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Interface;

namespace MiniSQL.Executor
{
    public class InsertExecutor : SQLExecutor
    {
        private IDirectExe exeInterface = DirectExeInterface.Instance;
        private Insert insert;

        public InsertExecutor(Insert insert, IDirectExe exeInterface = null)
        {
            this.insert = insert;

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            exeInterface.In.WriteLines("begin_insert", insert.TableName, insert.ValuesList.Count);

            foreach (var value in insert.ValuesList)
            {
                exeInterface.In.WriteLine(value.Value);
            }

            exeInterface.AcceptOutput(outStream);
        }
    }

    public class UpdateExecutor : SQLExecutor
    {
        private IDirectExe exeInterface = DirectExeInterface.Instance;
        private Update update;

        public UpdateExecutor(Update update, IDirectExe exeInterface = null)
        {
            this.update = update;

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            exeInterface.In.WriteLines("begin_update", update.TableName, update.AttrName, update.Value.Value);
            exeInterface.In.WriteLine(update.Where.ToCompareList().ToCommandString());
            exeInterface.AcceptOutput(outStream);
        }
    }

    public class DeleteExecutor : SQLExecutor
    {
        private IDirectExe exeInterface = DirectExeInterface.Instance;
        private Delete delete;

        public DeleteExecutor(Delete delete, IDirectExe exeInterface = null)
        {
            this.delete = delete;

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            exeInterface.In.WriteLines("begin_delete", delete.TableName);
            exeInterface.In.WriteLine(delete.Where.ToCompareList().ToCommandString());
            exeInterface.AcceptOutput(outStream);
        }
    }
}
