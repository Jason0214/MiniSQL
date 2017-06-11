using System;
using System.Collections.Generic;
using MiniSQL.Errors;

namespace MiniSQL.Executor.Interface
{
    public struct TypeInfo
    {
        public string TypeName;
        public int Param;

        public TypeInfo(string name, int param = 0)
        {
            TypeName = name;
            Param = param;
        }

        public static TypeInfo Int()
        {
            return new TypeInfo("int");
        }

        public static TypeInfo Float()
        {
            return new TypeInfo("float");
        }

        public static TypeInfo Char(int length)
        {
            return new TypeInfo("char", length);
        }

        public static TypeInfo Varchar(int maxLength)
        {
            return new TypeInfo("varchar", maxLength);
        }
    }

    public struct AttrInfo
    {
        public string AttrName;
        public TypeInfo AttrType;

        public AttrInfo(string name, TypeInfo type)
        {
            AttrName = name;
            AttrType = type;
        }
    }

    public struct Schema
    {
        public string TableName;
        public List<AttrInfo> AttrList;

        public Schema(string name, List<AttrInfo> attributes = null)
        {
            TableName = name;
            AttrList = attributes;

            if (AttrList == null)
            {
                AttrList = new List<AttrInfo>();
                return;
            }
        }

        public static Schema operator + (Schema table, AttrInfo attr)
        {
            table.TryAddAttr(attr);
            return table;
        }

        private void TryAddAttr(AttrInfo attr)
        {
            try
            {
                AttrList.Add(attr);
            }
            catch (Exception)
            {
                throw new InterfaceError("Invalid TableInfo, with duplicate attributes");
            }
        }

        public AttrInfo? GetAttr(string attrName)
        {
            foreach (AttrInfo attr in AttrList)
            {
                if (attr.AttrName == attrName)
                {
                    return attr;
                }
            }

            return null;
        }
    }
}
