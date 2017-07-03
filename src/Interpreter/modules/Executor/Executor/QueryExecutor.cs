using System.IO;
using System.Linq;
using System.Collections.Generic;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Executor.Optimizer;
using MiniSQL.Executor.Interface;
using MiniSQL.Errors;

namespace MiniSQL.Executor
{
    public class QueryExecutor : SQLExecutor
    {
        private IQuery queryInterface = QueryInterface.Instance;
        private IDirectExe exeInterface = DirectExeInterface.Instance;

        private Query query;
        private BinaryLogicStep plan;

        private List<List<string>> cartesianTables;
        private Dictionary<string, List<CompareExpression>> tablesNeedSelect;

        private static int IndexGenerator = 0;

        private static string GetTmpName()
        {
            return "_tmp" + IndexGenerator++;
        }

        public QueryExecutor(Query q, IQuery queryInterface = null, IDirectExe exeInterface = null)
        {
            query = q;
            
            if (queryInterface != null)
            {
                this.queryInterface = queryInterface;
            }

            if (exeInterface != null)
            {
                this.exeInterface = exeInterface;
            }
        }

        public override void Execute()
        {
            if (query.RemoveConstantExp_IsEmptySet() == true)
            {
                outStream.WriteLine("Empty Set");
                return;
            }

            plan = GenerateLogicPlan();

            ExecutePlan();
        }

        private void ExecutePlan()
        {
            IEnumerable<WithAlias<string>> tables =
            (
                from table in query.fromClause.TableList
                from x in table.JoinedTablesList
                select x
            );

            exeInterface.In.WriteLine("begin_query");
            queryInterface.TableInfo(tables.ToList());

            string resultTableName = ExecuteStep(plan);
            queryInterface.CopyResultTo(resultTableName, outStream);

            exeInterface.In.WriteLine("end_query");
        }

        private string ExecuteStep(BinaryLogicStep step)
        {
            if (step == null)
            {
                throw new ExecutionError("Execution Error: Null Step");
            }

            if (step.Operation == "read")
            {
                return step.ResultTableName;
            }

            if (step.Operation == "select" || step.Operation == "project")
            {
                string sourceTableName = ExecuteStep(step.LeftChildStep);
                string param = TranslateParamToString(step.Param);

                exeInterface.In.WriteLines(step.Operation, sourceTableName, step.ResultTableName, param);

                return step.ResultTableName;
            }

            if (step.Operation == "natural_join" || step.Operation == "cartesian")
            {
                string sourceTableName1 = ExecuteStep(step.LeftChildStep);
                string sourceTableName2 = ExecuteStep(step.RightChildStep);

                exeInterface.In.WriteLines(step.Operation, sourceTableName1, sourceTableName2, step.ResultTableName);

                return step.ResultTableName;
            }

            throw new ExecutionError("Execution Error: Unknown Step");
        }

        private string TranslateParamToString(object param)
        {
            string result = "";

            if (param is List<CompareExpression>)
            {
                result += (param as List<CompareExpression>).Count + "\n";

                foreach (var cmp in param as List<CompareExpression>)
                {
                    result += cmp.ToCommandString();
                }

                return result;
            }

            if (param is List<WithAlias<Attr>>)
            {
                result += (param as List<WithAlias<Attr>>).Count + "\n";

                foreach (var attr in param as List<WithAlias<Attr>>)
                {
                    result += attr.ToCommandString();
                }

                return result;
            }

            throw new ExecutionError("Execution Error: Invalid param");
        }

        private BinaryLogicStep GenerateLogicPlan()
        {
        //   cartesianTables = query.CartesianToJoin();
        //   tablesNeedSelect = query.PushSelectDown();
            cartesianTables =
            (
                from joinTable in query.fromClause.TableList
                select
                (
                    from aliasPair in joinTable.JoinedTablesList
                    select aliasPair.Alias == "__Default_Alias" ? aliasPair.Entity : aliasPair.Alias
                ).ToList()
            ).ToList();

            BinaryLogicStep cartesianStep = GenerateCartesianStep();
            BinaryLogicStep selectStep = cartesianStep;
            List<CompareExpression> expList = query.whereClause.ToCompareList();

            if (expList.Count > 0)
            {
                selectStep = BinaryLogicStep.Select(selectStep, expList, GetTmpName());
            }

            BinaryLogicStep projectStep = selectStep;
            List<WithAlias<Attr>> attrs = 
            (
                from attralias in query.selectClause.AttributesList
                orderby attralias.Entity.AttributeName
                select attralias
            ).ToList();

            if (!(attrs.Count == 1 && attrs[0].Entity.AttributeName == "*"))
            {
                projectStep = BinaryLogicStep.Project(projectStep, attrs, GetTmpName());
            }

            return projectStep;
        }

        private BinaryLogicStep GenerateCartesianStep()
        {
            if (cartesianTables == null || cartesianTables.Count == 0)
            {
                throw new ExecutionError("Error in Generate Cartesian Step: Cartesian List is Empty");
            }

            return MergeSteps_Skew
            (
                from joinList in cartesianTables
                select GenerateJoinTree(joinList)
            );
        }

        private BinaryLogicStep GenerateReadStep(string tableName)
        {
            BinaryLogicStep step = BinaryLogicStep.Read(tableName);

         /*   if (tablesNeedSelect.ContainsKey(tableName))
            {
                step = BinaryLogicStep.Select(step, tablesNeedSelect[tableName], GetTmpName());
            }*/

            return step;
        }

        private BinaryLogicStep GenerateJoinTree(List<string> tables)
        {
            if (tables == null || tables.Count == 0)
            {
                throw new ExecutionError("Error in Generate Join Tree: Table List is Empty");
            }

            return MergeSteps_Skew
            (
                from tableName in tables
                select GenerateReadStep(tableName)
            );
        }

        private BinaryLogicStep MergeSteps_Skew(IEnumerable<BinaryLogicStep> steps)
        {
            Stack<BinaryLogicStep> stepStack = new Stack<BinaryLogicStep>(steps);

            if (stepStack.Count == 0)
            {
                throw new ExecutionError("Error in Merge Steps: Empty Stack");
            }

            while (stepStack.Count > 1)
            {
                BinaryLogicStep readStep1 = stepStack.Pop();
                BinaryLogicStep readStep2 = stepStack.Pop();

                if (string.Compare(readStep1.ResultTableName, readStep2.ResultTableName) > 0)
                {
                    stepStack.Push(BinaryLogicStep.NaturalJoin(readStep2, readStep1, GetTmpName()));
                }
                else
                {
                    stepStack.Push(BinaryLogicStep.NaturalJoin(readStep1, readStep2, GetTmpName()));
                }

            }

            return stepStack.Pop();
        }
    }
}
