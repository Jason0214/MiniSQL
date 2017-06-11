using System.Collections.Generic;
using Compiler.Lexer;
using Compiler.Parser;
using MiniSQL.Errors;

namespace MiniSQL.SQLAnalyzer
{
    public class SQLLexer
    {
        private Lexer lexer;
        private List<Lexeme> lexemes;

        private static List<string> symbols = new List<string>() 
        { 
            "+", "-", "*", "/", ".", ">", ">=", "=", "<>", "<", "<=", ",", ";", "(", ")" 
        };

        private static List<string> keywords = new List<string>() 
        { 
            "select", "where", "from", "group by", "having", "order by", 
            "in", "not in", "and", "or", "between", "union", "intersect", "except", "as", "all", "distinct", "join", 
            "natural join", "using", "is null", "is not null", "avg", "count", "sum", "is not null", "exists", 
            "not exists", "unique", "not unique", "delete", "insert into", "values", "update", "set", "char", "varchar", 
            "int", "numeric", "create", "table", "index", "primary key", "foreign key", "not null", "check", "drop", "on"
        };

        public SQLLexer()
        {
            TokenTypeCollection.DoubleQuote.Enabled = false;
            lexer = new Lexer(symbols, keywords);
        }

        public List<Lexeme> Lexemes { get { return lexemes; } }

        public List<SQLError> Interpret(string input)
        {
            List<SQLError> errList = new List<SQLError>();
            List<LexerError> lexerErrors = lexer.Interpret(input);

            foreach (LexerError err in lexerErrors)
            {
                errList.Add(new SQLError(err.Message));
            }

            lexemes = Lexeme.ToLexemeList(lexer.GetResultTokens());

            return errList;
        }
    }
}
