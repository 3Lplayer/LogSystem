#include "../source/log.hpp"

void TestLog()
{
    wcm::Logger::ptr logger = wcm::GetLogger("Async_Logger"); //获取指定日志器
    logger->debug("%s", "这是一条测试日志");
    logger->info("%s", "这是一条测试日志");
    logger->warn("%s", "这是一条测试日志");
    logger->error("%s", "这是一条测试日志");
    logger->fatal("%s", "这是一条测试日志");

    //使用默认日志器输出
    // DEBUG("%s", "这是一条测试日志");
    // INFO("%s", "这是一条测试日志");
    // WARN("%s", "这是一条测试日志");
    // ERROR("%s", "这是一条测试日志");
    // FATAL("%s", "这是一条测试日志");

    int cnt = 0;
    while (cnt < 30000)
    {
        std::string msg("这是一条测试日志-");
        msg += std::to_string(cnt++);
        //FATAL("%s", msg.c_str());
        logger->fatal("%s", msg.c_str());
    }
}

int main()
{
    std::shared_ptr<wcm::GlobalLoggerBuilder> builder(std::make_shared<wcm::GlobalLoggerBuilder>());
    //设置日志器类型 -- 同步/异步
    builder->BuildType(wcm::LoggerType::Async);
    //日志器名 -- 必须设置
    builder->BuildName("Async_Logger");
    //日志输出等级,大于等于输出等级才输出
    builder->BuildLevel(wcm::levels::WARN);
    //日志信息落地方式 -- 可扩展
    builder->BuildSink<wcm::StdoutSink>();
    builder->BuildSink<wcm::FileSink>("./logsfile/logfile.log");
    builder->BuildSink<wcm::RollFileSink>("./logsfile/rolllogs/roll-", 1024 * 1024);
    builder->Build();

    TestLog();
    return 0;
}