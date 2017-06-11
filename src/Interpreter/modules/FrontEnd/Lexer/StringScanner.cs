using System.Text.RegularExpressions;

namespace Compiler.Lexer
{
    internal class StringScanner
    {
        public string OriginalString { get; protected set; }

        public int CurrentIndex { get; protected set; }

        public bool EndOfString { get; protected set; }

        public StringScanner(string input)
        {
            OriginalString = input;
            CurrentIndex = 0;
        }

        public string Read(int length)
        {
            if (EndOfString == true || length < 0)
            {
                return string.Empty;
            }

            if (CurrentIndex + length >= OriginalString.Length)
            {
                length = OriginalString.Length - CurrentIndex;
                EndOfString = true;
            }

            string result = OriginalString.Substring(CurrentIndex, length);
            CurrentIndex += length;

            return result;
        }

        public string RegexMatch(Regex regexEngine)
        {
            if (EndOfString == true || regexEngine == null)
            {
                return string.Empty;
            }

            Match match = regexEngine.Match(OriginalString.Substring(CurrentIndex));
            CurrentIndex += match.Value.Length;

            if (CurrentIndex == OriginalString.Length)
            {
                EndOfString = true;
            }

            return match.Value;
        }

        public string TokenMatch(TokenType tokenType)
        {
            if (tokenType.Enabled)
            {
                return RegexMatch(tokenType.RegexEngine);
            } 
            else
            {
                return string.Empty;
            }
        }
    }
}
