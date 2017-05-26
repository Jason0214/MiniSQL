using System;
using System.Linq;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Diagnostics.Contracts;

namespace Compiler.Lexer
{
    public static class TokenTypeCollection
    {
        private static string EscapeSpecialSymbols(string input)
        {
            return input.Replace(@"\", @"\\").Replace("+", @"\+").Replace(".", @"\.").Replace("?", @"\?")
                .Replace("*", @"\*").Replace("^", @"\^").Replace("$", @"\$").Replace("(", @"\(").Replace(")", @"\)")
                .Replace("[", @"\[").Replace("]", @"\]").Replace("{", @"\{").Replace("}", @"\}");
        }

        private static TokenType GenerateCollcetioinType(IEnumerable<string> collection, string typeName, uint processOrder, string tail)
        {
            string[] symbols = collection.ToArray();
            Array.Sort(symbols);
            Array.Reverse(symbols);

            string RegexExpForSymbols = string.Format("({0})", EscapeSpecialSymbols(string.Join("|", symbols))) + tail;
            return new TokenType(typeName, RegexExpForSymbols, processOrder);
        }

        public static TokenType GenerateSymbolType(IEnumerable<string> symbols)
        {
            return GenerateCollcetioinType(symbols, "symbol", 3, "");
        }

        public static TokenType GenerateKeywordType(IEnumerable<string> keywords)
        {
            return GenerateCollcetioinType(keywords, "keyword", 7, @"\b");
        }

        public static TokenType SingleQuote = new TokenType("single", @"'(.|\s)*?(?<!\\)'", 0);
        public static TokenType DoubleQuote = new TokenType("double", @"""(.|\s)*?(?<!\\)""", 1);
        public static TokenType FrontDotFloat = new TokenType("float", @"\.[0-9]+", 2);
        /*public static TokenType Symbols = new TokenType("symbol", "_", 3);*/
        public static TokenType CompleteFloat = new TokenType("float", @"[0-9]+\.[0-9]+", 4);
        public static TokenType PostDotFloat = new TokenType("float", @"[0-9]+\.", 5);
        public static TokenType Integer = new TokenType("int", @"[0-9]+", 6);
        /*public static TokenType Keywords = new TokenType("keyword", "_", 7);*/
        public static TokenType Identifier = new TokenType("id", @"[_a-zA-Z\u4e00-\u9fa5][\w\u4e00-\u9fa5]*", 8);
        public static TokenType Empty = new TokenType("empty", @"\s+", 65535);
    }

    public class TokenType
    {
        internal Regex RegexEngine { get; set; }

        public bool Enabled { get; set; }

        public uint ProcessOrder { get; internal set; }

        public string TypeName { get; internal set; }

        public string RegexSyntax { get; internal set; }

        internal TokenType(string name, string regex, uint order)
        {
            Contract.Requires(name != null && name != "", "The syntax must have a type name!");
            Contract.Requires(regex != null && regex != "", "The syntax must have a regular expression!");

            Enabled = true;
            ProcessOrder = order;
            TypeName = name;
            RegexSyntax = regex;
            RegexEngine = new Regex("^" + regex);
        }
    }

    internal class TokenPriorityComparer : IComparer<TokenType>
    {
        public int Compare(TokenType x, TokenType y)
        {
            if (x.ProcessOrder != y.ProcessOrder) return (int)x.ProcessOrder - (int)y.ProcessOrder;
            else if (x.TypeName != y.TypeName) return 1;
            else return 0;
        }
    }
}
