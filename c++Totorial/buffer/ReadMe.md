
# 双缓冲区管理器(DoubleGraphBufferMgr)

这段代码实现了一个双缓冲区管理器(DoubleGraphBufferMgr)类，用于管理两个纹理缓冲区对象，提供获取绘制缓冲区(GetDrawBuffer())和缓存缓冲区(GetCacheBuffer())的接口。

当需要进行绘制时，调用GetDrawBuffer()方法可以获取一个绘制缓冲区对象，进行渲染操作。当绘制操作完成后，调用EmplaceCacheBuffer()方法将该缓冲区对象置于缓存队列中，等待下一次使用。

当需要进行离线处理时，调用GetCacheBuffer()方法可以获取一个缓存缓冲区对象，进行离线处理操作。当处理操作完成后，调用EmplaceDrawBuffer()方法将该缓冲区对象置于绘制队列中，等待下一次绘制使用。

使用双缓冲区可以避免绘制和离线处理操作之间的竞争条件，从而提高了程序的效率。在这个双缓冲区管理器中，利用互斥锁(mutex)和条件变量(condition_variable)实现了线程间同步。同时，还使用了智能指针(shared_ptr和weak_ptr)来管理缓冲区对象的生命周期，确保在对象不再被使用时能够正确地释放资源。

关键点：draw_available_在一开始开辟2个空间的buffer，保存其指针，cache_available_在一开始为0。当draw_available其中一块buffer被pop_front使用后，draw_available内的buffer指针析构，在删除器中将其buffer指针传递到cache_available_中，此时draw_available和cache_available_的buffer指针数量都是1，循环往复该过程。

## 细节
### 解释三个重要的 C++ 工具：

1. **unique_lock**:
```cpp
std::unique_lock<std::mutex> lock(_mutex);
```
- 是 C++ 标准库中的 RAII 锁管理工具
- 特点：
  - 自动加锁和解锁（构造时加锁，析构时解锁）
  - 支持条件变量的等待操作（`wait`）
  - 可以手动控制加锁和解锁（`lock()` 和 `unlock()`）
  - 支持移动语义，可以转移锁的所有权
- 使用场景：
  - 需要配合条件变量使用时
  - 需要手动控制锁的加锁和解锁时
  - 需要转移锁的所有权时

2. **weak_ptr**:
```cpp
std::weak_ptr<DoubleGraphBufferMgr> weak_mgr = shared_from_this();
```
- 是 `shared_ptr` 的弱引用，不增加引用计数
- 特点：
  - 不拥有对象的所有权
  - 不会阻止对象的销毁
  - 可以通过 `lock()` 获取 `shared_ptr`
  - 用于打破循环引用
- 使用场景：
  - 需要观察对象但不影响其生命周期时
  - 需要避免循环引用时
  - 需要缓存对象但允许其被销毁时

3. **shared_ptr**:
```cpp
std::shared_ptr<GlTextureBuffer>(
    buffer.release(),
    [weak_mgr](GlTextureBuffer* buf) {
        auto mgr = weak_mgr.lock();
        if (mgr) {
            mgr->EmplaceCacheBuffer(buf);
        } else {
            delete buf;
        }
    }
);
```
- 是智能指针，实现共享所有权
- 特点：
  - 自动管理对象的生命周期
  - 使用引用计数
  - 支持自定义删除器
  - 线程安全的引用计数
- 使用场景：
  - 多个对象共享同一个资源时
  - 需要自动管理资源生命周期时
  - 需要自定义资源释放行为时

在我们的双缓冲区实现中的具体应用：

```cpp
std::shared_ptr<GlTextureBuffer> DoubleGraphBufferMgr::GetDrawBuffer()
{
    // 1. 使用 unique_lock 保护共享资源访问
    std::unique_lock<std::mutex> lock(_mutex);
    
    // 2. 等待直到有可用的缓冲区
    _draw_cv.wait(lock, [this]() { return !_draw_available.empty(); });
    
    // 3. 获取缓冲区
    auto buffer = std::move(_draw_available.front());
    _draw_available.pop_front();
    buffer->Reuse();

    // 4. 创建 weak_ptr 避免循环引用
    std::weak_ptr<DoubleGraphBufferMgr> weak_mgr = shared_from_this();
    
    // 5. 创建 shared_ptr 并设置自定义删除器
    return std::shared_ptr<GlTextureBuffer>(
        buffer.release(),
        [weak_mgr](GlTextureBuffer* buf) {
            auto mgr = weak_mgr.lock();
            if (mgr) {
                mgr->EmplaceCacheBuffer(buf);
            } else {
                delete buf;
            }
        }
    );
}
```

这三个工具的组合使用实现了：
1. 线程安全的资源访问（通过 `unique_lock`）
2. 自动的资源生命周期管理（通过 `shared_ptr`）
3. 避免循环引用（通过 `weak_ptr`）
4. 自定义的资源释放行为（通过 `shared_ptr` 的自定义删除器）

这种组合使得我们的双缓冲区实现既安全又高效，同时代码也相对简洁和易于维护。

### lock_guard和unique_lock
让我详细解释 `lock_guard` 和 `unique_lock` 的区别：

1. **lock_guard**:
```cpp
std::lock_guard<std::mutex> lock(_mutex);
```
- 特点：
  - 最简单的 RAII 锁管理工具
  - 构造时加锁，析构时解锁
  - 不支持手动加锁和解锁
  - 不支持条件变量的等待操作
  - 不支持移动语义
- 使用场景：
  - 简单的加锁解锁操作
  - 不需要手动控制锁
  - 不需要配合条件变量
  - 不需要转移锁的所有权

2. **unique_lock**:
```cpp
std::unique_lock<std::mutex> lock(_mutex);
```
- 特点：
  - 更灵活的 RAII 锁管理工具
  - 构造时加锁，析构时解锁
  - 支持手动加锁和解锁（`lock()` 和 `unlock()`）
  - 支持条件变量的等待操作（`wait`）
  - 支持移动语义
  - 支持延迟加锁（`defer_lock`）
  - 支持尝试加锁（`try_lock`）
- 使用场景：
  - 需要配合条件变量
  - 需要手动控制锁
  - 需要转移锁的所有权
  - 需要延迟加锁
  - 需要尝试加锁

在我们的双缓冲区实现中，我们使用 `unique_lock` 是因为需要配合条件变量：

```cpp
std::shared_ptr<GlTextureBuffer> DoubleGraphBufferMgr::GetDrawBuffer()
{
    // 使用 unique_lock 因为需要配合条件变量
    std::unique_lock<std::mutex> lock(_mutex);
    
    // 等待直到有可用的缓冲区
    _draw_cv.wait(lock, [this]() { return !_draw_available.empty(); });
    
    // ... 其他操作 ...
}
```

如果不需要条件变量，我们可以使用更简单的 `lock_guard`：

```cpp
void DoubleGraphBufferMgr::EmplaceCacheBuffer(GlTextureBuffer* buffer)
{
    if (!buffer) {
        throw std::invalid_argument("Cannot emplace null buffer");
    }

    // 使用 lock_guard 因为只需要简单的加锁解锁
    std::lock_guard<std::mutex> lock(_mutex);
    _cache_available.emplace_back(std::unique_ptr<GlTextureBuffer>(buffer));
    _cache_cv.notify_one();
}
```

主要区别总结：
1. **功能范围**：
   - `lock_guard`: 简单的加锁解锁
   - `unique_lock`: 更丰富的功能，包括条件变量、手动控制等

2. **灵活性**：
   - `lock_guard`: 不灵活，只能构造时加锁，析构时解锁
   - `unique_lock`: 非常灵活，支持多种锁操作

3. **性能**：
   - `lock_guard`: 更轻量，性能更好
   - `unique_lock`: 功能更多，但可能带来一些性能开销

4. **使用场景**：
   - `lock_guard`: 简单的加锁解锁场景
   - `unique_lock`: 需要更复杂锁操作的场景

选择建议：
1. 如果只需要简单的加锁解锁，使用 `lock_guard`
2. 如果需要配合条件变量，使用 `unique_lock`
3. 如果需要手动控制锁，使用 `unique_lock`
4. 如果需要转移锁的所有权，使用 `unique_lock`
5. 如果需要延迟加锁或尝试加锁，使用 `unique_lock`

在我们的双缓冲区实现中，我们主要使用 `unique_lock` 是因为需要配合条件变量来实现缓冲区的等待机制。如果某些方法不需要等待操作，我们可以考虑使用 `lock_guard` 来简化代码。
