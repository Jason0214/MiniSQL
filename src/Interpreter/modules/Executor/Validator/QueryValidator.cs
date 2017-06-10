using System;
using System.Collections.Generic;
using MiniSQL.Errors;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Interface;

namespace MiniSQL.Executor.Validator
{
    public class QueryValidator : SQLValidator
    {
        /*
         * 检测内容：
         * 
         * 1. 表是否存在
         * 2. 属性是否存在
         * 3. 表达式类型匹配
         * 4. 属性名歧义判断（是否属于多张表）
         * 
         */
        private Query InitQuery;
        private TableManager tableManager;
        private ExpManager expManager;

        public QueryValidator(Query query, ICatalog catalogInterface = null)
        {
            InitQuery = query;
            Errors = new List<SQLError>();
            tableManager = new TableManager(Errors, catalogInterface);
            expManager = new ExpManager(query.whereClause.Exp, Errors, tableManager);
        }

        public override List<SQLError> Validate()
        {
            ScanFromClause();
            ScanSelectClause();
            ScanWhereClause();
            return Errors;
        }

        private void ScanFromClause()
        {
            foreach (Table table in InitQuery.fromClause.TableList)
            {
                foreach (WithAlias<string> namePair in table.JoinedTablesList)
                {
                    tableManager.AddInfo(namePair);
                }
            }
        }

        private void ScanSelectClause()
        {
            var AttrList = InitQuery.selectClause.AttributesList;

            if (AttrList.Count == 1 && AttrList[0].Entity.AttributeName == "*" && AttrList[0].Entity.TableName == "*")
            {
                return;
            }

            foreach (var attrAlias in AttrList)
            {
                tableManager.CheckAttr(attrAlias.Entity);
            }
        }

        private void ScanWhereClause()
        {
            expManager.OnlyConjunctionSupported();
            expManager.ScanExpression();
        }
    }
}
