using System;

namespace MiniSQL.Errors
{
    public class SQLError : Exception
    {
        public SQLError(string errMsg)
            : base(errMsg)
        {
        }
    }
}
