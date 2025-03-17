#include <iostream>
#include <mutex>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

class stringDfsScheduler
{
public:
    void addDependency(const string& from, const string& to)
    {
        adj[from].push_back(to);
        knownTasks[from] = true;
        knownTasks[to] = true;
    }

    vector<string> schedule()
    {
        for (auto& task : knownTasks)
        {
            if (visited[task.first] == 0)
            {
                dfs(task.first);
            }
        }

        // 检查是否所有任务都被处理
        if (result.size() != knownTasks.size()) {
            throw runtime_error("存在未注册的独立任务或循环依赖");
        }

        // 反转结果得到拓扑顺序
        reverse(result.begin(), result.end());
        return result;
    }
private:
    void dfs(const string& task)
    {
        if (visited[task] == 1)
        {
            throw runtime_error("检测到循环依赖: " + task);
        }
        if (visited[task] == 2)
        {
            return;
        }

        for (auto& next : adj[task])
        {
            dfs(next);
        }

        visited[task] = 2;
        result.push_back(task);
    }

private:
    // 任务状态：0=未访问, 1=访问中, 2=已完成
    unordered_map<string, int> visited;
    // 邻接表：记录每个任务的后继任务
    unordered_map<string, vector<string> > adj;
    unordered_map<string, bool> knownTasks;
    vector<string> result;
};

class StringDAGScheduler
{
private:
    // 存储每个任务的入度
    unordered_map<string, int> inDegree;
    // 邻接表：记录每个任务的后继任务
    unordered_map<string, vector<string> > adj;
    // 所有已知的任务集合（避免重复计数）
    unordered_map<string, bool> knownTasks;
    
public:
    // 添加边：from -> to
    void addDependency(const string& from, const string& to) {
        adj[from].push_back(to);
        inDegree[to]++;
        knownTasks[from] = true;
        knownTasks[to] = true;
    }

    // 拓扑排序调度
    vector<string> schedule() {
        queue<string> q;
        vector<string> result;

        // 初始化队列（入度为0的任务）
        for (const auto& pair : knownTasks) {
            const string& task = pair.first;
            if (inDegree[task] == 0) {
                q.push(task);
            }
        }

        // 处理队列
        while (!q.empty()) {
            string u = q.front();
            q.pop();
            result.push_back(u);

            // 更新后继任务的入度
            for (const string& v : adj[u]) {
                if (--inDegree[v] == 0) {
                    q.push(v);
                }
            }
        }

        // 环检测
        if (result.size() != knownTasks.size()) {
            throw runtime_error("存在循环依赖或未注册的任务！");
        }

        return result;
    }
};

void test1()
{
    stringDfsScheduler scheduler1;
    scheduler1.addDependency("编译", "链接");
    scheduler1.addDependency("预处理", "编译");
    scheduler1.addDependency("清理数据", "预处理");
    scheduler1.addDependency("下载依赖", "编译");

    try {
        auto order = scheduler1.schedule();
        cout << "正常调度顺序：";
        for (const auto& task : order) {
            cout << task << " → ";
        }
        cout << "结束\n";
    } catch (const exception& e) {
        cerr << "错误：" << e.what() << endl;
    }

    // 示例2：循环依赖
    stringDfsScheduler scheduler2;
    scheduler2.addDependency("A", "B");
    scheduler2.addDependency("B", "C");
    scheduler2.addDependency("C", "A"); // 形成循环

    try {
        scheduler2.schedule();
    } catch (const exception& e) {
        cout << "\n测试循环依赖：" << e.what() << endl;
    }

}
void test2()
{
    StringDAGScheduler scheduler;

    // 添加依赖关系
    scheduler.addDependency("编译", "链接");
    scheduler.addDependency("清理数据", "预处理");
    scheduler.addDependency("预处理", "编译");
    scheduler.addDependency("下载依赖", "编译");

    try
    {
        vector<string> order = scheduler.schedule();
        cout << "调度顺序：";
        for (const string& task : order) {
            cout << task << " → ";
        }
        cout << "结束" << endl;
    } catch (const exception& e) {
        cerr << "错误：" << e.what() << endl;
    }
}
int main()
{
    test1();
    //test2();
}
