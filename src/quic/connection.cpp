#include "connection.h"
#include "core.h"
#include "configuration.h"
#include "../util/address.h"
#include "../util/exception.h"
#include <msquic.h>
#include <fmt/core.h>
#include <boost/log/trivial.hpp>

namespace quic {

connection::connection() {
    QUIC_STATUS status;
    if (QUIC_FAILED(status = core::get()->library->ConnectionOpen(core::get()->registration, connection::handle_event, this, &handle_))) {
        throw util::exception(fmt::format("failed to open connection (status = {})", status));
    }
}

connection::connection(HQUIC handle)
: handle_(handle) {
    core::get()->library->SetCallbackHandler(handle_, reinterpret_cast<void*>(connection::handle_event), this);
}

connection::~connection() {
    if (handle_) close();
}

void connection::start(gsl::czstring address, gsl::czstring local) {
    QUIC_STATUS status;

    if (std::strlen(local) > 0) {
        // const auto [host, port] = util::address::split_host_port(local);
        // std::string bind { host };
        QUIC_ADDR addr;
        if (QuicAddrFromString(local, 0, &addr)) {
            status = core::get()->library->SetParam(handle_, QUIC_PARAM_CONN_LOCAL_ADDRESS, sizeof(QUIC_ADDR), &addr);
            if (QUIC_FAILED(status)) {
                throw util::exception(fmt::format("failed to set connection local address (status = {})", status));
            }
        }
    }

    const auto [host, port] = util::address::split_host_port(address);
    std::string server_name { host };
    
    if (QUIC_FAILED(status = core::get()->library->ConnectionStart(handle_, configuration::get(), QUIC_ADDRESS_FAMILY_UNSPEC, server_name.c_str(), port))) {
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
        QUIC_ADDR     addr;
        std::uint32_t size = sizeof(QUIC_ADDR);
        core::get()->library->GetParam(self->handle_, QUIC_PARAM_CONN_LOCAL_ADDRESS, &size, &addr);
        QUIC_ADDR_STR addrstr;
        QuicAddrToString(&addr, &addrstr);
        BOOST_LOG_TRIVIAL(debug) << "<Client.Connection.Event> LocalAddress: " << addrstr.Address;
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