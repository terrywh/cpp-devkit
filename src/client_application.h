#ifndef DEVKIT_CLIENT_H_
#define DEVKIT_CLIENT_H_
#include "application.h"
#include "quic/fwd.h"
#include <gsl/zstring>

class ApplicationOptions;
class ClientApplication: public Application {
public:
    ClientApplication(const ApplicationOptions& options);
    ~ClientApplication();
    void Start();
private:
    static QUIC_STATUS OnConnectionEvent(
        struct MsQuicConnection* connection,
        void* context,
        QUIC_CONNECTION_EVENT* Event
    );
    static QUIC_STATUS OnStreamEvent(
        struct MsQuicStream* stream,
        void* context,
        QUIC_STREAM_EVENT* event
    );
    MsQuicConnection* conn_;
};

#endif // DEVKIT_CLIENT_H_
