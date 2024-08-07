#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

namespace wcm
{
    // 获取当前时间戳
    time_t Time()
    {
        // time返回值time_t,强转一下
        return time(nullptr);
    }

    // 判断文件是否存在
    bool Exisit(const std::string &file)
    {
        // //1.使用access函数,设置F_OK判断文件是否存在
        // return access(str.c_str(), F_OK) == 0;
        // 2.使用stat函数,能否获取文件的struct stat属性来判断文件是否存在
        struct stat buf;
        return stat(file.c_str(), &buf) == 0;
    }

    // 获取文件所在目录 -- ./a/b/c/d.cpp -> ./a/b/c
    std::string Path(const std::string &path)
    {
        int pos = path.find_last_of("/\\"); // Linux和Win的目录分隔符不一致
        // 表示不存在'/'或'\',则表示当前所处目录为./
        if (pos == std::string::npos)
            return ".";
        return path.substr(0, pos + 1);
    }

    // 创建目录
    void CreateDir(const std::string &path)
    {
        umask(0); // 先设置默认掩码为0
        if (path.empty())
            return;
        std::string cur_path = Path(path);
        if (Exisit(cur_path))
            return;
        int pos = 0; // 查找到的'/','\'下标
        int idx = 0; // 开始查找的下标
        while (idx < cur_path.size())
        {
            pos = cur_path.find_first_of("/\\", idx);
            // 只存在一级目录,直接创建并退出
            if (pos == std::string::npos)
            {
                mkdir(cur_path.c_str(), 0755);
                return;
            }
            std::string dir = cur_path.substr(0, pos);
            // 如果当前目录已经存在或者是不用创建的目录就跳过
            if ((dir == "." || dir == "..") || Exisit(dir))
            {
                idx = pos + 1;
                continue;
            }
            mkdir(dir.c_str(), 0755);
            idx = pos + 1;
        }
    }
}