#include "application_options.h"
#include "util/output.h"
#include <boost/program_options.hpp>
// #include <boost/describe.hpp> // 使用 Reflection 机制映射 Command 参数值
#include <iostream>

application_options::application_options(int argc, char* argv[]) {
    ins_ = this;

    desc_ = new boost::program_options::options_description();
    boost::program_options::positional_options_description cmd;
    cmd.add("command", 1);
    desc_->add_options()
        ("help,h", "显示此帮助信息")
        ("command,c", boost::program_options::value<std::string>()->default_value("listen"), "待执行命令")
        ("address", boost::program_options::value<std::string>(&address_)->default_value("0.0.0.0:60000"), "监听或连接地址");
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
        .options(*desc_).positional(cmd).run(), vm);
    boost::program_options::notify(vm);

    const std::string command = vm["command"].as<std::string>();
    if (command == "listen") {
        command_ = command::listen;
    }
    else if (command == "connect") {
        if (address_ == "0.0.0.0:60000") address_ = "127.0.0.1:60000";
        command_ = command::connect;
    }
    else {
        command_ = command::unknown;
    }

    if (vm.count("help") > 0) {
        print_usage();
        exit(0);
    }
}

application_options::~application_options() {
    delete desc_;
    ins_ = nullptr;
}

void application_options::print_usage() {
    std::cout << "辅助开发工具，用于 SSH / SHELL 连接，文件传输等\n"
        << util::Output::Info("命令行:\n")
        << "  devkit COMMAND/命令 {OPTIONS/选项}\n"
        << util::Output::Info("命令 (COMMAND):\n")
        << "  listen\t启动服务器（等待客户端连接）\n"
        << "  connect\t启动客户端（连接服务器地址）\n"
        << util::Output::Info("选项 (OPTIONS):\n")
        << *desc_ << std::endl;
}

application_options* application_options::ins_ = nullptr;
