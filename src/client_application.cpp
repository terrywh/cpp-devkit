#include "client_application.h"
#include <boost/asio/io_context.hpp>
#include "application_options.h"
#include "quic/msquic-cpp.h"
#include <boost/log/trivial.hpp>

ClientApplication::ClientApplication(const ApplicationOptions& options)
: Application(options)
, conn_(new MsQuicConnection(Context().Registration(), MsQuicCleanUpMode::CleanUpAutoDelete, OnConnectionEvent, this)) {
    MsQuicCredentialConfig cred { QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION };
    cred.Type = QUIC_CREDENTIAL_TYPE_NONE;

    MsQuicSettings settings;
    settings.SetKeepAlive(25 * 1000);
    Context().Configuration().SetSettings(settings);
    Context().Configuration().LoadCredential(&cred);
}

ClientApplication::~ClientApplication() {
}

void ClientApplication::Start() {
    QUIC_STATUS status;
    if (QUIC_FAILED(status = conn_->Start(Context().Configuration(), Options().Address().c_str(), Options().Port()))) {
        BOOST_LOG_TRIVIAL(error) << "failed to connect to server: " << status << "\n";
    }
}

struct SendBuffer {
    QUIC_BUFFER       buffer;
    std::uint8_t payload[10];
};

QUIC_STATUS ClientApplication::OnConnectionEvent(
    _In_ struct MsQuicConnection* connection,
    _In_opt_ void* context,
    _Inout_ QUIC_CONNECTION_EVENT* event
) {
    ClientApplication* self = static_cast<ClientApplication*>(context);
    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED: {
        BOOST_LOG_TRIVIAL(debug) << "<Client.Connection.Event> CONNECTED: " << connection;
        auto* stream = new MsQuicStream(*connection, QUIC_STREAM_OPEN_FLAG_NONE, MsQuicCleanUpMode::CleanUpAutoDelete, OnStreamEvent, self);
        stream->Start();

        auto* buffer = static_cast<SendBuffer*>(malloc(sizeof(SendBuffer) + 10));
        buffer->buffer.Buffer = buffer->payload;
        buffer->buffer.Length = 11;
        std::memcpy(buffer->payload, "helloworld", 11);
        stream->Send(&buffer->buffer, 1, QUIC_SEND_FLAG_NONE, buffer);
        break;
    }
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        BOOST_LOG_TRIVIAL(warning) << "<Client.Connection.Event> SHUTDOWN_INITIATED_BY_TRANSPORT: "
            << event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status << "\n";
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        BOOST_LOG_TRIVIAL(warning) << "<Client.Connection.Event> SHUTDOWN_INITIATED_BY_PEER: "
            << event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status << "\n";
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        BOOST_LOG_TRIVIAL(debug) << "<Client.Connection.Event> SHUTDOWN_COMPLETE: " << connection;
        self->conn_ = nullptr; // CleanUpAutoDelete
        if (!event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
            connection->Close();
        }
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}


QUIC_STATUS ClientApplication::OnStreamEvent(
    struct MsQuicStream* stream,
    void* context,
    QUIC_STREAM_EVENT* event
) {
    auto self = static_cast<ClientApplication*>(context);
    switch (event->Type) {
    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        //
        // A previous StreamSend call has completed, and the context is being
        // returned back to the app.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Client.Stream.Event> SEND_COMPLETE";
        free(event->SEND_COMPLETE.ClientContext);
        break;
    case QUIC_STREAM_EVENT_RECEIVE:
        //
        // Data was received from the peer on the stream.
        //
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        //
        // The peer gracefully shut down its send direction of the stream.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Client.Stream.Event> PEER_SEND_SHUTDOWN: " << stream;
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        //
        // The peer aborted its send direction of the stream.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Client.Stream.Event> PEER_SEND_ABORTED: " << stream;
        stream->Shutdown(QUIC_STREAM_SHUTDOWN_FLAG_ABORT);
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        //
        // Both directions of the stream have been shut down and MsQuic is done
        // with the stream. It can now be safely cleaned up.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Client.Stream.Event> SHUTDOWN_COMPLETE: " << stream;
        if (!event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
            stream->Close(); // AutoCleanUp
        }
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}
