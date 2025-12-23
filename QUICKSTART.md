# Quick Start Guide - Online Testing System

Hướng dẫn nhanh để chạy hệ thống thi trắc nghiệm.

## 1. Yêu Cầu Hệ Thống

- Linux (Ubuntu 18.04+)
- GCC 7+ (build C++ code)
- OpenSSL development files: `sudo apt-get install libssl-dev`
- Python 3 (để serve web client)
- tmux (tùy chọn, để chạy tất cả services): `sudo apt-get install tmux`

## 2. Cài Đặt Dependencies

```bash
sudo apt-get update
sudo apt-get install -y build-essential libssl-dev tmux python3
```

Or use Makefile:
```bash
cd /path/to/online-testing-multiplechoice
make install-deps
```

## 3. Khởi Động Hệ Thống

### Option A: Chạy Tất Cả Một Lệnh (Recommended)

Yêu cầu: tmux đã cài đặt

```bash
cd /path/to/online-testing-multiplechoice
make run
```

Hoặc:
```bash
./run.sh
```

Điều này sẽ:
1. Build server và gateway
2. Start Backend Server (port 8888)
3. Start WebSocket Gateway (port 8080)
4. Start Web Server (port 3000)

### Option B: Chạy Không Có tmux

```bash
cd /path/to/online-testing-multiplechoice
make run-simple
```

### Option C: Chạy Từng Service Riêng

**Terminal 1 - Backend Server:**
```bash
cd server
make run
```

**Terminal 2 - WebSocket Gateway:**
```bash
cd gateway
make run
```

**Terminal 3 - Web Server:**
```bash
cd client
python3 -m http.server 3000
```

## 4. Truy Cập Ứng Dụng

Mở browser và truy cập:

```
http://localhost:3000
```

## 5. Demo Account

### User Account:
- Username: `user_a`
- Password: `hashed_password_abc`

### Teacher Account:
- Username: `teacher_a`
- Password: `hashed_password_abc`

Hoặc tạo tài khoản mới trực tiếp trong app.

## 6. Các Lệnh Hữu Ích

### Build only:
```bash
make clean && make
```

### Run tests:
```bash
make test
```

### Quick connectivity test:
```bash
make quick-test
```

### View help:
```bash
make help
```

### Stop all services (nếu dùng tmux):
```bash
tmux kill-session -t online-testing
```

## 7. Troubleshooting

### Gateway build error: undefined reference to `SHA1`
**Solution:** Install libssl-dev
```bash
sudo apt-get install libssl-dev
```

### Cannot connect to server
1. Kiểm tra backend server chạy: `netstat -tlnp | grep 8888`
2. Kiểm tra gateway chạy: `netstat -tlnp | grep 8080`
3. Check browser console (F12) xem error message

### Port already in use
```bash
# Find process using port
lsof -i :8888
lsof -i :8080
lsof -i :3000

# Kill process
kill -9 <PID>
```

### WebSocket connection timeout
- Đảm bảo firewall không chặn port 8080
- Kiểm tra backend server có chạy không
- Check latency: `ping localhost`

## 8. Cấu Trúc Thư Mục

```
online-testing-multiplechoice/
├── server/           # TCP Server (C++)
│   ├── src/         # Source files
│   ├── include/     # Header files
│   ├── bin/         # Executable
│   └── Makefile
├── gateway/         # WebSocket Gateway (C++)
│   ├── src/
│   ├── include/
│   ├── bin/
│   └── Makefile
├── client/          # Web Client (HTML/JS/CSS)
│   ├── index.html
│   ├── js/
│   ├── css/
│   └── lib/
├── database/        # SQLite files
│   ├── schema.sql
│   └── init_data.sql
├── tests/           # Test files
├── docs/            # Documentation
├── Makefile         # Root Makefile
├── run.sh           # Startup script (tmux)
└── run-simple.sh    # Startup script (no tmux)
```

## 9. Kiến Trúc Hệ Thống

```
User Browser (localhost:3000)
        ↓ HTTP & WebSocket
Web Server + WebSocket Gateway (port 8080)
        ↓ WebSocket ↔ [Type][Length][JSON]
TCP Server (port 8888)
        ↓ SQL queries
SQLite Database (testing_app.db)
```

## 10. Features

- ✅ User registration & login
- ✅ Practice mode with customizable questions
- ✅ Test rooms with timer
- ✅ Real-time updates
- ✅ History and statistics
- ✅ Teacher can create rooms
- ✅ Auto-grading
- ✅ Responsive web UI

## 11. Next Steps

1. **Explore features:**
   - Login as user or teacher
   - Try practice mode
   - Create a test room (teacher)
   - Join and take test

2. **Check logs:**
   - Backend: Terminal running server
   - Gateway: Terminal running gateway
   - Browser console: F12 → Console tab

3. **Edit customizations:**
   - Add more questions: `database/init_data.sql`
   - Change topics/difficulties
   - Modify UI: `client/css/main.css`

## 12. Documentation

- [Backend Server](server/README.md)
- [WebSocket Gateway](gateway/README.md)
- [Web Client](client/README.md)
- [Protocol Specification](docs/application_design.md)
- [Database Schema](database_design.md)

## 13. Support

For issues:
1. Check browser console (F12)
2. Check server/gateway terminal logs
3. Read [TECHNICAL_STATUS.md](TECHNICAL_STATUS.md)
4. Review [docs/](docs/) folder
