using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Lexer;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer;
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
                where, lex, less, lex, and, left, lex, less, lex, or, lex, between, lex, and, Int, right };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void Query_WithoutWhereClause()
        {
            Input = new List<Lexeme>() { select, lex, comma, lex, dot, lex,
                from, lex, join, lex, comma, lex, As, lex };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void Query_WhereSegment()
        {
            Input = new List<Lexeme>() { select, lex, comma, lex, dot, lex,
                from, lex, join, lex, comma, lex, As, lex, where };
            AssertPartialCorrect(14);
        }
    }
}
