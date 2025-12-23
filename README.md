# Online Testing Multiple Choice System

Một hệ thống thi trắc nghiệm trực tuyến hoàn chỉnh được xây dựng bằng C++ backend, WebSocket gateway, và Web frontend.

## 🎯 Tính Năng Chính

### Authentication & User Management
- ✅ Đăng ký tài khoản (USER/TEACHER)
- ✅ Đăng nhập / Đăng xuất
- ✅ Session token management
- ✅ Role-based access control

### Practice Mode
- ✅ Chọn số câu hỏi, chủ đề, độ khó
- ✅ Làm bài luyện tập với timer
- ✅ Xem kết quả tức thì
- ✅ Lưu lịch sử luyện tập

### Test Mode
- ✅ TEACHER: Tạo phòng thi
- ✅ USER: Xem danh sách và tham gia phòng
- ✅ Real-time participant list
- ✅ Timer-controlled exam
- ✅ Auto-grading
- ✅ Xem kết quả cá nhân & bảng điểm phòng

### Analytics & History
- ✅ Lịch sử tất cả bài làm
- ✅ Thống kê điểm theo thời gian
- ✅ Phân bố điểm theo chủ đề
- ✅ Visualize with Chart.js

## 🏗️ Kiến Trúc Hệ Thống

```
┌─────────────────────────────────┐
│   Web Browser (localhost:3000)   │
│   - HTML/CSS/JavaScript          │
│   - Responsive UI                │
│   - Real-time updates            │
└─────────────┬───────────────────┘
              │ WebSocket
              │ (ws://localhost:8080)
┌─────────────▼───────────────────┐
│  WebSocket Gateway (C++)         │
│  - RFC 6455 Handshake            │
│  - JSON ↔ Protocol Bridge        │
│  - Connection Mapping            │
└─────────────┬───────────────────┘
              │ TCP Socket
              │ [Type][Length][JSON]
              │ Network Byte Order
┌─────────────▼───────────────────┐
│   TCP Server (C++) (port 8888)   │
│   - Epoll multiplexing           │
│   - Multi-client support         │
│   - Protocol parsing             │
│   - Business logic               │
└─────────────┬───────────────────┘
              │ SQL
┌─────────────▼───────────────────┐
│   SQLite Database                │
│   - 8 tables (Users, Questions,  │
│     Tests, History, etc.)        │
└──────────────────────────────────┘
```

## 📁 Cấu Trúc Dự Án

```
online-testing-multiplechoice/
├── server/              # C++ TCP Server
├── gateway/            # C++ WebSocket Gateway
├── client/             # Web Client (HTML/JS/CSS)
├── database/           # SQLite files
├── tests/              # Test files
├── docs/               # Documentation
├── Makefile            # Root Makefile
├── QUICKSTART.md       # Quick start guide
├── TECHNICAL_STATUS.md # Technical details
└── run.sh             # Startup script
```

## 🚀 Quick Start

### 1. Cài Đặt Dependencies
```bash
sudo apt-get install -y build-essential libssl-dev tmux python3
```

### 2. Build & Run
```bash
cd /path/to/online-testing-multiplechoice
make clean && make
make run
```

### 3. Access Application
```
http://localhost:3000
```

### 4. Demo Accounts
- **User:** username=`user_a`, password=`hashed_password_abc`
- **Teacher:** username=`teacher_a`, password=`hashed_password_abc`

## 📋 Yêu Cầu Hệ Thống

- **OS:** Linux (Ubuntu 18.04+)
- **Compiler:** GCC 7+, C++17
- **Libraries:** OpenSSL (libssl-dev)
- **Database:** SQLite 3

## 📚 Dịch Vụ

| Service | Port | Type | Language |
|---------|------|------|----------|
| Web Client | 3000 | HTTP | HTML/JS/CSS |
| WebSocket Gateway | 8080 | WebSocket | C++ |
| TCP Server | 8888 | TCP | C++ |
| Database | - | SQLite | SQL |

## 📖 Documentation

- [QUICKSTART.md](QUICKSTART.md) - Quick start guide
- [TECHNICAL_STATUS.md](TECHNICAL_STATUS.md) - Technical details
- [server/README.md](server/README.md) - Server docs
- [gateway/README.md](gateway/README.md) - Gateway docs
- [client/README.md](client/README.md) - Client docs
- [docs/application_design.md](docs/application_design.md) - Protocol spec
- [docs/database_design.md](docs/database_design.md) - Database schema

## 📝 Phases Completed

- ✅ Phase 1-6: Backend Server (100%)
- ✅ Phase 1.5: WebSocket Gateway (100%)
- ✅ Phase 7: Web Frontend (100%)
- ✅ Testing & Documentation (100%)

## 🎓 Learning Value

This project demonstrates:
- Network programming (TCP sockets, epoll)
- Protocol design (custom binary format)
- WebSocket communication (RFC 6455)
- Database design and SQL
- Multi-tier architecture
- Real-time web applications
- C++ best practices
- JavaScript async patterns

## 🚀 Getting Started

```bash
# 1. Extract/clone repository
cd online-testing-multiplechoice

# 2. Install dependencies
make install-deps

# 3. Build all components
make clean && make

# 4. Run system
make run

# 5. Open http://localhost:3000 in browser
```

## ⚙️ Commands

```bash
make              # Build all
make clean        # Clean build artifacts
make run          # Start all services (requires tmux)
make run-simple   # Start all services (no tmux)
make test         # Run tests
make help         # Show all commands
```

---

**Version:** 1.0 | **Status:** Production Ready | **Updated:** December 2025
