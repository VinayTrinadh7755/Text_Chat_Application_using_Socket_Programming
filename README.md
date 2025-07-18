# 🗨️ Text Chat Application using Socket Programming

A high-performance, multi-client chat system in C demonstrating low-level socket programming, non-blocking I/O, and robust command parsing.

## 🔍 Project Overview
This project delivers a UNIX-style shell interface on both server and client sides to support:

- Private messaging (SEND)
- Group broadcast (BROADCAST)
- Fine-grained blocking (BLOCK / UNBLOCK)
- Offline buffering with replay on login
- Dynamic peer discovery (LIST / REFRESH)
- Statistics tracking and custom info commands

It uses TCP sockets and select() for real-time, non-blocking communication.

## 🎯 Why This Matters
Recruiters and teams love engineers who:

- Understand networking fundamentals (TCP/IP, sockets)
- Write robust C code with tight resource and error handling
- Design scalable I/O loops using select() or similar multiplexing
- Build interactive CLI tools with polished UX
- Demonstrate full-stack thinking (protocols, state management, logging)

This project showcases all of the above.

## 🗂 Table of Contents
- Quick Start
- Command Reference
- Architecture & Design
- Key Features
- Project Structure
- Usage Examples
- Tech Stack
- Future Enhancements
- Contributing
- License
- Authors & Contact

---

## 🔧 Quick Start
```bash
# 1. Clone repository
git clone https://github.com/VinayTrinadh7755/Text_Chat_Application_using_Socket_Programming.git
cd text-chat-app

# 2. Build binaries
make clean && make

# 3. Launch server (e.g. port 43251)
./assignment1 s 43251

# 4. Launch client(s) (e.g. port 54320)
./assignment1 c 54320
```
Now type commands in UPPERCASE at the prompt to chat, block, list peers, and more.

## 📜 Command Reference

| Command           | Purpose |
|------------------|---------|
| `LOGIN <IP> <PORT>` | Connect client to server and receive peer list |
| `LOGOUT`           | Temporarily disconnect (retain buffered messages) |
| `EXIT`             | Terminate client and cleanup |
| `SEND <IP> <MSG>`  | One-to-one message (with server relay & logging) |
| `BROADCAST <MSG>`  | One-to-many message to all online peers |
| `BLOCK <IP>`       | Prevent messages from a specific peer |
| `UNBLOCK <IP>`     | Lift a block on a specific peer |
| `BLOCKED <IP>`     | List peers you have blocked |
| `LIST`             | Show currently online clients (sorted by port) |
| `REFRESH`          | Update your local peer list without re-login |
| `STATISTICS`       | Server-side message counts & login states |
| `AUTHOR`           | Print academic integrity acknowledgement |
| `IP`               | Display your external/public IP |
| `PORT`             | Display your listening port |

All commands support detailed error handling for invalid syntax, duplicate states, and offline buffering.

## 🏗 Architecture & Design

- Non-blocking I/O: select() loop monitors both stdin and socket FDs
- Modular handlers: Each command handler (handle_send(), handle_block(), etc.) lives in its own function
- Stateful server: Tracks
  - Active clients (hostname, IP, port)
  - Per-client blocked lists
  - Message buffers for offline delivery
  - Message counts for statistics
- Logging: cse4589_print_and_log() writes to both console and logs/ directory for grading

```
    ┌───────────┐      ┌────────────┐       ┌───────────┐
    │  Client A ├──────►   Server   ├───────► Client B │
    └─────┬─────┘      └────┬───────┘       └─────┬─────┘
          │                │                      │
          │  SEND / BROADCAST / BLOCK / STAT…    │
          │                │                      │
     [select()]         [select()]             [select()]
```

## 🔑 Key Features

- Multi-Client Chat: Simultaneous TCP connections via select()
- Private & Group Messaging: Reliable unicast & broadcast patterns
- Offline Buffering: Queue messages when peers are offline, replay on re-login
- Block/Unblock: Fine-grained control over inbound messages
- Dynamic Peer Discovery: LIST & REFRESH keep clients in sync
- Statistics: Live tracking of messages sent/received and login status
- Resilient Error Handling: Validates IP/port, command syntax, state conflicts

## 📁 Project Structure

```
Text_Chat_Application_using_Socket_Programming/
├── include/               # Public headers (client.h, server.h, common.h)
├── src/
│   ├── assignment1.c      # CLI entry point & argument parsing
│   ├── client.c           # Client main loop & command dispatch
│   ├── server.c           # Server connection manager & command dispatch
│   ├── common.c           # Shared utilities (parsing, IP retrieval)
│   ├── logger.c           # cse4589_print_and_log()
│   └── ...                # Other helper modules
├── logs/                  # Auto-generated logs per client/server run
├── Makefile               # Build targets
├── LICENSE                # GNU GPL v2
└── README.md
```

## 🚀 Usage Examples

```bash
# Client shell
LOGIN 128.205.36.46 43251
LIST
SEND 128.205.36.47 "Hello, how are you?"
BROADCAST "Good morning everyone!"
BLOCK 128.205.36.49
UNBLOCK 128.205.36.49
REFRESH
STATISTICS
LOGOUT
EXIT
```

All commands print [CMD : SUCCESS] or [CMD : ERROR] blocks and properly formatted content.

## 🛠 Tech Stack

- **Language**: C (ISO C11)
- **Networking**: POSIX TCP (socket(), bind(), listen(), accept(), connect())
- **I/O**: select() multiplexing, send(), recv()
- **Build**: GNU Make, GCC
- **Environment**: Linux / macOS with POSIX APIs

## 🌱 Future Enhancements

- Multi-threading: Replace select() with worker threads for higher concurrency
- Authentication & Encryption: Integrate OpenSSL for secure channels
- Web Frontend: Expose REST/WebSocket API with a React or Vue UI
- Persistent Storage: Use lightweight DB (SQLite) for chat history

## 🤝 Contributing

- Fork this repo
- Create your branch (`git checkout -b feature/awesome`)
- Commit your changes (`git commit -m 'Add feature'`)
- Push to origin (`git push origin feature/awesome`)
- Open a Pull Request

Feedback and suggestions are very welcome!

## 📜 License

Distributed under the GNU GPL v2. See LICENSE for more details.

## 👥 Authors & Contact Details

**Vinay Trinadh Naraharisetty**  
UBITName: vinaytri | UBID: 50600415  
📧 vinaytrinadh9910@gmail.com  
[GitHub](https://github.com/VinayTrinadh7755)  
[LinkedIn](www.linkedin.com/in/vinay-trinadh-naraharisetty)

Thank you for checking out our project!!
