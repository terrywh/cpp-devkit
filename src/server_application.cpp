#include "server_application.h"
#include <boost/asio/io_context.hpp>
#include "application_options.h"
#include "quic/msquic-cpp.h"
#include <boost/log/trivial.hpp>
#include <iostream>
#include <string_view>

ServerApplication::ServerApplication(const ApplicationOptions& options)
: Application(options)
, listener_(new MsQuicListener(Context().Registration(), MsQuicCleanUpMode::CleanUpAutoDelete, OnListenerEvent, this)) {
    // settings
    MsQuicSettings settings;
    settings.SetPeerBidiStreamCount(8);
    settings.SetServerResumptionLevel(QUIC_SERVER_RESUMPTION_LEVEL::QUIC_SERVER_RESUME_AND_ZERORTT);
    Context().Configuration().SetSettings(settings);
    // credential
    QUIC_CERTIFICATE_FILE cert;
    cert.CertificateFile = "server.crt";
    cert.PrivateKeyFile = "server.key";
    MsQuicCredentialConfig cred { QUIC_CREDENTIAL_FLAG_NONE };
    cred.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    cred.CertificateFile = &cert;
    Context().Configuration().LoadCredential(&cred);
}

ServerApplication::~ServerApplication() {
    
}

void ServerApplication::Start() {
    QUIC_STATUS status;
    QUIC_ADDR addr;
    QuicAddrFromString(Options().Address().c_str(), Options().Port(), &addr);
    
    if (QUIC_FAILED(status = listener_->Start(Context().Alpn(), &addr))) {
        std::cerr << "failed to listen on address: " << status << "\n";
    } else {
        BOOST_LOG_TRIVIAL(info) << "listening on port: " << QuicAddrGetPort(&addr) << "\n";
    }
}

QUIC_STATUS ServerApplication::OnListenerEvent(
    [[maybe_unused]] _In_ struct MsQuicListener* listener,
    [[maybe_unused]] _In_opt_ void* context,
    _Inout_ QUIC_LISTENER_EVENT* event
) {
    ServerApplication* self = static_cast<ServerApplication*>(context);

    QUIC_STATUS Status = QUIC_STATUS_NOT_SUPPORTED;
    switch (event->Type) {
    case QUIC_LISTENER_EVENT_NEW_CONNECTION: {
        BOOST_LOG_TRIVIAL(debug) << "<Server.Listener.Event> NEW_CONNECTION";
        //
        // A new connection is being attempted by a client. For the handshake to
        // proceed, the server must provide a configuration for QUIC to use. The
        // app MUST set the callback handler before returning.
        //
        MsQuicConnection* conn = new MsQuicConnection(event->NEW_CONNECTION.Connection,
            MsQuicCleanUpMode::CleanUpAutoDelete, // cleanup on QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE
            OnConnectionEvent, self);
        return conn->SetConfiguration(self->Context().Configuration());
    }
    case QUIC_LISTENER_EVENT_STOP_COMPLETE:
        BOOST_LOG_TRIVIAL(debug) << "<Server.Listener.Event> STOP_COMPLETE";
        self->listener_ = nullptr; // CleanUpAutoDelete
        break;
    default:
        break;
    }
    return Status;
}

QUIC_STATUS ServerApplication::OnConnectionEvent(
    struct MsQuicConnection* connection,
    void* context,
    QUIC_CONNECTION_EVENT* event
) {
    auto* self = static_cast<ServerApplication*>(context);
    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
     BOOST_LOG_TRIVIAL(debug) << "<Server.Listener.Event> NEW_CONNECTION: " << connection
            << "\n  alpn: " << std::string_view((const char*)event->CONNECTED.NegotiatedAlpn, event->CONNECTED.NegotiatedAlpnLength);
        connection->SendResumptionTicket(QUIC_SEND_RESUMPTION_FLAG_NONE);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        BOOST_LOG_TRIVIAL(warning) << "<Server.Connection.Event> SHUTDOWN_INITIATED_BY_TRANSPORT: "
            << event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status;
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        BOOST_LOG_TRIVIAL(warning) << "<Server.Connection.Event> SHUTDOWN_INITIATED_BY_PEER: "
            << event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status;
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        BOOST_LOG_TRIVIAL(debug) << "<Server.Connection.Event> SHUTDOWN_COMPLETE: " << connection;
        connection->Close();
    case QUIC_CONNECTION_EVENT_RESUMED:
        BOOST_LOG_TRIVIAL(debug) << "<Server.Connection.Event> RESUMED: " << connection;
        break;
    case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED: {
        BOOST_LOG_TRIVIAL(debug) << "<Server.Connection.Event> PEER_STREAM_START: " << connection << "\n"
            << "  stream = " << event->PEER_STREAM_STARTED.Stream;
        MsQuicStream* stream = new MsQuicStream(event->PEER_STREAM_STARTED.Stream, MsQuicCleanUpMode::CleanUpAutoDelete,
            OnStreamEvent, self);
        break;
    }
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS ServerApplication::OnStreamEvent(
    struct MsQuicStream* stream,
    void* context,
    QUIC_STREAM_EVENT* event
) {
    auto self = static_cast<ServerApplication*>(context);
    switch (event->Type) {
    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        //
        // A previous StreamSend call has completed, and the context is being
        // returned back to the app.
        //
        break;
    case QUIC_STREAM_EVENT_RECEIVE:
        BOOST_LOG_TRIVIAL(debug) << "<Server.Stream.Event> RECEIVE: " << stream << "\n"
            << "  length = " << event->RECEIVE.TotalBufferLength << " offset = " << event->RECEIVE.AbsoluteOffset;
        for (int i=0;i<event->RECEIVE.BufferCount;++i) {
            std::cout << "payload: [" << std::string_view(reinterpret_cast<const char*>(event->RECEIVE.Buffers[i].Buffer), event->RECEIVE.Buffers[i].Length) << "]\n";
        }
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        //
        // The peer gracefully shut down its send direction of the stream.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Server.Stream.Event> PEER_SEND_SHUTDOWN: " << stream;
        break;
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        //
        // The peer aborted its send direction of the stream.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Server.Stream.Event> PEER_SEND_ABORTED: " << stream;
        stream->Shutdown(QUIC_STREAM_SHUTDOWN_FLAG_ABORT);
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        //
        // Both directions of the stream have been shut down and MsQuic is done
        // with the stream. It can now be safely cleaned up.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Server.Stream.Event> SHUTDOWN_COMPLETE: " << stream;
        stream->Close(); // AutoCleanUp
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}
