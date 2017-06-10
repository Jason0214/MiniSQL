using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;

namespace ParserTest
{
    [TestClass]
    public class InsertTest : ParserResultTest<Insert>
    {
        protected override Parser<Insert> MyParser
        {
            get 
            {
                return SQLParserComponents.P_Insert;
            }
        }

        [TestMethod]
        public void DML_Insert()
        {
            Input = new List<Lexeme>() { insertinto, lex, values, left, str, comma, Int, comma, Float, right, end };
            AssertCompleteCorrect();
        }
    }

    [TestClass]
    public class UpdateTest : ParserResultTest<Update>
    {
        protected override Parser<Update> MyParser
        {
            get
            {
                return SQLParserComponents.P_Update;
            }
        }

        [TestMethod]
        public void DML_Update()
        {
            Input = new List<Lexeme>() { update, lex, set, lex, eq, Float, end };
            AssertCompleteCorrect();
        }
    }

    [TestClass]
    public class DeleteTest : ParserResultTest<Delete>
    {
        protected override Parser<Delete> MyParser
        {
            get
            {
                return SQLParserComponents.P_Delete;
            }
        }

        [TestMethod]
        public void DML_Delete1()
        {
            Input = new List<Lexeme>() { delete, from, lex, end };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void DML_Delete2()
        {
            Input = new List<Lexeme>() { delete, from, lex, where, lex, eq, lex, and, Int, less, Float, end };
            AssertCompleteCorrect();
        }
    }
}
