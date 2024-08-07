#pragma once
#include <iostream>

namespace wcm
{
    //日志等级 -- 日志等级大于等于设置等级的允许输出
    enum levels
    {
        UNKNOW,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };

    //将输出等级字符串化
    const char* LevelStr(levels l)
    {
        switch(l)
        {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        case OFF: return "OFF";
        }
        return "UNKNOW";
    }
}