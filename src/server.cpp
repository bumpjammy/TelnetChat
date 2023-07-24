#define _BSD_SOURCE
#include "../include/server.hpp"
#include "../include/exception.hpp"
#include "../include/handle.hpp"
#include "../include/client_handler.hpp"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <csignal>

// Global/static server instance pointer
server* server::instance = nullptr;

server::server(int port) {
    instance = this;
    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE signals

    // Create a new TCP/IP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw socket_exception(); // Throw an exception if unable to create socket
    }

    int optval = 1; // enable the option
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port); // Convert the port number to network byte order

    // Bind the newly created socket to the specified IP and port
    if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
        throw bind_exception();  // Throw an exception if binding failed
    }

    // Mark the socket so it will listen for incoming connections
    if (listen(sockfd, 5) < 0) {
        throw listen_exception();  // Throw an exception if unable to put the server socket in listening mode
    }
}

// Destructor for the server class
server::~server() {
    // Close all connected client sockets
    for(auto& client : clients) {
        close(client);
    }
    // Close the main server socket
    close(sockfd);
    std::cout << "Server stopped" << std::endl;
}

void server::remove_client(int socket) {
    send_to_all(usernames[socket] + " has left the chat\n");
    // Ensure thread safety when removing a client
    std::lock_guard<std::mutex> lock(clients_mutex);
    // Remove the client from our list of clients
    clients.remove(socket);
    // Remove the client's username from our list of usernames
    usernames.erase(socket);
    // Close the client socket
    close(socket);
}

void server::add_client(int socket, const std::string& username) {
    // Ensure thread safety when adding a new client
    std::lock_guard<std::mutex> lock(server::instance -> clients_mutex);
    // Add the client to our list of clients
    server::instance -> clients.push_back(socket);
    // Add the client's username to our list of usernames
    server::instance -> usernames[socket] = username;
}

void server::run() {
    // Create a signal handler for graceful server shutdown
    handle signal_handler;
    std::cout << "Server started" << std::endl;

    // Run the server loop until a stop signal is received
    while (!signal_handler.stop_requested()) {
        // Basic setup for the select() call
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sockfd, &set);  // Add our main server socket to the set

        // Set timeout for select()
        struct timespec timeout = {1, 0};

        // Wait for a client to make a connection
        int result = pselect(sockfd+1, &set, nullptr, nullptr, &timeout, nullptr);

        if(result == -1 && errno == EINTR) break;  // If an interrupt signal occurred, break the loop and quit
        if(result <= 0) continue;  // If no client connected, continue at the start of the loop

        // Set up struct to hold client address information
        sockaddr_in cli_addr {};
        socklen_t clilen = sizeof(cli_addr);  // Size of client address

        // Accept a connection from a client
        int newsockfd = accept(sockfd, reinterpret_cast<sockaddr *>(&cli_addr), &clilen);
        if (newsockfd < 0) {  // If the connection failed for any reason
            std::cerr << "Cannot accept connection: " << strerror(errno) << std::endl;
            continue;
        }

        {
            // Ensure thread safety when adding a new client connection
            std::lock_guard<std::mutex> lock(clients_mutex);
        }

        // Create a new thread for each client
        client_handler ch(newsockfd);
        std::thread client_thread(ch);
        // Don't block on this thread
        client_thread.detach();
    }
}

void server::send_to_all(const std::string &message) {
    // Ensure thread safety when iterating through the clients
    std::lock_guard<std::mutex> lock(clients_mutex);
    for(auto& client : clients) {
        std::cout << "Sending message to client " << client << ": '" << message << "'" << std::endl;
        std::string clear_line = "\r\033[K";
        // Write the message to each client, check if it fails
        if (write(client, clear_line.c_str(), clear_line.length()) < 0) {
            std::cerr << "Failed to send message to client on socket " << client << ": " << strerror(errno) << std::endl;
        }
        if (write(client, message.c_str(), message.length()) < 0) {
            std::cerr << "Failed to send message to client on socket " << client << ": " << strerror(errno) << std::endl;
        }
        std::string prompt = usernames[client] + ": ";
        if (write(client, prompt.c_str(), prompt.length()) < 0) {
            std::cerr << "Failed to send message to client on socket " << client << ": " << strerror(errno) << std::endl;
        }
    }
}

void server::send_to_all(const std::string &message, int exclude) {
    // Ensure thread safety when iterating through the clients
    std::lock_guard<std::mutex> lock(clients_mutex);
    for(auto& client : clients) {
        if(client == exclude) continue;
        std::cout << "Sending message to client " << client << ": '" << message << "'" << std::endl;
        std::string clear_line = "\r\033[K";
        // Write the message to each client, check if it fails
        if (write(client, clear_line.c_str(), clear_line.length()) < 0) {
            std::cerr << "Failed to send message to client on socket " << client << ": " << strerror(errno) << std::endl;
            remove_client(client);
        }
        if (write(client, message.c_str(), message.length()) < 0) {
            std::cerr << "Failed to send message to client on socket " << client << ": " << strerror(errno) << std::endl;
            remove_client(client);
        }
        std::string prompt = usernames[client] + ": ";
        if (write(client, prompt.c_str(), prompt.length()) < 0) {
            std::cerr << "Failed to send message to client on socket " << client << ": " << strerror(errno) << std::endl;
            remove_client(client);
        } //TODO: extract this into a function
    }
}