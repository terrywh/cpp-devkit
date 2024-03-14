#ifndef DEVKIT_QUIC_CONFIGURATION_H_
#define DEVKIT_QUIC_CONFIGURATION_H_
#include "core-fwd.h"

namespace quic {

class configuration {
public:
    operator HQUIC() {
        return handle_;
    }
protected:
    HQUIC handle_;
};
class client_configuration: public configuration {
    client_configuration();
};
class server_configuration: public configuration {
    server_configuration();
};

} // namespace quic

#endif // DEVKIT_QUIC_CONFIGURATION_H_