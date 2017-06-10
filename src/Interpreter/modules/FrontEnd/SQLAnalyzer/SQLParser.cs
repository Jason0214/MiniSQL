using System.Collections.Generic;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;
using MiniSQL.Errors;

namespace MiniSQL.SQLAnalyzer
{
    public class SQLParser
    {
        private static Parser<Statement> parser =
            SQLParserComponents.P_SimpleQuery.ToBaseClass<Query, Statement>() |
            SQLParserComponents.P_Insert.ToBaseClass<Insert, Statement>() |
            SQLParserComponents.P_Delete.ToBaseClass<Delete, Statement>() |
            SQLParserComponents.P_Update.ToBaseClass<Update, Statement>() |
            SQLParserComponents.P_CreateTable.ToBaseClass<CreateTable, Statement>() |
            SQLParserComponents.P_DropTable.ToBaseClass<DropTable, Statement>() |
            SQLParserComponents.P_CreateIndex.ToBaseClass<CreateIndex, Statement>() |
            SQLParserComponents.P_DropIndex.ToBaseClass<DropIndex, Statement>();
        
        public Statement resultStatement { get; private set; }

        public List<SQLError> Parse(List<Lexeme> inputLexemes)
        {
            List<SQLError> errors = new List<SQLError>();

            CopiableTokenScanner scanner = new CopiableTokenScanner(inputLexemes);

            Result<Statement> result = parser.ParserFunction(scanner);

            if (result.ErrorOccurs)
            {
                errors.Add(new SQLError(result.Error.Message));
                return errors;
            }

            if (result.ReturnedScanner.CurrentIndex != inputLexemes.Count)
            {
                errors.Add(new SQLError("Parser Error: Error = " + inputLexemes[result.ReturnedScanner.CurrentIndex].Content));
            }

            resultStatement = result.Value;

            return errors;
        }
    }
}
