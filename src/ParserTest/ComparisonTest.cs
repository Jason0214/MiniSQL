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
    public class ComparandTest : ParserResultTest<CompareOperand>
    {
        protected override Parser<CompareOperand> MyParser
        {
            get 
            {
                return SQLParserComponents.P_CompareOperand; 
            }
        }

        [TestMethod]
        public void Comparand_SingleValue()
        {
            Input = new List<Lexeme>() { lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { Float };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { Int };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { str };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void Comparand_TableAttribute()
        {
            Input = new List<Lexeme>() { lex, dot, lex };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void Comparand_NoIdAfterDot()
        {
            Input = new List<Lexeme>() { lex, dot, Int };
            AssertPartialCorrect(1);
        }

        [TestMethod]
        public void Comparand_BeginWithDot()
        {
            Input = new List<Lexeme>() { dot, lex };
            AssertError(0);
        }
    }

    [TestClass]
    public class ComparisonTest : ParserResultTest<Predict>
    {
        protected override Parser<Predict> MyParser
        {
            get
            {
                return SQLParserComponents.P_ComparePredict;
            }
        }

        [TestMethod]
        public void Comparison_Simple()
        {
            Input = new List<Lexeme>() { lex, less, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, lesseq, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, eq, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, greateq, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, great, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, great, Int };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { str, great, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { Int, great, Float };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { Float, great, str };
            AssertCompleteCorrect();
        }

        [TestMethod]
        public void Comparison_TableAttribute()
        {
            Input = new List<Lexeme>() { lex, dot, lex, less, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, dot, lex, less, lex, dot, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, less, lex, dot, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, dot, less, lex };
            AssertError(1);

            Input = new List<Lexeme>() { lex, dot, Int, less, lex };
            AssertError(1);

            Input = new List<Lexeme>() { Int, less, eq };
            AssertError(2);

            Input = new List<Lexeme>() { Int, less, lex, dot };
            AssertPartialCorrect(3);
        }

        [TestMethod]
        public void Comparison_BetweenAnd()
        {
            Input = new List<Lexeme>() { lex, between, lex, and, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, dot, lex, between, Int, and, Float };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, dot, lex, between, lex, dot, lex, and, lex, dot, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, dot, lex, between, lex, and };
            AssertError(5);

            Input = new List<Lexeme>() { lex, dot, lex, between, and };
            AssertError(4);

            Input = new List<Lexeme>() { lex, dot, lex, between };
            AssertError(3);
        }

        [TestMethod]
        public void Comparison_Brackets()
        {
            Input = new List<Lexeme>() { left, left, lex, dot, lex, right, right, less, left, lex, dot, lex, right };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { left, left, lex, dot, lex, right, right, between, left, lex, dot, lex, right, 
                and, left, lex, dot, lex, right };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { left, lex, dot, lex, less, lex };
            AssertError(4);
        }
    }
}
