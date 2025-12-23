# Status Kỹ Thuật Project - Online Testing Multiple Choice

## 📊 Tổng Quan

**Tình trạng:** ✅ **Giai đoạn 1-6 HOÀN THÀNH** (Core Features) | ❌ **Giai đoạn 7 CHƯA BẮT ĐẦU** (Web Frontend)

**Progress:** 
- Backend (Server): **100%**
- Protocol: **100%**
- Database: **100%**
- Testing: **100%**
- WebSocket Gateway: **0%** (Cần thiết cho web frontend)
- Web Frontend: **0%** (HTML/CSS/JavaScript)

---

## 1️⃣ Socket Implementation

### Loại Socket Được Sử Dụng
✅ **Custom Implementation (Tự code)** - Không dùng thư viện ngoài, dựng hoàn toàn từ scratch

### Chi tiết Socket:

#### File: [server/src/server.cpp](server/src/server.cpp)

**1. Socket Type:**
```cpp
server_fd = socket(AF_INET, SOCK_STREAM, 0);  // TCP socket
```
- **AF_INET**: IPv4
- **SOCK_STREAM**: TCP (reliable, connection-oriented)
- **Platform**: Linux only (yêu cầu dự án)

**2. Server Socket Setup:**
- ✅ Bind: `bind()` - gắn socket vào port
- ✅ Listen: `listen()` - chờ kết nối
- ✅ Accept: `accept()` - chấp nhận kết nối từ client
- ✅ SO_REUSEADDR: Cho phép reuse port nhanh hơn

**3. Non-blocking Mode:**
```cpp
int flags = fcntl(server_fd, F_GETFL, 0);
fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);  // Non-blocking I/O
```

**4. I/O Multiplexing Model:**
- ✅ **epoll** (edge-triggered) - Linux advanced I/O multiplexing
- File: [server/src/server.cpp](server/src/server.cpp), line 59-72
- Lợi ích: Xử lý đa kết nối hiệu quả, phù hợp cho server có nhiều clients
- `epoll_create1(0)` - tạo epoll instance
- `epoll_ctl()` - thêm/xóa sockets từ epoll
- `epoll_wait()` - chờ events từ sockets

**5. Connection Management:**
- Mỗi client connection có `ClientInfo` struct:
  ```cpp
  struct ClientInfo {
      int sockfd;
      std::string session_token;
      int user_id;
      std::string username;
      std::string role;
  };
  ```
- Map: `std::map<int, ClientInfo> clients` - quản lý tất cả active connections

**6. Data Transfer:**
- `send_exact()` - gửi đủ N bytes (xử lý partial sends)
- `recv_exact()` - nhận đủ N bytes (xử lý partial receives)
- Handling MSG_NOSIGNAL: tránh SIGPIPE khi gửi tới closed socket

---

## 2️⃣ Protocol & Byte Order

### Protocol Format (Đúng đặc tả)
✅ **[Type (2 bytes)][Length (4 bytes)][JSON Payload]**

**File:** [server/include/protocol.h](server/include/protocol.h) + [server/src/protocol.cpp](server/src/protocol.cpp)

#### Header Structure:
```cpp
struct __attribute__((packed)) ProtocolHeader {
    uint16_t msg_type;        // 2 bytes
    uint32_t payload_length;  // 4 bytes
};  // Total: 6 bytes
```
- **`__attribute__((packed)`**: Tránh compiler padding issues
- Payload: UTF-8 JSON string (variable length)

### Byte Order (Network Byte Order)

✅ **Sử dụng Network Byte Order (Big-Endian)**

**Functions Used:**
```cpp
// Sending (Host to Network)
uint16_t msg_type_net = htons(msg_type);           // host -> network (short)
uint32_t payload_length_net = htonl(payload_length); // host -> network (long)

// Receiving (Network to Host)
uint16_t msg_type = ntohs(msg_type_raw);           // network -> host (short)
uint32_t payload_length = ntohl(payload_length_raw); // network -> host (long)
```

**Location:** [server/src/protocol.cpp](server/src/protocol.cpp)
- Line 52-58: `send_message()` - convert to network byte order
- Line 106-108: `recv_message()` - convert from network byte order

**Why Network Byte Order:**
- ✅ Cross-platform compatibility
- ✅ Standard cho network protocols
- ✅ htons/htonl thực hiện endian conversion tự động (nếu cần)

### Byte Order Details:

| Function | Direction | What It Does | Example |
|----------|-----------|-------------|---------|
| `htons()` | Host → Network | Convert 16-bit to big-endian | 0x1234 → 0x3412 (nếu little-endian) |
| `htonl()` | Host → Network | Convert 32-bit to big-endian | 0x12345678 → 0x78563412 (nếu little-endian) |
| `ntohs()` | Network → Host | Convert 16-bit from big-endian | 0x3412 → 0x1234 (trên little-endian) |
| `ntohl()` | Network → Host | Convert 32-bit from big-endian | 0x78563412 → 0x12345678 (trên little-endian) |

**Implementation Details:**
```cpp
// Send: Host format → Network format
char header_buf[6];
memcpy(header_buf, &msg_type_net, 2);          // Copy network byte order
memcpy(header_buf + 2, &payload_length_net, 4); // Copy network byte order

// Receive: Network format → Host format
memcpy(&msg_type_raw, header_buf, 2);          // Read network bytes
uint16_t msg_type = ntohs(msg_type_raw);        // Convert to host format
```

---

## 3️⃣ Protocol Stream Handling (Loop Implementation)

✅ **Luồng xử lý chính xác theo đặc tả:**

**File:** [server/src/protocol.cpp](server/src/protocol.cpp), `recv_message()` function

```
Lặp lại từ bước 1 (vì bộ đệm có thể còn dữ liệu của gói tin tiếp theo)
↓
1. Nhận (recv) ít nhất 6 byte → msg_type + payload_length
2. Đọc 2 byte đầu → ntohs() → msg_type
3. Đọc 4 byte tiếp → ntohl() → payload_length
4. Nếu payload_length > 0 → recv_exact() cho đủ N bytes
5. Trích xuất JSON từ buffer
6. Parse JSON
7. [QUAY LẠI 1] - xử lý message tiếp theo (nếu có)
```

**Return Values:**
```cpp
enum RecvResult {
    RECV_SUCCESS,      // Nhận thành công 1 complete message
    RECV_ERROR,        // Connection error
    RECV_NO_DATA,      // EAGAIN - không có data (non-blocking)
    RECV_INCOMPLETE    // Partial message (không đủ bytes)
};
```

**Main Loop (trong server.cpp):**
```cpp
void Server::handle_client_message(int client_fd) {
    Protocol::Message msg;
    while (true) {
        Protocol::RecvResult result = Protocol::recv_message(client_fd, msg);
        if (result == Protocol::RECV_SUCCESS) {
            // Process message
            handle_message(msg);
            // Loop continues - xử lý message tiếp theo (nếu có trong buffer)
        } else if (result == Protocol::RECV_NO_DATA) {
            break; // No more data, wait for next epoll event
        } else {
            // Error
            break;
        }
    }
}
```

---

## 4️⃣ Kiến Trúc Tổng Thể

```
┌──────────────────────────────────────────────────────┐
│           TCP Client (test scripts)                  │
│  (Python: test_client.py, test_protocol*.py)        │
└──────────────┬───────────────────────────────────────┘
               │ TCP Socket
               │ [Type][Length][JSON]
               │ Network Byte Order (Big-Endian)
               ▼
┌──────────────────────────────────────────────────────┐
│         Linux TCP Server (C++)                       │
│  - epoll (edge-triggered)                           │
│  - Non-blocking sockets                             │
│  - Multi-client support                             │
│                                                      │
│  Core Modules:                                       │
│  ├─ server.cpp: Socket + epoll management           │
│  ├─ protocol.cpp: [Type][Length][JSON] + hton*/     │
│  │              ntoh* byte order conversion          │
│  ├─ database.cpp: SQLite wrapper                    │
│  ├─ session.cpp: Token management                   │
│  └─ logger.cpp: Event logging                       │
│                                                      │
│  Features Implemented:                               │
│  ✅ Phase 1-6: Auth, Practice, Test Mode (100%)     │
└──────────────┬───────────────────────────────────────┘
               │
               │ (FUTURE) WebSocket
               │ (FUTURE) HTTP Gateway
               ▼
        ┌──────────────────┐
        │  Web Browser     │
        │  (Not yet built) │
        └──────────────────┘
```

---

## 5️⃣ Test Status

### ✅ Các Test Đã Pass:

**Unit Tests (C++)** - `tests/test_protocol_unit.cpp`
- ✅ Protocol packet parsing
- ✅ Network byte order conversion
- ✅ Multiple messages in buffer
- ✅ Error handling

**Integration Tests (Python)** - `tests/test_client.py`
- ✅ 13/13 tests PASSED (100%)
- ✅ Registration
- ✅ Login/Logout
- ✅ Practice Mode
- ✅ Test Mode (list rooms, create, join)
- ✅ Statistics & History

**Protocol Multiple Messages** - `tests/test_protocol_multiple_messages.py`
- ✅ 3/3 tests PASSED
- ✅ Verify loop implementation

### Test Command:
```bash
cd /home/duong/Downloads/github/online-testing-multiplechoice
python3 tests/test_client.py          # Integration tests
python3 tests/test_protocol_multiple_messages.py  # Protocol loop test
```

---

## 6️⃣ Giai Đoạn Còn Lại (TODO)

### Phase 1.5: WebSocket Gateway (0%)
- ❌ WebSocket server (libwebsockets hoặc lwip)
- ❌ Protocol bridge: WebSocket ↔ TCP
- ❌ Connection mapping

### Phase 7: Web Frontend (0%)
- ❌ HTML/CSS/JavaScript UI
- ❌ WebSocket client
- ❌ Charts (Chart.js)
- ❌ Responsive design

---

## 7️⃣ Key Files

| File | Mục Đích |
|------|---------|
| [server/src/server.cpp](server/src/server.cpp) | Socket, epoll, client management |
| [server/src/protocol.cpp](server/src/protocol.cpp) | [Type][Length][JSON], hton*/ntoh* |
| [server/include/server.h](server/include/server.h) | Server class definition |
| [server/include/protocol.h](server/include/protocol.h) | Protocol constants & headers |
| [server/src/database.cpp](server/src/database.cpp) | SQLite wrapper |
| [tests/test_client.py](tests/test_client.py) | Integration tests |

---

## 🎯 Kết Luận

**Backend Server:** ✅ **HOÀN THÀNH (100%)**
- ✅ Custom TCP socket implementation
- ✅ epoll multiplexing
- ✅ Correct byte order (Network Byte Order Big-Endian)
- ✅ Protocol loop implementation
- ✅ All phases 1-6 completed
- ✅ All tests passing

**Cần Làm Tiếp:** WebSocket Gateway + Web Frontend (Phase 1.5 + Phase 7)

