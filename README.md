# Mohammed ALHasani Student ID: MAKFW

# TCP File Transfer Project

## Overview

This project implements a TCP-based file transfer system consisting of:

1. A C++ TCP Server
2. A C++ Command-Line TCP Client
3. An Android GUI TCP Client

The system supports reliable file transfer with timeout handling and bandwith measurement

---

## Components

### 1. TCP Server (server.cpp)

The server:

- Listens on a user-specified port
- Accepts incoming TCP connections
- Recieves file data
- Saves each connection as a separate file
- Implements a 10-second receive timeout
- Writes "ERROR" to the file if a timeout occurs
- Handles SIGTERM and SIGQUIT for graceful shutdown

### Usage

./server <port> <directory>

Example:

./server 5000 file.txt

---

### 2. TCP Client (client.cpp)

The client:

- Connects to a server using hostanme and port
- Sends a specified file
- Uses an 8192-byte Buffer
- Implements a 10-second send timeout
- Validates port range (1024-65535)
- Displays appropriate error messages

### Usage

./client <hostname> <port> <filename>

Example:

./client 10.0.2.2 500 file.txt

---

### 3. Android GUI Client

The Android Application:

- Allows user to input:
    - Server IP address
    - Port number
- Establishes TCP connection
- Sends a test file to the server
- Uses a background thread for networking
- Calculates:
    - Transfer time
    - Bandwith (KB/s)
- Displays success or error message

Special emulator use:
When using Android Emulator to connect to a local server on the same machine, use: 10.0.2.2 after port forwarding local machine network

---

## Implementation Details

- Buffer size: 8192 bytes
- Port validation: 1024-65535
- Timeout: 10 seconds
- Server timeout uses SO_RCVTIMEO
- Client timeout uses SO_SNDTIMEO
- Android client uses socket.connect() timeout

---

## Error Handling

All components handle

- Invalid arguments
- Invalid ports
- Socket failures
- Connection failures
- File error
- Timeout events

---

## Build Instructions

### Client & Server

Utilize the Makefile with "make"

### The Android App

Open project in Android Studio and run on emulator

---