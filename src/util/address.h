#ifndef DEVKIT_UTIL_ADDRESS_H_
#define DEVKIT_UTIL_ADDRESS_H_
#include <string_view>
#include <tuple>
#include <cstdint>

namespace util {

class address {
public:
    static std::tuple<std::string_view, std::uint16_t> split_host_port(std::string_view address);

};
} // namespace util

#endif // DEVKIT_UTIL_ADDRESS_H_
