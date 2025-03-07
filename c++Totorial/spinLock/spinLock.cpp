#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <functional>

class SpinLock
{
public:
    //Spinlock() : flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        // 自旋，直到成功获取锁
        // 这里使用test_and_set函数进行原子操作，设置内存序为memory_order_acquire以保证同步
        while (flag.test_and_set(std::memory_order_acquire)) {
            // 如果之前已经被其他线程锁住了，则自旋等待
            std::cout << "Thread " << std::this_thread::get_id() << " is spinning..."
                << std::endl;
            // 让线程休眠一段很短的时间
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            //std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
        // 如果成功获取锁，输出获取锁的线程信息
        std::cout << "Thread " << std::this_thread::get_id()
            << " acquired the lock." << std::endl;
    }

    void unlock() {
        // 将flag变量设置为false，释放锁
        // 这里使用clear函数进行原子操作，设置内存序为memory_order_release以保证同步
        flag.clear(std::memory_order_release);
        // 输出释放锁的线程信息
        std::cout << "Thread " << std::this_thread::get_id()
            << " released the lock." << std::endl;
    }

private:
    // 原子标志，用于表示是否有线程占用锁
    //std::atomic_flag flag;
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    // ATOMIC_FLAG_INIT 的使用问题：ATOMIC_FLAG_INIT 在 C++11 中被引入，用于初始化 std::atomic_flag。
    // 但是在 C++17 标准中，它被标记为弃用，并在某些编译器中可能被移除。
    // 构造函数初始化列表的问题：在构造函数的初始化列表中直接使用 ATOMIC_FLAG_INIT 来初始化 std::atomic_flag 会导致编译器无法正确解析，因而报错。
};

void test1()
{
    // 创建SpinLock对象
    SpinLock spinLock1;

    std::function<void()> threadFunc = [&spinLock1]() {
        // 请求获取锁
        spinLock1.lock();

        // 操作共享资源，这里简单地输出获取锁的线程信息，然后休眠1秒钟
        std::cout << "Thread " << std::this_thread::get_id()
            << " is accessing the shared resource." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // 释放锁
        spinLock1.unlock();
    };
    // 创建两个线程
    std::thread t1(threadFunc);
    std::thread t2(threadFunc);

    // 等待线程执行完毕
    t1.join();
    t2.join();

}

void test2()
{
    SpinLock spinLock2;
    int counter = 0;

    const int num_threads = 4;
    const int num_iterations = 1000;

    std::function<void()> increment = [&counter, num_iterations, &spinLock2]() {
        for (int i = 0; i < num_iterations; ++i) {
            spinLock2.lock();
            ++counter;
            spinLock2.unlock();
        }
    };
    std::vector<std::thread> threads;

    // 创建多个线程，执行increment函数
    for (int i = 0; i < num_threads; ++i) {
        //threads.emplace_back(increment, num_iterations);
        threads.emplace_back(increment);
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 输出最终的计数器值
    std::cout << "Final counter value: " << counter << std::endl;
}

int main() {
    test1();
    test2();

    return 0;
}
