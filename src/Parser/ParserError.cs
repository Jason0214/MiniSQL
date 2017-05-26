
namespace Compiler.Parser
{
    public class ParserError : System.Exception
    {
        public int ErrorIndex { get; internal set; }

        public Lexeme ErrorLexeme { get; internal set; }

        public Lexeme ExpectedLexeme { get; internal set; }

        internal ParserError(int errorIndex, Lexeme errorLexeme, Lexeme expectedLexeme)
            : base(string.Format("Parser Error: Error = {0}, Expected = {1}", errorLexeme.TypeName, expectedLexeme.TypeName))
        {
            ErrorIndex = errorIndex;
            ErrorLexeme = errorLexeme;
            ExpectedLexeme = expectedLexeme;
        }
    }
}
