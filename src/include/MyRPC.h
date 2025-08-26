#pragma once

#include <memory>

#include "MyRPCConfig.h"

class MyRPC
{
public:
    static void Init(int argc, char **argv);
    static MyRPC &GetInstance()
    {
        static MyRPC app;
        return app;
    }
    static MyRPCConfig &GetConfig()
    {
        return m_config;
    }

private:
    static MyRPCConfig &m_config;
    MyRPC() {}
    ~MyRPC() {}
    MyRPC(const MyRPC &) = delete;
    MyRPC(MyRPC &&) = delete;
    MyRPC &operator=(const MyRPC &) = delete;
    MyRPC &operator=(MyRPC &&) = delete;
};