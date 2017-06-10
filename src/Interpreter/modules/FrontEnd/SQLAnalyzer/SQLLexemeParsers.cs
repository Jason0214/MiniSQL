using Compiler.Parser;

namespace MiniSQL.SQLAnalyzer.ParserComponents
{
    public static partial class SQLParserComponents
    {
        internal static Lexeme _Select = new Lexeme("keyword", "select");
        internal static Lexeme _From = new Lexeme("keyword", "from");
        internal static Lexeme _Where = new Lexeme("keyword", "where");
        internal static Lexeme _And = new Lexeme("keyword", "and");
        internal static Lexeme _Or = new Lexeme("keyword", "or");
        internal static Lexeme _As = new Lexeme("keyword", "as");
        internal static Lexeme _Between = new Lexeme("keyword", "between");
        internal static Lexeme _NaturalJoin = new Lexeme("keyword", "natural join");
        internal static Lexeme _Create = new Lexeme("keyword", "create");
        internal static Lexeme _Drop = new Lexeme("keyword", "drop");
        internal static Lexeme _Table = new Lexeme("keyword", "table");
        internal static Lexeme _Index = new Lexeme("keyword", "index");
        internal static Lexeme _On = new Lexeme("keyword", "on");
        internal static Lexeme _InsertInto = new Lexeme("keyword", "insert into");
        internal static Lexeme _Set = new Lexeme("keyword", "set");
        internal static Lexeme _Update = new Lexeme("keyword", "update");
        internal static Lexeme _Values = new Lexeme("keyword", "values");
        internal static Lexeme _Delete = new Lexeme("keyword", "delete");
        internal static Lexeme __Char = new Lexeme("keyword", "char");
        internal static Lexeme __Varchar = new Lexeme("keyword", "varchar");
        internal static Lexeme __Int = new Lexeme("keyword", "int");
        internal static Lexeme __Float = new Lexeme("keyword", "float");
        internal static Lexeme _NotNull = new Lexeme("keyword", "not null");
        internal static Lexeme _PrimaryKey = new Lexeme("keyword", "primary key");
        internal static Lexeme _Unique = new Lexeme("keyword", "unique");
        internal static Lexeme _Comma = new Lexeme("symbol", ",");
        internal static Lexeme _Dot = new Lexeme("symbol", ".");
        internal static Lexeme _Star = new Lexeme("symbol", "*");
        internal static Lexeme _Semicolon = new Lexeme("symbol", ";");
        internal static Lexeme _LeftBracket = new Lexeme("symbol", "(");
        internal static Lexeme _RightBracket = new Lexeme("symbol", ")");
        internal static Lexeme _Greater = new Lexeme("symbol", ">");
        internal static Lexeme _GreaterEq = new Lexeme("symbol", ">=");
        internal static Lexeme _Equal = new Lexeme("symbol", "=");
        internal static Lexeme _NotEqual = new Lexeme("symbol", "<>");
        internal static Lexeme _Less = new Lexeme("symbol", "<");
        internal static Lexeme _LessEq = new Lexeme("symbol", "<=");
        internal static Lexeme _String = new Lexeme("single", "");
        internal static Lexeme _Int = new Lexeme("int", "");
        internal static Lexeme _Float = new Lexeme("float", "");
        internal static Lexeme _Id = new Lexeme("id", "");

        public static Parser<Lexeme> P_Select { get { return _Select.AsParser(); } }

        public static Parser<Lexeme> P_From { get { return _From.AsParser(); } }

        public static Parser<Lexeme> P_Where { get { return _Where.AsParser(); } }

        public static Parser<Lexeme> P_And { get { return _And.AsParser(); } }

        public static Parser<Lexeme> P_Or { get { return _Or.AsParser(); } }

        public static Parser<Lexeme> P_As { get { return _As.AsParser(); } }

        public static Parser<Lexeme> P_Between { get { return _Between.AsParser(); } }

        public static Parser<Lexeme> P_NaturalJoin { get { return _NaturalJoin.AsParser(); } }

        public static Parser<Lexeme> P_Create { get { return _Create.AsParser(); } }

        public static Parser<Lexeme> P_Drop { get { return _Drop.AsParser(); } }

        public static Parser<Lexeme> P__Table { get { return _Table.AsParser(); } }

        public static Parser<Lexeme> P_Index { get { return _Index.AsParser(); } }

        public static Parser<Lexeme> P_On { get { return _On.AsParser(); } }

        public static Parser<Lexeme> P_InsertInto { get { return _InsertInto.AsParser(); } }

        public static Parser<Lexeme> P_Set { get { return _Set.AsParser(); } }

        public static Parser<Lexeme> P__Update { get { return _Update.AsParser(); } }

        public static Parser<Lexeme> P_Values { get { return _Values.AsParser(); } }

        public static Parser<Lexeme> P__Delete { get { return _Delete.AsParser(); } }

        public static Parser<Lexeme> P__Char { get { return __Char.AsParser(); } }

        public static Parser<Lexeme> P__Varchar { get { return __Varchar.AsParser(); } }

        public static Parser<Lexeme> P__Int { get { return __Int.AsParser(); } }

        public static Parser<Lexeme> P__Float { get { return __Float.AsParser(); } }

        public static Parser<Lexeme> P_NotNull { get { return _NotNull.AsParser(); } }

        public static Parser<Lexeme> P_PrimaryKey { get { return _PrimaryKey.AsParser(); } }

        public static Parser<Lexeme> P_Unique { get { return _Unique.AsParser(); } }

        public static Parser<Lexeme> P_Comma { get { return _Comma.AsParser(); } }

        public static Parser<Lexeme> P_Dot { get { return _Dot.AsParser(); } }

        public static Parser<Lexeme> P_Star { get { return _Star.AsParser(); } }

        public static Parser<Lexeme> P_Semicolon { get { return _Semicolon.AsParser(); } }

        public static Parser<Lexeme> P_LeftBracket { get { return _LeftBracket.AsParser(); } }

        public static Parser<Lexeme> P_RightBracket { get { return _RightBracket.AsParser(); } }

        public static Parser<Lexeme> P_Greater { get { return _Greater.AsParser(); } }

        public static Parser<Lexeme> P_GreaterEq { get { return _GreaterEq.AsParser(); } }

        public static Parser<Lexeme> P_Equal { get { return _Equal.AsParser(); } }

        public static Parser<Lexeme> P_NotEqual { get { return _NotEqual.AsParser(); } }

        public static Parser<Lexeme> P_Less { get { return _Less.AsParser(); } }

        public static Parser<Lexeme> P_LessEq { get { return _LessEq.AsParser(); } }

        public static Parser<Lexeme> P_String { get { return _String.AsParser(); } }

        public static Parser<Lexeme> P_Int { get { return _Int.AsParser(); } }

        public static Parser<Lexeme> P_Float { get { return _Float.AsParser(); } }

        public static Parser<Lexeme> P_Id { get { return _Id.AsParser(); } }
    }
}
