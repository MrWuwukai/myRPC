#include <iostream>

#include "MyRPC.h"
#include "MyRPCChannel.h"
#include "MyRPCController.h"
#include "user.pb.h"

// 调用的发起方，继承stab必须使用channel构造，里面的方法就是channel的callmethod方法
// 无论调用什么方法，最终都会被包装进callmethod里面序列化之后发出去
int main(int argc, char **argv)
{

    // 1. 框架初始化
    MyRPC::Init(argc, argv);

    // 2. 创建Stub对象，传入channel
    WWK::UserServiceRpc_Stub stub(new MyRPCChannel());

    // 3. 创建请求、请求参数、创建响应、创建Controller（可选）
    WWK::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    WWK::LoginResponse response;

    MyRPCController controller;

    // 4. 发起调用，其本质为MprpcChannel::callmethod
    stub.Login(&controller, &request, &response, nullptr);

    // 5. 一次rpc调用完成，从response读调用的结果

    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc login response :" << response.success() << std::endl;
        }
        else
        {
            std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}