#include "application_context.h"
#include "quic/msquic-cpp.h"

const MsQuicApi* MsQuic;

ApplicationContext::ApplicationContext(gsl::czstring alpn) {
    MsQuic = api_ = new(std::nothrow) MsQuicApi;
    reg_   = new MsQuicRegistration(true);
    alpn_  = new MsQuicAlpn(alpn);
    cfg_   = new MsQuicConfiguration(*reg_, *alpn_);
}

ApplicationContext::~ApplicationContext() {
    delete cfg_;
    delete alpn_;
    delete reg_;
    delete api_;
    MsQuic = nullptr;
}

MsQuicRegistration& ApplicationContext::Registration() {
    return *reg_;
}
MsQuicAlpn& ApplicationContext::Alpn() {
    return *alpn_;
}

MsQuicConfiguration& ApplicationContext::Configuration() {
    return *cfg_;
}