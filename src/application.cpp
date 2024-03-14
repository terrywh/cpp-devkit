#include "application.h"
#include "application_options.h"
#include "quic/core.h"
#include "util/core.h"
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

using work_guard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

application::application()
: work_(new work_guard(util::core::get()->context.get_executor())) {}

void application::run() {
    util::core::get()->context.run();
}

void application::close() {
    if (work_) {
        delete static_cast<work_guard*>(work_);
        work_ = nullptr;
    }
}

client_application::client_application()
: conn_(quic::core::get()->create_connection()) {
    
}

client_application::~client_application() {
    
}

void client_application::run() {
    conn_->start(application_options::get()->address());
    application::run();
}

server_application::server_application()
: listener_(quic::core::get()->create_listener("./var/server.cert", "./var/server.pkey")) {

}

server_application::~server_application() {
    
}

void server_application::run() {
    listener_->start(application_options::get()->address());
    application::run();
}

QUIC_STATUS OnStreamEvent(
    struct MsQuicStream* stream,
    void* context,
    QUIC_STREAM_EVENT* event
) {
    auto self = static_cast<client_application*>(context);
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
        // stream->Shutdown(QUIC_STREAM_SHUTDOWN_FLAG_ABORT);
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        //
        // Both directions of the stream have been shut down and MsQuic is done
        // with the stream. It can now be safely cleaned up.
        //
        BOOST_LOG_TRIVIAL(debug) << "<Client.Stream.Event> SHUTDOWN_COMPLETE: " << stream;
        if (!event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
            // stream->Close(); // AutoCleanUp
        }
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}
