#include "../include/handle.hpp"

#include <signal.h>

volatile sig_atomic_t stop; // Boolean value to know when to stop server

void Inthand(int signum) {
    stop = 1; // Change stop flag to 1 if SIGINT (interrupt signal) is received
}

handle::handle() {
    signal(SIGINT, Inthand); // Attach Inthand function to SIGINT signal
}

bool handle::stop_requested() const {
    return stop; // Return status of stop flag
}
