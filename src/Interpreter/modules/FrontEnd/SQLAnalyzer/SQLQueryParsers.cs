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
                    select new Attr(attr.Content, "__Default_Table")
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
                Parser<ListNode<WithAlias<Attr>>> selectAllParser =
                (
                    from _star in P_Star
                    select new ListNode<WithAlias<Attr>>(new WithAlias<Attr>(new Attr("*", "*"), "__Default_Alias"),
                        ListNode<WithAlias<Attr>>.NullNode)
                );

                return
                (
                    from _key in P_Select
                    from list in selectAllParser | P_List(P_Attr.CanBeParenthesized().CanHaveAlias(), P_Comma, attr => attr)
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

        public static Parser<Comparand> P_Comparand
        {
            get
            {
                Parser<Comparand> attrOperand =
                (
                    from attr in P_Attr
                    select new Comparand("Attribute", attr)
                );

                Parser<Comparand> otherOperand =
                (
                    from lex in P_Id | P_Int | P_Float | P_String
                    select new Comparand(lex.TypeName, lex.Content)
                );

                return attrOperand | otherOperand;
            }
        }

        public static Parser<Expression> P_CompareExpression
        {
            get
            {
                Parser<Expression> simpleCompareParser =
                (
                    from op1 in P_Comparand.CanBeParenthesized()
                    from op in P_Less | P_LessEq | P_Equal | P_GreaterEq | P_Greater | P_NotEqual
                    from op2 in P_Comparand.CanBeParenthesized()
                    select (Expression)new CompareExpression(op1, op.Content, op2)
                );

                Parser<ListNode<Expression>> betweenAnd =
                (
                    from op1 in P_Comparand.CanBeParenthesized()
                    from _1 in P_Between
                    from op2 in P_Comparand.CanBeParenthesized()
                    from _2 in P_And
                    from op3 in P_Comparand.CanBeParenthesized()
                    select new ListNode<Expression>
                    (
                        (Expression)new CompareExpression(op2, "<=", op1),
                        new ListNode<Expression>
                        (
                            (Expression)new CompareExpression(op1, "<=", op3),
                            ListNode<Expression>.NullNode
                        )
                    )
                );

                Parser<Expression> betweenAndParser =
                (
                    from p in betweenAnd
                    select (Expression)new ConjunctionExpression(p)
                );

                return simpleCompareParser | betweenAndParser;
            }
        }

        public static Parser<Expression> P_SimpleCompareExpression
        {
            get
            {
                return
                (
                    from op1 in P_Comparand.CanBeParenthesized()
                    from op in P_Less | P_LessEq | P_Equal | P_GreaterEq | P_Greater | P_NotEqual
                    from op2 in P_Comparand.CanBeParenthesized()
                    select (Expression)new CompareExpression(op1, op.Content, op2)
                );
            }
        }

        public static Parser<Expression> P_Expression
        {
            get
            {
                Parser<Expression> subParser = new Parser<Expression>(null);
                Parser<Expression> conjParser = new Parser<Expression>(null);
                Parser<Expression> disjParser = new Parser<Expression>(null);

                subParser.ParserFunction = disjParser.Parenthesized().ParserFunction;

                disjParser.ParserFunction =
                (
                    from conjList in P_List(conjParser | subParser, P_Or, predict => predict)
                    select (Expression)new DisjunctionExpression(conjList)
                ).ParserFunction;

                conjParser.ParserFunction =
                (
                    from list in P_List(P_CompareExpression | subParser, P_And, predict => predict)
                    select (Expression)new ConjunctionExpression(list)
                ).ParserFunction;

                return disjParser;
            }
        }

        public static Parser<Expression> P_SimpleExpression
        {
            get
            {
                return
                (
                    from conjList in P_List(P_SimpleCompareExpression, P_And, exp => exp)
                    select (Expression)new ConjunctionExpression(conjList)
                );
            }
        }

        public static Parser<WhereClause> P_WhereClause
        {
            get
            {
                return
                (
                    from _key in P_Where
                    from p in P_Expression
                    select new WhereClause(p)
                );
            }
        }

        public static Parser<WhereClause> P_SimpleWhereClause
        {
            get
            {
                return
                (
                    from _key in P_Where
                    from p in P_SimpleExpression
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
                    from _ in P_Semicolon
                    select new Query(s, f, w)
                );
            }
        }

        public static Parser<Query> P_SimpleQuery
        {
            get
            {
                Parser<WhereClause> nullWhereParser = ParserCombinators.Succeed(WhereClause.nullWhere);

                return
                (
                    from s in P_SelectClause
                    from f in P_FromClause
                    from w in P_SimpleWhereClause | nullWhereParser
                    from _ in P_Semicolon
                    select new Query(s, f, w)
                );
            }
        }
    }
}
