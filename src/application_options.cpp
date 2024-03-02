#include "application_options.h"
#include "util/output.h"
#include <boost/program_options.hpp>
// #include <boost/describe.hpp> // 使用 Reflection 机制映射 Command 参数值
#include <iostream>

ApplicationOptions::ApplicationOptions(int argc, char* argv[]) {
    desc_ = new boost::program_options::options_description();
    boost::program_options::positional_options_description cmd;
    cmd.add("command", 1);
    desc_->add_options()
        ("help,h", "显示此帮助信息")
        ("command,c", boost::program_options::value<std::string>()->default_value("listen"), "待执行命令")
        ("address", boost::program_options::value<std::string>(&address_)->default_value("0.0.0.0"), "监听或连接地址")
        ("port", boost::program_options::value<std::uint16_t>(&port_)->default_value(60000), "监听或连接端口");
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
        .options(*desc_).positional(cmd).run(), vm);
    boost::program_options::notify(vm);

    const std::string command = vm["command"].as<std::string>();
    if (command == "listen") command_ = Command::Listen;
    else if (command == "connect") command_ = Command::Connect;
    else command_ = Command::Unknown;

    if (vm.count("help") > 0) {
        PrintUsage();
        exit(0);
    }
}

ApplicationOptions::~ApplicationOptions() {
    delete desc_;
}

void ApplicationOptions::PrintUsage() {
    std::cout << "辅助开发工具，用于 SSH / SHELL 连接，文件传输等\n"
        << util::Output::Info("命令行:\n")
        << "  devkit COMMAND/命令 {OPTIONS/选项}\n"
        << util::Output::Info("命令 (COMMAND):\n")
        << "  listen\t启动服务器（等待客户端连接）\n"
        << "  connect\t启动客户端（连接服务器地址）\n"
        << util::Output::Info("选项 (OPTIONS):\n")
        << *desc_ << std::endl;
}