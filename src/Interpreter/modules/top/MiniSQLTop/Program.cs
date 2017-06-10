using System;
using System.Collections.Generic;
using MiniSQL.Errors;
using MiniSQL.Executor;
using MiniSQL.Executor.Validator;
using MiniSQL.SQLAnalyzer;

namespace MiniSQL
{
    class Program
    {
        static List<SQLError> errors;

        static List<SQLError> Errors
        {
            get
            {
                return errors;
            }

            set
            {
                errors = value;

                if (value.Count > 0)
                {
                    foreach (var er in value)
                    {
                        Console.WriteLine(er.Message);
                    }
                }
            }
        }

        static void Main(string[] args)
        {
            while (true)
            {
                string input = "";
                string tmp;
                string trim;

                Console.WriteLine("MiniSQL --> Enter your command:");

                do 
                {
                    tmp = Console.ReadLine();
                    input += tmp + "\n";
                    trim = tmp.TrimEnd();
                } while (trim.Length == 0 || trim[trim.Length - 1] != ';');

                try
                {
                    SQLLexer lexer = new SQLLexer();
                    Errors = lexer.Interpret(input);
                    if (Errors.Count > 0) goto NextInput;

                    SQLParser parser = new SQLParser();
                    Errors = parser.Parse(lexer.Lexemes);
                    if (Errors.Count > 0) goto NextInput;

                    SQLValidator validator = SQLValidator.GetValidator(parser.resultStatement);
                    Errors = validator.Validate();
                    if (Errors.Count > 0) goto NextInput;

                    SQLExecutor executor = SQLExecutor.GetExecutor(parser.resultStatement, Console.Out);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                    Console.WriteLine("---- MiniSQL: Fatal Error ----");
                    return;
                }

            NextInput:
                Console.WriteLine();
            }
        }
    }
}
