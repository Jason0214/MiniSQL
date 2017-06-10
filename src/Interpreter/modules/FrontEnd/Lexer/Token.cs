
namespace Compiler.Lexer
{
    public class Token
    {
        public string TypeName { get; internal set; }

        public string Content { get; internal set; }

        public bool BeSpecialType
        {
            get
            {
                return TypeName == "keyword" || TypeName == "symbol";
            }
        }

        public Token(string typeName, string content)
        {
            TypeName = typeName;
            Content = content;
        }
    }
}
