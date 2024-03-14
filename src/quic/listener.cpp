#include "listener.h"
#include "core.h"
#include "connection.h"
#include "../util/exception.h"
#include <fmt/core.h>
#include <boost/log/trivial.hpp>
#include <cstdint>

namespace quic {

listener::~listener() {
    if (handle_) close();
}

void listener::start(std::string_view address) {
    QUIC_ADDR addr;
    QuicAddrFromString(address.data(), 0, &addr);
    QUIC_STATUS status;
    if (QUIC_FAILED(status = core::get()->library->ListenerStart(handle_, &core::get()->protocol, 1, &addr))) {
        throw util::exception(fmt::format("failed to start listener (status = {})", status));
    }
}

void listener::close() {
    core::get()->library->ListenerClose(handle_);
    handle_ = nullptr;
}

unsigned int listener::handle_event(HQUIC l, void* ctx, QUIC_LISTENER_EVENT* e) {
    listener* self = static_cast<listener*>(ctx);

    QUIC_STATUS status;
    switch (e->Type) {
    case QUIC_LISTENER_EVENT_NEW_CONNECTION: {
        QUIC_ADDR addr;
        std::uint32_t size = sizeof(QUIC_ADDR);
        core::get()->library->GetParam(e->NEW_CONNECTION.Connection, QUIC_PARAM_CONN_REMOTE_ADDRESS, &size, &addr);
        QUIC_ADDR_STR addr_str;
        QuicAddrToString(&addr, &addr_str);

        BOOST_LOG_TRIVIAL(debug) << "<Server.Listener.Event> NEW_CONNECTION: " << addr_str.Address;
        //
        // A new connection is being attempted by a client. For the handshake to
        // proceed, the server must provide a configuration for QUIC to use. The
        // app MUST set the callback handler before returning.
        //
        auto conn = std::make_unique<connection>(e->NEW_CONNECTION.Connection);
        core::get()->library->SetCallbackHandler(e->NEW_CONNECTION.Connection, 
                reinterpret_cast<void*>(connection::handle_event), conn.release());
        status = core::get()->library->ConnectionSetConfiguration(
                e->NEW_CONNECTION.Connection, core::get()->server);
        break;
    }
    case QUIC_LISTENER_EVENT_STOP_COMPLETE:
        BOOST_LOG_TRIVIAL(debug) << "<Server.Listener.Event> STOP_COMPLETE";
        self->close();
        status = QUIC_STATUS_SUCCESS;
        break;
    default:
        status = QUIC_STATUS_NOT_SUPPORTED;
        break;
    }
    return status;
}

} // namespace quic
