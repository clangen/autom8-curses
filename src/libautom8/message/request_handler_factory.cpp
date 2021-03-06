#include <autom8/message/request_handler_factory.hpp>
#include <autom8/net/session.hpp>

using namespace autom8;

using session_ptr = session::session_ptr;
using factory_ptr = request_handler_factory::ptr;

static std::mutex instance_mutex_;

request_handler_factory::request_handler_factory() {
}

factory_ptr request_handler_factory::instance() {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);

    static factory_ptr instance;

    if (!instance) {
        instance = ptr(new request_handler_factory());
    }

    return instance;
}

bool request_handler_factory::handle_request(session_ptr session, message_ptr request) {
    std::unique_lock<decltype(state_mutex_)> lock(state_mutex_);

    typedef request_handler_list::iterator iterator;

    iterator it = request_handlers_.begin();
    iterator end = request_handlers_.end();
    request_handler_ptr handler;
    for ( ; it != end; it++) {
        handler = *it;
        if (handler->can_handle(session, request)) {
            (*handler)(session, request);
            return true;
        }
    }

    return false;
}

void request_handler_factory::register_handler(request_handler_ptr handler) {
    std::unique_lock<decltype(state_mutex_)> lock(state_mutex_);
    request_handlers_.push_back(handler);
}