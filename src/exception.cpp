#include "../include/exception.hpp"

const char* socket_exception::what() const noexcept {
    return "Cannot open socket"; // Message to show when a socket_exception is thrown
}

const char* bind_exception::what() const noexcept {
    return "Cannot bind to port"; // Message to show when a bind_exception is thrown
}

const char* listen_exception::what() const noexcept {
    return "Cannot listen on port"; // Message to show when a listen_exception is thrown
}
