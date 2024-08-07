#pragma once
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <memory>
#include "util.hpp"

namespace wcm
{
    class Sink
    {
    public:
        using ptr = std::shared_ptr<Sink>;
        virtual ~Sink()
        {
        }
        virtual void log(const char *data, size_t len) = 0; // 输出data指向的数据的指定长度len
    };

    // 输出到标准输出
    class StdoutSink : public Sink
    {
    public:
        void log(const char *data, size_t len) override
        {
            std::cout.write(data, len);
        }
    };

    // 输出到指定文件中
    class FileSink : public Sink
    {
    public:
        FileSink(const std::string &path)
            : _path(path)
        {
            wcm::CreateDir(wcm::Path(_path));                   // 如果存储文件所在路径不存在则创建之
            _ofs.open(_path, std::ios::binary | std::ios::app); // 以二进制追加的方式打开指定文件
            assert(_ofs.is_open());
        }

        void log(const char *data, size_t len) override
        {
            _ofs.write(data, len);
            assert(_ofs.good());
        }

    private:
        std::string _path;  // 文件路径
        std::ofstream _ofs; // 管理打开文件的句柄
    };

    // 输出到滚动文件中
    class RollFileSink : public Sink
    {
    public:
        RollFileSink(const std::string &base, size_t capacity, int cnt = 0)
            : _base(base), _capacity(capacity), _size(0), _cnt(cnt)
        {
            std::string file_name = GetBaseName();
            wcm::CreateDir(wcm::Path(file_name));                   // 如果存储文件所在路径不存在则创建之
            _ofs.open(file_name, std::ios::binary | std::ios::app); // 以二进制追加的方式打开指定文件
            assert(_ofs.is_open());
        }

        void log(const char *data, size_t len) override
        {
            // 如果当前滚动文件存满了,需要创建下一个滚动文件继续存储
            if (_size >= _capacity)
            {
                _ofs.close();
                std::string file_name = GetBaseName();
                wcm::CreateDir(wcm::Path(file_name));                   // 如果存储文件所在路径不存在则创建之
                _ofs.open(file_name, std::ios::binary | std::ios::app); // 以二进制追加的方式打开指定文件
                assert(_ofs.is_open());
                _size = 0;
            }
            _ofs.write(data, len);
            assert(_ofs.good());
            _size += len; // 累加大小
        }

    private:
        // 获取滚动文件全名(基础开头 + 扩展结尾)
        std::string GetBaseName()
        {
            struct tm tm;
            time_t t = time(nullptr);
            localtime_r(&t, &tm);
            std::stringstream ss;
            ss << _base;
            ss << tm.tm_year + 1900 << tm.tm_mon + 1 << tm.tm_mday << tm.tm_hour << tm.tm_min << tm.tm_sec << "-" << _cnt++ << ".log";
            return ss.str();
        }

    private:
        std::string _base;  // 基础文件名,需要扩展,如固定为 base- -> base-20240804111820(后面跟上年月日时分秒)
        size_t _capacity;   // 每个滚动文件的总大小
        size_t _size;       // 当前存储的大小
        std::ofstream _ofs; // 管理打开文件的句柄
        size_t _cnt;        //_base扩展的标记,防止在1s内出现多个重复名字的文件
    };

    // 简单工厂
    class SinkFactory
    {
    public:
        template <class SinkType, class... Args>
        static Sink::ptr CreateSink(Args... args)
        {
            return std::make_shared<SinkType>(args...);
        }
    };
}