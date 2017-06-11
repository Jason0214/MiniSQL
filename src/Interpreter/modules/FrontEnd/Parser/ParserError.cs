
namespace Compiler.Parser
{
    public class ParserError : System.Exception
    {
        public int ErrorIndex { get; internal set; }

        public Lexeme ErrorLexeme { get; internal set; }

        public Lexeme ExpectedLexeme { get; internal set; }

        internal ParserError(int errorIndex, Lexeme errorLexeme, Lexeme expectedLexeme)
            : base(string.Format("Parser Error: {0}", errorLexeme.Content))
        {
            ErrorIndex = errorIndex;
            ErrorLexeme = errorLexeme;
            ExpectedLexeme = expectedLexeme;
        }
    }
}
