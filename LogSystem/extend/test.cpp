#include "../source/log.hpp"

// 扩展落地方式 -- 以时间为间隔
class RollTimeSink : public wcm::Sink
{
public:
    RollTimeSink(const std::string &base, size_t time_gap)
        : _base(base), _time_gap(time_gap)
    {
        _time_no = _time_gap == 1 ? wcm::Time() : wcm::Time() % _time_gap; // 初始化当前滚动文件的序号
        std::string file_name = GetBaseName();
        wcm::CreateDir(wcm::Path(file_name));
        _ofs.open(file_name, std::ios::binary | std::ios::app);
        assert(_ofs.is_open());
    }

    void log(const char *data, size_t len) override
    {
        // 当前滚动文件执行时间已经超过_time_gap,创建新的滚动文件存储
        if (wcm::Time() % _time_gap != _time_no)
        {
            _ofs.close();
            _time_no = _time_gap == 1 ? wcm::Time() : wcm::Time() % _time_gap;
            std::string file_name = GetBaseName();
            wcm::CreateDir(wcm::Path(file_name));
            _ofs.open(file_name, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }
        _ofs.write(data, len);
        assert(_ofs.good());
    }

private:
    // 获得文件全名
    std::string GetBaseName()
    {
        struct tm tm;
        time_t t = wcm::Time();
        localtime_r(&t, &tm);
        std::stringstream ss;
        ss << _base << tm.tm_year + 1900 << tm.tm_mon + 1 << tm.tm_mday << tm.tm_hour << tm.tm_min << tm.tm_sec << ".log";
        return ss.str();
    }

private:
    std::string _base; // 固定前缀
    size_t _time_gap;  // 滚动文件切换时间,单位秒
    size_t _time_no;   // 当前是第几个滚动文件
    std::ofstream _ofs;
};

int main()
{
    std::shared_ptr<wcm::GlobalLoggerBuilder> builder(std::make_shared<wcm::GlobalLoggerBuilder>()); // 使用全局建造模式进行建造
    // 建造日志器的各项属性,日志器名称必须指定,其他可以省略,有默认
    builder->BuildType(wcm::LoggerType::Async);
    builder->BuildName("Async_Logger");
    builder->BuildLevel(wcm::levels::WARN);
    builder->BuildSink<RollTimeSink>("./logsfile/time-", 1);
    wcm::Logger::ptr logger = builder->Build();
    //跑5秒
    time_t old_time = wcm::Time();
    while (wcm::Time() < old_time + 5)
    {
        logger->error("这是一条测试日志...");
        
    }
    return 0;
}