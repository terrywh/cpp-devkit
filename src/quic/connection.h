#ifndef DEVKIT_QUIC_CONNECTION_H_
#define DEVKIT_QUIC_CONNECTION_H_
#include "core-fwd.h"
#include <string_view>

namespace quic {

class connection {
public:
    connection() = default;
    connection(HQUIC handle)
    : handle_(handle) {}
    ~connection();
    void start(std::string_view address);
    void close();
private:
    HQUIC handle_;
    static unsigned int handle_event(HQUIC conn, void* ctx, QUIC_CONNECTION_EVENT* evt);

    friend struct core;
    friend class listener;
};

} // namespace quic

#endif // DEVKIT_QUIC_CONNECTION_H_