using System;
using Compiler.Parser;
using MiniSQL.SQLAnalyzer.Structures;

namespace MiniSQL.SQLAnalyzer.ParserComponents
{
    public static partial class SQLParserComponents
    {
        public static Parser<AttrType> P_AttrType
        {
            get
            {
                Parser<AttrType> charParser =
                (
                    from _char in P__Char
                    from length in P_Int.SingleParenthesized()
                    select new AttrType("char", Convert.ToInt32(length.Content))
                );

                Parser<AttrType> varcharParser =
                (
                    from _varchar in P__Varchar
                    from length in P_Int.SingleParenthesized()
                    select new AttrType("varchar", Convert.ToInt32(length.Content))
                );

                Parser<AttrType> intParser =
                (
                    from _int in P__Int
                    select new AttrType("int", 0)
                );

                Parser<AttrType> floatParser =
                (
                    from _float in P__Float
                    select new AttrType("float", 0)
                );

                return charParser | varcharParser | intParser | floatParser;
            }
        }

        public static Parser<AttrDefinition> P_AttrDefinition
        {
            get
            {
                return
                (
                    from name in P_Id
                    from type in P_AttrType 
                    from restricts in P_List(P_NotNull | P_Unique | P_PrimaryKey, 
                        ParserCombinators.Succeed<Lexeme>(null), id => id.Content) |
                        ParserCombinators.Succeed(ListNode<string>.NullNode)
                    select new AttrDefinition(name.Content, type, restricts.ToList())
                );
            }
        }

        public static Parser<CreateTable> P_CreateTable
        {
            get
            {
                Parser<ListNode<string>> primaryKeyListParser =
                (
                    from _1 in P_Comma
                    from _2 in P_PrimaryKey
                    from idList in P_List(P_Id, P_Comma, id => id.Content).SingleParenthesized()
                    select idList
                ) | ParserCombinators.Succeed(ListNode<string>.NullNode);

                return
                (
                    from _1 in P_Create
                    from _2 in P__Table
                    from name in P_Id
                    from _3 in P_LeftBracket
                    from defList in P_List(P_AttrDefinition, P_Comma, def => def)
                    from primaryKeys in primaryKeyListParser
                    from _4 in P_RightBracket
                    from _5 in P_Semicolon
                    select new CreateTable(name.Content, defList, primaryKeys)
                );
            }
        }

        public static Parser<DropTable> P_DropTable
        {
            get
            {
                return
                (
                    from _1 in P_Drop
                    from _2 in P__Table
                    from name in P_Id
                    from _3 in P_Semicolon
                    select new DropTable(name.Content)
                );
            }
        }

        public static Parser<CreateIndex> P_CreateIndex
        {
            get
            {
                return
                (
                    from _1 in P_Create
                    from _2 in P_Index
                    from name in P_Id
                    from _3 in P_On
                    from table in P_Id
                    from attr in P_Id.SingleParenthesized()
                    from _4 in P_Semicolon
                    select new CreateIndex(name.Content, table.Content, attr.Content)
                );
            }
        }

        public static Parser<DropIndex> P_DropIndex
        {
            get
            {
                return
                (
                    from _1 in P_Drop
                    from _2 in P_Index
                    from name in P_Id
                    from _3 in P_Semicolon
                    select new DropIndex(name.Content)
                );
            }
        }

        public static Parser<NewValue> P_NewValue
        {
            get
            {
                Parser<NewValue> stringParser =
                (
                    from id in P_String
                    select new NewValue(id.TypeName, id.Content.Substring(1, id.Content.Length - 2))
                );

                return
                (
                    from id in P_Int | P_Float
                    select new NewValue(id.TypeName, id.Content)
                ) | stringParser;
            }
        }

        public static Parser<Insert> P_Insert
        {
            get
            {
                return
                (
                    from _1 in P_InsertInto
                    from table in P_Id
                    from _2 in P_Values
                    from list in P_List(P_NewValue, P_Comma, value => value).SingleParenthesized()
                    from _3 in P_Semicolon
                    select new Insert(table.Content, list)
                );
            }
        }

        public static Parser<Update> P_Update
        {
            get
            {
                Parser<WhereClause> nullWhereParser = ParserCombinators.Succeed(WhereClause.nullWhere);

                return
                (
                    from _1 in P__Update
                    from table in P_Id
                    from _2 in P_Set
                    from attr in P_Id
                    from _3 in P_Equal
                    from value in P_NewValue
                    from w in P_SimpleWhereClause | nullWhereParser
                    from _4 in P_Semicolon
                    select new Update(table.Content, attr.Content, value, w)
                );
            }
        }

        public static Parser<Delete> P_Delete
        {
            get
            {
                Parser<WhereClause> nullWhereParser = ParserCombinators.Succeed(WhereClause.nullWhere);

                return
                (
                    from _1 in P__Delete
                    from _2 in P_From
                    from table in P_Id
                    from whereClause in P_SimpleWhereClause | nullWhereParser
                    from _3 in P_Semicolon
                    select new Delete(table.Content, whereClause)
                );
            }
        }
    }
}
