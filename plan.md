# Kế Hoạch Thực Hiện Dự Án: Hệ Thống Thi Trắc Nghiệm Online

## 1. Tổng Quan Dự Án

### 1.1. Mục Tiêu
Xây dựng hệ thống thi trắc nghiệm trực tuyến cho phép:
- **Test Mode**: Tạo phòng thi, quản lý thời gian, chấm điểm tự động
- **Practice Mode**: Luyện tập với câu hỏi tùy chỉnh
- **Quản lý**: Lịch sử, thống kê, phân quyền người dùng

### 1.2. Phạm Vi
- Server đa kết nối (TCP sockets, C/C++)
- Client với giao diện người dùng (GUI hoặc CLI)
- Database để lưu trữ dữ liệu
- Giao thức tầng ứng dụng tự định nghĩa

---

## 2. Phân Tích Yêu Cầu Chi Tiết

### 2.1. Chức Năng Core (Bắt Buộc)

#### 2.1.1. Xác Thực & Quản Lý Phiên
- ✅ Đăng ký tài khoản (USER/TEACHER)
- ✅ Đăng nhập/Đăng xuất
- ✅ Quản lý session token với expiry
- ✅ Phân quyền (USER chỉ làm bài, TEACHER tạo phòng)

#### 2.1.2. Practice Mode
- ✅ Yêu cầu bài luyện tập với filters (topic, difficulty, số câu)
- ✅ Nhận bộ câu hỏi (không có đáp án)
- ✅ Nộp bài và nhận kết quả (số câu đúng/tổng)
- ✅ Lưu lịch sử vào PracticeHistory

#### 2.1.3. Test Mode
- ✅ **Tạo phòng thi** (TEACHER): Tên, số câu, thời gian, filters
- ✅ **Xem danh sách phòng**: Trạng thái (NOT_STARTED/ONGOING/FINISHED)
- ✅ **Tham gia phòng**: Chỉ khi status = NOT_STARTED
- ✅ **Bắt đầu thi**: Chủ phòng start → Server gửi đề cho tất cả participants
- ✅ **Làm bài**: Cập nhật đáp án trong thời gian cho phép
- ✅ **Nộp sớm**: Submit trước khi hết giờ
- ✅ **Tự động chấm**: Server tự động chấm khi hết giờ hoặc tất cả đã nộp
- ✅ **Xem kết quả**: Cá nhân và bảng điểm phòng (sau khi FINISHED)

#### 2.1.4. Lịch Sử & Thống Kê
- ✅ Xem lịch sử bài đã làm (Practice + Test)
- ✅ Thống kê điểm theo thời gian
- ✅ Phân bố điểm theo topic/difficulty
- ✅ Hiển thị đồ thị (client-side)

#### 2.1.5. Phân Loại Câu Hỏi
- ✅ Phân loại theo difficulty: easy, medium, hard
- ✅ Phân loại theo topic: Toán, Lịch sử, Địa lý, v.v.
- ✅ Lọc câu hỏi khi tạo test/practice

#### 2.1.6. Logging
- ✅ Ghi log các hoạt động quan trọng:
  - Authentication (login/logout)
  - Tạo phòng, join phòng
  - Start test, submit
  - Scoring, errors

### 2.2. Yêu Cầu Kỹ Thuật

#### 2.2.1. Backend (Server)
- **Ngôn ngữ**: C/C++
- **Platform**: Linux (bắt buộc)
- **Socket**: TCP sockets
- **I/O Model**: select()/poll()/epoll hoặc thread-pool
- **Database**: SQLite (C API) hoặc file JSON/CSV + in-memory cache
- **JSON Parsing**: Thư viện C/C++ (json-c, nlohmann/json, hoặc tự implement)

#### 2.2.2. Frontend (Client)
**Đề xuất các phương án:**

**Phương án 1: Qt C++**
- ✅ Cross-platform, GUI đẹp
- ✅ Dễ tích hợp với backend C++
- ✅ Hỗ trợ đồ thị (QChart)
- ⚠️ Cần cài đặt Qt framework

**Phương án 2: Web-based (HTML/CSS/JavaScript) - ĐÃ CHỌN**
- ✅ Không cần cài đặt, chạy trên browser
- ✅ Dễ phát triển UI/UX, responsive design
- ✅ Thư viện đồ thị phong phú (Chart.js, D3.js)
- ✅ Cross-platform (mọi OS có browser)
- ✅ Dễ maintain và update
- ⚠️ Cần WebSocket gateway hoặc HTTP wrapper cho TCP protocol
- ⚠️ Cần implement WebSocket server hoặc HTTP server wrapper trong C++

**Phương án 3: ncurses (CLI)**
- ✅ Nhẹ, không cần GUI framework
- ✅ Phù hợp cho demo nhanh
- ⚠️ Khó hiển thị đồ thị
- ⚠️ UX kém hơn GUI

**Quyết định: Web-based (HTML/CSS/JavaScript)** 
- Sử dụng WebSocket để kết nối với backend TCP server
- Backend cần thêm WebSocket gateway hoặc HTTP/WebSocket wrapper

#### 2.2.3. Giao Thức
- **Format**: [Type (2 bytes)][Length (4 bytes)][Payload (JSON, UTF-8)]
- **Network Byte Order**: htons/ntohs, htonl/ntohl
- **Message Types**: Đã định nghĩa trong `application_design.md`
- **Stream Handling**: Length-prefixed để xử lý TCP stream

#### 2.2.4. Database Schema
- 8 bảng đã được thiết kế trong `database_design.md`
- Sử dụng SQLite với C API

---

## 3. Kiến Trúc Hệ Thống

### 3.1. Kiến Trúc Tổng Quan

```
┌─────────────┐    WebSocket/HTTP         ┌─────────────┐         TCP Socket          ┌─────────────┐
│   Browser   │◄─────────────────────────►│  Gateway   │◄──────────────────────────►│   Server    │
│  (Web UI)   │    JSON Messages          │  (C/C++)   │    [Type][Length][JSON]    │  (C/C++)    │
│ HTML/JS/CSS │                            │            │                            │             │
└─────────────┘                            └─────────────┘                            └─────────────┘
                                                                                                │
                                                                                                ▼
                                                                                        ┌─────────────┐
                                                                                        │  SQLite DB  │
                                                                                        └─────────────┘
```

**Lưu ý:** Gateway có thể là:
- **Option 1**: WebSocket server (C++) chuyển đổi WebSocket ↔ TCP protocol
- **Option 2**: HTTP REST API server (C++) với WebSocket cho real-time events
- **Option 3**: Tích hợp WebSocket handler trực tiếp vào TCP server

### 3.2. Cấu Trúc Thư Mục Dự Án

```
online-testing-multiplechoice/
├── server/                    # Server code
│   ├── src/
│   │   ├── main.cpp          # Entry point
│   │   ├── socket_handler.h/cpp
│   │   ├── protocol.h/cpp    # Protocol parsing
│   │   ├── auth.h/cpp        # Authentication
│   │   ├── practice.h/cpp    # Practice mode
│   │   ├── test_room.h/cpp   # Test room management
│   │   ├── database.h/cpp    # Database wrapper
│   │   ├── session.h/cpp     # Session management
│   │   └── logger.h/cpp      # Logging
│   ├── include/
│   ├── Makefile
│   └── README.md
├── client/                    # Web client (HTML/CSS/JavaScript)
│   ├── index.html             # Entry point
│   ├── css/
│   │   ├── main.css
│   │   ├── login.css
│   │   ├── practice.css
│   │   ├── test-room.css
│   │   └── statistics.css
│   ├── js/
│   │   ├── main.js            # Entry point
│   │   ├── websocket-client.js # WebSocket connection handler
│   │   ├── protocol.js        # Protocol message handling
│   │   ├── auth.js            # Authentication logic
│   │   ├── practice.js         # Practice mode logic
│   │   ├── test-room.js       # Test room logic
│   │   ├── history.js         # History & statistics
│   │   └── utils.js           # Utility functions
│   ├── pages/                 # HTML pages (SPA hoặc multi-page)
│   │   ├── login.html
│   │   ├── register.html
│   │   ├── dashboard.html
│   │   ├── practice.html
│   │   ├── test-room.html
│   │   ├── history.html
│   │   └── statistics.html
│   ├── assets/                # Images, icons
│   ├── lib/                   # Third-party libraries
│   │   ├── chart.js           # Chart.js for statistics
│   │   └── ...
│   └── README.md
├── gateway/                   # WebSocket/HTTP Gateway (C++)
│   ├── src/
│   │   ├── main.cpp           # Gateway entry point
│   │   ├── websocket_server.h/cpp # WebSocket server
│   │   ├── tcp_client.h/cpp   # TCP client to backend
│   │   ├── protocol_bridge.h/cpp # Protocol conversion
│   │   └── ...
│   ├── Makefile
│   └── README.md
├── database/
│   ├── schema.sql            # Database schema
│   ├── init_data.sql         # Sample questions
│   └── migrations/           # DB migrations (nếu cần)
├── docs/
│   ├── database_design.md
│   ├── application_design.md
│   ├── sequences_code.md
│   ├── plan.md               # File này
│   ├── protocol.md           # Chi tiết giao thức
│   ├── security.md
│   ├── test-plan.md
│   └── log-policy.md
├── scripts/
│   ├── run.sh                # Script chạy server + gateway
│   ├── setup_db.sh           # Script setup database
│   └── serve_web.sh          # Script serve web client (nếu cần)
├── Makefile                  # Root Makefile
├── README.md                 # Hướng dẫn tổng quan
└── Dockerfile                # (Optional) Docker setup
```

---

## 4. Kế Hoạch Triển Khai

### Phase 1: Foundation & Infrastructure (Tuần 1-2)

#### 1.1. Setup Project Structure
- [ ] Tạo cấu trúc thư mục
- [ ] Setup Makefile cho server và client
- [ ] Setup build system (CMake hoặc Make)
- [ ] Setup version control (Git)

#### 1.2. Database Setup
- [ ] Tạo file schema.sql từ `database_design.md`
- [ ] Implement database wrapper (SQLite C API)
- [ ] Tạo script init_data.sql với sample questions
- [ ] Test database operations (CRUD)

#### 1.3. Protocol Implementation (Core)
- [ ] Implement packet format: [Type][Length][Payload]
- [ ] Implement network byte order conversion (hton*/ntoh*)
- [ ] Implement JSON parsing (chọn thư viện: json-c hoặc nlohmann/json)
- [ ] Implement message serialization/deserialization
- [ ] Unit test cho protocol parsing

#### 1.4. Socket Infrastructure
- [ ] Implement TCP server socket (bind, listen, accept)
- [ ] Implement I/O model (chọn: epoll hoặc thread-pool)
- [ ] Implement recv_exact() để đọc đủ N bytes
- [ ] Implement send_message() với header + payload
- [ ] Handle connection lifecycle (connect, disconnect, cleanup)

#### 1.5. WebSocket Gateway (Mới)
**Mục đích:** Bridge giữa WebSocket (browser) và TCP protocol (backend server)

**Kiến trúc:**
```
Browser (WebSocket) ←→ Gateway (WebSocket Server) ←→ Backend (TCP Server)
     JSON                    Protocol Bridge              [Type][Length][JSON]
```

**Tasks:**
- [ ] Research WebSocket library cho C++:
  - **libwebsockets**: Mature, cross-platform, phù hợp
  - **uWebSockets**: High performance, modern API
  - **Tự implement**: Phức tạp nhưng full control
- [ ] Implement WebSocket server:
  - Listen trên port riêng (ví dụ: 8080 cho WebSocket, 8888 cho TCP)
  - Handle WebSocket handshake (RFC 6455)
  - Handle WebSocket frames (text/binary)
- [ ] Implement protocol bridge:
  - **WebSocket → TCP**: Parse JSON từ WebSocket, convert sang [Type][Length][JSON], gửi qua TCP
  - **TCP → WebSocket**: Nhận [Type][Length][JSON] từ TCP, parse, convert sang JSON, gửi qua WebSocket
  - Map message types: WebSocket JSON có field "type" tương ứng với TCP msg_type
- [ ] Connection management:
  - Mỗi WebSocket connection tạo 1 TCP connection tới backend
  - Map WebSocket session ↔ TCP socket
  - Handle disconnect: cleanup cả 2 connections
- [ ] Error handling:
  - Handle WebSocket errors
  - Handle TCP connection errors
  - Forward errors từ backend tới client
- [ ] Test WebSocket ↔ TCP bridge:
  - Test với WebSocket client tool (wscat, browser console)
  - Test protocol conversion đúng
  - Test concurrent connections

**Alternative: Tích hợp WebSocket vào TCP Server**
- Thay vì tách riêng gateway, có thể implement WebSocket handler trực tiếp trong TCP server
- Server listen trên 2 ports: TCP (8888) và WebSocket (8080)
- Shared protocol logic, chỉ khác transport layer

**Deliverables:**
- Server có thể accept TCP connections
- WebSocket gateway có thể accept WebSocket connections
- Protocol bridge hoạt động đúng (WebSocket JSON ↔ TCP [Type][Length][JSON])
- Client web có thể connect qua WebSocket và gửi/nhận messages
- Static file server để serve web client (hoặc embedded HTTP server)

---

### Phase 2: Authentication & Session Management (Tuần 2-3)

#### 2.1. User Management
- [ ] Implement C2S_REGISTER (101)
- [ ] Implement password hashing (bcrypt hoặc SHA-256)
- [ ] Implement C2S_LOGIN (102)
- [ ] Implement S2C_LOGIN_OK (803) với session_token
- [ ] Implement C2S_LOGOUT (103)
- [ ] Implement session token generation (random, unique)
- [ ] Implement session expiry check

#### 2.2. Session Validation Middleware
- [ ] Middleware để validate session_token cho mọi request
- [ ] Return S2C_RESPONSE_ERROR nếu token invalid/expired
- [ ] Map session_token → user_id

**Deliverables:**
- User có thể đăng ký, đăng nhập, đăng xuất
- Session token được quản lý đúng
- Tất cả requests (trừ register) yêu cầu valid token

---

### Phase 3: Practice Mode (Tuần 3-4)

#### 3.1. Practice Request & Questions
- [ ] Implement C2S_PRACTICE_REQUEST (201)
- [ ] Validate session_token
- [ ] Query Questions từ DB với filters (topic, difficulty)
- [ ] Randomize và select N questions
- [ ] Implement S2C_PRACTICE_QUESTIONS (901) - KHÔNG gửi đáp án

#### 3.2. Practice Submit & Scoring
- [ ] Implement C2S_PRACTICE_SUBMIT (202)
- [ ] Validate answers với correct_option trong DB
- [ ] Calculate correct_count
- [ ] Insert vào PracticeHistory
- [ ] Implement S2C_PRACTICE_RESULT (902)

**Deliverables:**
- User có thể làm bài practice
- Kết quả được lưu vào database
- Scoring hoạt động đúng

---

### Phase 4: Test Mode - Core (Tuần 4-5)

#### 4.1. Room Management
- [ ] Implement C2S_CREATE_ROOM (302) - chỉ TEACHER
- [ ] Validate creator_id là TEACHER
- [ ] Insert vào TestRooms
- [ ] Implement S2C_ROOM_CREATED (1002)
- [ ] Implement C2S_LIST_ROOMS (301)
- [ ] Implement S2C_ROOM_LIST (1001)

#### 4.2. Join Room
- [ ] Implement C2S_JOIN_ROOM (303)
- [ ] Validate room exists và status = NOT_STARTED
- [ ] Insert vào RoomParticipants
- [ ] Implement S2C_JOIN_OK (1003)
- [ ] Implement S2C_USER_JOINED_ROOM (1004) - PUSH to all in room

**Deliverables:**
- TEACHER có thể tạo phòng
- User có thể xem danh sách và join phòng

---

### Phase 5: Test Mode - Execution (Tuần 5-6)

#### 5.1. Start Test
- [ ] Implement C2S_START_TEST (401)
- [ ] Validate user là creator (creator_id)
- [ ] Validate room status = NOT_STARTED
- [ ] Select questions từ DB với filters
- [ ] Insert vào TestRoomQuestions
- [ ] Update TestRooms: status = ONGOING, start_timestamp, end_timestamp
- [ ] Start timer (thread hoặc event-based)
- [ ] Implement S2C_TEST_STARTED (1101) - PUSH to all participants
- [ ] Implement S2C_ROOM_STATUS_CHANGED (1005) - PUSH to lobby

#### 5.2. Answer Management
- [ ] Implement C2S_CHANGE_ANSWER (402)
- [ ] Update/Insert vào UserTestAnswers
- [ ] Update last_updated timestamp
- [ ] (Không cần response để giảm tải)

#### 5.3. Submit Test
- [ ] Implement C2S_SUBMIT_TEST (403)
- [ ] Update RoomParticipants: status = SUBMITTED
- [ ] Return S2C_RESPONSE_OK
- [ ] Check nếu tất cả đã submit → trigger auto-grade

#### 5.4. Auto-Grade & Results
- [ ] Timer expires hoặc tất cả submitted
- [ ] Auto-grade: Query UserTestAnswers, so sánh với correct_option
- [ ] Update UserTestAnswers.is_correct
- [ ] Calculate score cho mỗi participant
- [ ] Update RoomParticipants.score
- [ ] Update TestRooms: status = FINISHED
- [ ] Implement S2C_TEST_ENDED (1102) - PUSH
- [ ] Implement S2C_YOUR_RESULT (1103) - PUSH to each participant
- [ ] Implement S2C_ROOM_STATUS_CHANGED (1005) - FINISHED

**Deliverables:**
- Test có thể start, làm bài, submit, auto-grade
- Timer hoạt động đúng
- Results được gửi đến participants

---

### Phase 6: History & Statistics (Tuần 6-7)

#### 6.1. History
- [ ] Implement C2S_GET_HISTORY (501)
- [ ] Query PracticeHistory và RoomParticipants (join với TestRooms)
- [ ] Format data theo format trong protocol
- [ ] Implement S2C_HISTORY_DATA (1201)

#### 6.2. Statistics
- [ ] Implement C2S_GET_STATS (502)
- [ ] Query và aggregate data:
  - Score over time (PracticeHistory + RoomParticipants)
  - Topic distribution (group by topic)
  - Difficulty distribution
- [ ] Implement S2C_STATS_DATA (1202)

#### 6.3. Room Results
- [ ] Implement C2S_VIEW_ROOM_RESULTS (503)
- [ ] Validate room status = FINISHED
- [ ] Query RoomParticipants + Users (join)
- [ ] Sort by score (descending)
- [ ] Implement S2C_ROOM_RESULTS_DATA (1203)

**Deliverables:**
- User có thể xem lịch sử và thống kê
- Data được format đúng theo protocol

---

### Phase 7: Client Implementation - Web Frontend (Tuần 7-9)

#### 7.1. WebSocket Client & Protocol
- [ ] Implement WebSocket client trong JavaScript
  - Connect to WebSocket server (ws://localhost:8080)
  - Handle WebSocket events (onopen, onmessage, onerror, onclose)
- [ ] Implement protocol message handling:
  - Send: Tạo JSON object với field "type" và data, stringify, send qua WebSocket
  - Receive: Parse JSON từ WebSocket message, extract "type" và data
  - Map message types: C2S_* và S2C_* constants
- [ ] Implement message queue cho async operations
- [ ] Handle reconnection logic:
  - Auto-reconnect on disconnect (exponential backoff)
  - Retry failed messages
- [ ] Handle connection states (connecting, connected, disconnected, error)
- [ ] Session management:
  - Store session_token từ S2C_LOGIN_OK
  - Include session_token trong mọi request (trừ register/login)

#### 7.2. HTML/CSS Structure
- [ ] Setup project structure:
  - HTML files trong `pages/` hoặc SPA với single `index.html`
  - CSS files trong `css/` (modular)
  - JavaScript files trong `js/` (modular, ES6 modules)
- [ ] Implement responsive layout:
  - Mobile-first design
  - Breakpoints: mobile (< 768px), tablet (768-1024px), desktop (> 1024px)
- [ ] Create base CSS framework:
  - CSS variables cho colors, typography, spacing
  - Reset/normalize CSS
  - Base component styles (buttons, inputs, cards)
- [ ] Implement navigation/routing:
  - **Option 1**: Single Page Application (SPA) với JavaScript routing
  - **Option 2**: Multi-page với navigation links
  - **Recommendation**: SPA cho better UX
- [ ] Create reusable UI components:
  - Buttons, forms, modals, cards, tables
  - Loading spinners, error messages, success notifications
  - Timer component (reusable cho practice và test)

#### 7.3. Authentication UI
- [ ] Login page (HTML + CSS + JS)
  - Form: username, password
  - Handle C2S_LOGIN, S2C_LOGIN_OK, S2C_RESPONSE_ERROR
  - Store session_token in localStorage
- [ ] Register page
  - Form: username, password, role (USER/TEACHER)
  - Handle C2S_REGISTER
- [ ] Logout functionality
- [ ] Session management (check token on page load)

#### 7.4. Dashboard/Main Page
- [ ] Main dashboard sau khi login
- [ ] Navigation menu (Practice, Test Rooms, History, Statistics)
- [ ] User info display (username, role)
- [ ] Role-based UI (TEACHER thấy "Create Room", USER không thấy)

#### 7.5. Practice Mode UI
- [ ] Practice setup page
  - Form: num_questions, topic (dropdown), difficulty (dropdown)
  - Handle C2S_PRACTICE_REQUEST
- [ ] Practice exam page
  - Display questions (radio buttons cho options)
  - Timer countdown (JavaScript)
  - Submit button
  - Handle S2C_PRACTICE_QUESTIONS, C2S_PRACTICE_SUBMIT
- [ ] Practice result page
  - Display correct_count/total_questions
  - Handle S2C_PRACTICE_RESULT

#### 7.6. Test Mode UI
- [ ] Room list page
  - Table: room_id, name, status, num_questions, duration_minutes
  - Filter by status
  - Handle C2S_LIST_ROOMS, S2C_ROOM_LIST
  - Handle S2C_ROOM_STATUS_CHANGED (real-time updates)
- [ ] Create room page (TEACHER only)
  - Form: name, num_questions, duration_minutes, topic, difficulty
  - Handle C2S_CREATE_ROOM, S2C_ROOM_CREATED
- [ ] Room waiting page
  - Display room info, participants list
  - Handle C2S_JOIN_ROOM, S2C_JOIN_OK
  - Handle S2C_USER_JOINED_ROOM (real-time updates)
  - Start button (chỉ owner thấy)
- [ ] Test exam page
  - Display questions (radio buttons)
  - Timer countdown (dựa trên end_timestamp từ server)
  - Auto-save answers (gửi C2S_CHANGE_ANSWER)
  - Submit button
  - Handle S2C_TEST_STARTED, C2S_CHANGE_ANSWER, C2S_SUBMIT_TEST
- [ ] Test result page
  - Individual result: correct_count/total_questions, rank
  - Room ranking table (sau khi FINISHED)
  - Handle S2C_TEST_ENDED, S2C_YOUR_RESULT, S2C_ROOM_RESULTS_DATA

#### 7.7. History & Statistics UI
- [ ] History page
  - Table: mode, date, score, room_name (nếu TEST)
  - Handle C2S_GET_HISTORY, S2C_HISTORY_DATA
- [ ] Statistics page
  - Line chart: score over time (Chart.js)
  - Bar chart: topic distribution
  - Pie chart: difficulty distribution
  - Handle C2S_GET_STATS, S2C_STATS_DATA

#### 7.8. Real-time Updates
- [ ] Implement WebSocket event listeners
- [ ] Handle push notifications:
  - S2C_USER_JOINED_ROOM
  - S2C_TEST_STARTED
  - S2C_ROOM_STATUS_CHANGED
  - S2C_TEST_ENDED
  - S2C_YOUR_RESULT
- [ ] Update UI real-time khi nhận push events

**Deliverables:**
- Web client hoàn chỉnh với tất cả pages
- WebSocket connection hoạt động ổn định
- Real-time updates hoạt động
- Responsive design
- Tất cả chức năng được tích hợp

---

### Phase 8: Logging & Error Handling (Tuần 9)

#### 8.1. Logging System
- [ ] Implement logger với levels (DEBUG, INFO, WARN, ERROR)
- [ ] Log file rotation
- [ ] Log format: timestamp, level, message, context
- [ ] Log các events quan trọng:
  - Authentication (login/logout)
  - Room creation, join, start
  - Test submission, scoring
  - Errors

#### 8.2. Error Handling
- [ ] Handle network errors (disconnect, timeout)
- [ ] Handle database errors
- [ ] Handle protocol errors (invalid message, parse error)
- [ ] Return appropriate error codes (1001, 1002, 2001, v.v.)

**Deliverables:**
- Logging system hoạt động
- Error handling đầy đủ

---

### Phase 9: Testing & Documentation (Tuần 10)

#### 9.1. Unit Testing
- [ ] Test protocol parsing
- [ ] Test database operations
- [ ] Test authentication flow
- [ ] Test practice mode flow
- [ ] Test test mode flow

#### 9.2. Integration Testing
- [ ] Test full authentication flow
- [ ] Test practice mode end-to-end
- [ ] Test test mode với multiple clients
- [ ] Test concurrent connections
- [ ] Test timer accuracy

#### 9.3. Documentation
- [ ] Complete README.md (setup, build, run)
- [ ] Complete protocol.md (chi tiết giao thức)
- [ ] Complete security.md
- [ ] Complete test-plan.md
- [ ] Complete log-policy.md
- [ ] Code comments và documentation

**Deliverables:**
- Test suite hoàn chỉnh
- Documentation đầy đủ

---

### Phase 10: Polish & Advanced Features (Tuần 11-12)

#### 10.1. Performance Optimization
- [ ] Database query optimization
- [ ] Connection pooling (nếu dùng thread-pool)
- [ ] Memory leak check
- [ ] Load testing

#### 10.2. Advanced Features (Optional, 1-10 điểm)
- [ ] Question bank management (TEACHER có thể thêm/sửa/xóa questions)
- [ ] Real-time notifications (desktop notifications)
- [ ] Export results to CSV/PDF
- [ ] Multi-language support
- [ ] Dark mode cho UI
- [ ] Question shuffling (randomize order)
- [ ] Answer shuffling (randomize option order)
- [ ] Anti-cheat measures (detect rapid answer changes)

**Deliverables:**
- System được optimize
- Advanced features (nếu có)

---

## 5. Phân Công Công Việc (Gợi Ý)

### Backend Developer
- Phase 1-6: Server implementation
- Phase 8: Logging & Error handling
- Phase 9: Backend testing

### Frontend Developer
- Phase 1.5: WebSocket gateway (có thể làm cùng Backend)
- Phase 7: Web client implementation
- Phase 9: Client testing
- Phase 10: UI polish

### Full-stack/Lead
- Phase 1: Project setup
- Phase 9: Integration testing
- Phase 10: Performance optimization
- Documentation

---

## 6. Timeline Tổng Quan

| Phase | Tuần | Mô Tả | Deliverables |
|-------|------|-------|--------------|
| 1 | 1-2 | Foundation | Protocol, Socket, DB |
| 2 | 2-3 | Authentication | Login, Register, Session |
| 3 | 3-4 | Practice Mode | Practice flow hoàn chỉnh |
| 4 | 4-5 | Test Mode - Core | Create, List, Join room |
| 5 | 5-6 | Test Mode - Execution | Start, Submit, Auto-grade |
| 6 | 6-7 | History & Stats | History, Statistics APIs |
| 7 | 7-9 | Client Implementation | Web UI hoàn chỉnh |
| 8 | 9 | Logging | Logging system |
| 9 | 10 | Testing & Docs | Test suite, Documentation |
| 10 | 11-12 | Polish | Optimization, Advanced features |

**Tổng thời gian dự kiến: 10-12 tuần**

---

## 7. Rủi Ro & Giải Pháp

### 7.1. Rủi Ro Kỹ Thuật
- **JSON parsing phức tạp**: Sử dụng thư viện có sẵn (json-c)
- **TCP stream handling**: Implement recv_exact() cẩn thận
- **Timer accuracy**: Sử dụng monotonic clock, không dùng wall clock
- **Concurrent access DB**: SQLite có thread-safety, cần lock đúng cách

### 7.2. Rủi Ro Thời Gian
- **WebSocket gateway complexity**: Cần thời gian implement protocol bridge
- **Frontend learning curve**: Nếu chưa quen với modern JavaScript/WebSocket
- **Protocol complexity**: Test từng message type riêng lẻ
- **Integration issues**: Test integration sớm, không đợi đến cuối
- **CORS issues**: Cần config CORS đúng nếu serve từ different origins

### 7.3. Giải Pháp
- **Prototype sớm**: Build MVP (Minimum Viable Product) trước
- **Incremental development**: Test từng phase trước khi chuyển phase tiếp theo
- **Version control**: Commit thường xuyên, branch cho features

---

## 8. Checklist Trước Khi Nộp

### 8.1. Code
- [ ] Server compile và chạy được trên Linux
- [ ] WebSocket gateway compile và chạy được
- [ ] Web client chạy được trên browser (Chrome, Firefox, Safari)
- [ ] Makefile hoạt động (make, make run, make clean)
- [ ] Code có comments đầy đủ
- [ ] Không có memory leak (valgrind check)
- [ ] Web client không có console errors

### 8.2. Functionality
- [ ] Tất cả message types đã implement
- [ ] Authentication flow hoạt động
- [ ] Practice mode hoạt động
- [ ] Test mode hoạt động (create, join, start, submit, grade)
- [ ] History & Statistics hoạt động
- [ ] Logging hoạt động

### 8.3. Documentation
- [ ] README.md đầy đủ
- [ ] protocol.md chi tiết
- [ ] security.md
- [ ] test-plan.md
- [ ] log-policy.md
- [ ] UML diagrams (Use Case, Sequence, Activity, ERD)

### 8.4. Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Multiple clients test (concurrent)
- [ ] Error handling test
- [ ] Network failure test

### 8.5. Demo
- [ ] Demo script hoặc video
- [ ] Sample data (questions)
- [ ] User guide (nếu cần)

---

## 9. Tài Nguyên & Tham Khảo

### 9.1. Thư Viện C/C++
- **JSON**: json-c (https://github.com/json-c/json-c) hoặc nlohmann/json
- **SQLite**: https://www.sqlite.org/capi3ref.html
- **Socket**: Standard POSIX sockets
- **WebSocket**: 
  - libwebsockets (https://libwebsockets.org/)
  - uWebSockets (https://github.com/uNetworking/uWebSockets)
  - Hoặc tự implement WebSocket protocol (RFC 6455)

### 9.1.1. Thư Viện Frontend (JavaScript)
- **WebSocket**: Native WebSocket API (built-in browser)
- **Charts**: Chart.js (https://www.chartjs.org/) hoặc D3.js (https://d3js.org/)
- **HTTP Client**: Fetch API (built-in) hoặc Axios
- **UI Framework** (Optional): Bootstrap, Tailwind CSS, hoặc Material Design

### 9.2. Tài Liệu
- TCP/IP Sockets in C: Practical Guide
- SQLite C API Documentation
- WebSocket Protocol: RFC 6455 (https://tools.ietf.org/html/rfc6455)
- WebSocket API: MDN Web Docs (https://developer.mozilla.org/en-US/docs/Web/API/WebSocket)
- Chart.js Documentation: https://www.chartjs.org/docs/
- Network Byte Order: man pages (htonl, ntohl)

### 9.3. Tools
- **Build**: Make, CMake
- **Debug**: GDB, Valgrind
- **Network**: Wireshark, tcpdump
- **Version Control**: Git

---

## 10. Kết Luận

Dự án này yêu cầu:
1. **Backend mạnh**: TCP sockets, protocol design, database
2. **WebSocket Gateway**: Bridge giữa Web và TCP protocol
3. **Frontend đẹp**: Web UI (HTML/CSS/JavaScript) với charts
4. **Integration tốt**: WebSocket ↔ TCP communication
5. **Testing kỹ**: Unit, integration, concurrent testing

**Ưu tiên:**
- Phase 1-6: Core functionality (bắt buộc)
- Phase 1.5: WebSocket Gateway (bắt buộc cho web client)
- Phase 7: Web Client (3 điểm)
- Phase 8-9: Quality assurance
- Phase 10: Advanced features (bonus)

**Lưu ý:** 
- Bắt đầu với MVP (Phase 1-3) để có hệ thống chạy được sớm, sau đó mở rộng dần.
- WebSocket gateway có thể được implement song song với TCP server hoặc tích hợp trực tiếp.
- Web client có thể được serve từ static file server (nginx, Apache) hoặc embedded HTTP server trong gateway.

---

**Ngày tạo:** [Ngày hiện tại]  
**Phiên bản:** 1.0  
**Trạng thái:** Draft

