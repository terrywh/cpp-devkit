#ifndef DEVKIT_APPLICATION_H_
#define DEVKIT_APPLICATION_H_
#include "quic/connection.h"
#include "quic/listener.h"
#include <gsl/pointers>
#include <memory>


class application {
    gsl::owner<void*> work_;
public:
    application();
    virtual void   run();
    virtual void close();
};

class server_application: public application {
public:
    server_application();
    ~server_application();
    void run() override;
private:
    std::unique_ptr<quic::listener> listener_;
};

class client_application: public application {
public:
    client_application();
    ~client_application();
    void run() override;
private:
    std::unique_ptr<quic::connection> conn_;
};

#endif // DEVKIT_APPLICATION_H_