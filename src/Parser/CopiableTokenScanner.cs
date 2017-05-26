using System;
using System.Collections.Generic;

namespace Compiler.Parser
{
    public struct CopiableTokenScanner
    {
        private List<Lexeme> lexemes;
        private int index;
        private int preIndex;
        private bool end;

        public int CurrentIndex { get { return index; } }

        public int PreviousIndex { get { return preIndex; } }

        public int ElementNum { get { return lexemes.Count; } }

        public bool EndOfList { get { return end; } }

        public CopiableTokenScanner(List<Lexeme> inputLexemes)
        {
            if (inputLexemes.Count > 0)
            {
                end = false;
            } 
            else
            {
                end = true;
            }

            index = 0;
            preIndex = -1;
            lexemes = inputLexemes;
        }

        public Lexeme Read()
        {
            if (end == true)
            {
                return null;
            }

            preIndex = index;

            Lexeme result = lexemes[index++];
            if (index >= lexemes.Count)
            {
                end = true;
            }

            return result;
        }

        public Lexeme Get(int index)
        {
            if (index >= 0 && index < lexemes.Count)
            {
                return lexemes[index];
            }

            return null;
        }
    }
}
