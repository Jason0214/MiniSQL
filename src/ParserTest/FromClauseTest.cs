using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;

namespace ParserTest
{
    [TestClass]
    public class FromClauseTest : ParserResultTest<FromClause>
    {
        protected override Parser<FromClause> MyParser
        {
            get
            {
                return SQLParserComponents.P_FromClause;
            }
        }

        private void ListCountEqualsTo(int i)
        {
            string prompt = "List count equals to ";
            Assert.AreEqual(prompt + i, prompt + Result.Value.TableList.Count);
        }

        private void TableCountEqualsTo(int i, int count)
        {
            string prompt = string.Format("Table[{0}] count equals to ", i);
            Assert.AreEqual(prompt + count, prompt + Result.Value.TableList[i].JoinedTablesList.Count);
        }

        private void TableListCountVerify(int nTable, params int[] Count)
        {
            ListCountEqualsTo(nTable);
            for (int i = 0; i < Count.Length; i++)
            {
                TableCountEqualsTo(i, Count[i]);
            }
        }

        [TestMethod]
        public void From_Single()
        {
            Input = new List<Lexeme>() { from, lex };
            AssertCompleteCorrect();
            TableListCountVerify(1, 1);
        }

        [TestMethod]
        public void From_MultipleSingle()
        {
            Input = new List<Lexeme>() { from, lex, comma, lex, comma, lex };
            AssertCompleteCorrect();
            TableListCountVerify(3, 1, 1, 1);
        }

        [TestMethod]
        public void From_SingleJoin()
        {
            Input = new List<Lexeme>() { from, lex, join, lex, join, lex };
            AssertCompleteCorrect();
            TableListCountVerify(1, 3);
        }

        [TestMethod]
        public void From_MultipleJoin()
        {
            Input = new List<Lexeme>() { from, lex, join, lex, comma, lex, join, lex };
            AssertCompleteCorrect();
            TableListCountVerify(2, 2, 2);
        }

        [TestMethod]
        public void From_Mixed()
        {
            Input = new List<Lexeme>() { from, lex, join, lex, comma, lex, comma, lex, comma, lex, join, lex };
            AssertCompleteCorrect();
            TableListCountVerify(4, 2, 1, 1, 2);
        }

        [TestMethod]
        public void From_ComplexBrackets()
        {
            // from (((a) join b)), c
            Input = new List<Lexeme>() { from, left, left, left, lex, right, join,
                    lex, right, right, comma, lex };
            AssertCompleteCorrect();
            TableListCountVerify(2, 2, 1);
        }

        [TestMethod]
        public void From_JoinSegment()
        {
            Input = new List<Lexeme>() { from, lex, join, comma, lex, comma, lex, comma, lex, join, lex };
            AssertPartialCorrect(2);
            TableListCountVerify(1, 1);
        }

        [TestMethod]
        public void From_BracketsMismatch()
        {
            Input = new List<Lexeme>() { from, left, lex, join, left, lex, right };
            AssertError(6);
        }

        [TestMethod]
        public void From_Empty()
        {
            Input = new List<Lexeme>() { from };
            AssertError(0);
        }

        [TestMethod]
        public void From_Alias()
        {
            Input = new List<Lexeme>() { from, lex, As, lex };
            AssertCompleteCorrect();
            TableListCountVerify(1, 1);
        }

        [TestMethod]
        public void From_AliasJoin()
        {
            Input = new List<Lexeme>() { from, lex, As, lex, join, lex, As, lex };
            AssertCompleteCorrect();
            TableListCountVerify(1, 2);
        }

        [TestMethod]
        public void From_AliasJoinBrackets()
        {
            // from t1 as a1 join (t2 as a2), ((t3 as a3 join t4))
            Input = new List<Lexeme>() {from, lex, As, lex, join, left,
                    lex, As, lex, right, comma, left, left, lex, As, lex, join, lex, right, right };
            AssertCompleteCorrect();
            TableListCountVerify(2, 2, 2);
        }

        [TestMethod]
        public void From_EndWithAs()
        {
            Input = new List<Lexeme>() { from, lex, As };
            AssertPartialCorrect(2);
            TableListCountVerify(1, 1);
        }
    }
}
