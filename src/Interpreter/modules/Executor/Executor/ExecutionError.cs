using System;

namespace MiniSQL.Errors
{
    public class ExecutionError : Exception
    {
        public ExecutionError(string msg)
            : base(msg)
        {
        }
    }
}
