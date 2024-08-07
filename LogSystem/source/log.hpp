#pragma once
#include "logger.hpp"

namespace wcm
{
    // 获取指定日志器
    Logger::ptr GetLogger(const std::string &name)
    {
        return LoggerManager::GetInstancce().GetLogger(name);
    }

    // 获取默认日志器
    Logger::ptr RootLogger()
    {
        return LoggerManager::GetInstancce().Root();
    }

//宏函数代理
#define debug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define info(fmt, ...) info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define warn(fmt, ...) warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define error(fmt, ...) error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define fatal(fmt, ...) fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

//宏函数默认使用默认日志器输出
#define DEBUG(fmt, ...) wcm::RootLogger()->debug(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) wcm::RootLogger()->info(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) wcm::RootLogger()->warn(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) wcm::RootLogger()->error(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) wcm::RootLogger()->fatal(fmt, ##__VA_ARGS__)
}