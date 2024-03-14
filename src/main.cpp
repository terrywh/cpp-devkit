#include "application_options.h"
#include "application.h"
#include "quic/core.h"
#include "util/core.h"
#include "util/output.h"
#include <iostream>

int main(int argc, char** argv) {
    quic::core qcore("devkit");
    util::core ucore;
    application_options options(argc, argv);
    std::unique_ptr<application> app {nullptr};
    switch (options.cmd()) {
    case application_options::command::connect:
        app.reset(new client_application());
        break;
    case application_options::command::listen:
        app.reset(new server_application());
        break;
    default:
        std::cerr << util::Output::Warning("<ERROR>") << " missing {command}" << std::endl;
        options.print_usage();
        return 0;
    }
    app->run();
    return 0;
}
