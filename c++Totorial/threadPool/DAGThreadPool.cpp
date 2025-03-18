#include <deque>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <iostream>
#include <condition_variable>
#include <functional>
#include <queue>
#include <sstream>
using namespace std;

class ThreadPool
{
public:
    ThreadPool(int numThreads) : _stop(false)
    {
        for (int i = 0; i < numThreads; i++)
        {
            AddThread();
        }
    }
    ~ThreadPool()
    {
        {
            unique_lock<mutex> lock(_mutex);
            _stop = true;
        }
        _cv.notify_all();
        for (auto &thread : _pool)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    void AddThread()
    {
        _pool.emplace_back([this]()
        {
            while (true)
            {
                function<void()> func;
                {
                    unique_lock<mutex> lock(_mutex);
                    _cv.wait(lock, [this]() { return _stop || !_tasks.empty(); });
                    if (_stop && _tasks.empty())
                    {
                        return;
                    }
                    func = move(_tasks.front());
                    _tasks.pop_front();
                }
                func();
            }
        });
    }

    template<typename F, typename... Args>
    void PutTask(F &&f, Args &&...args)
    {
        auto func = bind(forward<F>(f), forward<Args>(args)...);
        {
            unique_lock<mutex> lck(_mutex);
            _tasks.emplace_back(func);
        }
        _cv.notify_one();
    }
private:
    deque<function<void()>> _tasks;
    vector<thread> _pool;
    mutex _mutex;
    condition_variable _cv;
    bool _stop;
};

mutex g_log_mtx;

class LogQueue
{
public:
    // 使用 && (右值引用)
    // 1. 可以接受临时字符串：g_log_que.Push("hello"s);
    // 2. 可以接受 std::move 的字符串
    // 3. 支持移动语义，避免不必要的拷贝
    void Push(string&& str)
    { 
        lock_guard<mutex> lock(mtx_);
        log_que_.emplace(str);  // 直接移动构造，没有拷贝
        cv_.notify_one();
    }

    // 如果使用 & (左值引用)
    // void Push(string& str)
    // { 
    //     lock_guard<mutex> lock(mtx_);
    //     log_que_.push(str);  // 会产生一次拷贝
    //     cv_.notify_one();
    // }
    // 问题：
    // 1. 不能接受临时对象：g_log_que.Push("hello"s); // 错误！
    // 2. 不能接受 std::move 的字符串
    // 3. 每次都会产生一次拷贝

    void Cout()
    {
        unique_lock<mutex> lock(mtx_);
        cv_.wait_for(lock, chrono::milliseconds(100), [this]() { return !log_que_.empty(); });
        if (!log_que_.empty())
        {
            string log = move(log_que_.front());
            log_que_.pop();
            cout << log << endl;
        }
    }

private:
    queue<string> log_que_;
    mutex mtx_;
    condition_variable cv_;
};
LogQueue g_log_que;

class Module
{
public:
    Module(string name, vector<string> deps) : name_(name), deps_(deps) {}
    virtual void Execute() = 0;
    const string& Name() const { return name_; }
    const vector<string>& Deps() const { return deps_; }
    bool CheckSucc() { return is_succ_; }
    void SetSucc() { is_succ_ = true; }
    void ClearState()
    {
        is_succ_ = false;
        if (commit_log_thread_->joinable()) commit_log_thread_->join();
    }

protected:
    void AsyncCommitLog()
    {
        lock_guard<mutex> log_lock(g_log_mtx);
        commit_log_thread_ = make_unique<thread>([this]
        {
            stringstream ss;
            ss << "Executing: " << Name() << " Tid: " << this_thread::get_id()
            << endl;
            g_log_que.Push(ss.str());
        });
    }
    unique_ptr<thread> commit_log_thread_;

private:
    string name_;
    vector<string> deps_;
    bool is_succ_{false};
};

class ModuleA : public Module
{
public:
    ModuleA(string name, vector<string> deps) : Module(name, deps) {}
    void Execute() override
    {
        // compute
        this_thread::sleep_for(std::chrono::milliseconds(1000));
        SetSucc();
        // commit log
        AsyncCommitLog();
    }
};

class ModuleB : public Module
{
public:
    ModuleB(string name, vector<string> deps) : Module(name, deps) {}
    void Execute() override
    {
        // compute
        this_thread::sleep_for(std::chrono::milliseconds(1000));
        SetSucc();
        // commit log
        AsyncCommitLog();
    }
};

class ModuleC : public Module
{
public:
    ModuleC(string name, vector<string> deps) : Module(name, deps) {}
    void Execute() override 
    {
        // compute
        this_thread::sleep_for(std::chrono::milliseconds(1000));
        SetSucc();
        // commit log
        AsyncCommitLog();
    }
};

class ModuleD : public Module
{
public:
    ModuleD(string name, vector<string> deps) : Module(name, deps) {}
    void Execute() override
    {
        // compute
        this_thread::sleep_for(std::chrono::milliseconds(1000));
        SetSucc();
        // commit log
        AsyncCommitLog();
    }
};

class ModuleE : public Module
{
public:
    ModuleE(string name, vector<string> deps) : Module(name, deps) {}
    void Execute() override
    {
        // compute
        this_thread::sleep_for(std::chrono::milliseconds(1000));
        SetSucc();
        // commit log
        AsyncCommitLog();
    }

private:
    bool is_end_{false};
};

class Executor
{
public:
    void AddModule(Module* module)
    {
        _modules[module->Name()] = module;
    }

    void ExecuteAll(ThreadPool& tp)
    {
        unordered_map<string, bool> visited;
        for (auto& mod : _modules)
        {
            if (visited[mod.first]) continue;
            Execute(mod.second, tp, visited);
        }
    }
      
private:
    void Execute(Module* mod, ThreadPool& tp, unordered_map<string, bool>& visited)
    {
        visited[mod->Name()] = true;
        for (auto& dep : mod->Deps())
        {
            if (!visited[dep])
            {
                Execute(_modules[dep], tp, visited);
            }
        }
        
        // 使用条件变量等待依赖完成
        unique_lock<mutex> lock(_mutex);
        _cv.wait(lock, [this, mod]() {
            for (auto& dep : mod->Deps())
            {
                if (!_modules[dep]->CheckSucc()) return false;
            }
            return true;
        });
        
        tp.PutTask([this, mod]() {
            mod->Execute();
            // 执行完成后通知等待的线程
            _cv.notify_all();
        });
    }
private:
    unordered_map<string, Module*> _modules;
    mutex _mutex;
    condition_variable _cv;
};

void test()
{
    ModuleA a("A", {});
    ModuleB b("B", {});
    ModuleC c("C", {"A", "B"});
    ModuleD d("D", {"C"});
    ModuleE e("E", {"C", "D"});

    Executor executor;
    executor.AddModule(&a);
    executor.AddModule(&b);
    executor.AddModule(&c);
    executor.AddModule(&d);
    executor.AddModule(&e);

    bool is_program_finish{false};

    thread log_que_thread([&is_program_finish] {
        while (!is_program_finish)
        {
            g_log_que.Cout();
            this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    const int num_threads = 2;
    ThreadPool tp(num_threads);

    int graph_times = 2;
    while (graph_times--)
    {
        executor.ExecuteAll(tp);
        // 等待所有任务完成
        this_thread::sleep_for(std::chrono::milliseconds(2000));

        a.ClearState();
        b.ClearState();
        c.ClearState();
        d.ClearState();
        e.ClearState();
    }

    {
        is_program_finish = true;
        if (log_que_thread.joinable()) log_que_thread.join();
    }
}

int main()
{
    test();
    return 0;
}
