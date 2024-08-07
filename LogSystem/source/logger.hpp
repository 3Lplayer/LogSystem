#pragma once
#include <iostream>
#include <pthread.h>
#include <atomic>
#include <vector>
#include "level.hpp"
#include "sink.hpp"
#include "formatter.hpp"
#include "message.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <mutex>
#include "looper.hpp"
#include <unordered_map>

namespace wcm
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &name, levels level, const std::vector<Sink::ptr> &sinks)
            : _name(name), _level(level), _sinks(sinks.begin(), sinks.end()), _fmter(std::make_shared<Formatter>())
        {
        }

        void debug(const char *file, size_t line, const std::string &fmt, ...)
        {
            // 判断当前等级是否可以输出
            if (levels::DEBUG < _level)
            {
                return;
            }
            // 获取不定参
            va_list ap;
            va_start(ap, fmt);
            char *res;
            vasprintf(&res, fmt.c_str(), ap);
            va_end(ap);
            LogMsg msg(_name, Time(), levels::DEBUG, file, line, res); // 填充日志消息属性
            std::stringstream ss;
            _fmter->Output(ss, msg);
            log(ss.str().c_str(), ss.str().size());
            free(res); // vasprintf()函数会为res开辟一块存储空间,记得释放
        }

        void info(const char *file, size_t line, const std::string &fmt, ...)
        {
            // 判断当前等级是否可以输出
            if (levels::INFO < _level)
            {
                return;
            }
            // 获取不定参
            va_list ap;
            va_start(ap, fmt);
            char *res;
            vasprintf(&res, fmt.c_str(), ap);
            va_end(ap);
            LogMsg msg(_name, Time(), levels::INFO, file, line, res); // 填充日志消息属性
            std::stringstream ss;
            _fmter->Output(ss, msg);
            log(ss.str().c_str(), ss.str().size());
            free(res); // vasprintf()函数会为res开辟一块存储空间,记得释放
        }

        void warn(const char *file, size_t line, const std::string &fmt, ...)
        {
            // 判断当前等级是否可以输出
            if (levels::WARN < _level)
            {
                return;
            }
            // 获取不定参
            va_list ap;
            va_start(ap, fmt);
            char *res;
            vasprintf(&res, fmt.c_str(), ap);
            va_end(ap);
            LogMsg msg(_name, Time(), levels::WARN, file, line, res); // 填充日志消息属性
            std::stringstream ss;
            _fmter->Output(ss, msg);
            log(ss.str().c_str(), ss.str().size());
            free(res); // vasprintf()函数会为res开辟一块存储空间,记得释放
        }

        void error(const char *file, size_t line, const std::string &fmt, ...)
        {
            // 判断当前等级是否可以输出
            if (levels::ERROR < _level)
            {
                return;
            }
            // 获取不定参
            va_list ap;
            va_start(ap, fmt);
            char *res;
            vasprintf(&res, fmt.c_str(), ap);
            va_end(ap);
            LogMsg msg(_name, Time(), levels::ERROR, file, line, res); // 填充日志消息属性
            std::stringstream ss;
            _fmter->Output(ss, msg);
            log(ss.str().c_str(), ss.str().size());
            free(res); // vasprintf()函数会为res开辟一块存储空间,记得释放
        }

        void fatal(const char *file, size_t line, const std::string &fmt, ...)
        {
            // 判断当前等级是否可以输出
            if (levels::FATAL < _level)
            {
                return;
            }
            // 获取不定参
            va_list ap;
            va_start(ap, fmt);
            char *res;
            vasprintf(&res, fmt.c_str(), ap);
            va_end(ap);
            LogMsg msg(_name, Time(), levels::FATAL, file, line, res); // 填充日志消息属性
            std::stringstream ss;
            _fmter->Output(ss, msg);
            log(ss.str().c_str(), ss.str().size());
            free(res); // vasprintf()函数会为res开辟一块存储空间,记得释放
        }

        virtual void log(const char *data, size_t len) = 0;

    protected:
        std::string _name; // 日志器名
        std::mutex _mutex;
        std::atomic<levels> _level;    // 日志器允许输出等级
        std::vector<Sink::ptr> _sinks; // 落地方式数组
        Formatter::ptr _fmter;
    };

    // 同步日志器
    class SyncLogger : public Logger
    {
    public:
        SyncLogger(const std::string &name, levels level, const std::vector<Sink::ptr> &sinks)
            : Logger(name, level, sinks)
        {
        }

        void log(const char *data, size_t len)
        {
            std::unique_lock<std::mutex> lock(_mutex); // 进入函数自动上锁,出了函数作用域自动解锁
            for (const auto &e : _sinks)
            {
                e->log(data, len);
            }
        }
    };

    // 异步日志器
    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(const std::string &name, levels level, const std::vector<Sink::ptr> &sinks, AsyncType safe = AsyncType::SAFE)
            : Logger(name, level, sinks), _looper(std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::CallBack, this, std::placeholders::_1), safe))
        {
        }

        // 主线程只需要把日志消息push进缓冲区即可
        void log(const char *data, size_t len)
        {
            _looper->Push(data, len);
        }

        // 由异步工作器执行真实的消息落地工作
        void CallBack(Buffer &buffer)
        {
            for (const auto &e : _sinks)
            {
                e->log(buffer.begin(), buffer.ReadAbleSize());
            }
        }

    private:
        AsyncLooper::ptr _looper; // 异步工作器
    };

    // 日志器类型 -- 同步,异步
    enum LoggerType
    {
        Sync,
        Async
    };

    class LoggerBuilder
    {
    public:
        LoggerBuilder()
            : _type(LoggerType::Sync), _level(levels::DEBUG), _safe(AsyncType::SAFE)
        {
        }

        // 各建造部件
        void BuildType(LoggerType type)
        {
            _type = type;
        }

        void BuildName(const std::string &name)
        {
            _name = name;
        }

        void BuildLevel(levels level)
        {
            _level = level;
        }

        template <class SinkType, class... Args>
        void BuildSink(Args &&...args)
        {
            _sinks.push_back(SinkFactory::CreateSink<SinkType>(std::forward<Args>(args)...));
        }

        void BuildFormatter(std::string &pattern)
        {
            _fmter = std::make_shared<Formatter>(pattern);
        }

        void BuildUnSafe()
        {
            _safe = AsyncType::UNSAFE;
        }

        // 建造日志器
        virtual Logger::ptr Build() = 0;

    protected:
        LoggerType _type;              // 日志器类型
        std::string _name;             // 日志器名
        std::atomic<levels> _level;    // 日志器允许输出等级
        std::vector<Sink::ptr> _sinks; // 落地方式数组
        Formatter::ptr _fmter;
        AsyncType _safe; // 异步日志器的工作模式
    };

    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr Build() override
        {
            // 日志器名称不能为空
            if (_name.empty())
            {
                std::cerr << "日志器名称不能为空." << std::endl;
                abort();
            }
            // 落地方式为空,默认将日志消息输出到标准输出
            if (_sinks.empty())
            {
                _sinks.push_back(SinkFactory::CreateSink<StdoutSink>());
            }
            // 格式化器为空,用默认的输出格式
            if (_fmter.get() == nullptr)
            {
                _fmter = std::make_shared<Formatter>();
            }

            if (_type == LoggerType::Async)
            {
                return std::make_shared<AsyncLogger>(_name, _level, _sinks, _safe);
            }
            else
            {
                return std::make_shared<SyncLogger>(_name, _level, _sinks);
            }
        }
    };
    
    //日志器管理类
    class LoggerManager
    {
    public:
        //获取单例对象
        static LoggerManager& GetInstancce()
        {
            //c++11后,局部静态对象没有线程安全问题
            static LoggerManager lm;
            return lm;
        }

        //根据日志器名查看某一日志器是否存在
        bool Count(const std::string& name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _loggers.count(name);
        }

        //添加日志器
        void Push(const std::string& name, const Logger::ptr& logger)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _loggers[name] = logger;
        }

        //根据日志器名获取一个日志器
        Logger::ptr GetLogger(const std::string& name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            //首先判断该日志器是否存在,不存在返回空
            const auto& it = _loggers.find(name);
            if(it != _loggers.end())
            {
                return _loggers[name];
            }
            return Logger::ptr();
        }

        //获取默认日志器
        Logger::ptr Root()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _root;
        }
    private:
        LoggerManager()
        {
            std::unique_ptr<LoggerBuilder> up(new LocalLoggerBuilder());
            up->BuildName("root");
            _root = up->Build();
            _loggers["root"] = _root;
        }

    private:
        std::mutex _mutex;
        Logger::ptr _root; //默认日志器
        std::unordered_map<std::string, Logger::ptr> _loggers; //日志器组
    };

    class GlobalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr Build() override
        {
            // 日志器名称不能为空
            if (_name.empty())
            {
                std::cerr << "日志器名称不能为空." << std::endl;
                abort();
            }
            // 落地方式为空,默认将日志消息输出到标准输出
            if (_sinks.empty())
            {
                _sinks.push_back(SinkFactory::CreateSink<StdoutSink>());
            }
            // 格式化器为空,用默认的输出格式
            if (_fmter.get() == nullptr)
            {
                _fmter = std::make_shared<Formatter>();
            }

            //在用户使用全局建造日志器时,直接将其添加到日志器管理中
            Logger::ptr logger; 
            if (_type == LoggerType::Async)
            {
                logger = std::make_shared<AsyncLogger>(_name, _level, _sinks, _safe);
            }
            else
            {
                logger = std::make_shared<SyncLogger>(_name, _level, _sinks);
            }
            LoggerManager::GetInstancce().Push(_name, logger);
            return logger;
        }
    };
}