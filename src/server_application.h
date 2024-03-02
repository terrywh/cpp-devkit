#ifndef DEVKIT_SERVER_H_
#define DEVKIT_SERVER_H_
#include "application.h"
#include "quic/fwd.h"

class ApplicationOptions;
class ServerApplication: public Application {
public:
    ServerApplication(const ApplicationOptions& options);
    ~ServerApplication();
    void Start();
private:
    static QUIC_STATUS OnListenerEvent(
        struct MsQuicListener* listener,
        void* context,
        QUIC_LISTENER_EVENT* event
    );
    static QUIC_STATUS OnConnectionEvent(
        struct MsQuicConnection* Connection,
        void* Context,
        QUIC_CONNECTION_EVENT* Event
    );
    static QUIC_STATUS OnStreamEvent(
        struct MsQuicStream* stream,
        void* context,
        QUIC_STREAM_EVENT* event
    );

    MsQuicListener* listener_;
};

#endif // DEVKIT_SERVER_H_
