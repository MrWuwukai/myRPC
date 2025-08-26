#include <iostream>
#include <string>
#include "user.pb.h"
#include "MyRPC.h"
#include "MyRPCProvider.h"

// 本地服务 -> 升级成远程调用方法
class UserService : public WWK::UserServiceRpc // WWK: namespace in .proto file
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    // 重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    void Login(::google::protobuf::RpcController *controller,
               const ::WWK::LoginRequest *request,
               ::WWK::LoginResponse *response,
               ::google::protobuf::Closure *done) override
    {
        // 1. 从Request获取参数的值
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 2. 执行本地服务，并获取返回值
        bool login_result = Login(name, pwd);

        // 3. 用上面的返回值填写Response
        WWK::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 4. 一个回调，把Response发回
        done->Run();
    }
};

int main(int argc, char **argv)
{

    // 1. 框架初始化
    MyRPC::Init(argc, argv);

    // 2. provider把UserService对象发布到rpc节点上
    MyRPCProvider provider;
    provider.NotifyService(new UserService());
    // provider.NotifyService(new XX()); // 可发布多个

    // 3. 启动节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    std::cout << "hello" << std::endl;

    return 0;
}