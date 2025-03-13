#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
using namespace std;

class threadPool
{
public:
    threadPool(int nums)
    {
        for (int i = 0; i < nums; ++i)
        {
            AddThread();
        }
    }

    ~threadPool()
    {
        {
            unique_lock<mutex> lock(_mutex);
            _stop = true;
        }

        // 唤醒所有线程
        _cv.notify_all();

        for (auto& thread : _pool)
        {
            if (thread.joinable())
            {
                thread.join(); // 等待线程结束
            }
        }
    }

    void AddThread()
    {
        _pool.emplace_back([this]()
        {
            while (true)
            {
                function<void()> task;
                {
                    unique_lock<mutex> lock(_mutex);
                    _cv.wait(lock, [this]() { return !_task.empty() || _stop; });
                    
                    if (_stop && _task.empty())
                    {
                        return; // 停止线程
                    }
                    
                    task = std::move(_task.front()); // 取出任务
                    _task.pop();
                }

                task();
            }
        });
    }

    // 提交任务
    template <typename F, typename... Args>
    void Commit(F &&f, Args &&...args)
    {
        auto task = bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            unique_lock<mutex> lock(_mutex);
            _task.emplace(task);  // 添加任务到队列
        }

        _cv.notify_one();  // 唤醒一个线程执行任务
    }

private:
    vector<thread> _pool;          // 线程池
    queue<function<void()>> _task; // 任务队列
    mutex _mutex;                  // 互斥锁
    condition_variable _cv;        // 条件变量
    bool _stop = false;           // 停止标记位
};

// 打印函数
void Print(int num)
{
    cout << "Thread ID: " << this_thread::get_id() << " -> " << num << endl;
}

// 主函数
int main()
{
    threadPool pool(4);  // 创建线程池，容量为4

    // 提交8个任务
    for (int i = 0; i < 8; i++) {
        pool.Commit(Print, i);
    }
    return 0;
}
