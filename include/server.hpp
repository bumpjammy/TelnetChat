#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <list>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <thread>
#include "client_handler.hpp"

class server {
private:
    int sockfd; // Socket file descriptor
    sockaddr_in serv_addr; // Server address

public:
    explicit server(int port);

    ~server();

    void remove_client(int socket);

    void add_client(int socket, const std::string& username);

    void run();

    void send_to_all(const std::string& message);
    void send_to_all(const std::string& message, int exclude);

    static server* instance;

    std::unordered_map<int, std::atomic_bool> stop_clients;
    std::unordered_map<int, std::string> usernames;
    std::mutex clients_mutex;
    std::list<int> clients;
};

#endif // SERVER_HPP
