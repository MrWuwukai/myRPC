#pragma once

#include <unordered_map>
#include <string>

// 配置器
class MyRPCConfig
{
public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char *config_file);
    // 查询配置项信息
    std::string Load(const std::string &key);
    static MyRPCConfig &GetInstance()
    {
        static MyRPCConfig cfg;
        return cfg;
    }

private:
    std::unordered_map<std::string, std::string> m_configMap;
    MyRPCConfig() {}
    ~MyRPCConfig() {}
    MyRPCConfig(const MyRPCConfig &) = delete;
    MyRPCConfig(MyRPCConfig &&) = delete;
    MyRPCConfig &operator=(const MyRPCConfig &) = delete;
    MyRPCConfig &operator=(MyRPCConfig &&) = delete;
};