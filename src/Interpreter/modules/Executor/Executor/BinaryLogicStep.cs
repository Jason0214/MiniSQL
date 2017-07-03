using System.Collections.Generic;
using MiniSQL.SQLAnalyzer.Structures;
using MiniSQL.Errors;

namespace MiniSQL.Executor
{
    public partial class BinaryLogicStep
    {
        public string Operation { get; private set; }

        public string ResultTableName { get; private set; }

        public object Param { get; private set; }

        public BinaryLogicStep ParentStep { get; private set; }

        public BinaryLogicStep LeftChildStep { get; private set; }

        public BinaryLogicStep RightChildStep { get; private set; }

        private BinaryLogicStep()
        {
        }
    }

    public partial class BinaryLogicStep
    {
        private static void NotNullAssert(BinaryLogicStep step)
        {
            if (step == null)
            {
                throw new ExecutionError("Executioin: Unexpected step");
            }
        }

        private static BinaryLogicStep SingleSource(string operation, BinaryLogicStep sourceStep,
            string resultTableName, object param = null)
        {
            NotNullAssert(sourceStep);

            BinaryLogicStep step = new BinaryLogicStep();

            step.Operation = operation;
            step.ResultTableName = resultTableName;
            step.Param = param;

            sourceStep.ParentStep = step;
            step.LeftChildStep = sourceStep;
            step.RightChildStep = null;
            step.ParentStep = null;

            return step;
        }

        private static BinaryLogicStep BinarySource(string operation, BinaryLogicStep sourceStep1, BinaryLogicStep sourceStep2,
            string resultTableName, object param = null)
        {
            NotNullAssert(sourceStep1);
            NotNullAssert(sourceStep2);

            BinaryLogicStep step = new BinaryLogicStep();

            step.Operation = operation;
            step.ResultTableName = resultTableName;
            step.Param = param;

            sourceStep1.ParentStep = step;
            sourceStep2.ParentStep = step;
            step.LeftChildStep = sourceStep1;
            step.RightChildStep = sourceStep2;
            step.ParentStep = null;

            return step;
        }

        public static BinaryLogicStep Read(string tableName)
        {
            BinaryLogicStep step = new BinaryLogicStep();

            step.ParentStep = step.LeftChildStep = step.RightChildStep = null;
            step.Operation = "read";
            step.ResultTableName = tableName;
            step.Param = null;

            return step;
        }

        public static BinaryLogicStep Select(BinaryLogicStep sourceStep, List<CompareExpression> compares, string resultTableName)
        {
            return SingleSource("select", sourceStep, resultTableName, compares);
        }

        public static BinaryLogicStep Project(BinaryLogicStep sourceStep, List<WithAlias<Attr>> attributeAlias, string resultTableName)
        {
            return SingleSource("project", sourceStep, resultTableName, attributeAlias);
        }

        public static BinaryLogicStep NaturalJoin(BinaryLogicStep sourceStep1, BinaryLogicStep sourceStep2, string resultTableName)
        {
            return BinarySource("natural_join", sourceStep1, sourceStep2, resultTableName);
        }

        public static BinaryLogicStep Cartesian(BinaryLogicStep sourceStep1, BinaryLogicStep sourceStep2, string resultTableName)
        {
            return BinarySource("cartesian", sourceStep1, sourceStep2, resultTableName);
        }
    }
}
