using System;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MiniSQL.Executor.Optimizer;

namespace ExecutorTest
{
    internal class A
    {
        public int x;

        public A(int x)
        {
            this.x = x;
        }
    }

    [TestClass]
    public class DisjointSetTest
    {
        private DisjointSet<A> set;
        private List<List<A>> result;

        private List<A> GenerateSample(params int[] nums)
        {
            return
            (
                from x in nums
                select new A(x)
            ).ToList();
        }

        private void Check(int index, List<A> source, List<int> check)
        {
            string prompt1 = string.Format("Set[{0}]: Length = ", index);
            Assert.AreEqual(prompt1 + check.Count, prompt1 + source.Count);
            
            for (int i = 0; i < check.Count; i++)
            {
                string prompt2 = string.Format("Set[{0}]: Value {1} in set", index, check[i]);
                if (source.Where(a => a.x == check[i]).ToList().Count != 1)
                {
                    Assert.AreEqual(prompt2, "");
                }
            }
        }

        private void CheckSets(DisjointSet<A> disjSet, int nSets, params int[] check)
        {
            string prompt = "Total Sets: ";
            result = disjSet.GetSets();

            result.Sort(new Comparison<List<A>>((list1, list2) => list1.Min(a => a.x) - list2.Min(a => a.x)));

            Assert.AreEqual(prompt + nSets, prompt + result.Count);

            int listCounter = 0;
            List<int> partialCheck = new List<int>();

            for (int i = 0; i < check.Length; i++)
            {
                if (check[i] == 0)
                {
                    Check(listCounter, result[listCounter], partialCheck);
                    listCounter++;
                    partialCheck.Clear();
                }
                else
                {
                    partialCheck.Add(check[i]);
                }
            }
        }

        [TestMethod]
        public void DisjSet_InitState()
        {
            set = new DisjointSet<A>(GenerateSample(1, 2, 3, 4, 5));
            CheckSets(set, 5, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0);
        }

        [TestMethod]
        public void DisjSet_JustFind()
        {
            List<A> source = GenerateSample(1, 2, 3, 4, 5);
            set = new DisjointSet<A>(source);
            set.FindRoot(source[0]);
            set.FindRoot(source[1]);
            set.FindRoot(source[2]);
            set.FindRoot(source[3]);
            set.FindRoot(source[4]);
            CheckSets(set, 5, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0);
        }

        [TestMethod]
        public void DisjSet_Union()
        {
            List<A> source = GenerateSample(1, 2, 3, 4, 5);
            set = new DisjointSet<A>(source);
            set.Union(source[0], source[4]);
            set.Union(source[4], source[3]);
            set.Union(source[1], source[2]);
            CheckSets(set, 2, 1, 4, 5, 0, 3, 2, 0);
        }

        [TestMethod]
        public void DisjSet_UnionAll()
        {
            List<A> source = GenerateSample(1, 2, 3, 4, 5);
            set = new DisjointSet<A>(source);
            set.Union(source[0], source[4]);
            set.Union(source[4], source[3]);
            set.Union(source[0], source[1]);
            set.Union(source[1], source[2]);
            CheckSets(set, 1, 1, 4, 5, 3, 2, 0);
        }

        [TestMethod]
        public void DisjSet_TrivialUnion()
        {
            List<A> source = GenerateSample(1, 2, 3, 4, 5);
            set = new DisjointSet<A>(source);
            set.Union(source[0], source[4]);
            set.Union(source[4], source[3]);
            set.Union(source[1], source[2]);
            set.Union(source[1], source[1]);
            set.Union(source[0], source[3]);
            set.Union(source[4], source[4]);
            CheckSets(set, 2, 1, 4, 5, 0, 3, 2, 0);
        }

        [TestMethod]
        public void DisjSet_Empty()
        {
            set = new DisjointSet<A>();
            CheckSets(set, 0);
        }
    }
}
