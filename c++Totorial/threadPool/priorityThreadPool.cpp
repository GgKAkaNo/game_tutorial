#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>
using namespace std;

class Task
{
public:
    Task() : priority(0), task(nullptr) {}
    Task(int priority, function<void()> task) : priority(priority), task(task) {}
    bool operator<(const Task& other) const { return priority < other.priority; }
    const int getPriority() const { return priority; }
    const function<void()> getTask() const { return task; }
private:
    int priority;
    function<void()> task;
};

class ThreadPool
{
public:
    ThreadPool(int numThreads) : _stop(false)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            AddThread();
        }
    }
    ~ThreadPool()
    {
        {
            unique_lock<mutex> lock(_queue_mutex);
            _stop = true;
        }
        _cv.notify_all();

        for (thread& t : _threads)
        {
            if (t.joinable())
            {
                t.join();
            }         
        }
        _cv.notify_all();
    }
    void AddThread()
    {
        _threads.emplace_back([this]() {
            while (true)
            {
                Task task;
                {
                    unique_lock<mutex> lock(_queue_mutex);
                    _cv.wait(lock, [this]() { return _stop || !_tasks.empty(); });
                    if (_stop && _tasks.empty())
                    {
                        return;
                    }
                    task = _tasks.top();
                    _tasks.pop();
                }
                try
                {
                    task.getTask()();
                }
                catch(const std::exception& e)
                {
                    std::cerr << "Task exception: " << e.what() << endl;
                }
                catch(...)
                {
                    std::cerr << "Unknown task exception" << endl;
                }

            }
        });
    }

    template<typename F, typename... Args>
    void PutTask(int priority, F&& f, Args&&... args)
    {
        auto task = bind(forward<F>(f), forward<Args>(args)...);
        {
            unique_lock<mutex> lock(_queue_mutex);
            _tasks.emplace(priority, task);
        }
        _cv.notify_one();
    }
private:
    priority_queue<Task> _tasks;
    vector<thread> _threads;
    mutex _queue_mutex;
    condition_variable _cv;
    bool _stop;
};

void ThreadPoolTest() {
  std::cout << "------------ThreadPoolTest Start------------" << endl;

    ThreadPool tp(2);
    
    // 测试优先级顺序
    for (int i = 0; i < 10; i++) {
        int priority = (i % 3) * 10;  // 产生优先级 0, 10, 20
        tp.PutTask(
            priority,
            [](int pri) {
                this_thread::sleep_for(chrono::milliseconds(100));
                cout << "Tid: " << this_thread::get_id() 
                     << " Priority: " << pri 
                     << " Time: " << chrono::system_clock::now().time_since_epoch().count() 
                     << endl;
            },
            priority);
    }

    // 等待所有任务完成
    this_thread::sleep_for(chrono::seconds(2));
    
    // 测试异常处理
    tp.PutTask(1, []() {
        throw runtime_error("Test exception handling");
    });

    std::cout << "------------ThreadPoolTest End------------" << endl;
}

int main()
{
    ThreadPoolTest();
    return 0;
}
