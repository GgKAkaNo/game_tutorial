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

class executor_kahn
{
public:
    void add_module(module* mod)
    {
        _modules[mod->GetName()] = mod;
    }
    void execute_all()
    {
        unordered_map<string, vector<module*> > dep_modules;
        // 初始化入度和依赖模块
        for (auto& mod_pair : _modules)
        {
            const string& mod_name = mod_pair.first;
            const vector<string>& mod_deps = mod_pair.second->GetDeps();
            _indegree[mod_name] = mod_deps.size();
            for (const string& dep_name : mod_deps)
            {
                dep_modules[dep_name].push_back(mod_pair.second);
                cout << dep_name << "->" << mod_pair.second->GetName() << endl;
            }
        }
        // 将入度为0的模块加入队列
        queue<module*> que;
        for (auto& mod_pair : _modules)
        {
            if (_indegree[mod_pair.first] == 0) {
                que.push(mod_pair.second);
            }
        }

        // 按照队列中的顺序执行模块
        while (!que.empty()) {
            module* mod = que.front();
            que.pop();
            mod->execute();
            for (module* dep_mod : dep_modules[mod->GetName()])
            {
                if (--_indegree[dep_mod->GetName()] == 0) {
                    que.push(dep_mod);
                }
            }
        }
    }
private:
    unordered_map<string, int> _indegree;
    unordered_map<string, module*> _modules;
};

void test1()
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

}
void test2()
{
    // 创建moduleA和moduleB对象，并指定它们的依赖关系
    // 举例:A2依赖A1
    moduleA mA1("A1", {});
    moduleA mA2("A2", {"A1"});
    moduleA mA3("A3", {"B1"});
    moduleB mB1("B1", {"A1", "A2"});

    // 创建executor对象，并添加moduleA和moduleB对象
    executor_kahn e;
    e.add_module(&mA1);
    e.add_module(&mA2);
    e.add_module(&mB1);
    e.add_module(&mA3);

    // 执行所有modules的操作
    e.execute_all();
}

int main()
{
    test2();
    return 0;
}
