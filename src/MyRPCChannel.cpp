#include "message.pb.h"
#include "MyRPC.h"
#include "MyRPCChannel.h"
#include "MyRPCController.h"
#include "MyZookeeper.h"

#include <arpa/inet.h>
#include <errno.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void MyRPCChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                              ::google::protobuf::RpcController *controller,
                              const ::google::protobuf::Message *request,
                              ::google::protobuf::Message *response,
                              ::google::protobuf::Closure *done)
{
    // 1. 从参数中获取service_name、method_name
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();    // service_name
    std::string method_name = method->name(); // method_name

    // 2. 将args序列化，并获取args_size
    int args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        std::cout << "serialize request error!" << std::endl;
        controller->SetFailed("serialize request error!");
        return;
    }

    // 3. 将请求头（service_name、method_name、args_size）序列化，再在最前面添加请求头的长度
    MyRPCnamespace::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        std::cout << "serialize rpc header error!" << std::endl;
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 4. 将最终的请求头的长度、请求头、args都拼接起来，预备发送
    std::string request_str;
    request_str.insert(0, std::string((char *)&header_size, 4)); // header_size
    request_str += rpc_header_str;                               // rpcheader
    request_str += args_str;                                     // args
    std::cout << request_str << std::endl;

    // 5. 原生socket库把请求发出去
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        std::cout << "create socket error! errno:" << errno << std::endl;
        controller->SetFailed("create socket error!");
        exit(EXIT_FAILURE);
    }

    // std::string ip = MyRPC::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = stoi(MyRPC::GetInstance().GetConfig().Load("rpcserverport"));
    // 6. RPC caller想调用service_name的method_name服务，需要查询Zookeeper上该服务所在的IP+端口
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        std::cout << "connect error! errno:" << errno << std::endl;
        controller->SetFailed("connect error!");
        close(clientfd);
        return;
    }

    if (-1 == send(clientfd, request_str.c_str(), request_str.size(), 0))
    {
        std::cout << "send error! errno:" << errno << std::endl;
        controller->SetFailed("send error!");
        close(clientfd);
        return;
    }

    // 6. 接收rpc请求的返回值，反序列化，并写入response
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        std::cout << "recv error! errno:" << errno << std::endl;
        controller->SetFailed("recv error!");
        close(clientfd);
        return;
    }

    std::string response_str(recv_buf, recv_size);
    if (!response->ParseFromString(response_str))
    {
        std::cout << "parse error! response_str:" << response_str << std::endl;
        controller->SetFailed("parse error!");
        close(clientfd);
        return;
    }
    close(clientfd);
    return;
}