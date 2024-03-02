#ifndef DEVKIT_PROGRAM_OPTIONS_H_
#define DEVKIT_PROGRAM_OPTIONS_H_
#include <gsl/pointers>
#include <string>
#include <cstdint>

namespace boost::program_options {
    class options_description;
}

class ApplicationOptions {
public:
    enum class Command {
        Unknown, Listen, Connect,
    };
private:
    gsl::owner<boost::program_options::options_description*> desc_;

    Command     command_;
    std::string address_;
    std::uint16_t  port_;
public:
    ApplicationOptions(int argc, char* argv[]);
    ~ApplicationOptions();
    void PrintUsage();

    Command     Command() const { return command_; }
    std::string Address() const { return address_; }
    std::uint16_t  Port() const { return port_; }
};

#endif // DEVKIT_PROGRAM_OPTIONS_H_
