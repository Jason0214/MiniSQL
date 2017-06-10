using System;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;

namespace MiniSQL.SQLAnalyzer.ParserComponents
{
    public static partial class SQLParserComponents
    {
        public static Parser<ListNode<TResult>> P_List<TSource, TSeparator, TResult>(Parser<TSource> sourceParser,
             Parser<TSeparator> separatorParser, Func<TSource, TResult> resultGenerator)
        {
            Parser<ListNode<TResult>> nullNodeParser = ParserCombinators.Succeed(ListNode<TResult>.NullNode);
            Parser<ListNode<TResult>> subNodeParser = null;
            Parser<ListNode<TResult>> listParser = null;

            subNodeParser =
            (
                from _0 in separatorParser
                from subNode in listParser
                select subNode
            ) | nullNodeParser;

            listParser =
            (
                from t in sourceParser
                from subNode in subNodeParser
                select new ListNode<TResult>(resultGenerator(t), subNode)
            );

            return listParser;
        }

        public static Parser<TBase> ToBaseClass<TDerived, TBase>(this Parser<TDerived> parser) 
            where TDerived : TBase
            where TBase : class
        {
            return
            (
                from t in parser
                select t as TBase
            );
        }

        public static Parser<T> SingleParenthesized<T>(this Parser<T> parser)
        {
            return
            (
                from _left in P_LeftBracket
                from _t in parser
                from _right in P_RightBracket
                select _t
            );
        }

        public static Parser<T> Parenthesized<T>(this Parser<T> parser)
        {
            Parser<T> newParser = null;

            newParser =
            (
                from _left in P_LeftBracket
                from _t in parser | newParser
                from _right in P_RightBracket
                select _t
            );

            return newParser;
        }

        public static Parser<T> CanBeParenthesized<T>(this Parser<T> parser)
        {
            return parser | parser.Parenthesized();
        }

        public static Parser<WithAlias<T>> CanHaveAlias<T>(this Parser<T> parser)
        {
            Parser<WithAlias<T>> WithoutAlias =
            (
                from t in parser
                select new WithAlias<T>(t, "__Default_Alias")
            );

            Parser<WithAlias<T>> WithAlias =
            (
                from t1 in parser
                from _as in P_As
                from alias in P_Id
                select new WithAlias<T>(t1, alias.Content)
            );

            return WithAlias | WithoutAlias;
        }

        public static Parser<string> ExtractString(this Parser<Lexeme> parser)
        {
            return
            (
                from id in parser
                select id.Content
            );
        }
    }
}
