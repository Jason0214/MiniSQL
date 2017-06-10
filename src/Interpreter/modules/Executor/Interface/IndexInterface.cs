
namespace MiniSQL.Executor.Interface
{
    public interface IIndex
    {
        bool IndexExist(string name);
    }

    public class IndexInterface : DatabaseInterface, IIndex
    {
        public static IndexInterface Instance = new IndexInterface();

        public bool IndexExist(string name)
        {
            CheckConnection();

            In.WriteLines("index_exist_check", name);
            return Out.ReadLine() == "true";
        }
    }
}
