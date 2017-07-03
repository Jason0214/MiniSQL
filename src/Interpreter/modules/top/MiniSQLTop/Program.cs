using System;
using System.Collections.Generic;
using MiniSQL.Errors;
using MiniSQL.Executor;
using MiniSQL.Executor.Validator;
using MiniSQL.Executor.Interface;
using MiniSQL.SQLAnalyzer;
using System.Reflection;
using System.Resources;
using System.IO;

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

        static TextReader In = Console.In;
        static TextWriter Out = Console.Out;

        static void Main(string[] args)
        {
            IDirectExe exeInterface = DirectExeInterface.Instance;

            if (args.Length == 4 && args[0] == "<" && args[2] == ">")
            {
                string inputFileName = args[1];
                string outputFileName = args[3];

                try
                {
                    In = new StreamReader(inputFileName);
                    Out = new StreamWriter(outputFileName);
                }
                catch (System.Exception ex)
                {
                    Console.WriteLine(ex.Message);
                    return;
                }
            }

            while (true)
            {
                string input = "";
                string tmp;
                string trim;

                try
                {
                    Out.WriteLine("MiniSQL --> Enter your command:");

                    do
                    {
                        if (In is StreamReader && (In as StreamReader).EndOfStream)
                        {
                            return;
                        }

                        tmp = In.ReadLine();
                        trim = tmp.TrimEnd();

                        if (trim == "quit" | trim == "exit")
                        {
                            exeInterface.In.WriteLine("quit");
                            exeInterface.Out.ReadLine();
                            Console.WriteLine("----------------------------");
                            return;
                        }

                        input += tmp + "\n";
                    } while (trim.Length == 0 || trim[trim.Length - 1] != ';');

                    SQLLexer lexer = new SQLLexer();
                    Errors = lexer.Interpret(input);
                    if (Errors.Count > 0) goto NextInput;

                    SQLParser parser = new SQLParser();
                    Errors = parser.Parse(lexer.Lexemes);
                    if (Errors.Count > 0) goto NextInput;

                /*    SQLValidator validator = SQLValidator.GetValidator(parser.resultStatement);
                    Errors = validator.Validate();
                    if (Errors.Count > 0) goto NextInput;*/

                    SQLExecutor executor = SQLExecutor.GetExecutor(parser.resultStatement, Out);
                    executor.Execute();
                }
                catch (Exception ex)
                {
                    Out.WriteLine(ex.Message);
                    Out.WriteLine("---- MiniSQL: Fatal Error ----");
                    return;
                }

            NextInput:
                Out.WriteLine();
            }
        }
    }
}
