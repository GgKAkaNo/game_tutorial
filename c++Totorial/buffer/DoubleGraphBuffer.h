#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>

using namespace std;
enum class GpuBufferFormat : uint32_t {
  kUnknown = 0,
  kBGRA32  = 1,
};

class GlTextureBuffer
{
public:
    void Create(int width, int height, GpuBufferFormat _format);
    void Reuse() {}
private:
    const int _width = 0;
    const int _height = 0;
    const GpuBufferFormat format_ = GpuBufferFormat::kUnknown;
};

class DoubleGraphBufferMgr
    : public std::enable_shared_from_this<DoubleGraphBufferMgr>
{
public:
    DoubleGraphBufferMgr(int width, int height, GpuBufferFormat format);
    ~DoubleGraphBufferMgr();

    // 获取一个可用的绘制缓冲区，用于渲染操作
    // 如果没有可用的缓冲区，会等待直到有缓冲区可用
    // 使用完后会自动进入缓存队列
    std::shared_ptr<GlTextureBuffer> GetDrawBuffer();
    
    // 获取一个可用的缓存缓冲区，用于离线处理
    // 如果没有可用的缓冲区，会等待直到有缓冲区可用
    // 使用完后会自动回到绘制队列
    std::shared_ptr<GlTextureBuffer> GetCacheBuffer();

private:
    void CreateBuffer();
    // 辅助方法：将缓冲区放入缓存队列
    void EmplaceCacheBuffer(GlTextureBuffer* buffer);
    // 辅助方法：将缓冲区放回绘制队列
    void EmplaceDrawBuffer(GlTextureBuffer* buffer);

private:
    const int _width;
    const int _height;
    const GpuBufferFormat _format;
    const int _buffer_count = 2;  // 双缓冲区，所以数量为2

    std::mutex _mutex;
    std::condition_variable _draw_cv;    // 用于等待可用的绘制缓冲区
    std::condition_variable _cache_cv;   // 用于等待可用的缓存缓冲区

    deque<unique_ptr<GlTextureBuffer>> _draw_available;  // 可用的绘制缓冲区队列
    deque<unique_ptr<GlTextureBuffer>> _cache_available; // 可用的缓存缓冲区队列
};