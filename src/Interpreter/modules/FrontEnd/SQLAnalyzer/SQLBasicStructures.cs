using System.Collections.Generic;

namespace MiniSQL.SQLAnalyzer.Structures
{
    public abstract class Statement
    {
    }

    public abstract class Expression : Statement
    {
    }

    public class ListNode<T> : Statement
    {
        public static ListNode<T> NullNode = new ListNode<T>(default(T), null);

        public T Content { get; protected set; }

        public ListNode<T> Next { get; protected set; }

        public ListNode(T content, ListNode<T> next)
        {
            Content = content;
            Next = next;
        }

        public List<T> ToList()
        {
            ListNode<T> startNode = this;
            List<T> result = new List<T>();

            while (startNode != null && startNode != NullNode)
            {
                result.Add(startNode.Content);
                startNode = startNode.Next;
            }

            return result;
        }
    }

    public class WithAlias<T> : Statement
    {
        public T Entity { get; protected set; }

        public string Alias { get; protected set; }

        public WithAlias(T entity, string alias)
        {
            Entity = entity;
            Alias = alias;
        }
    }
}
