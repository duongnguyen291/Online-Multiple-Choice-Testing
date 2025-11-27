# Online Testing System - Server

## Overview

TCP server implementation cho hệ thống thi trắc nghiệm online, sử dụng C++ với SQLite database.

## Features Implemented (Phase 1-4)

### ✅ Phase 1: Foundation
- Protocol handling ([Type][Length][JSON])
- Database wrapper (SQLite)
- Logger system
- Session management

### ✅ Phase 2: Authentication
- User registration
- Login/Logout
- Session token management

### ✅ Phase 3: Practice Mode
- Request practice questions
- Submit practice answers
- Auto-scoring

### ✅ Phase 4: Test Mode Core
- Create test room (TEACHER only)
- List all rooms
- Join room
- View room results

## Requirements

- Linux (tested on Ubuntu)
- g++ (C++17 support)
- SQLite3 development libraries
- OpenSSL development libraries

## Installation

### Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y g++ make sqlite3 libsqlite3-dev libssl-dev
```

### Build

```bash
cd server
make
```

Executable sẽ được tạo tại `bin/server`.

## Usage

### Run Server

```bash
# Option 1: Direct
./bin/server

# Option 2: With options
./bin/server --port 8888 --db testing_app.db

# Option 3: Using helper script
../scripts/run_server.sh
```

### Command Line Options

```
--port, -p <port>    Server port (default: 8888)
--db, -d <path>      Database path (default: testing_app.db)
--help, -h           Show help message
```

### First Run

Khi chạy lần đầu, server sẽ:
1. Tạo database file nếu chưa có
2. Initialize schema từ `database/schema.sql`
3. Insert sample data từ `database/init_data.sql` (nếu database trống)

## Testing

Sau khi server đang chạy, mở terminal khác và chạy tests:

```bash
cd tests

# Quick test
python3 quick_test.py

# Full test suite
python3 test_client.py

# Individual tests
python3 test_individual.py
```

Hoặc sử dụng helper script:
```bash
../scripts/run_tests.sh          # Full test
../scripts/run_tests.sh quick    # Quick test
../scripts/run_tests.sh individual  # Individual tests
```

## Project Structure

```
server/
├── src/
│   ├── main.cpp          # Entry point
│   ├── server.cpp        # Server implementation
│   ├── protocol.cpp      # Protocol handling
│   ├── database.cpp      # Database wrapper
│   ├── session.cpp       # Session management
│   └── logger.cpp        # Logging
├── include/
│   ├── server.h
│   ├── protocol.h
│   ├── database.h
│   ├── session.h
│   └── logger.h
├── Makefile
└── README.md
```

## Logs

Server logs được ghi vào:
- Console: INFO, WARN, ERROR levels
- File: `server.log` (trong thư mục chạy server)

## Database

Database file: `testing_app.db` (SQLite3)

Schema được định nghĩa trong `database/schema.sql`

## Protocol

Server sử dụng custom protocol:
- Format: `[Type (2 bytes)][Length (4 bytes)][JSON Payload]`
- Network byte order (big-endian)
- Message types định nghĩa trong `include/protocol.h`

Chi tiết xem `docs/application_design.md`

## Troubleshooting

### Build Errors

**Error: sqlite3.h not found**
```bash
sudo apt-get install libsqlite3-dev
```

**Error: openssl/sha.h not found**
```bash
sudo apt-get install libssl-dev
```

### Runtime Errors

**Error: Cannot bind to port**
- Port đã được sử dụng bởi process khác
- Cần quyền root để bind port < 1024
- Giải pháp: Dùng port khác (--port 8888)

**Error: Database locked**
- Database đang được sử dụng bởi process khác
- Giải pháp: Đóng các connections khác

**Error: Cannot open database**
- Kiểm tra quyền ghi trong thư mục
- Kiểm tra đường dẫn database

## Development

### Adding New Features

1. Thêm message type constants trong `include/protocol.h`
2. Implement handler trong `server.cpp`
3. Thêm database operations nếu cần trong `database.cpp`
4. Update tests trong `tests/`

### Code Style

- C++17 standard
- Use smart pointers where appropriate
- Error handling với try-catch
- Logging cho debugging

## Next Steps

- Phase 5: Test Mode Execution (Start test, Change answer, Submit, Auto-grade)
- Phase 6: Statistics (full implementation)
- Phase 7: WebSocket Gateway for web client

## License

[Your License Here]

