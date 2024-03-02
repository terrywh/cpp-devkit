#include "application_options.h"
#include "client_application.h"
#include "server_application.h"
#include "util/output.h"
#include <iostream>

int main(int argc, char** argv) {
    ApplicationOptions options(argc, argv);
    gsl::owner<Application*> app;
    switch (options.Command()) {
    case ApplicationOptions::Command::Connect:
        app = new ClientApplication(options);
        break;
    case ApplicationOptions::Command::Listen:
        app = new ServerApplication(options);
        break;
    default:
        std::cerr << util::Output::Warning("<ERROR>") << " missing {command}" << std::endl;
        options.PrintUsage();
        return 0;
    }
    app->Run();
    return 0;
}
