#include "configuration.h"
#include "core.h"
#include "../util/exception.h"
#include <msquic.h>
#include <fmt/core.h>

namespace quic {

static void init_client(HQUIC* phandle);
static void init_server(HQUIC* phandle);

configuration* configuration::ins_ = nullptr;

configuration::configuration(bool server) {
    server ? init_server(&handle_) : init_client(&handle_);
    ins_ = this;
}

configuration::~configuration() {
    close();
}

void configuration::close() {
    if (handle_) {
        core::get()->library->ConfigurationClose(handle_);
        handle_ = nullptr;
    }
}

static void init_client(HQUIC* phandle) {
    QUIC_STATUS status;
    QUIC_SETTINGS settings;
    settings.IsSetFlags = 0;
    settings.PeerBidiStreamCount = 8;
    settings.IsSet.PeerBidiStreamCount = true;
    settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
    settings.IsSet.ServerResumptionLevel = true;
    if (QUIC_FAILED(status = core::get()->library->ConfigurationOpen(core::get()->registration,
            &core::get()->protocol, 1, &settings, sizeof(QUIC_SETTINGS), nullptr, phandle))) {
        throw util::exception(fmt::format("failed to open configuration with status = {}", status));
    }

    QUIC_CREDENTIAL_CONFIG cred;
    std::memset(&cred, 0, sizeof(QUIC_CREDENTIAL_CONFIG));
    cred.Flags = QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;
    cred.Type = QUIC_CREDENTIAL_TYPE_NONE;
    if (QUIC_FAILED(status = core::get()->library->ConfigurationLoadCredential(*phandle, &cred))) {
        throw util::exception(fmt::format("failed to load credential (status = {})", status));
    }
}

static void init_server(HQUIC* phandle) {
    QUIC_STATUS status;
    QUIC_SETTINGS settings;
    settings.IsSetFlags = 0;
    settings.KeepAliveIntervalMs = 25 * 1000;
    settings.IsSet.KeepAliveIntervalMs = true;

    if (QUIC_FAILED(status = core::get()->library->ConfigurationOpen(core::get()->registration,
            &core::get()->protocol, 1, &settings, sizeof(QUIC_SETTINGS), nullptr, phandle))) {
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
    if (QUIC_FAILED(status = core::get()->library->ConfigurationLoadCredential(*phandle, &cred))) {
        throw util::exception(fmt::format("failed to load credential with status = {}", status));
    }
}

} // namespace quic
