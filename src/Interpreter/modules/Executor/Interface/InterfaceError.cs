using System;

namespace MiniSQL.Errors
{
    public class InterfaceError : Exception
    {
        public InterfaceError(string msg)
            : base(msg)
        {
        }
    }
}
