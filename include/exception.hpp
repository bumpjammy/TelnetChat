#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>

class socket_exception : public std::exception {
public:
    const char* what() const noexcept override;
};

class bind_exception : public std::exception {
public:
    const char* what() const noexcept override;
};

class listen_exception : public std::exception {
public:
    const char* what() const noexcept override;
};

#endif // EXCEPTION_HPP
