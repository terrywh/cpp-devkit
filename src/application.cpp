#include "application.h"
#include <boost/asio/io_context.hpp>

Application::Application(const ApplicationOptions& options)
: io_(std::make_unique<boost::asio::io_context>())
, context_("devkit-p2p")
, options_(options) {}

void Application::Run() {
    io_->run();
}