```
proto:
protoc XX.proto --cpp_out=./

make and excute:
./autobuild.sh
callee: bin/callee -i bin/rpc.conf
caller: bin/caller -i bin/rpc.conf

git:
git add .
git commit -m ""
git push

发布流程：
callee（provider）
   |              \
   |                    \
   |                          \
service1（User）                 service2（Bag）                   service3（Task） 
   |       \                            |      \
   |           \                        |        \
   |              \                     |          \
method1（登录） method2（注册）   method1（存背包）   method2（检查背包）  ...
1. 本地方法升级成本地RPC方法
  1. 写一个本地方法
  2. 写一个proto文件
  3. 继承XXServiceRpc，升级成本地RPC方法。此方法的参数为controller、request、response、done。在此方法中：
    1. 从Request获取参数的值
    2. 执行本地服务，并获取返回值
    3. 用上面的返回值填写Response
    4. 一个回调，把Response发回
2. 框架初始化
  1. 读取配置信息
3. 发布服务（告诉大家哪些service、哪些method是你要对外暴露的；并且有一个map记录这些信息）
  1. 获取service信息（名字、method数量）
  2. 获取service的methods
  3. 添加service信息到总表
4. 正式启动这个节点，等到被调用
  1. 设置IP和端口号
  2. 启动TCP服务器（muduo）
    1. 创建TcpServer对象
    2. 绑定连接回调（connCb，没用的）和消息读写回调方法（messageCb）
    3. 设置muduo库的线程数量、启动网络服务
  3. 阻塞等待被连接、被调用
5. 等待
6. 远程连接来了，执行messageCb回调，在此回调中：
  1. 接收的远程rpc调用请求的字符流
  2. 处理TCP粘包，固定设计为前四个字节放数据头的长度，按照长度来分隔包
  3. 反序列化数据，从数据头里获取service_name、method_name、args_size
  4. 又根据args_size获取args
  5. 根据远程需要的service_name和method_name，从发布的map里获取service对象和method对象
  6. 生成rpc方法调用的请求request和响应response参数
  7. 生成rpc方法调用的done参数，这是一个Closure回调函数
    1. 这个回调函数里设置的是把结果如何发回
  8. 在框架上根据远端RPC请求，调用本地RPC方法（第一步做的那个）。传入controller、request、response、done
  9. 执行done里的回调函数
    1. response进行序列化
    2. 序列化成功后，通过网络把rpc方法执行的结果发送会rpc的调用方
    3. 由rpcprovider主动断开连接



```