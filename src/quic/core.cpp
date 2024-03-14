#include "core.h"
#include "../util/exception.h"
// #include <msquic.h>
#include <fmt/core.h>
// #include <gsl/zstring>
#include <cstring>

namespace quic {

HQUIC init_server() {
    QUIC_STATUS status;
    HQUIC config;
    QUIC_SETTINGS settings;
    settings.IsSetFlags = 0;
    settings.KeepAliveIntervalMs = 25 * 1000;
    settings.IsSet.KeepAliveIntervalMs = true;

    if (QUIC_FAILED(status = core::get()->library->ConfigurationOpen(core::get()->registration,
            &core::get()->protocol, 1, &settings, sizeof(QUIC_SETTINGS), nullptr, &config))) {
        throw util::exception(fmt::format("failed to open configuration with status = {}", status));
    }

    QUIC_CREDENTIAL_CONFIG cred;
    std::memset(&cred, 0, sizeof(QUIC_CREDENTIAL_CONFIG));
    cred.Flags = QUIC_CREDENTIAL_FLAG_NONE;
    cred.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;

    QUIC_CERTIFICATE_FILE file;
    file.CertificateFile = "./var/server.cert";
    file.PrivateKeyFile  = "./var/server.pkey";
    cred.CertificateFile = &file;
    if (QUIC_FAILED(status = core::get()->library->ConfigurationLoadCredential(config, &cred))) {
        throw util::exception(fmt::format("failed to load credential with status = {}", status));
    }
    return config;
}

HQUIC init_client() {
    QUIC_STATUS status;
    HQUIC config;
    QUIC_SETTINGS settings;
    settings.IsSetFlags = 0;
    settings.PeerBidiStreamCount = 8;
    settings.IsSet.PeerBidiStreamCount = true;
    settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
    settings.IsSet.ServerResumptionLevel = true;
    if (QUIC_FAILED(status = core::get()->library->ConfigurationOpen(core::get()->registration,
            &core::get()->protocol, 1, &settings, sizeof(QUIC_SETTINGS), nullptr, &config))) {
        throw util::exception(fmt::format("failed to open configuration with status = {}", status));
    }

    QUIC_CREDENTIAL_CONFIG cred;
    std::memset(&cred, 0, sizeof(QUIC_CREDENTIAL_CONFIG));
    cred.Flags = QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;
    cred.Type = QUIC_CREDENTIAL_TYPE_NONE;
    if (QUIC_FAILED(status = core::get()->library->ConfigurationLoadCredential(config, &cred))) {
        throw util::exception(fmt::format("failed to load credential (status = {})", status));
    }
    return config;
}

core::core(gsl::czstring name)
: library(nullptr)
, registration(nullptr)
, protocol(0)
, client(nullptr)
, server(nullptr) {
    ins_ = this;
    QUIC_STATUS status;
    if (QUIC_FAILED(status = MsQuicOpen2(&library))) {
        library = nullptr;
        throw util::exception(fmt::format("failed to initilize library (status = {})", status));
    }
    QUIC_REGISTRATION_CONFIG config;
    if (QUIC_FAILED(status = library->RegistrationOpen(&config, &registration))) {
        registration = nullptr;
        throw std::runtime_error(fmt::format("failed to open registration (status = {})", status));
    }
    protocol.Buffer = const_cast<uint8_t *>(reinterpret_cast<const unsigned char*>(name));
    protocol.Length = std::strlen(name);

    client = init_client();
    server = init_server();
}

core::~core() {
    close();
    ins_ = nullptr;
}

std::unique_ptr<connection> core::create_connection() {
    auto conn = std::make_unique<connection>();
    core::get()->library->ConnectionOpen(core::get()->registration, connection::handle_event, conn.get(), &conn->handle_);
    return std::move(conn);
}

std::unique_ptr<listener> core::create_listener(gsl::czstring cert, gsl::czstring pkey) {
    auto l = std::make_unique<listener>();
    core::get()->library->ListenerOpen(core::get()->registration, listener::handle_event, l.get(), &l->handle_);
    return std::move(l);
}

void core::close() {
    if (client) {
        library->ConfigurationClose(client);
        client = nullptr;
    }
    if (server) {
        library->ConfigurationClose(server);
        server = nullptr;
    }
    if (registration) {
        library->RegistrationClose(registration);
        registration = nullptr;
    }
    if (library) {
        MsQuicClose(library);
        library = nullptr;
    }
}

core* core::ins_ = nullptr;

} // namespace quic
