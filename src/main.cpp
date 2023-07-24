#include "../include/server.hpp"

int main() {
    server serv(12345); // Create a server object listening on port 12345

    serv.run(); // Execute server to start accepting client connections

    return 0;
}
