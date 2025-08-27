#include "MyRPC.h"
#include "MyRPCConfig.h"

#include <iostream>
#include <string>

#include <unistd.h>

MyRPCConfig &MyRPC::m_config = MyRPCConfig::GetInstance();

void ShowArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MyRPC::Init(int argc, char **argv)
{
    if (argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout << "invalid args!" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 开始加载配置文件了
    m_config.LoadConfigFile(config_file.c_str());
    std::cout << "Init over" << std::endl;
}