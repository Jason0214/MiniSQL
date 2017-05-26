using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Diagnostics.Contracts;

namespace Compiler.Lexer
{
    public class Lexer
    {
        internal SortedSet<TokenType> DefinedSyntax = new SortedSet<TokenType>(new TokenPriorityComparer());
        internal List<Token> ResultTokens = null;
        internal List<LexerError> lexerErrors = null;

        public Lexer(IEnumerable<string> Symbols, IEnumerable<string> Keywords)
        {
            DefinedSyntax.Add(TokenTypeCollection.SingleQuote);
            DefinedSyntax.Add(TokenTypeCollection.DoubleQuote);
            DefinedSyntax.Add(TokenTypeCollection.FrontDotFloat);
            DefinedSyntax.Add(TokenTypeCollection.GenerateSymbolType(Symbols));
            DefinedSyntax.Add(TokenTypeCollection.CompleteFloat);
            DefinedSyntax.Add(TokenTypeCollection.PostDotFloat);
            DefinedSyntax.Add(TokenTypeCollection.Integer);
            DefinedSyntax.Add(TokenTypeCollection.GenerateKeywordType(Keywords));
            DefinedSyntax.Add(TokenTypeCollection.Identifier);
        }

        public virtual List<LexerError> Interpret(string input)
        {
            lexerErrors = new List<LexerError>();
            ResultTokens = new List<Token>();

            StringScanner scanner = new StringScanner(input);
            scanner.RegexMatch(TokenTypeCollection.Empty.RegexEngine);

            while (scanner.EndOfString == false)
            {
                bool beMatched = false;

                foreach (TokenType syntax in DefinedSyntax)
                {
                    string matchString = scanner.TokenMatch(syntax);

                    if (matchString != string.Empty)
                    {
                        ResultTokens.Add(new Token(syntax.TypeName, matchString));
                        beMatched = true;
                        break;
                    }

                    if (syntax.Enabled && (
                        (input[scanner.CurrentIndex] == '\'' && syntax.TypeName == "single") ||
                        (input[scanner.CurrentIndex] == '\"' && syntax.TypeName == "double")))
                    {
                        lexerErrors.Add(new LexerError(scanner.CurrentIndex, "Quote Does Not Match", 
                            input.Substring(scanner.CurrentIndex)));
                        return lexerErrors;
                    }
                }

                if (beMatched == false)
                {
                    string errorChar = scanner.Read(1);
                    lexerErrors.Add(new LexerError(scanner.CurrentIndex, "Unknown Char", errorChar));
                }

                scanner.RegexMatch(TokenTypeCollection.Empty.RegexEngine);
            }

            return lexerErrors;
        }

        public virtual List<Token> GetResultTokens()
        {
            Contract.Requires(ResultTokens != null, "Lexer.Interpret must be called first!");
            return ResultTokens;
        }
    }
}
