using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Compiler.Lexer;
using Compiler.Parser;

namespace ParserTest
{
    [TestClass]
    public class TokenScannerTest
    {
        [TestMethod]
        public void Scanner_ReadAll()
        {
            CopiableTokenScanner scanner = GetScanner();
            int num = 0;

            while (scanner.Read() != null)
            {
                num++;
            }

            Assert.AreEqual("total tokens: 5", "total tokens: " + num);
        }

        [TestMethod]
        public void Scanner_CopyScanner()
        {
            CopiableTokenScanner scanner1 = GetScanner();
            CopiableTokenScanner scanner2 = scanner1;

            int num1 = 0;
            int num2 = 0;

            scanner1.Read();
            scanner1.Read();

            while (scanner1.Read() != null)
            {
                num1++;
            }

            while (scanner2.Read() != null)
            {
                num2++;
            }

            Assert.AreEqual("scanner1 left tokens: 3", "scanner1 left tokens: " + num1);
            Assert.AreEqual("scanner2 left tokens: 5", "scanner2 left tokens: " + num2);
        }

        [TestMethod]
        public void Scanner_EmptyList()
        {
            CopiableTokenScanner scanner = new CopiableTokenScanner(new List<Lexeme>());
            Assert.AreEqual(null, scanner.Read());
        }

        private CopiableTokenScanner GetScanner()
        {
            Lexer lexer = new Lexer(new string[3] { "+", "<", "<=" }, new string[3] { "aa", "bb", "cc" });
            lexer.Interpret("aa <= bb + cc");
            return new CopiableTokenScanner((from token in lexer.GetResultTokens() select new Lexeme(token)).ToList());
        }
    }
}
