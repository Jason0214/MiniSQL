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
            exeInterface.In.WriteLines(insert.TableName, insert.ValuesList.Count);

            foreach (var value in insert.ValuesList)
            {
                exeInterface.In.WriteLine(value.Value);
            }

            outStream.WriteLine(exeInterface.Out.ReadLine());
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
            exeInterface.In.WriteLines(update.TableName, update.AttrName, update.Value);
            outStream.WriteLine(exeInterface.Out.ReadLine());
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
            exeInterface.In.WriteLine(delete.TableName);

            string cmpCommand = "";

            List<CompareExpression> compares = delete.Where.ToCompareList();

            foreach (var cmp in compares)
            {
                cmpCommand += cmp.ToCommandString();
            }

            exeInterface.In.WriteLine(cmpCommand);

            outStream.WriteLine(exeInterface.Out.ReadLine());
        }
    }
}
