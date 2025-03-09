#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
using namespace std;

class StringDAGScheduler {
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

int main()
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
