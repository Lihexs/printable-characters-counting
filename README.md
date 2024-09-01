# Printable Characters Counting (PCC) Server

## 1. Project Title and Brief Overview

**Project Name:** Printable Characters Counting (PCC) Server

**Main Purpose:** This project implements a client-server architecture for counting printable characters in data streams. The server accepts TCP connections from clients, counts the number of printable characters in the received data, and returns the count to the client. It also maintains overall statistics on the number of printable characters received from all clients.

**Context:** This project was developed as a course assignment for the Operating Systems (0368-2162) class. It aims to provide students with hands-on experience in network programming, socket usage, and handling concurrent connections.

## 2. Features

- TCP-based client-server communication
- Counting of printable characters (ASCII 32-126) in data streams
- Real-time statistics tracking for each printable character across all connections
- Graceful handling of SIGINT signals for server shutdown
- Support for multiple concurrent client connections
- Robust error handling and reporting

**Noteworthy Aspects:**
- The server uses a signal handler to ensure atomic processing of client requests when receiving a SIGINT signal.
- The implementation efficiently handles large files by reading and processing data in chunks.

## 3. Implementation Details

**Structure:**
The project consists of two main components:
1. `pcc_server.c`: Implements the server functionality
2. `pcc_client.c`: Implements the client functionality

**Key Algorithms and Techniques:**
- Use of `select()` or `poll()` for handling multiple client connections (implied by the assignment description, though not explicitly shown in the provided code)
- Efficient character counting using array indexing
- Signal handling for graceful server shutdown
- Network byte order conversions for cross-platform compatibility

**Important Libraries and System Calls:**
- Socket API: `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`, `recv()`
- Signal handling: `sigaction()`
- File I/O: `open()`, `read()`, `close()`
- Network functions: `htonl()`, `ntohl()`, `inet_pton()`

**Challenging Parts:**
1. Ensuring atomic processing of client requests during SIGINT handling
   - Solved by using a flag to indicate when a client is being processed
2. Handling partial sends and receives
   - Implemented loops to ensure all data is sent/received
3. Managing concurrent connections
   - Used a loop in the server to accept and handle multiple clients

## 4. Usage Examples

### Example 1: Starting the Server

```bash
./pcc_server 8080
```

This command starts the PCC server listening on port 8080.

### Example 2: Sending a File to the Server

```bash
./pcc_client 127.0.0.1 8080 /path/to/file.txt
```

This command sends the contents of `file.txt` to the server running on the local machine (127.0.0.1) on port 8080. The client will print the number of printable characters counted by the server.

Output:
```
# of printable characters: 1234
```

### Example 3: Server Statistics

When the server receives a SIGINT signal (e.g., by pressing Ctrl+C), it will print statistics before exiting:

```
char ' ' : 1000 times
char '!' : 50 times
char 'A' : 200 times
...
char '~' : 25 times
```

This output shows the count of each printable character received by the server across all client connections.
