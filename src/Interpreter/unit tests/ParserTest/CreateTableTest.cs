using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;

namespace ParserTest
{
    [TestClass]
    public class AttrTypeTest : ParserResultTest<AttrType>
    {
        protected override Parser<AttrType> MyParser
        {
            get
            {
                return SQLParserComponents.P_AttrType;
            }
        }

        [TestMethod]
        public void DDL_AttrTypes()
        {
            Input = new List<Lexeme>() { _int };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { _float };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { Char, left, Int, right };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { vchar, left, Int, right };
            AssertCompleteCorrect();
        }
    }

    [TestClass]
    public class AttrDefinitionTest : ParserResultTest<AttrDefinition>
    {
        protected override Parser<AttrDefinition> MyParser
        {
            get
            {
                return SQLParserComponents.P_AttrDefinition;
            }
        }

        [TestMethod]
        public void DDL_AttrDefinition1()
        {
            Input = new List<Lexeme>() { lex, Char, left, Int, right };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void DDL_AttrDefinition2()
        {
            Input = new List<Lexeme>() { lex, Char, left, Int, right, notnull, pkey, pkey, notnull, unique };
            AssertCompleteCorrect();
        }
    }

    [TestClass]
    public class CreateTableTest : ParserResultTest<CreateTable>
    {
        protected override Parser<CreateTable> MyParser
        {
            get
            {
                return SQLParserComponents.P_CreateTable;
            }
        }

        [TestMethod]
        public void DDL_CreateTable1()
        {
            Input = new List<Lexeme>() { create, table, lex, left, lex, _int, right, end };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void DDL_CreateTable2()
        {
            Input = new List<Lexeme>() { create, table, lex, left, lex, _int, comma, lex, Char, left, Int, right, right, end };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void DDL_CreateTable3()
        {
            Input = new List<Lexeme>() { create, table, lex, left, lex, _int, pkey, right, end };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void DDL_CreateTable4()
        {
            Input = new List<Lexeme>() { create, table, lex, left, lex, _int, pkey, comma, lex, _int, notnull, unique, right, end };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void DDL_CreateTable5()
        {
            Input = new List<Lexeme>() { create, table, lex, left, lex, _int, pkey, comma, lex, _int, notnull, unique, comma, 
                pkey, left, lex, comma, lex, right, right, end };
            AssertCompleteCorrect();
        }
    }
}
