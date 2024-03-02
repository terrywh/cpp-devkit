#ifndef DEVKIT_APPLICATION_H_
#define DEVKIT_APPLICATION_H_
#include "application_context.h"
#include "application_options.h"
#include <gsl/zstring>

namespace boost::asio {
    class io_context;
}

class Application {
public:
    Application(const ApplicationOptions& options);
    virtual ~Application() = default;
    virtual void Run();
protected:
    ApplicationContext& Context() { return context_; }
    const ApplicationOptions& Options() const { return options_; }
    boost::asio::io_context& IoContext() const { return *io_; }

private:
    ApplicationContext context_;
    const ApplicationOptions& options_;
    std::unique_ptr<boost::asio::io_context> io_;
};

#endif // DEVKIT_APPLICATION_H_
