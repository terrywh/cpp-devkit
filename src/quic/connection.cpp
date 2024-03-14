#include "connection.h"
#include "core.h"
#include "../util/address.h"
#include "../util/exception.h"
#include <msquic.h>
#include <fmt/core.h>
#include <boost/log/trivial.hpp>

namespace quic {

connection::~connection() {
    if (handle_) close();
}

void connection::start(std::string_view address) {
    const auto [host, port] = util::address::split_host_port(address);
    std::string server_name { host };
    QUIC_STATUS status;
    if (QUIC_FAILED(status = core::get()->library->ConnectionStart(handle_, core::get()->client, QUIC_ADDRESS_FAMILY_UNSPEC, server_name.c_str(), port))) {
        throw util::exception(fmt::format("failed to start connection (status = {})", status));
    }
}

void connection::close() {
    core::get()->library->ConnectionClose(handle_);
    handle_ = nullptr;
}

unsigned int connection::handle_event(HQUIC conn, void* ctx, QUIC_CONNECTION_EVENT* evt) {
    connection* self = static_cast<connection*>(ctx);
    switch (evt->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED: {
        BOOST_LOG_TRIVIAL(debug) << "<Client.Connection.Event> CONNECTED: " << conn;
        // auto* stream = new MsQuicStream(*connection, QUIC_STREAM_OPEN_FLAG_NONE, MsQuicCleanUpMode::CleanUpAutoDelete, OnStreamEvent, self);
        // stream->Start();

        // auto* buffer = static_cast<SendBuffer*>(malloc(sizeof(SendBuffer) + 10));
        // buffer->buffer.Buffer = buffer->payload;
        // buffer->buffer.Length = 11;
        // std::memcpy(buffer->payload, "helloworld", 11);
        // stream->Send(&buffer->buffer, 1, QUIC_SEND_FLAG_NONE, buffer);
        break;
    }
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        BOOST_LOG_TRIVIAL(warning) << "<Client.Connection.Event> SHUTDOWN_INITIATED_BY_TRANSPORT: "
            << evt->SHUTDOWN_INITIATED_BY_TRANSPORT.Status << "\n";
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        BOOST_LOG_TRIVIAL(warning) << "<Client.Connection.Event> SHUTDOWN_INITIATED_BY_PEER: "
            << evt->SHUTDOWN_INITIATED_BY_TRANSPORT.Status << "\n";
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        BOOST_LOG_TRIVIAL(debug) << "<Client.Connection.Event> SHUTDOWN_COMPLETE: " << conn
            << " AppCloseInProgress = " << evt->SHUTDOWN_COMPLETE.AppCloseInProgress;
        self->close();
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

} // namespace quic