#ifndef DEVKIT_QUIC_CONTEXT_H_
#define DEVKIT_QUIC_CONTEXT_H_
#include "configuration.h"
#include "connection.h"
#include "listener.h"
#include <msquic.h>
#include <gsl/zstring>
#include <memory>

namespace quic {
struct core {
    const QUIC_API_TABLE* library;
    HQUIC   registration;
    QUIC_BUFFER protocol;

    core(gsl::czstring name);
    ~core();
    static core* get() {
        return ins_;
    }

private:
    void close();
    static core* ins_;
};

} // namespace quic

#endif // DEVKIT_QUIC_CONTEXT_H_
