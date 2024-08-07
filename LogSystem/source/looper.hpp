// 异步工作器
#pragma once
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include "buffer.hpp"

namespace wcm
{
    using func_t = std::function<void(Buffer &)>; // 回调函数类型

    // 启用安全状态(不允许扩容)还是不安全状态(允许扩容,用于极限测试)
    enum AsyncType
    {
        SAFE,
        UNSAFE
    };

    class AsyncLooper
    {
    public:
        using ptr = std::shared_ptr<AsyncLooper>;
        AsyncLooper(func_t callback, AsyncType safe = AsyncType::SAFE)
            : _safe(safe), _sflag(false), _callback(callback), _thread(std::thread(&AsyncLooper::ThreadRoutine, this))
        {
        }

        ~AsyncLooper()
        {
            Stop();
        }

        // 停止工作
        void Stop()
        {
            _sflag = true;
            _con_cv.notify_all(); // 唤醒所有消费者线程,做完其应做工作后赶紧退出
            _thread.join();
        }

        void Push(const char *data, size_t len)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 如果输入缓冲区空间还够则允许输入新数据,否则阻塞,等待消费者唤醒
            if (_safe == AsyncType::SAFE)
            {
                _pro_cv.wait(lock, [&]()
                             { return len <= _pro_buffer.WriteAbleSize(); });
            }
            _pro_buffer.Push(data, len);
            _con_cv.notify_one(); // 唤醒一个消费者
        }

    private:
        // 线程入口函数
        void ThreadRoutine()
        {
            while (1)
            {
                // 该作用域用于增加效率,当_buffer交换后就可以解锁了
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    // 只有当停止标志为true并且输入缓冲区为空时才退出,避免剩于数据未被处理
                    if (_sflag && _pro_buffer.Empty())
                    {
                        break;
                    }
                    _con_cv.wait(lock, [&]()
                                 { return !_pro_buffer.Empty() || _sflag; });
                    _con_buffer.Swap(_pro_buffer);

                    // 如果输入缓冲区有数据可读或者已经处于停止状态了,那就赶紧读
                    if (_safe == AsyncType::SAFE)
                    {
                        _pro_cv.notify_one(); // 唤醒生产者可以开始生产了
                    }
                }
                _callback(_con_buffer); // 回调处理
                _con_buffer.Clear();    // 处理完回调后清空缓冲区
            }
        }

    private:
        AsyncType _safe;
        Buffer _pro_buffer; // 输入缓冲区
        Buffer _con_buffer; // 读取缓冲区
        std::mutex _mutex;
        std::condition_variable _pro_cv; // 生产者信号量
        std::condition_variable _con_cv; // 消费者信号量
        std::thread _thread;             // 工作线程
        std::atomic<bool> _sflag;        // 停止标志
        func_t _callback;                // 回调函数
    };
}