using System;
using System.Linq;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;

namespace MiniSQL.SQLAnalyzer.ParserComponents
{
    public static partial class SQLParserComponents
    {
        public static Parser<Attr> P_Attr
        {
            get
            {
                 Parser<Attr> withTable =
                 (
                    from table in P_Id
                    from _Dot in P_Dot
                    from attr in P_Id
                    select new Attr(attr.Content, table.Content)
                 );

                 Parser<Attr> withoutTable =
                 (
                    from attr in P_Id
                    select new Attr(attr.Content, "Default_Table")
                 );

                 return withTable | withoutTable;
            }
        }

        public static Parser<Table> P_Table
        {
            get
            {
                return
                (
                    from list in P_List(P_Id.ExtractString().CanHaveAlias().CanBeParenthesized(), P_NaturalJoin, tb => tb)
                    select new Table(list)
                );
            }
        }

        public static Parser<SelectClause> P_SelectClause
        {
            get
            {
                return
                (
                    from _key in P_Select
                    from list in P_List(P_Attr.CanBeParenthesized().CanHaveAlias(), P_Comma, attr => attr)
                    select new SelectClause(list)
                );
            }
        }

        public static Parser<FromClause> P_FromClause
        {
            get
            {
                return
                (
                    from _key in P_From
                    from list in P_List(P_Table.CanBeParenthesized(), P_Comma, tb => tb)
                    select new FromClause(list)
                );
            }
        }

        public static Parser<CompareOperand> P_CompareOperand
        {
            get
            {
                Parser<CompareOperand> attrOperand =
                (
                    from attr in P_Attr
                    select new CompareOperand("Attribute", attr)
                );

                Parser<CompareOperand> otherOperand =
                (
                    from lex in P_Id | P_Int | P_Float | P_String
                    select new CompareOperand(lex.TypeName, lex.Content)
                );

                return attrOperand | otherOperand;
            }
        }

        public static Parser<Predict> P_ComparePredict
        {
            get
            {
                Parser<Predict> simpleCompareParser =
                (
                    from op1 in P_CompareOperand.CanBeParenthesized()
                    from op in (P_Less | P_LessEq | P_Equal | P_GreaterEq | P_Greater)
                    from op2 in P_CompareOperand.CanBeParenthesized()
                    select (Predict)new ComparePredict(op1, op.Content, op2)
                );

                Parser<ListNode<Predict>> betweenAnd =
                (
                    from op1 in P_CompareOperand.CanBeParenthesized()
                    from _1 in P_Between
                    from op2 in P_CompareOperand.CanBeParenthesized()
                    from _2 in P_And
                    from op3 in P_CompareOperand.CanBeParenthesized()
                    select new ListNode<Predict>
                    (
                        (Predict)new ComparePredict(op2, "<=", op1),
                        new ListNode<Predict>
                        (
                            (Predict)new ComparePredict(op1, "<=", op3),
                            ListNode<Predict>.NullNode
                        )
                    )
                );

                Parser<Predict> betweenAndParser =
                (
                    from p in betweenAnd
                    select (Predict)new ConjunctionPredict(p)
                );

                return simpleCompareParser | betweenAndParser;
            }
        }

        public static Parser<Predict> P_Predict
        {
            get
            {
                Parser<Predict> subParser = new Parser<Predict>(null);
                Parser<Predict> conjParser = new Parser<Predict>(null);
                Parser<Predict> disjParser = new Parser<Predict>(null);

                subParser.ParserFunction = disjParser.Parenthesized().ParserFunction;

                disjParser.ParserFunction =
                (
                    from conjList in P_List(conjParser | subParser, P_Or, predict => predict)
                    select (Predict)new DisjunctionPredict(conjList)
                ).ParserFunction;

                conjParser.ParserFunction =
                (
                    from list in P_List(P_ComparePredict | subParser, P_And, predict => predict)
                    select (Predict)new ConjunctionPredict(list)
                ).ParserFunction;

                return disjParser;
            }
        }

        public static Parser<WhereClause> P_WhereClause
        {
            get
            {
                return
                (
                    from _key in P_Where
                    from p in P_Predict
                    select new WhereClause(p)
                );
            }
        }

        public static Parser<Query> P_Query
        {
            get
            {
                Parser<WhereClause> nullWhereParser = ParserCombinators.Succeed(WhereClause.nullWhere);

                return
                (
                    from s in P_SelectClause
                    from f in P_FromClause
                    from w in P_WhereClause | nullWhereParser
                    select new Query(s, f, w)
                );
            }
        }
    }
}
