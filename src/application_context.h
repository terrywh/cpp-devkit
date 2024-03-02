#ifndef DEVKIT_CONTEXT_H_
#define DEVKIT_CONTEXT_H_
// #include "fwd.h"
#include "quic/fwd.h"
#include <gsl/pointers>
#include <gsl/zstring>

class ApplicationContext {
    gsl::owner<MsQuicApi*>           api_;
    gsl::owner<MsQuicRegistration*>  reg_;
    gsl::owner<MsQuicAlpn*>         alpn_;
    gsl::owner<MsQuicConfiguration*> cfg_;

public:
    ApplicationContext(gsl::czstring alpn);
    ~ApplicationContext();

    MsQuicRegistration& Registration();
    MsQuicAlpn& Alpn();
    MsQuicConfiguration& Configuration();
};

#endif // DEVKIT_CONTEXT_H_
