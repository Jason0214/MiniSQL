using System.IO;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Errors;

namespace MiniSQL.Executor
{
    public abstract class SQLExecutor
    {
        protected TextWriter outStream;

        public abstract void Execute();

        public static SQLExecutor GetExecutor(Statement statement, TextWriter outStream)
        {
            SQLExecutor executor = null;

            if (statement is Query) executor = new QueryExecutor(statement as Query);
            else if (statement is CreateTable) executor = new CreateTableExecutor(statement as CreateTable);
            else if (statement is CreateIndex) executor = new CreateIndexExecutor(statement as CreateIndex);
            else if (statement is DropTable) executor = new DropTableExecutor(statement as DropTable);
            else if (statement is DropIndex) executor = new DropIndexExecutor(statement as DropIndex);
            else if (statement is Insert) executor = new InsertExecutor(statement as Insert);
            else if (statement is Update) executor = new UpdateExecutor(statement as Update);
            else if (statement is Delete) executor = new DeleteExecutor(statement as Delete);
            else throw new ExecutionError("Unknown Statement");

            executor.outStream = outStream;

            return executor;
        }
    }
}
