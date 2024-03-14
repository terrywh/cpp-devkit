#ifndef DEVKIT_PROGRAM_OPTIONS_H_
#define DEVKIT_PROGRAM_OPTIONS_H_
#include <gsl/pointers>
#include <string>
#include <cstdint>

namespace boost::program_options {
    class options_description;
}

class application_options {
public:
    enum class command {
        unknown, listen, connect,
    };
private:
    static application_options* ins_;
    gsl::owner<boost::program_options::options_description*> desc_;

    command     command_;
    std::string address_;
public:
    application_options(int argc, char* argv[]);
    ~application_options();
    static application_options* get() { return ins_; }
    void print_usage();

    command         cmd() const { return command_; }
    std::string address() const { return address_; }
};

#endif // DEVKIT_PROGRAM_OPTIONS_H_
