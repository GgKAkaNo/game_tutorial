#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>
#include <stdexcept>
#include "DoubleGraphBuffer.h"

using namespace std;

void GlTextureBuffer::Create(int width, int height, GpuBufferFormat format)
{
    // 这里实现具体的纹理缓冲区创建逻辑
    // 实际项目中需要根据具体的图形API来实现
}

DoubleGraphBufferMgr::DoubleGraphBufferMgr(int width, int height, GpuBufferFormat format)
    : _width(width), _height(height), _format(format)
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Invalid buffer dimensions");
    }
    CreateBuffer();
}

DoubleGraphBufferMgr::~DoubleGraphBufferMgr() 
{
    // 清理所有缓冲区
    _draw_available.clear();
    _cache_available.clear();
}

void DoubleGraphBufferMgr::CreateBuffer()
{
    for (int i = 0; i < _buffer_count; i++)
    {
        auto buffer = std::make_unique<GlTextureBuffer>();
        buffer->Create(_width, _height, _format);
        _draw_available.emplace_back(std::move(buffer));
    }
}

std::shared_ptr<GlTextureBuffer> DoubleGraphBufferMgr::GetDrawBuffer()
{
    std::unique_lock<std::mutex> lock(_mutex);
    // 等待直到有可用的绘制缓冲区
    _draw_cv.wait(lock, [this]() { return !_draw_available.empty(); });
    
    auto buffer = std::move(_draw_available.front());
    _draw_available.pop_front();
    buffer->Reuse();

    // 创建weak_ptr指向管理器
    std::weak_ptr<DoubleGraphBufferMgr> weak_mgr = shared_from_this();
    
    // 使用自定义删除器创建shared_ptr，使用完后进入缓存队列
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

std::shared_ptr<GlTextureBuffer> DoubleGraphBufferMgr::GetCacheBuffer()
{
    std::unique_lock<std::mutex> lock(_mutex);
    // 等待直到有可用的缓存缓冲区
    _cache_cv.wait(lock, [this]() { return !_cache_available.empty(); });
    
    auto buffer = std::move(_cache_available.front());
    _cache_available.pop_front();
    buffer->Reuse();

    // 创建weak_ptr指向管理器
    std::weak_ptr<DoubleGraphBufferMgr> weak_mgr = shared_from_this();
    
    // 使用自定义删除器创建shared_ptr，使用完后回到绘制队列
    return std::shared_ptr<GlTextureBuffer>(
        buffer.release(),
        [weak_mgr](GlTextureBuffer* buf) {
            auto mgr = weak_mgr.lock();
            if (mgr) {
                mgr->EmplaceDrawBuffer(buf);
            } else {
                delete buf;
            }
        }
    );
}

void DoubleGraphBufferMgr::EmplaceCacheBuffer(GlTextureBuffer* buffer)
{
    if (!buffer) {
        throw std::invalid_argument("Cannot emplace null buffer");
    }

    std::unique_lock<std::mutex> lock(_mutex);
    _cache_available.emplace_back(std::unique_ptr<GlTextureBuffer>(buffer));
    _cache_cv.notify_one();
}

void DoubleGraphBufferMgr::EmplaceDrawBuffer(GlTextureBuffer* buffer)
{
    if (!buffer) {
        throw std::invalid_argument("Cannot emplace null buffer");
    }

    std::unique_lock<std::mutex> lock(_mutex);
    _draw_available.emplace_back(std::unique_ptr<GlTextureBuffer>(buffer));
    _draw_cv.notify_one();
}

