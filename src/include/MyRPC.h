#pragma once

#include <memory>

class MyRPC
{
public:
    static void Init(int argc, char **argv);
    static MyRPC &GetInstance()
    {
        static MyRPC app;
        return app;
    }

private:
    // static std::unique_ptr<MyRPCConfig> m_config;
    MyRPC() {}
    ~MyRPC() {}
    MyRPC(const MyRPC &) = delete;
    MyRPC(MyRPC &&) = delete;
    MyRPC &operator=(const MyRPC &) = delete;
    MyRPC &operator=(MyRPC &&) = delete;
};