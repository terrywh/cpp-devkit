#ifndef DEVKIT_IO_H_
#define DEVKIT_IO_H_
#include <boost/asio.hpp>

namespace util {

class core {
public:
    static core* get() { return ins_; }
    core();
    ~core();
    boost::asio::io_context context;
private:
    static core* ins_;
};

} // namespace util

#endif // DEVKIT_IO_H_
