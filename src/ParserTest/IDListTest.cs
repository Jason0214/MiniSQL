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
    public class IDListTest : ParserResultTest<ListNode<string>>
    {
        protected override Parser<ListNode<string>> MyParser
        {
            get
            {
                return SQLParserComponents.P_List(
                    SQLParserComponents.P_Id.CanBeParenthesized(), SQLParserComponents.P_Comma, lexeme => lexeme.Content);
            }
        }

        [TestMethod]
        public void IDList_Empty()
        {
            Input = new List<Lexeme>();
            AssertError(0);
        }

        [TestMethod]
        public void IDList_OnlyComma()
        {
            Input = new List<Lexeme>() { comma };
            AssertError(0);
        }

        [TestMethod]
        public void IDList_TwoIds()
        {
            Input = new List<Lexeme>() { lex, lex };
            AssertPartialCorrect(1);
        }

        [TestMethod]
        public void IDList_EndWithTwoIds()
        {
            Input = new List<Lexeme>() { lex, comma, lex, comma, lex, lex };
            AssertPartialCorrect(5);
        }

        [TestMethod]
        public void IDList_DifferentLengths()
        {
            LexemeListTest(1);
            LexemeListTest(2);
            LexemeListTest(3);
            LexemeListTest(4);
            LexemeListTest(10);
            LexemeListTest(11);
            LexemeListTest(101);
        }

        [TestMethod]
        public void IDList_CommaEnd()
        {
            Input = new List<Lexeme>() { lex, comma, lex, comma };
            AssertPartialCorrect(3);
        }

        [TestMethod]
        public void IDList_SingleWithBrackets()
        {
            Input = new List<Lexeme>() { left, lex, right };
            AssertCompleteCorrect();
            ListCountEqualsTo(1);
        }

        [TestMethod]
        public void IDList_TwoLevelBrackets()
        {
            Input = new List<Lexeme>() { left, left, lex, right, right };
            AssertCompleteCorrect();
            ListCountEqualsTo(1);
        }

        [TestMethod]
        public void IDList_ComplexBrackets()
        {
            Input = new List<Lexeme>() { left, lex, right, comma, left, left, lex, right, right, comma, lex };
            AssertCompleteCorrect();
            ListCountEqualsTo(3);
        }

        private void ListCountEqualsTo(int i)
        {
            string prompt = "List count equals to ";
            Assert.AreEqual(prompt + i, prompt + Result.Value.ToList().Count);
        }

        [TestMethod]
        public void IDList_BracketsMismatch_1()
        {
            Input = new List<Lexeme>() { left, lex };
            AssertError(1);
        }

        [TestMethod]
        public void IDList_BracketsMismatch_2()
        {
            Input = new List<Lexeme>() { left, lex, right, comma, left, left, lex, right };
            AssertPartialCorrect(3);
        }

        [TestMethod]
        public void IDList_BracketsMismatch_3()
        {
            Input = new List<Lexeme>() { left, lex, comma, right };
            AssertError(2);
        }

        private void LexemeListTest(int num)
        {
            List<Lexeme> list = new List<Lexeme>();

            for (int i = 0; i < num; i++)
            {
                list.Add(lex);
                if (i != num - 1)
                {
                    list.Add(comma);
                }
            }

            Input = list;

            AssertCompleteCorrect();

            List<string> ids = Result.Value.ToList();
            Assert.AreNotEqual(null, ids);
            Assert.AreEqual(num, ids.Count);
        }
    }
}
