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
        internal static Lexeme _Comma = new Lexeme("symbol", ",");
        internal static Lexeme _Dot = new Lexeme("symbol", ".");
        internal static Lexeme _Semicolon = new Lexeme("symbol", ";");
        internal static Lexeme _LeftBracket = new Lexeme("symbol", "(");
        internal static Lexeme _RightBracket = new Lexeme("symbol", ")");
        internal static Lexeme _Greater = new Lexeme("symbol", ">");
        internal static Lexeme _GreaterEq = new Lexeme("symbol", ">=");
        internal static Lexeme _Equal = new Lexeme("symbol", "=");
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

        public static Parser<Lexeme> P_Comma { get { return _Comma.AsParser(); } }

        public static Parser<Lexeme> P_Dot { get { return _Dot.AsParser(); } }

        public static Parser<Lexeme> P_Semicolon { get { return _Semicolon.AsParser(); } }

        public static Parser<Lexeme> P_LeftBracket { get { return _LeftBracket.AsParser(); } }

        public static Parser<Lexeme> P_RightBracket { get { return _RightBracket.AsParser(); } }

        public static Parser<Lexeme> P_Greater { get { return _Greater.AsParser(); } }

        public static Parser<Lexeme> P_GreaterEq { get { return _GreaterEq.AsParser(); } }

        public static Parser<Lexeme> P_Equal { get { return _Equal.AsParser(); } }

        public static Parser<Lexeme> P_Less { get { return _Less.AsParser(); } }

        public static Parser<Lexeme> P_LessEq { get { return _LessEq.AsParser(); } }

        public static Parser<Lexeme> P_String { get { return _String.AsParser(); } }

        public static Parser<Lexeme> P_Int { get { return _Int.AsParser(); } }

        public static Parser<Lexeme> P_Float { get { return _Float.AsParser(); } }

        public static Parser<Lexeme> P_Id { get { return _Id.AsParser(); } }
    }
}
