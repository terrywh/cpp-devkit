#include "listener.h"
#include "core.h"
#include "configuration.h"
#include "connection.h"
#include "../util/exception.h"
#include <fmt/core.h>
#include <boost/log/trivial.hpp>
#include <cstdint>

namespace quic {

listener::listener() {
    QUIC_STATUS status;
    if (QUIC_FAILED(status = core::get()->library->ListenerOpen(core::get()->registration, listener::handle_event, this, &handle_))) {
        throw util::exception(fmt::format("failed to open listener (status = {})", status));
    }
}

listener::~listener() {
    if (handle_) close();
}

void listener::start(gsl::czstring address) {
    QUIC_ADDR addr;
    QuicAddrFromString(address, 0, &addr);
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
        // 由回调流程管理 connection 的生命周期
        auto* escape = new connection(e->NEW_CONNECTION.Connection);
        status = core::get()->library->ConnectionSetConfiguration(e->NEW_CONNECTION.Connection, configuration::get());
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
