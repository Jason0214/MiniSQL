using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.SQLAnalyzer;
using MiniSQL.Executor.Validator;
using MiniSQL.Errors;

namespace ValidatorTest
{
    [TestClass]
    public class QueryValidatorTest
    {
        private QueryValidator validator;
        private List<SQLError> errors;

        private string query 
        { 
            set
            {
                SQLLexer lexer = new SQLLexer();

                if (lexer.Interpret(value).Count > 0)
                {
                    Assert.Fail("Errors Happen in Lexer");
                }

                SQLParser parser = new SQLParser();

                if (parser.Parse(lexer.Lexemes).Count > 0)
                {
                    Assert.Fail("Errors Happen in Parser");
                }

                validator = new QueryValidator(parser.resultStatement as Query, new SimulationInterface());

                errors = validator.Validate();
            } 
        }

        private void AssertCorrect()
        {
            if (errors.Count > 0)
            {
                Assert.Fail("Error Num: " + errors.Count);
            }
        }

        private void AssertError(params string[] msg)
        {
            string prompt1 = "Error Num = ";
            Assert.AreEqual(prompt1 + msg.Length, prompt1 + errors.Count);

            for (int i = 0; i < msg.Length; i++)
            {
                string prompt2 = "Error " + i + ": ";
                Assert.AreEqual(prompt2 + msg[i], prompt2 + errors[i].Message);
            }
        }

        [TestMethod]
        public void VQ_CorrectSample1()
        {
            query = "select a1, b2, c3 from t1, t2, t3;";
            AssertCorrect();
        }

        [TestMethod]
        public void VQ_CorrectSample2()
        {
            query = "select a1, b2, c3 from t1, t2, t3 where t1.b1 = t2.c2 and 1 = 5;";
            AssertCorrect();
        }

        [TestMethod]
        public void VQ_CorrectSample3()
        {
            query = "select R.a1 as a, S.b2, t3.c3 from t1 as R natural join t2 as S, t3 where R.c1 = c2 and b2 = 'asd';";
            AssertCorrect();
        }

        [TestMethod]
        public void VQ_CorrectSample4()
        {
            query = "select * from t1, t2 where a1 > a2;";
            AssertCorrect();
        }

        [TestMethod]
        public void VQ_DuplicateTable()
        {
            query = "select a1, c3 from t1, t1, t3;";
            AssertError("Validator: Duplicate Table t1");
        }

        [TestMethod]
        public void VQ_UnknownAttr1()
        {
            query = "select a1, b2, c3 from t1, t3;";
            AssertError("Validator: Unexist Attribute: b2");
        }

        [TestMethod]
        public void VQ_UnknownAttr2()
        {
            query = "select a1, t1.c3 from t1, t3;";
            AssertError("Validator: Attribute does not exist: c3");
        }

        [TestMethod]
        public void VQ_UnknownTable1()
        {
            query = "select a1 from t1, t4;";
            AssertError("Validator: Unexist Table: t4");
        }

        [TestMethod]
        public void VQ_UnknownTable2()
        {
            query = "select a1 as P, b2, t2.c2 from t1, t2 as R;";
            AssertError("Validator: Unspecified Table: t2");
        }

        [TestMethod]
        public void VQ_Ambiguity1()
        {
            query = "select a1 from t1, t11;";
            AssertError("Validator: Attribute ambiguity: a1");
        }

        [TestMethod]
        public void VQ_Ambiguity2()
        {
            query = "select t1.a1, t11.a1 from t1, t11;";
            AssertCorrect();
        }

        [TestMethod]
        public void VQ_TypeMismatch1()
        {
            query = "select a1, b2 from t1, t2 where a1 = b2;";
            AssertError("int and string: Type mismatch");
        }

        [TestMethod]
        public void VQ_TypeMismatch2()
        {
            query = "select a1, b2, c3 from t1, t2, t3 where a1 = 6 and b2 = 'asd' and 1 = 'asd' and b2 = c3;";
            AssertError("int and string: Type mismatch");
        }
    }
}
