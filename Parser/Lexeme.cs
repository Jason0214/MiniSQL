using System.Collections.Generic;
using Compiler.Lexer;

namespace Compiler.Parser
{
    public class Lexeme
    {
        private Token token;

        public string TypeName { get { return token.TypeName; } }

        public string Content { get { return token.Content; } }

        public bool BeSpecialType { get { return token.BeSpecialType; } }

        public Lexeme(Token fromToken)
        {
            token = fromToken;
        }

        public Lexeme(string typeName, string content)
        {
            token = new Token(typeName, content);
        }


        public static implicit operator Lexeme(Token token)
        {
            return new Lexeme(token);
        }

        public static List<Lexeme> ToLexemeList(List<Token> tokenList)
        {
            List<Lexeme> list = new List<Lexeme>();

            foreach (Token t in tokenList)
            {
                list.Add(t);
            }

            return list;
        }
    }
}
