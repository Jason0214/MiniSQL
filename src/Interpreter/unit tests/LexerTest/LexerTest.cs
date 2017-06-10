using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Lexer;

namespace LexerTest
{
    public class LexerTest
    {
        protected void LexerTestEngine(Lexer lexer, string input,
            List<string> expectedTypes, List<string> expectedContents, List<string> expectedErrors)
        {
            var errors = lexer.Interpret(input);
            var result = lexer.GetResultTokens();

            AssertResultBeCorrect(result, expectedTypes, expectedContents);
            AssertErrorsBeExpected(errors, expectedErrors);
        }

        protected void AssertResultBeCorrect(List<Token> result, List<string> expectedTypes, List<string> expectedContents)
        {
            Assert.AreEqual("word_count:" + expectedTypes.Count, "word_count:" + result.Count);
            Assert.AreEqual("word_count:" + expectedContents.Count, "word_count:" + result.Count);

            for (int i = 0; i < result.Count; i++)
            {
                Assert.AreEqual("type:" + i + " " + expectedTypes[i], "type:" + i + " " + result[i].TypeName);
                Assert.AreEqual("str:" + i + " " + expectedContents[i], "str:" + i + " " + result[i].Content);
            }
        }

        protected void AssertErrorsBeExpected(List<LexerError> errors, List<string> expectedErrorSegments)
        {
            Assert.AreEqual("err_count:" + expectedErrorSegments.Count, "err_count:" + errors.Count);

            for (int i = 0; i < errors.Count; i++)
            {
                Assert.AreEqual("err:" + i + " " + expectedErrorSegments[i], "err:" + i + " " + errors[i].ErrorSegment);
            }
        }
    }

    [TestClass]
    public class SQLSyntaxTest : LexerTest
    {
        [TestMethod]
        public void Lexer_Sample()
        {
            List<string> expectedTypes = new List<string>() { "keyword", "keyword", "id", "symbol", "id", 
                "symbol", "int", "symbol", "keyword", "symbol",
                "id", "symbol", "keyword", "id", "keyword", "id", "symbol", "id", "keyword", "id", "keyword", 
                "float", "keyword", "float", "keyword", "float", "id", "symbol", "id",
                "symbol", "id", "symbol", "id", "keyword","id", "symbol", "id", "symbol", "single", "symbol"};
            List<string> expectedContents = new List<string>() { "select", "distinct", "T", ".", "a1", "*", "6", ",", 
                "avg", "(", "s", ")", 
                "from", "ins", "as", "T", ",", "ins", "as", "S", "where",
                "1.2", "between", ".13", "and", "2.", "T", ".", "sa",
                ">", "S", ".", "sa", "and","S", ".", "dpt", "=", @"'bio'", ";"};
            List<string> expectedErrors = new List<string>();

            SQLTest(@"select distinct T.a1*6, avg(s) 
                      from ins as T, ins as S
                      where 1.2 between.13and 2.T.sa >S.sa and S.dpt='bio';",
                expectedTypes, expectedContents, expectedErrors);
        }

        [TestMethod]
        public void Lexer_IllegalSymbols()
        {
            List<string> expectedTypes = new List<string>() { "keyword", "id", "single"};
            List<string> expectedContents = new List<string>() { "select", "dog", "\'cat\'" };
            List<string> expectedErrors = new List<string>() { "\"", "\"", "$", "$" };

            SQLTest("select \"dog\"\'cat\'$$", expectedTypes, expectedContents, expectedErrors);
        }

        [TestMethod]
        public void Lexer_Numbers()
        {
            List<string> expectedTypes = new List<string>() { "float", "id", "float", "float", "symbol", 
                "float","id", "float","id", "float", "id", "int", "float", "float" };
            List<string> expectedContents = new List<string>() { "0.", "a99", ".66", ".55", ".", "44.", 
                "a33", "22.", "b11", "2.0", "a", "11", "1.0", ".6"};
            List<string> expectedErrors = new List<string>();

            SQLTest("0.a99.66.55. 44.a33 22.b11 2.0a 11 1.0.6", expectedTypes, expectedContents, expectedErrors);
        }

        [TestMethod]
        public void Lexer_NotMatchedQuotes()
        {
            List<string> expectedTypes = new List<string>() { "single", "id", "single", "id" };
            List<string> expectedContents = new List<string>() { "\'a\'", "a", "\'\'", "a" };
            List<string> expectedErrors = new List<string>() { "\'123.4bbb+=<=*/!@?" };

            SQLTest("\'a\'a\'\'a\'123.4bbb+=<=*/!@?", expectedTypes, expectedContents, expectedErrors);
        }

        [TestMethod]
        public void Lexer_EmbeddedKeywords()
        {
            List<string> expectedTypes = new List<string>() { "id", "int", "keyword", "keyword", "symbol", "id", 
                "symbol", "keyword", "symbol", "id", "float", "keyword", "single" };
            List<string> expectedContents = new List<string>() { "select1", "2", "select", "select", "*", "selectand", 
                "<=", "and", ">=", "and1", ".0", "and", "\'and\'" };
            List<string> expectedErrors = new List<string>();

            SQLTest("select1 2select select* selectand<=and>=and1.0and\'and\'", expectedTypes, expectedContents, expectedErrors);
        }

        [TestMethod]
        public void Lexer_OverlappedSymbols()
        {
            List<string> expectedTypes = new List<string>() { "symbol", "symbol", "symbol", "symbol", "symbol", "symbol", "symbol" };
            List<string> expectedContents = new List<string>() { "<", "<", "<=", "=", ">", ">", ">=" };
            List<string> expectedErrors = new List<string>();

            SQLTest("<<<==>>>=", expectedTypes, expectedContents, expectedErrors);
        }

        [TestMethod]
        public void Lexer_EmptyString()
        {
            List<string> expectedTypes = new List<string>();
            List<string> expectedContents = new List<string>();
            List<string> expectedErrors = new List<string>();

            SQLTest("", expectedTypes, expectedContents, expectedErrors);
        }

        [TestMethod]
        public void Lexer_Chinese()
        {
            List<string> expectedTypes = new List<string>() { "keyword", "id", "keyword", "id", "single" };
            List<string> expectedContents = new List<string>() { "select", "哈哈", "from", "哇where好", "\'O(∩_∩)O哈哈~\'" };
            List<string> expectedErrors = new List<string>();

            SQLTest("select 哈哈 from 哇where好\'O(∩_∩)O哈哈~\'", expectedTypes, expectedContents, expectedErrors);
        }

        private void SQLTest(string input,
            List<string> expectedTypes, List<string> expectedContents, List<string> expectedErrorSegs)
        {
            List<string> symbols = new List<string>() { "+", "-", "*", "/", ".", ">", ">=", "=", "<", "<=", ",", ";", "(", ")" };
            List<string> keywords = new List<string>() { "select", "where", "from", "group by", "having", "order by", 
                "in", "not in", "and", "or", "between", "union", "intersect", "except", "as", "all", "distinct", "join", 
                "natural join", "using", "is null", "is not null", "avg", "count", "sum", "is not null", "exists", 
                "not exists", "unique", "not unique", "delete", "insert into", "values", "update", "set", "char", "varchar", 
                "int", "numeric", "create", "table", "index", "primary key", "foreign key", "not null", "check" };

            TokenTypeCollection.DoubleQuote.Enabled = false;
            Lexer lexer = new Lexer(symbols, keywords);

            LexerTestEngine(lexer, input, expectedTypes, expectedContents, expectedErrorSegs);
        }
    }
}
