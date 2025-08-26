#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <unordered_map>

class MyRPCProvider
{
    typedef std::unique_ptr<muduo::net::TcpServer> TcpPtr;
    typedef std::unique_ptr<muduo::net::EventLoop> EventPtr;

public:
    MyRPCProvider() {}
    // 发布RPC服务
    void NotifyService(::google::protobuf::Service *service);

    // 启动服务
    void Run();

private:
    MyRPCProvider::TcpPtr m_pTcpServer;
    MyRPCProvider::EventPtr m_pEventLoop = std::make_unique<muduo::net::EventLoop>();
    // 需要生成一张表，记录服务对象和其发布的所有的服务方法
    // 一个节点有多个service服务，一个服务有多个可调用方法
    struct ServiceInfo
    {
        google::protobuf::Service *m_service; // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;
    };
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    void connCb(const muduo::net::TcpConnectionPtr &);
    void messageCb(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    void sendbackCb(const muduo::net::TcpConnectionPtr &, ::google::protobuf::Message *);
};