#pragma once
#include <iostream>
#include "level.hpp"

// 日志消息组织: [日志器名称][时间][日志等级][文件名:行号][线程id] 有效载荷
namespace wcm
{
    class LogMsg
    {
    public:
        LogMsg(std::string logger_name, time_t time, levels level, std::string file, int line, std::string payload)
            : _logger_name(logger_name), _time(time), _level(level), _file(file), _line(line), _tid(pthread_self()), _payload(payload)
        {
        }

        std::string _logger_name; // 日志器名称
        time_t _time;             // 时间
        levels _level;            // 日志等级
        std::string _file;        // 文件名
        int _line;                // 行号
        pthread_t _tid;           // 线程id
        std::string _payload;     // 有效载荷
    };
}