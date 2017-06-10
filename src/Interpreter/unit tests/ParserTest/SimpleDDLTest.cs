using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;

namespace ParserTest
{
    [TestClass]
    public class DropTableTest : ParserResultTest<DropTable>
    {
        protected override Parser<DropTable> MyParser
        {
            get 
            {
                return SQLParserComponents.P_DropTable;
            }
        }

        [TestMethod]
        public void DDL_DropTable()
        {
            Input = new List<Lexeme>() { drop, table, lex, end };
            AssertCompleteCorrect();
        }
    }

    [TestClass]
    public class DropIndexTest : ParserResultTest<DropIndex>
    {
        protected override Parser<DropIndex> MyParser
        {
            get
            {
                return SQLParserComponents.P_DropIndex;
            }
        }

        [TestMethod]
        public void DDL_DropIndex()
        {
            Input = new List<Lexeme>() { drop, index, lex, on, lex, end };
            AssertCompleteCorrect();
        }
    }

    [TestClass]
    public class CreateIndexTest : ParserResultTest<CreateIndex>
    {
        protected override Parser<CreateIndex> MyParser
        {
            get
            {
                return SQLParserComponents.P_CreateIndex;
            }
        }

        [TestMethod]
        public void DDL_CreateIndex()
        {
            Input = new List<Lexeme>() { create, index, lex, on, lex, left, lex, right, end };
            AssertCompleteCorrect();
        }
    }
}
