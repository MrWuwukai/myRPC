#include "message.pb.h"
#include "MyRPC.h"
#include "MyRPCProvider.h"

#include <functional>

void MyRPCProvider::NotifyService(::google::protobuf::Service *service)
{
    ServiceInfo serviceInfo;
    // 1. 获取服务信息
    // 获取了服务对象的描述信息
    const ::google::protobuf::ServiceDescriptor *pSD = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pSD->name();
    // 获取服务对象service的方法的数量
    int methodCnt = pSD->method_count();

    // 2. 获取服务的方法，你proto文件里定义的(Login方法...)，记录到serviceInfo中
    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取了服务对象指定下标的服务方法的描述 (抽象描述)
        const google::protobuf::MethodDescriptor *pmethodDesc = pSD->method(i);
        std::string method_name = pmethodDesc->name();
        serviceInfo.m_methodMap.insert({method_name, pmethodDesc});
    }

    // 3. 添加service信息到总表
    serviceInfo.m_service = service;
    m_serviceMap.insert({service_name, serviceInfo});
}

void MyRPCProvider::Run()
{
    // 1. 设置IP和端口号
    std::string ip = MyRPC::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = stoi(MyRPC::GetInstance().GetConfig().Load("rpcserverport"));
    muduo::net::InetAddress address(ip, port);
    std::cout << ip << " " << port << std::endl;

    // 2. 启动TCP服务器
    // 2.1 创建TcpServer对象
    muduo::net::TcpServer server(m_pEventLoop.get(), address, "MyRPCProvider");
    // 2.2 绑定连接回调和消息读写回调方法 分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&MyRPCProvider::connCb, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&MyRPCProvider::messageCb, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));
    // 2.3 设置muduo库的线程数量
    server.setThreadNum(2);

    // 2.4 启动网络服务
    server.start();
    // 3. 阻塞等待被调用
    m_pEventLoop->loop(); // epoll_wait
}

void MyRPCProvider::connCb(const muduo::net::TcpConnectionPtr &conn)
{
    std::cout << "somebody conned" << std::endl;
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

void MyRPCProvider::messageCb(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp ts)
{
    // 在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
    // service name method name args
    // 定义proto的message类型，进行数据头的序列化和反序列化，所有的数据长度都要知道，因为要处理TCP粘包
    // 1. 网络上接收的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 2. 处理TCP粘包，固定设计为前四个字节放数据的长度，按照长度来分隔包
    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // 3. 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的service_name、method_name、args_size
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    MyRPCnamespace::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // 4. 获取args
    std::string args_str = recv_buf.substr(4 + header_size, args_size);
    std::cout << args_str << std::endl;

    // 5. 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    ::google::protobuf::Service *service = it->second.m_service;      // 获取service对象
    const ::google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象

    // 6. 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 7. 生成rpc方法调用的done参数，这是一个Closure回调函数
    ::google::protobuf::Closure *done =
        ::google::protobuf::NewCallback<MyRPCProvider,
                                        const muduo::net::TcpConnectionPtr &,
                                        ::google::protobuf::Message *>(this,
                                                                       &MyRPCProvider::sendbackCb,
                                                                       conn,
                                                                       response);

    // 8. 在框架上根据远端rpc请求,调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}

void MyRPCProvider::sendbackCb(const muduo::net::TcpConnectionPtr &conn, ::google::protobuf::Message *msg)
{
    std::string response_str;
    // 1. response进行序列化
    if (msg->SerializeToString(&response_str))
    {
        // 2. 序列化成功后,通过网络把rpc方法执行的结果发送会rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl;
    }
    // 3. 模拟http的短链接服务,由rpcprovider主动断开连接
    conn->shutdown();
}