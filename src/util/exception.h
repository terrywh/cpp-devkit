#ifndef DEVKIT_QUIC_EXCEPTION_H_
#define DEVKIT_QUIC_EXCEPTION_H_
#include <exception>
#include <string>

namespace util {

class exception: public std::exception {
    std::string data_;
public:
    exception(const std::string info);
    virtual const char* what() const noexcept;
};

} // namespace util

#endif // DEVKIT_QUIC_EXCEPTION_H_