#include "core.h"
#include "configuration.h"
#include "../util/exception.h"
// #include <msquic.h>
#include <fmt/core.h>
// #include <gsl/zstring>
#include <cstring>

namespace quic {

core::core(gsl::czstring name)
: library(nullptr)
, registration(nullptr)
, protocol(0) {
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
}

core::~core() {
    close();
    ins_ = nullptr;
}

void core::close() {
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
