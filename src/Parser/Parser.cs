
namespace Compiler.Parser
{
    public delegate Result<T> ParserFunc<T>(CopiableTokenScanner scanner);

    public class Parser<T>
    {
        public ParserFunc<T> ParserFunction { get; internal set; }

        public Parser(ParserFunc<T> function)
        {
            ParserFunction = function;
        }

        public static Parser<T> operator | (Parser<T> or1, Parser<T> or2)
        {
            return ParserCombinators.Union(or1, or2);
        }
    }
}
