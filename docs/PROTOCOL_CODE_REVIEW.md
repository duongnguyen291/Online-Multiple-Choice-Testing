# Protocol Code Review - So sánh với application_design.md

## Tóm tắt

Code implementation **HOÀN TOÀN TUÂN THỦ** quy định trong `application_design.md`.

---

## 1. Header Format ✅

### Quy định:
- **6 bytes**: 2 bytes msg_type + 4 bytes payload_length
- **Network Byte Order**: htons/ntohs cho msg_type, htonl/ntohl cho payload_length

### Code Implementation:
**File:** `server/src/protocol.cpp`

```cpp
// Send
char header_buf[6];
uint16_t msg_type_net = htons(msg_type);           // ✅ Network byte order
uint32_t payload_length_net = htonl(payload_length);
memcpy(header_buf, &msg_type_net, 2);
memcpy(header_buf + 2, &payload_length_net, 4);

// Receive
uint16_t msg_type_raw;
uint32_t payload_length_raw;
memcpy(&msg_type_raw, header_buf, 2);             // ✅ Đọc 2 bytes đầu
memcpy(&payload_length_raw, header_buf + 2, 4);   // ✅ Đọc 4 bytes tiếp
msg.type = ntohs(msg_type_raw);                   // ✅ Convert network byte order
uint32_t payload_length = ntohl(payload_length_raw);
```

**✅ ĐÚNG:** Sử dụng raw bytes để tránh struct padding issues.

---

## 2. Payload Format ✅

### Quy định:
- JSON (UTF-8)
- Nếu payload_length = 0, payload rỗng

### Code Implementation:

```cpp
// Send
std::string payload_str = payload.dump();          // ✅ JSON serialization
uint32_t payload_length = payload_str.length();
if (payload_length > 0) {
    send_exact(sockfd, payload_str.c_str(), payload_length);
}

// Receive
if (payload_length > 0) {
    std::vector<char> buffer(payload_length + 1);
    recv_exact(sockfd, buffer.data(), payload_length);
    msg.payload = json::parse(buffer.data());      // ✅ JSON parsing
} else {
    msg.payload = json::object();                  // ✅ Xử lý payload_length = 0
}
```

**✅ ĐÚNG:** Xử lý đúng JSON và payload_length = 0.

---

## 3. Receive Logic ✅

### Quy định (từ application_design.md):
1. Nhận (recv) ít nhất 6 byte vào bộ đệm
2. Đọc 2 byte đầu để lấy msg_type (dùng ntohs())
3. Đọc 4 byte tiếp theo để lấy payload_length (dùng ntohl())
4. Nếu payload_length > 0, tiếp tục nhận cho đến khi đủ payload_length byte
5. Trích xuất chuỗi JSON từ bộ đệm
6. Phân tích (parse) chuỗi JSON và xử lý logic
7. **Lặp lại từ bước 1** (vì bộ đệm có thể còn dữ liệu của gói tin tiếp theo)

### Code Implementation:

**File:** `server/src/protocol.cpp` - `recv_message()`

```cpp
// Bước 1: Nhận 6 bytes header
char header_buf[6];
recv_exact(sockfd, header_buf, 6);                // ✅ Bước 1

// Bước 2-3: Parse header
memcpy(&msg_type_raw, header_buf, 2);             // ✅ Bước 2
memcpy(&payload_length_raw, header_buf + 2, 4);   // ✅ Bước 3
msg.type = ntohs(msg_type_raw);                   // ✅ ntohs()
uint32_t payload_length = ntohl(payload_length_raw); // ✅ ntohl()

// Bước 4: Nhận payload
if (payload_length > 0) {
    recv_exact(sockfd, buffer.data(), payload_length); // ✅ Bước 4
}

// Bước 5-6: Parse JSON
msg.payload = json::parse(buffer.data());          // ✅ Bước 5-6
```

**File:** `server/src/server.cpp` - `handle_client_message()`

```cpp
// Bước 7: Lặp lại từ bước 1
while (messages_processed < MAX_MESSAGES_PER_LOOP) {
    Message msg;
    if (!Protocol::recv_message(client_fd, msg)) {
        break; // No more data
    }
    // Process message
    switch (msg.type) {
        // ... handle message
    }
}
```

**✅ ĐÚNG:** Đã implement đầy đủ 7 bước, bao gồm loop để xử lý nhiều messages.

---

## 4. TCP Stream Handling ✅

### Quy định:
- TCP là byte stream, có thể "dính gói" hoặc "vỡ gói"
- Cần đọc đủ N bytes trước khi parse

### Code Implementation:

**File:** `server/src/protocol.cpp` - `recv_exact()`

```cpp
bool Protocol::recv_exact(int sockfd, char* buffer, size_t length) {
    size_t total_received = 0;
    while (total_received < length) {
        ssize_t received = recv(sockfd, buffer + total_received, 
                               length - total_received, 0);
        if (received <= 0) {
            // Handle EAGAIN, EOF, errors
            return false;
        }
        total_received += received;                // ✅ Đảm bảo đủ N bytes
    }
    return true;
}
```

**✅ ĐÚNG:** Xử lý đúng TCP stream fragmentation, đảm bảo đọc đủ N bytes.

---

## 5. Security & Validation ✅

### Quy định:
- Giới hạn kích thước payload để tránh DoS

### Code Implementation:

```cpp
// Validate payload length (max 2MB to prevent DoS)
if (payload_length > 2 * 1024 * 1024) {
    LOG_ERROR("Payload too large: " + std::to_string(payload_length) + " bytes");
    return false;
}
```

**✅ ĐÚNG:** Có bảo vệ DoS với max payload 2MB.

---

## 6. Error Handling ✅

### Code Implementation:

```cpp
// Non-blocking socket handling
if (received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return false; // No data available, but connection OK
    }
    LOG_ERROR("recv() failed: " + std::string(strerror(errno)));
    return false;
}
if (received == 0) {
    return false; // EOF - connection closed
}
```

**✅ ĐÚNG:** Xử lý đúng các trường hợp:
- EAGAIN/EWOULDBLOCK (no data, connection OK)
- EOF (connection closed)
- Other errors

---

## 7. JSON Parsing ✅

### Code Implementation:

```cpp
try {
    msg.payload = json::parse(buffer.data());
} catch (const json::exception& e) {
    LOG_ERROR("Failed to parse JSON: " + std::string(e.what()));
    return false;
}
```

**✅ ĐÚNG:** Parse JSON với error handling đầy đủ.

---

## Kết luận

### ✅ Code hoàn toàn tuân thủ quy định:

1. ✅ **Header format**: 6 bytes, Network Byte Order
2. ✅ **Payload format**: JSON UTF-8, xử lý payload_length = 0
3. ✅ **Receive logic**: Đầy đủ 7 bước, bao gồm loop
4. ✅ **TCP stream handling**: Xử lý đúng fragmentation
5. ✅ **Security**: Bảo vệ DoS với max payload limit
6. ✅ **Error handling**: Xử lý đầy đủ các trường hợp
7. ✅ **Non-blocking**: Xử lý đúng với non-blocking socket

### 📍 Các điểm chính:

**File:** `server/src/protocol.cpp`
- `send_message()`: ✅ Đúng format, Network Byte Order
- `recv_message()`: ✅ Đúng 7 bước receive logic
- `recv_exact()`: ✅ Xử lý TCP stream fragmentation
- `send_exact()`: ✅ Đảm bảo gửi đủ N bytes

**File:** `server/src/server.cpp`
- `handle_client_message()`: ✅ Loop để xử lý nhiều messages (bước 7)
- Connection management: ✅ Xử lý disconnect đúng cách

### 🎯 Code sẵn sàng cho production!

