
namespace Compiler.Lexer
{
    public class LexerError : System.Exception
    {
        public int ErrorIndex { get; internal set; }

        public string ErrorType { get; internal set; }

        public string ErrorSegment { get; internal set; }

        internal LexerError(int errorIndex, string errorType, string erorSegment)
            : base(string.Format("Lexer Error: {0} -> Position = {1}, Segment = {2}", errorType, errorIndex, erorSegment))
        {
            ErrorIndex = errorIndex;
            ErrorType = errorType;
            ErrorSegment = erorSegment;
        }
    }
}
