# sudo bet online
is a multiplayer, client-server casino platform. The architecture allows a central server to manage multiple concurrent game tables, while clients can seamlessly connect, view available lobbies, and join active games in real-time.

# Features
- Centralized Server Architecture: A robust backend that maintains game states, calculates payouts, and broadcasts updates to all connected clients.
- Dynamic Lobbies: Players can query the server to see a list of active tables, current player counts, and game statuses before joining.
- Real-Time Multiplayer: Built with raw sockets for fast, peer-to-peer style communication and state synchronization.
- Graphical Client: A smooth user interface utilizing SFML and ImGui for a quality desktop experience. 

# Architecture Overview
The project is split into two primary components
- The Server: Written in C++, handling all socket connections, threading for multiple tables, and the core game logic. It acts as the single source of truth to prevent client-side manipulation.
- The (Client): A powerful C++ application that establishes a TCP connection to the server, renders the game state using SFML/ImGui, and handles user input to send back across the network.

# Prerequisites

To build and run sudo bet online, you will need the following installed on your system:

  - A modern C++ compiler (GCC or Clang)
  - CMake (v3.10+)
  - SFML
  - Note: ImGui is included in the source code

Clone the repository and compile both the client and server using CMake:
Bash

    git clone https://github.com/yourusername/sudo-bet-online.git
    cd sudo-bet-online
    cmake -B build
    cmake --build build
# Usage
1. Start the server
Navigate into the build folder we just created and run the server executable
    ./server

2. Connect the client
Launch one or more instances of the client
    ./client


# Roadmap
- Switch server architecture to asio c++ network library
- Implement login capabilites
- Add customizable screen layout with presents

# Contributing
Feel free to contribute
