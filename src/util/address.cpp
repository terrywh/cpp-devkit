#include "address.h"
#include <charconv>

namespace util {

std::tuple<std::string_view, std::uint16_t> address::split_host_port(std::string_view address) {
    const auto x = address.find_last_of(':');
    const auto host = address.substr(0, x);
    const auto port = address.substr(x+1);
    std::uint16_t p;
    std::from_chars(port.data(), port.data() + port.size(), p);
    return {host, p};
}


} // namespace util
