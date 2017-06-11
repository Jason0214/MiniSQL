using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer.ParserComponents;

namespace ParserTest
{
    [TestClass]
    public partial class ExpressionTest : ParserResultTest<Expression>
    {
        protected override Parser<Expression> MyParser
        {
            get
            {
                return SQLParserComponents.P_Expression;
            }
        }

        private Type Cmp = typeof(CompareExpression);
        private Type Conj = typeof(ConjunctionExpression);
        private Type Disj = typeof(DisjunctionExpression);

        private int Checking;
        private int Length;
        private object[] Elements;

        private void CheckPredict(params object[] type_or_nSubPredict)
        {
            Checking = 0;
            Length = type_or_nSubPredict.Length;
            Elements = type_or_nSubPredict;

            CheckPredict(Result.Value, "Root->");
            RunOutOfList();
        }

        private void CheckPredict(Expression p, string prompt)
        {
            string prompt1 = prompt + "Predict type = ";
            string prompt2 = prompt + "Sub Predict Num = ";

            Type T = GetTypeNameElement();
            Assert.AreEqual(prompt1 + T, prompt1 + p.GetType());

            if (T == typeof(ConjunctionExpression))
            {
                ConjunctionExpression conjP = p as ConjunctionExpression;
                Assert.AreEqual(prompt2 + GetIntElement(), prompt2 + conjP.ExpressionList.Count);
                for (int i = 0; i < conjP.ExpressionList.Count; i++)
                {
                    CheckPredict(conjP.ExpressionList[i], prompt + "Conj[" + i + "]->");
                }
            }
            else if (T == typeof(DisjunctionExpression))
            {
                DisjunctionExpression disjP = p as DisjunctionExpression;
                Assert.AreEqual(prompt2 + GetIntElement(), prompt2 + disjP.ExpressionList.Count);
                for (int i = 0; i < disjP.ExpressionList.Count; i++)
                {
                    CheckPredict(disjP.ExpressionList[i], prompt + "Disj[" + i + "]->");
                }
            }
        }

        private void RunOutOfList()
        {
            string prompt1 = "Run out of List";
            string prompt2 = "Element Left";

            Assert.AreEqual(prompt1, Checking == Length ? prompt1 : prompt2);
        }

        private void LegalIndexAssert()
        {
            string prompt1 = "Element Enough";
            string prompt2 = "Element not Enough";

            Assert.AreEqual(prompt1, Checking < Length ? prompt1 : prompt2);
        }

        private Type GetTypeNameElement()
        {
            LegalIndexAssert();
            object element = Elements[Checking++];

            if (element.GetType() == typeof(int))
            {
                Assert.AreEqual("Want a Type element", "Get a element " + element.GetType());
            }

            return element as Type;
        }

        private int GetIntElement()
        {
            LegalIndexAssert();
            object element = Elements[Checking++];

            if (element.GetType() != typeof(int))
            {
                Assert.AreEqual("Want a int element", "Get a element" + element.GetType());
            }

            return (int)element;
        }
    }

    public partial class ExpressionTest : ParserResultTest<Expression>
    {
        [TestMethod]
        public void Predict_BinaryConjunction()
        {
            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq, Int };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 2, Cmp, Cmp);
        }

        [TestMethod]
        public void Predict_MultipleConjunction()
        {
            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq, Int, and, Float, great, str };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 3, Cmp, Cmp, Cmp);
        }

        [TestMethod]
        public void Predict_BinaryDisjunction()
        {
            Input = new List<Lexeme>() { lex, less, lex, or, lex, eq, Int };
            AssertCompleteCorrect();
            CheckPredict(Disj, 2, Conj, 1, Cmp, Conj, 1, Cmp);
        }

        [TestMethod]
        public void Predict_MultipleDisjunction()
        {
            Input = new List<Lexeme>() { lex, less, lex, or, lex, eq, Int, or, Float, great, str };
            AssertCompleteCorrect();
            CheckPredict(Disj, 3, Conj, 1, Cmp, Conj, 1, Cmp, Conj, 1, Cmp);
        }

        [TestMethod]
        public void Predict_BetweenAnd()
        {
            Input = new List<Lexeme>() { lex, between, lex, and, lex };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 1, Conj, 2, Cmp, Cmp);
        }

        [TestMethod]
        public void Predict_BetweenAndConj()
        {
            Input = new List<Lexeme>() { lex, between, lex, and, lex, and, lex, between, lex, and, lex };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 2, Conj, 2, Cmp, Cmp, Conj, 2, Cmp, Cmp);
        }

        [TestMethod]
        public void Predict_MixConjDisj()
        {
            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq, Int, or, Float, great, str, and, lex, eq, Float };
            AssertCompleteCorrect();
            CheckPredict(Disj, 2, Conj, 2, Cmp, Cmp, Conj, 2, Cmp, Cmp);

            Input = new List<Lexeme>() { lex, less, lex, and, lex, between, Int, and, lex, or, Float, great, str, and, lex, eq, Float };
            AssertCompleteCorrect();
            CheckPredict(Disj, 2, Conj, 2, Cmp, Conj, 2, Cmp, Cmp, Conj, 2, Cmp, Cmp);
        }

        [TestMethod]
        public void Predict_Segment()
        {
            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq, Int, or, Float, great, str, and, lex };
            AssertPartialCorrect(11);
            CheckPredict(Disj, 2, Conj, 2, Cmp, Cmp, Conj, 1, Cmp);

            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq, Int, or, Float, great, str, and };
            AssertPartialCorrect(11);
            CheckPredict(Disj, 2, Conj, 2, Cmp, Cmp, Conj, 1, Cmp);
            
            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq, Int, or, Float };
            AssertPartialCorrect(7);
            CheckPredict(Disj, 1, Conj, 2, Cmp, Cmp);

            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq };
            AssertPartialCorrect(3);
            CheckPredict(Disj, 1, Conj, 1, Cmp);

            Input = new List<Lexeme>() { lex, less, lex };
            AssertCompleteCorrect();

            Input = new List<Lexeme>() { lex, less };
            AssertError(1);

            Input = new List<Lexeme>() { lex, between, lex, and };
            AssertError(3);
        }

        [TestMethod]
        public void Predict_OrBrackets1()
        {
            // cmp and cmp and (cmp or cmp)
            Input = new List<Lexeme>() { lex, less, lex, and, lex, eq, lex, and, left, lex, less, lex, or, lex, eq, lex, right };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 3, Cmp, Cmp, Disj, 2, Conj, 1, Cmp, Conj, 1, Cmp);
        }

        [TestMethod]
        public void Predict_OrBrackets2()
        {
            // (cmp or cmp) and (cmp or cmp)
            Input = new List<Lexeme>() { left, lex, less, lex, or, lex, eq, lex, right, and, 
                left, lex, less, lex, or, lex, eq, lex, right };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 2, Disj, 2, Conj, 1, Cmp, Conj, 1, Cmp, Disj, 2, Conj, 1, Cmp, Conj, 1, Cmp);
        }

        [TestMethod]
        public void Predict_OrBrackets3()
        {
            // cmp or cmp and (cmp or cmp)
            Input = new List<Lexeme>() { lex, less, lex, or, lex, eq, lex, and, left, lex, less, lex, or, lex, eq, lex, right };
            AssertCompleteCorrect();
            CheckPredict(Disj, 2, Conj, 1, Cmp, Conj, 2, Cmp, Disj, 2, Conj, 1, Cmp, Conj, 1, Cmp);
        }

        [TestMethod]
        public void Predict_OrBrackets4()
        {
            // (cmp or (cmp or (cmp or cmp)))
            Input = new List<Lexeme>() { left, lex, less, lex, or, 
                left, lex, eq, lex, or, left, lex, less, lex, or, lex, eq, lex, right, right, right };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 1, Disj, 2, Conj, 1, Cmp,
                Conj, 1, Disj, 2, Conj, 1, Cmp,
                Conj, 1, Disj, 2, Conj, 1, Cmp, Conj, 1, Cmp);
        }

        [TestMethod]
        public void Predict_BracketsForLogicAndAttr()
        {
            // (cmp or (bet or (cmp or cmp)))
            Input = new List<Lexeme>() { left, lex, less, lex, or, 
                left, lex, between, left, lex, dot, lex, right, and, left, left, lex, right, right, or,
                left, left, lex, dot, lex, right, less, lex, or, lex, eq, lex, right, right, right };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 1, Disj, 2, Conj, 1, Cmp,
                Conj, 1, Disj, 2, Conj, 1, Conj, 2, Cmp, Cmp,
                Conj, 1, Disj, 2, Conj, 1, Cmp, Conj, 1, Cmp);
        }

        [TestMethod]
        public void Predict_SelfBrackets()
        {
            // ((cmp))
            Input = new List<Lexeme>() { left, left, lex, less, lex, right, right };
            AssertCompleteCorrect();
            CheckPredict(Disj, 1, Conj, 1, Disj, 1, Conj, 1, Disj, 1, Conj, 1, Cmp);
        }
    }
}
