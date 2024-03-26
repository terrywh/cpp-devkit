#ifndef DEVKIT_QUIC_CONNECTION_H_
#define DEVKIT_QUIC_CONNECTION_H_
#include "core-fwd.h"
#include <gsl/zstring>

namespace quic {

class connection {
public:
    connection();
    connection(HQUIC handle);
    ~connection();
    void start(gsl::czstring server, gsl::czstring local = "");
    void close();
private:
    HQUIC handle_;
    static unsigned int handle_event(HQUIC conn, void* ctx, QUIC_CONNECTION_EVENT* evt);

    friend struct core;
    friend class listener;
};

} // namespace quic

#endif // DEVKIT_QUIC_CONNECTION_H_