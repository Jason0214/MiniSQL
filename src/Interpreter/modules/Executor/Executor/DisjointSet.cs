using System;
using System.Linq;
using System.Collections.Generic;
using MiniSQL.Errors;

namespace MiniSQL.Executor.Optimizer
{
    public class DisjointSet<T> where T : class
    {
        private Dictionary<T, object> ParentNodeRecord = new Dictionary<T, object>();

        public DisjointSet()
        {
        }

        public DisjointSet(IEnumerable<T> singleItemGroups)
        {
            foreach (T item in singleItemGroups)
            {
                AddGroup(new List<T>() { item });
            }
        }

        public DisjointSet(IEnumerable<IEnumerable<T>> initGroups)
        {
            foreach (var group in initGroups)
            {
                AddGroup(group);
            }
        }

        public void AddGroup(IEnumerable<T> groupMembers)
        {
            T root = null;
            int i = 0;

            foreach (T member in groupMembers)
            {
                if (i == 0)
                {
                    root = member;
                }
                else
                {
                    ParentNodeRecord.Add(member, root);
                }

                i++;
            }

            if (i > 0)
            {
                // Record Height, either 1 or 0
                ParentNodeRecord.Add(root, i > 1 ? 1 : 0);
            }
        }

        public void UnionRoot(T root1, T root2)
        {
            if (root1 == root2)
            {
                return;
            }

            try
            {
                int root1Height = (int)ParentNodeRecord[root1];
                int root2Height = (int)ParentNodeRecord[root2];

                if (root2Height > root1Height)
                {
                    ParentNodeRecord[root1] = root2; // root2 is the new root
                }
                else
                {
                    if (root2Height == root1Height)
                    {
                        ParentNodeRecord[root1] = root1Height + 1;
                    }

                    ParentNodeRecord[root2] = root1; // root1 is the new root
                }
            }
            catch (Exception)
            {
                throw new ExecutionError("Disjoint Set: Union Failed!");
            }
        }

        public void Union(T element1, T element2)
        {
            UnionRoot(FindRoot(element1), FindRoot(element2));
        }

        public T FindRoot(T Element)
        {
            if (ParentNodeRecord.ContainsKey(Element) == false)
            {
                return null;
            }

            if (ParentNodeRecord[Element] is int)
            {
                return Element;
            }
            else
            {
                T parentNode = FindRoot((T)ParentNodeRecord[Element]);
                ParentNodeRecord[Element] = parentNode;
                return parentNode;
            }
        }

        public List<List<T>> GetSets()
        {
            List<T> keys = (from t in ParentNodeRecord.Keys select t).ToList();
            List<List<T>> result = new List<List<T>>();
            Dictionary<T, int> resultIndexForRoots = new Dictionary<T, int>();

            foreach (T Element in keys)
            {
                T root = FindRoot(Element);

                if (resultIndexForRoots.ContainsKey(root))
                {
                    result[resultIndexForRoots[root]].Add(Element);
                }
                else
                {
                    resultIndexForRoots.Add(root, result.Count);
                    result.Add(new List<T>() { root });
                }
            }

            return result;
        }
    }
}
