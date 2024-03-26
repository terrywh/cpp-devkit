#ifndef DEVKIT_QUIC_CONFIGURATION_H_
#define DEVKIT_QUIC_CONFIGURATION_H_
#include "core-fwd.h"

namespace quic {

struct configuration {
public:
    static configuration& get() { return *ins_; }
    configuration(bool server);
    ~configuration();
    void close();
    operator HQUIC() { return handle_; }

private:
    static configuration* ins_;
    HQUIC handle_;
};

} // namespace quic

#endif // DEVKIT_QUIC_CONFIGURATION_H_