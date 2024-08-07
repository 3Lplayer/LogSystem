#include "../source/log.hpp"
#include <chrono>

// 参数:日志器名称,线程数,信息数,每条信息的大小
void bench(const std::string &name, size_t thread_cnt, size_t msg_cnt, size_t msg_size)
{
    wcm::Logger::ptr logger = wcm::LoggerManager::GetInstancce().GetLogger(name); // 获取指定日志器
    assert(logger.get());                                              // 判断是否成功获得日志器

    printf("测试信息:\n[线程个数:%d,日志条数:%d,每条大小:%d字节,总大小:%dMB]\n\n测试开始:\n\n",thread_cnt, msg_cnt, msg_size, msg_cnt * msg_size / 1024 / 1024);
    std::vector<std::thread> threads;   // 管理线程
    size_t cnt = msg_cnt / thread_cnt;  // 每个线程要输出的日志条数
    double max_cost = 0.0;              // 记录各线程中的最大花销
    std::string msg(msg_size - 1, 'S'); // 组织日志有效载荷,留一个位置保存换行
    for (int i = 0; i < thread_cnt; ++i)
    {
        threads.emplace_back([&, i](){
            auto begin = std::chrono::high_resolution_clock::now(); // 记录开始执行时间
            for (int j = 0; j < cnt; ++j)
            {
                logger->fatal("%s", msg.c_str());
            }
            auto end = std::chrono::high_resolution_clock::now(); // 记录结束执行时间
            std::chrono::duration<double> diff = end - begin; //计算执行间隔时间
            max_cost = max_cost > diff.count() ? max_cost : diff.count(); //更新最大时间
            std::cout << "线程" << i << "耗时:" << diff.count() << "s,共输出" << cnt << "条" << std::endl;
        });
    }

    //等待线程
    for(auto& e : threads)
    {
        e.join();
    }

    std::cout << "\n总耗时:" << max_cost << "s" << std::endl;
    std::cout << "每秒输出:" << msg_cnt / max_cost << "条" << std::endl;
    std::cout << "每秒输出大小:" << ((msg_cnt / max_cost) * msg_size) / 1024 / 1024 << "MB" << std::endl;
}

void SyncTest()
{
    std::unique_ptr<wcm::GlobalLoggerBuilder> builder(new wcm::GlobalLoggerBuilder());
    builder->BuildName("sync_logger");
    builder->BuildLevel(wcm::levels::DEBUG);
    builder->BuildSink<wcm::FileSink>("./logfile.log");
    builder->BuildType(wcm::LoggerType::Sync);
    builder->Build();

    bench("sync_logger", 3, 1000000, 100);
}

void AsyncTest()
{
    std::unique_ptr<wcm::GlobalLoggerBuilder> builder(new wcm::GlobalLoggerBuilder());
    builder->BuildName("async_logger");
    builder->BuildLevel(wcm::levels::DEBUG);
    builder->BuildSink<wcm::FileSink>("./logfile.log");
    builder->BuildType(wcm::LoggerType::Async);
    builder->BuildUnSafe();
    builder->Build();

    bench("async_logger", 3, 5000000, 100);
}

int main()
{
    //SyncTest();
    AsyncTest();
    return 0;
}