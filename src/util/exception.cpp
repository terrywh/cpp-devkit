#include "exception.h"
#include <sstream>
#include <boost/stacktrace.hpp>

namespace util {

exception::exception(const std::string info) {
    std::stringstream ss;
    ss << "exception: " << info << ", stacktrace: " << boost::stacktrace::stacktrace();
    data_ = ss.str();
}

const char* exception::what() const noexcept {
    return data_.c_str();
}

} // namespace util