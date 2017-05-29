using System;
using System.Collections.Generic;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;

namespace MiniSQL.SQLAnalyzer
{
    public class SQLParser
    {
        private Parser<Query> parser = SQLParserComponents.P_Query;
        
        public Statement resultStatement { get; private set; }

        public List<SQLError> Parse(List<Lexeme> inputLexemes)
        {
            List<SQLError> errors = new List<SQLError>();

            CopiableTokenScanner scanner = new CopiableTokenScanner(inputLexemes);

            Result<Query> result = parser.ParserFunction(scanner);

            if (result.ErrorOccurs)
            {
                errors.Add(new SQLError(result.Error));
            }

            if (result.ReturnedScanner.CurrentIndex != inputLexemes.Count)
            {
                errors.Add(new SQLError("Parser Error: Error = " + inputLexemes[result.ReturnedScanner.CurrentIndex]));
            }

            resultStatement = result.Value;

            return errors;
        }
    }
}
