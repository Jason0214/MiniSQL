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
    public class SelectClauseTest : ParserResultTest<SelectClause>
    {
        protected override Parser<SelectClause> MyParser
        {
            get 
            { 
                return SQLParserComponents.P_SelectClause; 
            }
        }

        private void ListCountEqualsTo(int i)
        {
            string prompt = "List count equals to ";
            Assert.AreEqual(prompt + i, prompt + Result.Value.AttributesList.Count);
        }

        [TestMethod]
        public void Select_AliasList()
        {
            Input = new List<Lexeme>() { select, lex, As, lex, comma, lex, As, lex };
            AssertCompleteCorrect();
            ListCountEqualsTo(2);
        }

        [TestMethod]
        public void Select_MixedList()
        {
            Input = new List<Lexeme>() { select, lex, As, lex, comma, lex, comma, lex, As, lex };
            AssertCompleteCorrect();
            ListCountEqualsTo(3);
        }

        [TestMethod]
        public void Select_EndWithComma()
        {
            Input = new List<Lexeme>() { select, lex, As, lex, comma };
            AssertPartialCorrect(4);
            ListCountEqualsTo(1);
        }

        [TestMethod]
        public void Select_EndWithAs()
        {
            Input = new List<Lexeme>() { select, lex, As };
            AssertPartialCorrect(2);
            ListCountEqualsTo(1);
        }

        [TestMethod]
        public void Select_Brackets()
        {
            Input = new List<Lexeme>() { select, left, left, lex, right, right, As, lex, comma, left, lex, right };
            AssertCompleteCorrect();
            ListCountEqualsTo(2);
        }

        [TestMethod]
        public void Select_NoAliasBrackets()
        {
            Input = new List<Lexeme>() { select, left, lex, As, lex, right };
            AssertError(3);
        }

        [TestMethod]
        public void Select_Empty()
        {
            Input = new List<Lexeme>() { select };
            AssertError(0);
        }
    }
}
