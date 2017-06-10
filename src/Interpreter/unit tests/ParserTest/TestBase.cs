using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Parser;

namespace ParserTest
{
    public abstract class ParserResultTest<T>
    {
        protected static Lexeme lex = new Lexeme("id", "idtest");
        protected static Lexeme comma = new Lexeme("symbol", ",");
        protected static Lexeme left = new Lexeme("symbol", "(");
        protected static Lexeme right = new Lexeme("symbol", ")");
        protected static Lexeme As = new Lexeme("keyword", "as");
        protected static Lexeme and = new Lexeme("keyword", "and");
        protected static Lexeme or = new Lexeme("keyword", "or");
        protected static Lexeme between = new Lexeme("keyword", "between");
        protected static Lexeme join = new Lexeme("keyword", "natural join");
        protected static Lexeme from = new Lexeme("keyword", "from");
        protected static Lexeme select = new Lexeme("keyword", "select");
        protected static Lexeme where = new Lexeme("keyword", "where");
        protected static Lexeme Int = new Lexeme("int", "0");
        protected static Lexeme Float = new Lexeme("float", "1.0");
        protected static Lexeme str = new Lexeme("single", "\'str\'");
        protected static Lexeme dot = new Lexeme("symbol", ".");
        protected static Lexeme less = new Lexeme("symbol", "<");
        protected static Lexeme lesseq = new Lexeme("symbol", "<=");
        protected static Lexeme eq = new Lexeme("symbol", "=");
        protected static Lexeme great = new Lexeme("symbol", ">");
        protected static Lexeme greateq = new Lexeme("symbol", ">=");
        protected static Lexeme star = new Lexeme("symbol", "*");
        protected static Lexeme end = new Lexeme("symbol", ";");
        protected static Lexeme create = new Lexeme("keyword", "create");
        protected static Lexeme drop = new Lexeme("keyword", "drop");
        protected static Lexeme table = new Lexeme("keyword", "table");
        protected static Lexeme index = new Lexeme("keyword", "index");
        protected static Lexeme on = new Lexeme("keyword", "on");
        protected static Lexeme insertinto = new Lexeme("keyword", "insert into");
        protected static Lexeme set = new Lexeme("keyword", "set");
        protected static Lexeme update = new Lexeme("keyword", "update");
        protected static Lexeme values = new Lexeme("keyword", "values");
        protected static Lexeme delete = new Lexeme("keyword", "delete");
        protected static Lexeme Char = new Lexeme("keyword", "char");
        protected static Lexeme vchar = new Lexeme("keyword", "varchar");
        protected static Lexeme _int = new Lexeme("keyword", "int");
        protected static Lexeme _float = new Lexeme("keyword", "float");
        protected static Lexeme notnull = new Lexeme("keyword", "not null");
        protected static Lexeme pkey = new Lexeme("keyword", "primary key");
        protected static Lexeme unique = new Lexeme("keyword", "unique");

        private List<Lexeme> _Input;

        protected abstract Parser<T> MyParser { get; }

        protected List<Lexeme> Input
        {
            get
            {
                return _Input;
            }

            set
            {
                _Input = value; Result = MyParser.ParserFunction(new CopiableTokenScanner(value));
            }
        }

        protected Result<T> Result { get; private set; }

        protected void AssertPartialCorrect(int exp_CurReadIndex)
        {
            ResultNotNull();
            NoErrorOccurs();
            ValueNotNull();
            ReadIndexEqualsTo(exp_CurReadIndex);
        }

        protected void AssertCompleteCorrect()
        {
            AssertPartialCorrect(Input.Count);
            RunOutOfList();
        }

        protected void AssertError(int errIndex)
        {
            ResultNotNull();
            ValueIsNull();
            ErrorOccurs();
            ErrorIndexEqualsTo(errIndex);
        }

        private void ResultNotNull()
        {
            Assert.AreEqual("result is not null", Result == null ? "result is null" : "result is not null");
        }

        private void ValueNotNull()
        {
            Assert.AreEqual("value is not null", Result == null ? "value is null" : "value is not null");
        }

        private void ValueIsNull()
        {
            Assert.AreEqual("value is null", Result.Value == null ? "value is null" : "value is not null");
        }

        private void ErrorOccurs()
        {
            Assert.AreEqual("Error occurs", Result.ErrorOccurs ? "Error occurs" : "No error occurs");
        }

        private void NoErrorOccurs()
        {
            Assert.AreEqual("No error occurs", Result.ErrorOccurs ? "Error occurs" : "No error occurs");
        }

        private void RunOutOfList()
        {
            Assert.AreEqual("Run out of List", Result.ReturnedScanner.EndOfList ? "Run out of List" : "Elements left");
        }

        private void ReadIndexEqualsTo(int i)
        {
            string prompt = "Read index equals to ";
            Assert.AreEqual(prompt + i, prompt + Result.ReturnedScanner.CurrentIndex);
        }

        private void ErrorIndexEqualsTo(int i)
        {
            string prompt = "Error index equals to ";
            Assert.AreEqual(prompt + i, prompt + Result.Error.ErrorIndex);
        }
    }
}
