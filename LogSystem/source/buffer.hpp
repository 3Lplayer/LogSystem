#pragma once
#include <iostream>
#include <vector>

namespace wcm
{
#define BUFF_SIZE 10 * 1024 * 1024     // 初始缓冲区大小
#define THRESHOLD 100 * 1024 * 1024    // 阈值,缓冲区大小小于阈值时双倍扩容,大于等于阈值后线性增长
#define LINEAR_GROWTH 10 * 1024 * 1024 // 线性增长大小
    typedef char data_type;            // 数据类型
    class Buffer
    {
    public:
        Buffer()
            : _buff(BUFF_SIZE), _widx(0), _ridx(0)
        {
        }

        // 向_buff插入长度为len的数据
        bool Push(const data_type *data, size_t len)
        {
            // //1.正常发布不允许扩容,缓冲区不够就不允许插入数据了
            // if(len > WriteAbleSize())
            // {
            //     return false;
            // }
            // 2.极限性能测试,允许无限扩容
            Expansion(len);
            std::copy(data, data + len, &_buff[_widx]); // 拷贝数据
            _widx += len;                              // 更新写指针
        }

        // 还能写入的空间大小
        size_t WriteAbleSize()
        {
            return _buff.size() - _widx;
        }

        // 目前能够读取的空间大小
        size_t ReadAbleSize()
        {
            return _widx - _ridx;
        }

        // 判空
        bool Empty()
        {
            return _ridx == _widx;
        }

        // 清空缓冲区
        void Clear()
        {
            _widx = _ridx = 0;
        }

        // 交换缓冲区及读写指针
        void Swap(Buffer &buff)
        {
            std::swap(_buff, buff._buff);
            std::swap(_widx, buff._widx);
            std::swap(_ridx, buff._ridx);
        }

        // 返回可读的起始位置
        const data_type *begin()
        {
            return &_buff[_ridx];
        }

        //移动读指针走len
        void MoveRIDX(size_t len)
        {
            _ridx += len;
        }
    private:
        // 扩容 -- len为这次要插入的数据的长度
        void Expansion(size_t len)
        {
            // 当前空间足够新数据的写入,不需要扩容
            if (len <= WriteAbleSize())
            {
                return;
            }
            // 扩容
            size_t new_size = 0;
            if (_buff.size() < THRESHOLD)
            {
                new_size = _buff.size() * 2 + len; //加len防止双倍扩容时也不够插入新数据
            }
            else
            {
                new_size = _buff.size() + LINEAR_GROWTH + len;
            }
            _buff.resize(new_size);
        }

    private:
        std::vector<data_type> _buff;
        size_t _ridx; // 读指针
        size_t _widx; // 写指针
    };
}