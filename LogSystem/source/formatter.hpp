#pragma once
#include <iostream>
#include <memory>
#include <ctime>
#include <vector>
#include <sstream>
#include <cassert>
#include "message.hpp"

// 控制日志格式化输出:%d--日期, %t--缩进, %T--线程id, %p--日志等级, %c--日志器名称, %f--文件名, %l--行号, %m--有效载荷, %n--换行
namespace wcm
{
    class FormatterItem
    {
    public:
        using ptr = std::shared_ptr<FormatterItem>;
        virtual void Output(std::ostream &out, const LogMsg &msg) = 0;
    };

    // 输出日期
    class DateFormatterItem : public FormatterItem
    {
    public:
        DateFormatterItem(const std::string &fmt = "%H:%M:%S")
            : _fmt(fmt)
        {
        }

        void Output(std::ostream &out, const LogMsg &msg) override
        {
            struct tm tm;
            localtime_r(&msg._time, &tm); // 将时间戳格式化进结构体tm
            char s[128] = {'\0'};
            strftime(s, 127, _fmt.c_str(), &tm); // 将struct tm按照指定格式_fmt输出
            out << s;
        }

    private:
        std::string _fmt; // 控制时间输出格式
    };

    // 输出缩进
    class TabFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << "\t";
        }
    };

    // 输出线程id
    class TidFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << msg._tid;
        }
    };

    // 输出日志等级
    class LevelFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << LevelStr(msg._level);
        }
    };

    // 输出日志器名称
    class LoggerFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << msg._logger_name;
        }
    };

    // 输出文件名
    class FileFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << msg._file;
        }
    };

    // 输出行号
    class LineFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << msg._line;
        }
    };

    // 输出有效载荷
    class PayloadFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << msg._payload;
        }
    };

    // 输出换行
    class NLineFormatterItem : public FormatterItem
    {
    public:
        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << "\n";
        }
    };

    // 输出其他字符
    class OtherFormatterItem : public FormatterItem
    {
    public:
        OtherFormatterItem(std::string other)
            : _other(other)
        {
        }

        void Output(std::ostream &out, const LogMsg &msg)
        {
            out << _other;
        }

    private:
        std::string _other;
    };

    // 根据默认格式化组织日志消息
    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern = "[%c][%d{%H:%M:%S}][%p][%f:%l][%T] %m%n")
            : _pattern(pattern)
        {
            assert(ParsePattern());
        }

        // 将日志信息以字符串的形式返回
        std::string Output(const LogMsg &msg)
        {
            std::stringstream ss;
            for (auto &it : _items)
            {
                it->Output(ss, msg);
            }
            return ss.str();
        }

        // 将日志信息直接输出
        std::ostream &Output(std::ostream &out, const LogMsg &msg)
        {
            for (auto &it : _items)
            {
                it->Output(out, msg);
            }
            return out;
        }

    private:
        // 解析_pattern,将对应格式化字符的类对象添加到_items
        // aaa%%[%d{%H:%M:%S}][%c] %m%n
        bool ParsePattern()
        {
            int i = 0;
            std::string key;
            std::string val;
            while (i < _pattern.size())
            {
                // 遇到其他字符
                if (_pattern[i] != '%')
                {
                    val += _pattern[i++];
                    continue;
                }
                // 表示%号后面没跟任何格式化字符,表示出错
                if (i + 1 == _pattern.size())
                {
                    std::cerr << "'%'后面没有格式化字符." << std::endl;
                    return false;
                }
                // 遇到%%,意为输出一个%
                if (i + 1 < _pattern.size() && _pattern[i + 1] == '%')
                {
                    val += '%';
                    i += 2;
                    continue;
                }
                // 走到这表示遇到格式化字符,先将之前的其他字符添加到_items
                if (key.empty() && !val.empty())
                {
                    _items.push_back(CreateItem(key, val));
                    val.clear();
                }
                i++; // 表示跳过%,直接到格式化字符的位置
                // 如果是格式化字符d,需要取其时间子格式
                if (_pattern[i] == 'd')
                {
                    key += _pattern[i];
                    i++;
                    // 如果格式化字符d后面没有跟{,那么该格式就是错误的
                    if (_pattern[i] != '{')
                    {
                        std::cerr << "'%d'格式错误,后面没有跟上正确的子格式." << std::endl;
                        return false;
                    }
                    // 走到这里表示i位置是{,跳过
                    i++;
                    while (i < _pattern.size() && _pattern[i] != '}')
                    {
                        val += _pattern[i++];
                    }
                    // 表示走到尾了都没有遇到},子格式出错了
                    if (i == _pattern.size())
                    {
                        std::cerr << "'%d'格式错误,后面没有跟上正确的子格式." << std::endl;
                        return false;
                    }
                    // 走到这表示遇到},跳过
                    i++;
                    _items.push_back(CreateItem(key, val)); // 添加到_items
                    // 清空数据,以免影响后续的数据
                    key.clear();
                    val.clear();
                }
                // 是其他格式化字符
                else
                {
                    key += _pattern[i];
                    i++;
                    _items.push_back(CreateItem(key, val)); // 添加到_items
                    // 清空数据,以免影响后续的数据
                    key.clear();
                }
            }
            return true;
        }

        // 根据格式化字符创建对应类
        FormatterItem::ptr CreateItem(const std::string &key, const std::string &val)
        {
            if (key == "d")
                return FormatterItem::ptr(new DateFormatterItem(val));
            if (key == "t")
                return FormatterItem::ptr(new TabFormatterItem());
            if (key == "T")
                return FormatterItem::ptr(new TidFormatterItem());
            if (key == "p")
                return FormatterItem::ptr(new LevelFormatterItem());
            if (key == "c")
                return FormatterItem::ptr(new LoggerFormatterItem());
            if (key == "f")
                return FormatterItem::ptr(new FileFormatterItem());
            if (key == "l")
                return FormatterItem::ptr(new LineFormatterItem());
            if (key == "m")
                return FormatterItem::ptr(new PayloadFormatterItem());
            if (key == "n")
                return FormatterItem::ptr(new NLineFormatterItem());
            if (key.empty())
                return FormatterItem::ptr(new OtherFormatterItem(val));
            std::cerr << "无效的格式化设置: '%%" << key << "'." << std::endl; //表示设置了无效的格式化字符
            abort();
        }

    private:
        std::string _pattern;                   // 格式化控制输出字符串
        std::vector<FormatterItem::ptr> _items; // 按顺序输出_items里的内容
    };
}