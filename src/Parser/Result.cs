
namespace Compiler.Parser
{
    public class Result<T>
    {
        public T Value { get; private set; }

        public bool ErrorOccurs { get; private set; }

        public ParserError Error { get; private set; }

        public CopiableTokenScanner ReturnedScanner { get; private set; }

        public Result(T value, CopiableTokenScanner returnedScanner)
        {
            ErrorOccurs = false;
            Value = value;
            ReturnedScanner = returnedScanner;
        }

        public Result(int errorIndex, Lexeme expected, CopiableTokenScanner returnedScanner)
        {
            ErrorOccurs = true;
            ReturnedScanner = returnedScanner;

            if (errorIndex >= returnedScanner.ElementNum)
            {
                Error = new ParserError(errorIndex > 0 ? errorIndex - 1 : 0, new Lexeme("EndOfStream", ""), expected);
            }
            else if (errorIndex >= 0)
            {
                Error = new ParserError(errorIndex, returnedScanner.Get(errorIndex), expected);
            }
            else
            {
                Error = new ParserError(0, new Lexeme("unknown", ""), expected);
            }
        }

        private Result()
        {
        }

        public Result<TResult> ErrorCopy<TResult>()
        {
            Result<TResult> result = new Result<TResult>();
            result.ErrorOccurs = ErrorOccurs;
            result.Error = Error;
            result.ReturnedScanner = ReturnedScanner;
            return result;
        }
    }
}
