#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
class module
{
public:
    module(std::string name, std::vector<std::string> deps) : _name(name), _deps(deps) {}
    virtual void execute() = 0;
    const std::string& GetName() const
    {
        return _name;
    }
    const std::vector<std::string> GetDeps() const
    {
        return _deps;
    }

private:
    std::string _name;
    std::vector<std::string> _deps;
};

class moduleA : public module
{
public:
    moduleA(std::string name, std::vector<std::string> deps) : module(name, deps) {}
    void execute()
    {
        std::cout << "Executing moduleA name:" << GetName() << std::endl;
    }
};

class moduleB : public module
{
public:
    moduleB(std::string name, std::vector<std::string> deps) : module(name, deps) {}
    void execute()
    {
        std::cout << "Executing moduleB name:" << GetName() << std::endl;
    }
};

// 递归 
class executor
{
public:
    void add_module(module* mod)
    {
        _modules[mod->GetName()] = mod;
    }

    void execute_all()
    {
        std::unordered_map<std::string, bool> visited;
        for (auto& mod : _modules)
        {
            if (!visited[mod.first])
            {
                execute(mod.second, visited);
            }
        }
    }
private:
    void execute(module* mod, std::unordered_map<std::string, bool>& visited)
    {
        visited[mod->GetName()] = true;
        for (auto& dep : mod->GetDeps())
        {
            if (!visited[dep])
            {
                execute(_modules[dep], visited);
            }
        }
        mod->execute();
    }

private:
    std::unordered_map<std::string, module*> _modules;
};

class exe_kahn
{

private:
    std::unordered_map<std::string, module*> _modules;
};

class DFSDAGScheduler
{

    void dfs(int node) {
        if (visited[node] == 1) 
            throw runtime_error("检测到循环依赖！");
        if (visited[node] == 2) 
            return;

        visited[node] = 1;  // 标记为"正在访问"
        for (int neighbor : adj[node]) {
            dfs(neighbor);
        }
        visited[node] = 2;  // 标记为"已完成"
        result.push_back(node);
    }

public:
    DFSDAGScheduler(int n) : adj(n), visited(n, 0) {}

    void addEdge(int from, int to) {
        adj[from].push_back(to);
    }
    void addEdge(module* target, module* depend)
    {

    }

    vector<int> schedule() {
        result.clear();
        fill(visited.begin(), visited.end(), 0);

        for (int i = 0; i < adj.size(); ++i) {
            if (visited[i] == 0) {
                dfs(i);
            }
        }

        reverse(result.begin(), result.end());
        return result;
    }
private:
    unordered_map<string, module*> _modules;
    vector<vector<int> > adj;   // 邻接表
    vector<int> visited;        // 0=未访问, 1=访问中, 2=完成
    vector<int> result;         // 逆序存储结果
};

int main()
{
    // 创建moduleA和moduleB对象，并指定它们的依赖关系
    // 举例:A2依赖A1
    moduleA mA1("A1", {});
    moduleA mA2("A2", {"A1"});
    moduleA mA3("A3", {"B1"});
    moduleB mB1("B1", {"A1", "A2"});

    // 创建executor对象，并添加moduleA和moduleB对象
    executor e;
    e.add_module(&mA1);
    e.add_module(&mA2);
    e.add_module(&mB1);
    e.add_module(&mA3);

    // 执行所有modules的操作
    e.execute_all();

    return 0;
}
