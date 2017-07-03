using System;

namespace Compiler.Parser
{
    public static class ParserCombinators
    {
        public static Parser<T> Succeed<T>(T result)
        {
            return new Parser<T>(scanner => new Result<T>(result, scanner));
        }

        public static Parser<Lexeme> AsParser(this Lexeme lex)
        {
            return new Parser<Lexeme>(scanner =>
            {
                var lexeme = scanner.Read();
                int readIndex = lexeme == null ? scanner.CurrentIndex : scanner.PreviousIndex;

                if (lexeme != null && lexeme.TypeName == lex.TypeName &&
                    (lexeme.Content == lex.Content || lexeme.BeSpecialType == false))
                {
                    return new Result<Lexeme>(lexeme, scanner);
                }
                else
                {
                    return new Result<Lexeme>(readIndex, lex, scanner);
                }
            });
        }

        public static Parser<TResult> Select<TSource, TResult>(this Parser<TSource> parser, 
            Func<TSource, TResult> resultSelector)
        {
            return new Parser<TResult>(scanner =>
            {
                var result1 = parser.ParserFunction(scanner);
                if (result1.ErrorOccurs)
                {
                    return result1.ErrorCopy<TResult>();
                }

                return new Result<TResult>(resultSelector(result1.Value), result1.ReturnedScanner);
            });
        }

        public static Parser<TResult> SelectMany<T1, T2, TResult>(this Parser<T1> parser,
            Func<T1, Parser<T2>> parserSelector, Func<T1, T2, TResult> resultSelector)
        {
            return new Parser<TResult>(scanner =>
            {
                var result1 = parser.ParserFunction(scanner);
                if (result1.ErrorOccurs)
                {
                    return result1.ErrorCopy<TResult>();
                }

                var parser2 = parserSelector(result1.Value);
                var result2 = parser2.ParserFunction(result1.ReturnedScanner);
                if (result2.ErrorOccurs)
                {
                    return result2.ErrorCopy<TResult>();
                }

                return new Result<TResult>(resultSelector(result1.Value, result2.Value), result2.ReturnedScanner);
            });
        }

        public static Parser<T> Union<T>(this Parser<T> parser1, Parser<T> parser2)
        {
            return new Parser<T>(scanner =>
            {
                var scanner1 = scanner;
                var scanner2 = scanner1;

                var result1 = parser1.ParserFunction(scanner1);

                if (result1.ErrorOccurs == false)
                {
                    return result1;
                }

                var result2 = parser2.ParserFunction(scanner2);

                if (result2.ErrorOccurs == false)
                {
                    return result2;
                }

                // 没有消耗新的词汇
                return new Result<T>(Math.Max(result1.Error.ErrorIndex, result2.Error.ErrorIndex)
                    , new Lexeme("special", ""), scanner);
            });
        }
    }
}
