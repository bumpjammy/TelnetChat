#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <string>
#include <utility>
#include <netinet/in.h>

class client_handler {
private:
    int client_socket;
    std::string username;
public:
    explicit client_handler(int socket) : client_socket(socket) {}
    void operator()();
    std::string read_from_socket() const;
    void send_to_socket(std::string message) const;
    void chat_loop();
    bool set_username_color(std::string color);
    bool interpret_command(std::string command, std::string args);
};

#endif // CLIENT_HANDLER_HPP
