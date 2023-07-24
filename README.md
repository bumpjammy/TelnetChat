# TelnetChat

## Description
A simple chat server in C++, using the telnet protocol.

## Usage
To run the server, compile the source code (or use the precompiled binary, if it works for you) and run it.
```./TelnetChat```<br>
This should start the server on port 12345.<br>
You can then connect to the server using telnet:<br>
```telnet localhost 12345```<br>
You can also connect to the server from another computer on the same network, by replacing ```localhost``` with the server's IP address.<br>

The server has a few basic commands:<br>

- ```$help``` - Displays a list of commands.
- ```$color <color>``` - Allows user to pick from a list of colors for their username.
- ```$afk``` - Announces that the user is away from keyboard.
- ```$users``` - Displays a list of users currently connected to the server.
- ```$exit``` - Disconnects the user from the server.