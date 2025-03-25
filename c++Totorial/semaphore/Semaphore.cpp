#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>


class Semaphore
{
public:
    Semaphore(int count = 0) : _count(count) {}

    // 等待信号量, 如果信号量小于0, 则阻塞等待;如果信号量大于0, 则直接返回
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, [this]() { return _count > 0; });
        _count--;
    }

    // 发送信号量,将信号量计数器加1, 并通知等待的线程
    void signal()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _count++;
        _condition.notify_one();
    }
private:
    std::mutex _mutex; // 互斥锁
    std::condition_variable _condition; // 条件变量
    int _count; // 信号量计数器,可用资源数
};

void task(std::shared_ptr<Semaphore> sem)
{
    sem->wait();
    std::cout << "task" << std::endl;
    sem->signal();
}

int main()
{
    std::shared_ptr<Semaphore> sem = std::make_shared<Semaphore>(1);
    std::thread t1(task, sem);
    std::thread t2(task, sem);

    // 等待两个线程执行完毕
    t1.join();
    t2.join();

    return 0;
}

