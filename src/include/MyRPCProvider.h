#pragma once
#include "google/protobuf/service.h"

class MyRPCProvider
{
public:
    MyRPCProvider() {}
    // 发布RPC服务
    void NotifyService(::google::protobuf::Service *service);

    // 启动服务
    void Run();
};