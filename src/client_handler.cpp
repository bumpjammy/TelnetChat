#include "../include/client_handler.hpp"
#include "../include/server.hpp"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

std::string client_handler::read_from_socket() const {
    std::vector<char> buffer(4096);
    ssize_t n = read(client_socket, buffer.data(), buffer.size() - 1);
    if (n < 0) {
        std::cerr << "Error: " << strerror(errno) << '\n';
    }

    std::string message = std::string(buffer.begin(), buffer.begin() + n);
    return message.substr(0, message.find('\n')).substr(0, message.find('\r'));
}

void client_handler::send_to_socket(std::string message) const {
    // Write specified message to the client socket, print error if write operation fails
    if (write(client_socket, message.c_str(), message.size()) < 0) {
        std::cerr << "Cannot write to socket: " << strerror(errno) << std::endl;
        server::instance -> remove_client(client_socket);
    }
}

bool client_handler::set_username_color(std::string color) {
    // Translate color to ANSI escape code
    if (color == "red") {
        color = "\033[91m";
    } else if (color == "green") {
        color = "\033[92m";
    } else if (color == "yellow") {
        color = "\033[93m";
    } else if (color == "blue") {
        color = "\033[94m";
    } else if (color == "magenta") {
        color = "\033[95m";
    } else if (color == "cyan") {
        color = "\033[96m";
    } else if (color == "white") {
        color = "\033[97m";
    } else {
        return false;
    }
    // Remove existing ANSI escape code from username
    username = username.substr(5, username.length() - 5);
    // Add new ANSI escape code to username
    username = color + username + "\033[39m";
    return true;
}

bool client_handler::interpret_command(std::string command, std::string args) {
    if(command == "$exit") {
        return true;
    }
    if(command == "$color") {
        if(set_username_color(args)){
            send_to_socket("Username color changed to " + args + "\n");
        } else {
            send_to_socket("Invalid color\n");
            send_to_socket("Valid colors: red, green, yellow, blue, magenta, cyan, white\n");
        }
        return false;
    }
    if(command == "$help") {
        send_to_socket("Available commands:\n");
        send_to_socket("$exit - Exit the chat\n");
        send_to_socket("$color <color> - Change your username color\n");
        send_to_socket("$help - Show this help message\n");
        return false;
    }
    if(command == "$afk") {
        send_to_socket("You are now AFK\n");
        server::instance->send_to_all(username + " is now AFK\n", client_socket);
        return false;
    }
    else {
        send_to_socket("Invalid command\n");
        return false;
    }
}

void client_handler::chat_loop() {
    try {
        send_to_socket("\033[96mWelcome to the chat!\033[39m\n");
        username = "";
        int usernamelen = 0;
        while (3 > usernamelen || usernamelen > 20) {
            send_to_socket("Please enter your username (3-20 characters): ");
            username = read_from_socket();
            usernamelen = username.length();
        }
        username = "\033[92m" + username + "\033[39m";
        server::instance->add_client(client_socket, username);
        send_to_socket("Welcome, " + username + "!\n");
        send_to_socket("\033[95m\"$help\" for a list of commands\033[39m\n");
        while (true) {
            send_to_socket(username + ": ");
            std::string message = read_from_socket();
            //If starts with $
            if (message[0] == '$') {
                //Split into command and args, make both lowercase
                std::string command = message.substr(0, message.find(' '));
                for (char &c : command) {
                    c = tolower(c);
                }
                std::string args = message.substr(message.find(' ') + 1, message.length() - message.find(' ') - 1);
                for (char &c : args) {
                    c = tolower(c);
                }
                if(interpret_command(command, args)){
                    break;
                }
            } else {
                std::string formatted_message = username + ": " + message;
                server::instance->send_to_all(formatted_message + "\n", client_socket);
                std::cout << "Message from client " << client_socket << ": '" << message << "'" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

void client_handler::operator()() {
    chat_loop(); // Start the chat loop
    std::cout << "Client " << client_socket << " disconnected" << std::endl;
    server::instance -> remove_client(client_socket); // Remove client after the message has been disseminated
}