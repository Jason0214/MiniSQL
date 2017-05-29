using System;
using Compiler.Lexer;
using Compiler.Parser;

namespace MiniSQL.SQLAnalyzer
{
    public class SQLError : Exception
    {
        public SQLError(LexerError error)
            : base(error.Message)
        {
        }

        public SQLError(ParserError error)
            : base(error.Message)
        {
        }

        public SQLError(string errMsg)
            : base(errMsg)
        {
        }
    }
}
