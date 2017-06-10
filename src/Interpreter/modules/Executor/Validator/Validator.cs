using System.Collections.Generic;
using MiniSQL.Errors;
using MiniSQL.SQLAnalyzer.Structures;

namespace MiniSQL.Executor.Validator
{
    public abstract class SQLValidator
    {
        protected List<SQLError> Errors;

        public abstract List<SQLError> Validate();

        public static SQLValidator GetValidator(Statement statement)
        {
            if (statement is Query) return new QueryValidator(statement as Query);
            if (statement is CreateTable) return new CreateTableValidator(statement as CreateTable);
            if (statement is CreateIndex) return new CreateIndexValidator(statement as CreateIndex);
            if (statement is DropTable) return new DropTableValidator(statement as DropTable);
            if (statement is DropIndex) return new DropIndexValidator(statement as DropIndex);
            if (statement is Insert) return new InsertValidator(statement as Insert);
            if (statement is Update) return new UpdateValidator(statement as Update);
            if (statement is Delete) return new DeleteValidator(statement as Delete);

            throw new SQLError("Unknown Statement");
        }
    }
}
