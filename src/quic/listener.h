#ifndef DEVKIT_QUIC_LISTENER_H_
#define DEVKIT_QUIC_LISTENER_H_
#include "core-fwd.h"
#include <string_view>

namespace quic {

class listener {
public:
    listener() = default;
    ~listener();
    void start(std::string_view address);
    void close();
private:
    HQUIC handle_;
    static unsigned int handle_event(HQUIC listener, void* context, QUIC_LISTENER_EVENT* event);
    friend struct core;
};

} // namespace quic

#endif // DEVKIT_QUIC_LISTENER_H_