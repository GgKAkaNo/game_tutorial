[toc]
## 一、DAG任务调度基本概念
DAG（有向无环图）的任务调度目标是为相互依赖的任务确定合理的执行顺序，核心要求：

依赖顺序正确：任务必须在其所有前置任务完成后执行

无循环依赖：图中不能存在循环依赖（即环）

高效调度：尽量利用并行性（当允许并行执行时）

## 二、两种实现方式对比

| 特性       | 递归DFS               | 拓扑排序（Kahn）        |
|------------|-----------------------|-----------------------|
| 核心思想   | 深度优先遍历+后序记录 | 维护入度表+队列管理   |
| 时间复杂度 | O(V+E)                | O(V+E)                |
| 空间复杂度 | 递归栈深度            | 队列空间              |
| 环检测方式 | 递归路径标记          | 结果长度校验          |
| 适用场景   | 小规模图/明确无环图   | 大规模图/需要稳定顺序 |

## 三、C++代码实现
### 1. 递归DFS实现（基于深度优先搜索）
```cpp
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class DFSDAGScheduler {
private:
    vector<vector<int>> adj;    // 邻接表
    vector<int> visited;        // 0=未访问, 1=访问中, 2=完成
    vector<int> result;         // 逆序存储结果

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
};
```
### 2. 拓扑排序实现（Kahn算法）
```cpp
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

class TopologicalScheduler {
private:
    vector<vector<int>> adj;    // 邻接表
    vector<int> inDegree;       // 入度表

public:
    TopologicalScheduler(int n) : adj(n), inDegree(n, 0) {}

    void addEdge(int from, int to) {
        adj[from].push_back(to);
        inDegree[to]++;
    }

    vector<int> schedule() {
        queue<int> q;
        vector<int> result;

        // 初始化队列（入度为0的节点）
        for (int i = 0; i < adj.size(); ++i) {
            if (inDegree[i] == 0) {
                q.push(i);
            }
        }

        // 处理队列
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            result.push_back(u);

            for (int v : adj[u]) {
                if (--inDegree[v] == 0) {
                    q.push(v);
                }
            }
        }

        // 环检测
        if (result.size() != adj.size()) {
            throw runtime_error("检测到循环依赖！");
        }

        return result;
    }
};
```
## 四、使用示例
```cpp
int main() {
    // 示例DAG结构：
    // 0 → 2 → 3 → 4
    // 1 ↗

    // 测试DFS实现
    {
        DFSDAGScheduler dfsScheduler(5);
        dfsScheduler.addEdge(0, 2);
        dfsScheduler.addEdge(1, 2);
        dfsScheduler.addEdge(2, 3);
        dfsScheduler.addEdge(3, 4);

        try {
            auto order = dfsScheduler.schedule();
            cout << "DFS调度顺序: ";
            for (int task : order) cout << task << " ";
        } catch (...) {
            cout << "DFS检测到循环依赖！";
        }
    }

    // 测试拓扑排序实现
    {
        TopologicalScheduler topoScheduler(5);
        topoScheduler.addEdge(0, 2);
        topoScheduler.addEdge(1, 2);
        topoScheduler.addEdge(2, 3);
        topoScheduler.addEdge(3, 4);

        try {
            auto order = topoScheduler.schedule();
            cout << "\n拓扑排序顺序: ";
            for (int task : order) cout << task << " ";
        } catch (...) {
            cout << "\n拓扑排序检测到循环依赖！";
        }
    }

    return 0;
}
```

```
输出结果
DFS调度顺序: 1 0 2 3 4 
拓扑排序顺序: 0 1 2 3 4 
```

## 五、扩展功能建议
1. 并行调度：修改拓扑排序算法，每次取出所有可执行任务
2. 优先级支持：使用优先队列替代普通队列

```
// 示例：优先执行编号小的任务
priority_queue<int, vector<int>, greater<int>> q;
```
3. 权重支持：添加任务执行时间属性，实现关键路径调度

可视化调试：输出Graphviz格式的DAG图

## 六、关键问题处理技巧
1. 循环依赖检测：

* DFS实现：通过访问状态标记及时检测

* 拓扑排序：最终结果长度校验

2. 执行顺序差异：

* DFS实现的结果依赖于遍历起始点顺序

* 拓扑排序的结果是确定性的（依赖队列实现）

3. 性能优化：

* 大规模图优先选择拓扑排序（避免递归栈溢出）


## 针对使用字符串作为任务名称的DAG调度实现方案：

### 1.数据结构调整
我们需要将原先基于数字索引的存储结构改为基于字符串的映射：

```cpp
#include <iostream>
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
    unordered_map<string, vector<string>> adj;
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
```
### 2. 使用示例
```cpp
int main() {
    StringDAGScheduler scheduler;

    // 添加依赖关系
    scheduler.addDependency("编译", "链接");
    scheduler.addDependency("清理数据", "预处理");
    scheduler.addDependency("预处理", "编译");
    scheduler.addDependency("下载依赖", "编译");

    try {
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
```
```
输出示例：
调度顺序：清理数据 → 下载依赖 → 预处理 → 编译 → 链接 → 结束
```
### 3. 实现关键点
1. 动态任务管理：

使用 unordered_map 自动处理新任务的添加

knownTasks 确保正确统计总任务数

2. 起始任务判定：

```cpp
// 检查所有已知任务中入度为0的
for (const auto& pair : knownTasks) {
    if (inDegree[pair.first] == 0) {
        q.push(pair.first);
    }
}
```
3. 环检测逻辑：

```cpp
if (result.size() != knownTasks.size()) {
    throw ...; // 存在未处理的任务，说明有环
}
```
### 4. 常见问题处理
场景1：存在多个独立任务链
输入依赖：

```cpp
scheduler.addDependency("A", "B");
scheduler.addDependency("C", "D");
```
输出可能：

```
A → C → B → D → 结束
或 
C → A → D → B → 结束 
（具体顺序取决于哈希表遍历顺序）
```
场景2：需要确定性顺序
如果需要按字母顺序调度起始任务，可以修改队列类型：
```
// 将 queue<string> 改为优先队列
priority_queue<string, vector<string>, greater<string>> q;
```
此时示例输出会变为：

```
清理数据 → 下载依赖 → 预处理 → 编译 → 链接 → 结束
```
## 5. 扩展功能建议
1. 任务元数据绑定：

```cpp
struct Task {
    string name;
    int priority;
    double estimatedTime;
};

unordered_map<string, Task> taskMetadata;
```
2. 并行调度支持：

```cpp
// 每次取出所有可并行执行的任务
while (!q.empty()) {
    vector<string> parallelTasks;
    size_t currentLevelSize = q.size();
    for (size_t i = 0; i < currentLevelSize; ++i) {
        string u = q.front();
        q.pop();
        parallelTasks.push_back(u);
        // ... 更新后继任务 ...
    }
    result.push_back(parallelTasks); // 结果变为vector<vector<string>>
}
```
3. 可视化输出：

```cpp
void exportToDOT(const vector<string>& order) {
    cout << "digraph G {\n";
    for (const auto& [from, toList] : adj) {
        for (const string& to : toList) {
            cout << "  \"" << from << "\" -> \"" << to << "\";\n";
        }
    }
    cout << "}\n";
}
```
### 6. 性能优化方向

 操作         | 优化方法                   |
|--------------|----------------------------|
| 频繁查询     | 使用unordered_map O(1)查找 |
| 大规模数据   | 改用 absl::flat_hash_map   |
| 确定顺序需求 | 使用 map + 插入时间戳      |

通过这种设计，可以灵活处理以字符串命名的任务调度需求，同时保持算法的高效性和可扩展性。