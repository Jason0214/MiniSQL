using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;

namespace ParserTest
{
    [TestClass]
    public class QueryTest : ParserResultTest<Query>
    {
        protected override Parser<Query> MyParser
        {
            get
            {
                return SQLParserComponents.P_Query;
            }
        }

        [TestMethod]
        public void Query_WithWhereClause()
        {
            Input = new List<Lexeme>() { select, lex, comma, lex, dot, lex,
                from, lex, join, lex, comma, lex, As, lex,
                where, lex, less, lex, and, left, lex, less, lex, or, lex, between, lex, and, Int, right, end };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void Query_WithoutWhereClause()
        {
            Input = new List<Lexeme>() { select, lex, comma, lex, dot, lex,
                from, lex, join, lex, comma, lex, As, lex, end };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void Query_WithoutSemicolon()
        {
            Input = new List<Lexeme>() { select, lex, comma, lex, dot, lex,
                from, lex, join, lex, comma, lex, As, lex };
            AssertError(Input.Count - 1);
        }

        [TestMethod]
        public void Query_WhereSegment()
        {
            Input = new List<Lexeme>() { select, lex, comma, lex, dot, lex,
                from, lex, join, lex, comma, lex, As, lex, where, end };
            AssertError(14);
        }
    }
}
